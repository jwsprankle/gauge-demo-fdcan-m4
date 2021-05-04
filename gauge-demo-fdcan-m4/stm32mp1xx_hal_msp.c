/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    FreeRTOS/FreeRTOS_ThreadCreation/Src/stm32mp1xx_hal_msp.c
  * @author  MCD Application Team
  * @brief   This file provides code for the MSP Initialization 
  *                      and de-Initialization codes.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */
 
/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

    // hardware semaphore Clock enable
   __HAL_RCC_HSEM_CLK_ENABLE();
    
  __HAL_RCC_SYSCFG_CLK_ENABLE();

  /* System interrupt init*/
  /* SVC_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SVCall_IRQn, DEFAULT_IRQ_PRIO, 0);
  /* PendSV_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(PendSV_IRQn, (DEFAULT_IRQ_PRIO + 3U), 0);
  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, (DEFAULT_IRQ_PRIO + 3U), 0);

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/**
  * DeInitializes the Global MSP.
  */
void HAL_MspDeInit()
{
  /* Disable IRQ */
  HAL_NVIC_DisableIRQ(TIM2_IRQn);

  /* Disable SYSCFG clock */
  __HAL_RCC_SYSCFG_CLK_DISABLE();

  /* Disable TIM2 clock */
  __HAL_RCC_TIM2_CLK_DISABLE();
}

void HAL_IPCC_MspInit(IPCC_HandleTypeDef* hipcc)
{

  if(hipcc->Instance==IPCC)
  {
  /* USER CODE BEGIN IPCC_MspInit 0 */

  /* USER CODE END IPCC_MspInit 0 */
  /* Peripheral clock enable */
  __HAL_RCC_IPCC_CLK_ENABLE();
  HAL_NVIC_SetPriority(IPCC_RX1_IRQn, DEFAULT_IRQ_PRIO, 1);
  HAL_NVIC_EnableIRQ(IPCC_RX1_IRQn);
  /* USER CODE BEGIN IPCC_MspInit 1 */

  /* USER CODE END IPCC_MspInit 1 */
  }

}

void HAL_IPCC_MspDeInit(IPCC_HandleTypeDef* hipcc)
{

    if (hipcc->Instance == IPCC)
    {
        /* USER CODE BEGIN IPCC_MspDeInit 0 */

        /* USER CODE END IPCC_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_IPCC_CLK_DISABLE();
        HAL_NVIC_DisableIRQ(IPCC_RX1_IRQn);
        /* USER CODE BEGIN IPCC_MspDeInit 1 */

        /* USER CODE END IPCC_MspDeInit 1 */
    }
}

/**
* @brief FDCAN MSP Initialization
* This function configures the hardware resources used in this example
* @param hfdcan: FDCAN handle pointer
* @retval None
*/
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* hfdcan)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_HSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }

    
    GPIO_InitTypeDef GPIO_InitStruct;
    if (hfdcan->Instance == FDCAN1)
    {
        __HAL_RCC_FDCAN_CLK_ENABLE();

        /**FDCAN1 GPIO Configuration
        PA12    ------> FDCAN1_TX
        PA11     ------> FDCAN1_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
//        PERIPH_LOCK(GPIOA);
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//        PERIPH_UNLOCK(GPIOA);

        GPIO_InitStruct.Pin = GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
//        PERIPH_LOCK(GPIOA);
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//        PERIPH_UNLOCK(GPIOA);

        /* USER CODE BEGIN FDCAN1_MspInit 1 */

        /* USER CODE END FDCAN1_MspInit 1 */
    }

}

/**
* @brief FDCAN MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hfdcan: FDCAN handle pointer
* @retval None
*/
void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* hfdcan)
{
    if (hfdcan->Instance == FDCAN1)
    {
        /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

        /* USER CODE END FDCAN1_MspDeInit 0 */
          /* Peripheral clock disable */
        __HAL_RCC_FDCAN_CLK_DISABLE();

        /**FDCAN1 GPIO Configuration
        PH13    ------> FDCAN1_TX
        PI9     ------> FDCAN1_RX
        */
//        PERIPH_LOCK(GPIOD);
        HAL_GPIO_DeInit(GPIOH, GPIO_PIN_1);
//        PERIPH_UNLOCK(GPIOD);

//        PERIPH_LOCK(GPIOA);
        HAL_GPIO_DeInit(GPIOI, GPIO_PIN_11);
//        PERIPH_UNLOCK(GPIOA);
 
        /* USER CODE BEGIN FDCAN1_MspDeInit 1 */

        /* USER CODE END FDCAN1_MspDeInit 1 */
    }

}


/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
