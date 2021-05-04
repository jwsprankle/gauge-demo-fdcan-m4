#include "main.h"
#include "obd2_fdcan.h"
#include "obd2_openamp.h"
#include <cmsis_os.h>
#include <copro_sync.h>
#include <stm32mp1xx_hal_fdcan.h>
#include <stm32mp1xx_hal_def.h>
#include <stdbool.h>

// Public vars
FDCAN_HandleTypeDef hfdcan1;

// Public function
void fdcan_init();
void publish_fdcan_msg(struct fdcan_msg msg);

// Private functions
static void fdcan_rx_init();
static void fdcan_tx_init();
static void fdcan_thd(const void *argument);
static void fdcan_thd_tmr(void const *arg);
static void fdcan_start();
static void fdcan_stop();
static void fdcan_req_speed();
static void fdcan_req_rpm();
static void fdcan_hw_init(void);

// Private vars
static osThreadId fdcan_thd_hndl;
static const uint32_t CAN_request_id = 0x18DB33F1;          // CAN request id
static const uint32_t CAN_reply_id =   0x18DAF111;          // CAN reply ID
static const uint8_t CAN_rpm_request[] = { 0x02, 0x01, 0x0C, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };          // Engine RPM Request
static const uint8_t CAN_speed_request[] = { 0x02, 0x01, 0x0D, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };        // Vehicle Speed Request
static FDCAN_TxHeaderTypeDef TxHeader = { 0 };
static bool openamp_ready = false;

// Mail box setup
static osMailQDef(fdcan_mail_que_id, 10, struct fdcan_msg);       
static osMailQId  fdcan_mail_que_id;

// Timer setup
static osTimerDef(fdcan_tmr, fdcan_thd_tmr);
static osTimerId fdcan_tmr_id;
static const uint32_t fdcan_tmr_rate_ms = 25;  //40hz rate

union fdcan_pack
{
    uint32_t val32;
    uint8_t bytes[2];
};



void fdcan_init()
{
    fdcan_hw_init();        // Init fdcan device
    
    fdcan_tx_init();        // Setup can transmitter
    
    fdcan_rx_init();        // Setup rx callback
        
    // create mail queue
    fdcan_mail_que_id = osMailCreate(osMailQ(fdcan_mail_que_id), NULL);
    if (fdcan_mail_que_id == NULL)
    {
        Error_Handler();
    }
        
    // Create scheduler timer 
    fdcan_tmr_id = osTimerCreate(osTimer(fdcan_tmr), osTimerPeriodic, NULL);
    if (fdcan_tmr_id == NULL)
    {
        Error_Handler();
    }
        
    // Kick off fdcan_thd thread    
    osThreadDef(FDCAN_THREAD, fdcan_thd, osPriorityNormal, 0, 128);
    fdcan_thd_hndl = osThreadCreate(osThread(FDCAN_THREAD), NULL);
    if (fdcan_thd_hndl == NULL)
    {
        Error_Handler();
    }
    
    // Start FDCAN
    if(HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
    {
        Error_Handler();
    }

    // Start scheduler
    osTimerStart(fdcan_tmr_id, fdcan_tmr_rate_ms);
}


// Reading pending message then forward to openamp_thd .
//
// NOTE: Have yet to get filtering to work
//       Have submitted request to STM for good example on how to make it work
//       All current examples and doc's have bugs or missing setup
//
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    FDCAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];
    struct obd_mail * p_new_mail = NULL;
        
    /* Retrieve message from Rx FIFO 0 */
    while (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0))
    {
        if (HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)
        {
            if (openamp_ready && RxHeader.Identifier == CAN_reply_id) // Xmit to openamp only if ready and received correct reply
                {
                    // Check for speed
                    if(RxData[2] == 0x0D)
                    {
                        struct obd_mail tx_msg;
                
                        // Forward to openamp 
                        tx_msg.id = OBD2_FDCAN_SPEED_RCV;
                        tx_msg.data = RxData[3];
                        obd2_pub_data_to_openamp(tx_msg);
                    }
                    else if(RxData[2] == 0x0C) // Check for RPM
                    {
                        struct obd_mail tx_msg;
                
                        // Re-pack data 
                        union fdcan_pack data_unpack = { 0 };
                        data_unpack.bytes[0] = RxData[4];
                        data_unpack.bytes[1] = RxData[3];
                
                        // Forward to openamp 
                        tx_msg.id = OBD2_FDCAN_RPM_RCV;
                        tx_msg.data = data_unpack.val32;
                        obd2_pub_data_to_openamp(tx_msg);
                    }
                    else
                    {
                        __NOP();           // here if invalid id, ignore for now   
                    }
                }                
        }
        else
        {
            // log error
        }
    }      
}


