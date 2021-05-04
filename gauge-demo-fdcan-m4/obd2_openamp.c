#include "obd2_openamp.h"
#include "main.h"
#include "obd2_fdcan.h"
#include <openamp.h>
#include <stm32mp1xx_hal.h>
#include <lock_resource.h>
#include <copro_sync.h>
#include <cmsis_os.h>
#include <stm32mp15xx_disco.h>
#include <stdatomic.h>
    
#define MAX_BUFFER_SIZE 512

// Special Public variable
IPCC_HandleTypeDef hipcc;

// Private vars
static VIRT_UART_HandleTypeDef huart0;
static osMailQDef(obd2_openamp_thd_que_id, 10, struct obd_mail);          // openamp thread message queue
static osMailQId  obd2_openamp_thd_que_id;                                // ID for above
static osThreadId odb2_openamp_thd_hndl;
static unsigned _Atomic openamp_rdy = ATOMIC_VAR_INIT(0);
static unsigned _Atomic lock_RxMsg = ATOMIC_VAR_INIT(0);
__IO FlagStatus VirtUart0RxMsg = RESET;
uint8_t VirtUart0ChannelBuffRx[MAX_BUFFER_SIZE];
uint16_t VirtUart0ChannelRxSize = 0;
uint32_t cur_speed_kph = 0;
uint32_t cur_rpm = 0;

// Private functions
static void VIRT_UART0_RxCpltCallback(VIRT_UART_HandleTypeDef *huart);
static void MX_IPCC_Init(void);
static void odb2_openamp_thd(const void *argument);
static void odb2_pub_data_req_tmr(void const *arg);
static void write_data_to_A7(struct obd_mail tx_msg);
static void wait_for_wakeup();

static osTimerDef(odb2_pub_data_tmr, odb2_pub_data_req_tmr);
static osTimerId odb2_pub_data_tmr_id;
static int32_t odb2_pub_A7_tmr_rate = 50;          //20hz
    
// External functions
void Error_Handler(void);

void obd2_openamp_init()
{
    // OpenAmp not compatible with ENGINEERING MODE, do not init in this mode
    if (IS_ENGINEERING_BOOT_MODE() == 0)
    {
        // Init interprocessor communication
        hipcc.Instance = IPCC;
        if (HAL_IPCC_Init(&hipcc) != HAL_OK)
        {
            Error_Handler();
        }

        // Init OpenAmp
        MX_OPENAMP_Init(RPMSG_REMOTE, NULL);

        // Init virtual uart
        if (VIRT_UART_Init(&huart0) != VIRT_UART_OK)
        {
            Error_Handler();
        }

        // Need to register callback for message reception by channels
        if (VIRT_UART_RegisterCallback(&huart0, VIRT_UART_RXCPLT_CB_ID, VIRT_UART0_RxCpltCallback) != VIRT_UART_OK)
        {
            Error_Handler();
        }
        
        // create mail queue
        obd2_openamp_thd_que_id = osMailCreate(osMailQ(obd2_openamp_thd_que_id), NULL);             // create mail queue
        if(obd2_openamp_thd_que_id == NULL)
        {
            Error_Handler();
        }

        // Create timer to publish data to the A7 processor
        odb2_pub_data_tmr_id = osTimerCreate(osTimer(odb2_pub_data_tmr), osTimerPeriodic, NULL);
        if (odb2_pub_data_tmr_id == NULL)
        {
            Error_Handler();
        }
        
        // Kick off odb2_openamp_thd thread    
        osThreadDef(ODB2_MAIN_THREAD, odb2_openamp_thd, osPriorityNormal, 0, 128);
        odb2_openamp_thd_hndl = osThreadCreate(osThread(ODB2_MAIN_THREAD), NULL);
        if (odb2_openamp_thd_hndl == NULL)
        {
            Error_Handler();
        }
        
    }    
}

// VIRT_UART0_RxCpltCallback is not thread safe, use lock_RxMsg to synchronize with odb2_openamp_thd
// TODO: Convert VirtUart0ChannelBuffRx to non_block_queue
static void VIRT_UART0_RxCpltCallback(VIRT_UART_HandleTypeDef *huart)
{
    if (lock_RxMsg == 0)  // This is hack, better to miss data than corrupt. Change to non_block_queue 
    {
        VirtUart0ChannelRxSize = huart->RxXferSize < MAX_BUFFER_SIZE ? huart->RxXferSize : MAX_BUFFER_SIZE - 1;
        memcpy(VirtUart0ChannelBuffRx, huart->pRxBuffPtr, VirtUart0ChannelRxSize);
        lock_RxMsg = 1;
    }
}


