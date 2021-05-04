#include "main.h"
#include "obd2_fdcan.h"
#include "obd2_openamp.h"
#include <cmsis_os.h>
#include <openamp.h>
#include <signal.h>

// Public defs
void obd2_main_init();

// Fwd def's
static void fdcan_tx(uint8_t * requestMsg);
static void odb2_main_thd(const void *argument);
static void odb2_scheduler(void const *arg);

// Thread
static osThreadId odb2_main_thd_hndl;

// Thread timer
static osTimerDef(odb2_scheduler_tmr, odb2_scheduler);
static osTimerId odb2_scheduler_tmr_id;
static osStatus odb2_xmt_tmr_status;
static int32_t odb_tmr_rate_ms = 10;    // 100hz

static osMailQDef(scheduler_que_id, 4, struct obd_mail);       // Declare a scheduler message queue
osMailQId  scheduler_que_id;                                   // Declare an ID for the scheduler message queue


// Current data
static uint32_t current_speed_kph = 0;
static uint32_t current_rpm = 0;

// External defs
void Error_Handler(void);
void openAmpInit();



void obd2_main_init()
{
    // Init openamp to transmit message to host linux S/W
    obd2_openamp_init();
    
    // Init fdcan hardware and software
    obd2_fdcan_init();

    // Create scheduler timer, will start it in odb2_main_thd
    odb2_scheduler_tmr_id = osTimerCreate(osTimer(odb2_scheduler_tmr), osTimerPeriodic, NULL);
    if (odb2_scheduler_tmr_id == NULL)
    {
        Error_Handler();
    }

    // create mail queue
    scheduler_que_id = osMailCreate(osMailQ(scheduler_que_id), NULL);         // create mail queue
    if (scheduler_que_id == NULL)
    {
        Error_Handler();
    }
     
    // Kick off main thread    
    osThreadDef(ODB2_MAIN_THREAD, odb2_main_thd, osPriorityNormal, 0, 128);
    odb2_main_thd_hndl = osThreadCreate(osThread(ODB2_MAIN_THREAD), NULL);
    if (odb2_main_thd_hndl == NULL)
    {
        Error_Handler();
    }
}

// All messaging will be directed to this thread, then redistributed to other threads as needed.
// This provides a common interface throughout the system, any special exceptions can be handled here.
// Also provides clear illustration of the organization of this module.
// Since the purpose of this module is to communicate with the CAN bus, the FDCAN peripheral is 
//  directly interfaced in this thread, thus, reducing an addition thread just to communicate with the FDCAN hardware and interrupts.
// Note: Some messages are scheduled at regular intervals, some are asynchronously received.
//
static void odb2_main_thd(const void *argument)
{
    osEvent  evt;
    struct obd_mail * p_new_mail = NULL;

    // Start CAN hardware
    obd2_fdcan_start();
    
    // Start xmt timer
    odb2_xmt_tmr_status = osTimerStart(odb2_scheduler_tmr_id, odb_tmr_rate_ms);
    
    while (1)
    {
        evt = osMailGet(scheduler_que_id, osWaitForever); // wait for mail
        if(evt.status == osEventMail)
        {
            p_new_mail = evt.value.p;            // Grab event id
            
            switch(p_new_mail->id)
            {
            case OBD2_FDCAN_SPEED_XMT:      // Transmit request for speed to FDCAN (Scheduled)
                obd2_fdcan_req_speed();
                break;
                
            case OBD2_FDCAN_RPM_XMT:        // Transmit request for RPM to FDCAN (Scheduled)
                obd2_fdcan_req_rpm();
                break;
                
            case OBD2_OPENAMP_SPEED_XMT:    // Forward speed to openamp interface (Scheduled)
                break;                
                
            case OBD2_OPENAMP_RPM_XMT:      // Forward rpm to openamp interface (Scheduled)
                break;
                
            case ODB2_OPENAMP_RCV:          // Process receive messages (Scheduled)
                break;

            case OBD2_FDCAN_SPEED_RCV:      // Process received messages from FDCAN (Asynchronous)
                {
                    struct obd_mail openamp_msg;
                    openamp_msg.id = OBD2_OPENAMP_SPEED_XMT;                    
                    obd2_openamp_tx(openamp_msg);
                }
                break;

            case OBD2_FDCAN_RPM_RCV:        // Process received messages from FDCAN (Asynchronous)
                    __NOP();
                break;
                
                
            default:
                // log error
                break;
            }
            
            osMailFree(scheduler_que_id, p_new_mail); 
        }
    }        
}    


static void odb2_scheduler(void const *arg)
{
    static uint32_t scd_cnt = 0;                    // schedule counter
    enum obd_mail_id new_mail_id = OBD2_NULL;
    struct obd_mail * p_new_mail = NULL;

    // Scheduler runs at 100hz rate
    // Each item is schedules at 20hz
    
    switch (scd_cnt)
    {
    case 0: 
        new_mail_id = OBD2_FDCAN_SPEED_XMT;           // Xmit FDCAN Speed request
        break;
    
    case 1:
        new_mail_id = OBD2_OPENAMP_SPEED_XMT;        // Forward Speed to OpanAmp
        break;
        
    case 2: 
        new_mail_id = OBD2_FDCAN_RPM_XMT;            // Xmit FDCAN RPM request
        break;
        
    case 3: 
        new_mail_id = OBD2_OPENAMP_RPM_XMT;         // Forward RPM to OpanAmp
        break;

    case 4:
        break;                                      // No action
    }

    // Send mail with new event
    if(new_mail_id != OBD2_NULL)
    {
        p_new_mail = osMailAlloc(scheduler_que_id, osWaitForever);  
        if (p_new_mail == NULL)
        {
            Error_Handler();
        }
        // Create new mail, Note: For now, no data will be generated for scheduled mail, just clear
        p_new_mail->id = new_mail_id;
        p_new_mail->data = 0;
        osMailPut(scheduler_que_id, p_new_mail);    
    }
    
    // Bump counter
    scd_cnt++;
    if (scd_cnt >= 5)
    {
        scd_cnt = 0;
    }
}


void odb2_main_speed_rx()
{
    static uint32_t current_speed_kph = 0;
    static uint32_t current_rpm = 0;
    
}


void odb2_main_rpm_rx()
{
    static uint32_t current_speed_kph = 0;
    static uint32_t current_rpm = 0;
}