// Note: If you're looking for received message from FDCAN, look at HAL_FDCAN_RxFifo0Callback
//       a good design would forward these messages here, however, very inefficient. 
static void fdcan_thd(const void *argument)
{
    osEvent  evt;
    struct fdcan_msg * pMsg = NULL;
    
    while (1)    
    {        
        evt = osMailGet(fdcan_mail_que_id, 1000);    // check for messages
        if(evt.status == osEventMail)
        {
            pMsg = evt.value.p;                  // Get mail from event
        
            switch(pMsg->id)
            {
            case FDCAN_START:
                fdcan_start();
                break;

            case FDCAN_STOP:
                fdcan_stop();            
                break;
        
            case FDCAN_SPEED_XMT:
                fdcan_req_speed();
                break;
        
            case FDCAN_RPM_XMT:
                fdcan_req_rpm();
                break;
            }
            osMailFree(fdcan_mail_que_id, pMsg); 
        }
    }
}



static void fdcan_thd_tmr(void const *arg)
{
    static bool tx_speed = true; 
    struct fdcan_msg tmr_msg = { 0 };
    
    // alternate between speed and rpm tx    
    if(tx_speed)
    {
        tmr_msg.id = FDCAN_SPEED_XMT;
    }
    else
    {
        tmr_msg.id = FDCAN_RPM_XMT;
    }

    tx_speed = !tx_speed;  // Alt speed/rpm tx
    
    publish_fdcan_msg(tmr_msg);
    
}



static void fdcan_start()
{
    openamp_ready = true;
    
}
    

static void fdcan_stop()
{
    openamp_ready = false;
}



static void fdcan_req_speed()
{
    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, (uint8_t *)CAN_speed_request) != HAL_OK)
    {
        Error_Handler();
    }
}

static void fdcan_req_rpm()
{
    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, (uint8_t *)CAN_rpm_request) != HAL_OK)
    {
        Error_Handler();
    }
}


void publish_fdcan_msg(struct fdcan_msg msg)
{
    struct fdcan_msg * p_new_mail = osMailAlloc(fdcan_mail_que_id, 1); 
    if (p_new_mail == NULL)                                             
    {                                                                   
        return;  // TODO: Log error
    }
    
    *p_new_mail = msg;
    
    osMailPut(fdcan_mail_que_id, p_new_mail);    
}



/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void fdcan_hw_init(void)
{
    // Init FDCAN clock
    
    hfdcan1.Instance = FDCAN1;
    hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;     //FDCAN_MODE_EXTERNAL_LOOPBACK; 
    hfdcan1.Init.AutoRetransmission = ENABLE;
    hfdcan1.Init.TransmitPause = DISABLE;
    hfdcan1.Init.ProtocolException = DISABLE;
    hfdcan1.Init.NominalPrescaler = 1;
    hfdcan1.Init.NominalSyncJumpWidth = 1;    //6  //1
    hfdcan1.Init.NominalTimeSeg1 = 45;       //115 //45
    hfdcan1.Init.NominalTimeSeg2 = 2;        //32  //2
    hfdcan1.Init.DataPrescaler = 1;
    hfdcan1.Init.DataSyncJumpWidth = 1;
    hfdcan1.Init.DataTimeSeg1 = 1;
    hfdcan1.Init.DataTimeSeg2 = 1;
    hfdcan1.Init.MessageRAMOffset = 0;
    hfdcan1.Init.StdFiltersNbr = 0;
    hfdcan1.Init.ExtFiltersNbr = 0;
    hfdcan1.Init.RxFifo0ElmtsNbr = 8;
    hfdcan1.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
    hfdcan1.Init.RxFifo1ElmtsNbr = 2;
    hfdcan1.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
    hfdcan1.Init.RxBuffersNbr = 8;
    hfdcan1.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
    hfdcan1.Init.TxEventsNbr = 2;
    hfdcan1.Init.TxBuffersNbr = 8;
    hfdcan1.Init.TxFifoQueueElmtsNbr = 8;
    hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
    hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
    
    
    if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
    {
        // De init and try again
        HAL_FDCAN_DeInit(&hfdcan1);
        
        if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
        {
            Error_Handler();
        }
    }
}


static void fdcan_rx_init(void)
{
    FDCAN_FilterTypeDef canFltr = { 0 };

    hfdcan1.Init.ExtFiltersNbr = 1;    
    
    canFltr.IdType = FDCAN_EXTENDED_ID;
    canFltr.FilterIndex = 0;
    canFltr.FilterType = FDCAN_FILTER_MASK;
    canFltr.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    canFltr.FilterID1 = CAN_reply_id; // CAN_reply_id = 0x18DAF111
    canFltr.FilterID2 = 0x1FFFFFFF;
    
    if (HAL_FDCAN_ConfigFilter(&hfdcan1, &canFltr) != HAL_OK)
    {
        Error_Handler();
    }


    if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_FLAG_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
    {
        Error_Handler();
    }
    
    if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_REJECT_REMOTE, FDCAN_FILTER_REMOTE))
    {
        Error_Handler();
    }
    
    /* FDCAN1 interrupt Init */
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
}


void fdcan_tx_init(uint8_t * requestMsg)
{
    TxHeader.Identifier = CAN_request_id;
    TxHeader.IdType = FDCAN_EXTENDED_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = FDCAN_DLC_BYTES_8;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    TxHeader.TxEventFifoControl = FDCAN_STORE_TX_EVENTS;
    TxHeader.MessageMarker = 0xCC;
}





