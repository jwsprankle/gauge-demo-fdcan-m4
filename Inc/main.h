/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    FreeRTOS/FreeRTOS_ThreadCreation/Inc/main.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the main.c
  *          file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stm32mp1xx_hal.h>
#include <lock_resource.h>
#include <copro_sync.h>
#include <openamp_log.h>
#include <cmsis_os.h>

void Error_Handler(void);

/* Private defines -----------------------------------------------------------*/
#define DEFAULT_IRQ_PRIO  1U
// TODO: Check into DEFAULT_IRQ_PRIO value

// Mail events, common to all threads
enum obd_mail_id {
    OBD2_NULL, 
    OBD2_PUB_DATA_A7,
    OBD2_OPENAMP_SPEED_XMT,
    OBD2_OPENAMP_RPM_XMT,
    ODB2_OPENAMP_RCV,
    OBD2_FDCAN_START,
    OBD2_FDCAN_STOP,
    OBD2_FDCAN_SPEED_XMT,
    OBD2_FDCAN_RPM_XMT,
    OBD2_FDCAN_SPEED_RCV,
    OBD2_FDCAN_RPM_RCV
};
// Note: May not be good idea to bunch all of these into same struct, look at this again later, separate FDCAN, OPENAMP,  NULL/WAKEUP

struct obd_mail
{
    enum obd_mail_id id;
    uint32_t data;
};

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