static void odb2_openamp_thd(const void *argument)
{
    osEvent  evt;
    struct obd_mail * p_new_mail = NULL;
    int32_t xmt_ready = 0;

    
    // Wait for wakeup
    // For now we will just process first wakeup message, and not listen for further message.
    wait_for_wakeup();

    // Start timer to forward speed and rpm data to A7 processor
    osTimerStart(odb2_pub_data_tmr_id, odb2_pub_A7_tmr_rate);
    
    // Now startup fdcan interface
    struct fdcan_msg _fdcan_msg = { FDCAN_START, 0 };
    publish_fdcan_msg(_fdcan_msg);
    
    while (1)
    {
        evt = osMailGet(obd2_openamp_thd_que_id, 100);   // check mail in openamp_que
        if(evt.status == osEventMail)
        {
            p_new_mail = evt.value.p;                // Get mail from event
            
            switch(p_new_mail->id)
            {
            case OBD2_FDCAN_SPEED_RCV:              // Handle new speed data from FDCAN
                cur_speed_kph = p_new_mail->data;
                break;
                
            case OBD2_FDCAN_RPM_RCV:                // Handle new rpm data from FDCAN
                cur_rpm = p_new_mail->data;
                break;
                
            case OBD2_PUB_DATA_A7:                  // Handle request to publish data to A7 side of OpenAmp
                write_data_to_A7(*p_new_mail);
                break;                
            }
            
            osMailFree(obd2_openamp_thd_que_id, p_new_mail); 
        }
    }
}



// TODO: More issues with STM code.
//      a. ept->dest_addr is not initialized until first message received from A7, thus, can't transmit until first message from A7. 
//      b. Demo S/W contains ept->dest_addr as private to OPENAMP_XXX and we can't initialize it ourself.
//      c. There are blocking waits for ept->dest_addr to become ready, however, not something you want to do in a thread.
//      d. To work around this we'll wait until first message received before using VIRT_UART_Transmit.
//      e. Defiantly need to come back to this.

static void wait_for_wakeup()
{
    while (lock_RxMsg == 0)
    {
        OPENAMP_check_for_message();    
        osDelay(100);
    }
        
    openamp_rdy = 1;
    
    // Note: For now, going to leave with lock_RxMsg = 1, this will prevent further messages from A7. 
    //       Will come back to this once we need rx from A7
}



void obd2_pub_data_to_openamp(struct obd_mail tx_msg)
{
    struct obd_mail * p_new_mail = osMailAlloc(obd2_openamp_thd_que_id, 1); 
    if (p_new_mail == NULL)                                             
    {                                                                   
        return;
    }
    
    *p_new_mail = tx_msg;
    
    osMailPut(obd2_openamp_thd_que_id, p_new_mail);    
}


void write_data_to_A7(struct obd_mail tx_msg)
{
    // TODO: More bad stuff from STM's example  VIRT_UART_Transmit(VIRT_UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
    //       pData must be persistent, question is, when is it free'd?  Will it be free before next obd2_pub_data_to_openamp ??
    //       This will be a good place to look if messages get corrupted.
    
    static char msg_buff[50];
    
    // No-workie in ENGINEERING MODE
    if(IS_ENGINEERING_BOOT_MODE() == 0)
    {
        // Format speed and rpm into simple strings separated by ","
        // TODO: Convert this into MQTT or better yet, bypass VIRT_UART and send as binary data package
        sprintf(msg_buff, "{speed_kph=%03d,rpm=%05d}", cur_speed_kph, cur_rpm);
        VIRT_UART_Transmit(&huart0, (uint8_t *)msg_buff, strlen(msg_buff));
    }
}

// Timer to publish speed and rpm data to A7 processor
static void odb2_pub_data_req_tmr(void const *arg)
{
    struct obd_mail * p_new_mail = osMailAlloc(obd2_openamp_thd_que_id, 1); 
    if (p_new_mail == NULL)                                             
    {                                                                   
        return;
    }
    
    p_new_mail->id = OBD2_PUB_DATA_A7;
    p_new_mail->data = 0;
    
    osMailPut(obd2_openamp_thd_que_id, p_new_mail);    
}

