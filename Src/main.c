/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "obd2_fdcan.h"
#include "obd2_openamp.h"
#include <stdbool.h>

// Local def's
static void SystemClock_Config(void);                                                    
static void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    bool holdUp = true;
//    while (holdUp) ;    
    HAL_Init();

    
    /* Configure the system clock */
    if (IS_ENGINEERING_BOOT_MODE())
    {
        /* Configure the system clock */
        SystemClock_Config();
        PeriphCommonClock_Config();
    }
   
    
    MX_GPIO_Init();
    
    // Init and kick off FDCAN thread
    fdcan_init();
    
    // Init and kick off openamp thread
    obd2_openamp_init();
    
    /* Start scheduler */
    osKernelStart();
    
    while (1)
    {
    }
}



void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    /** Configure LSE Drive Capability
    */
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMHIGH);
    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI
                                | RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS_DIG;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 16;
    RCC_OscInitStruct.HSIDivValue = RCC_HSI_DIV1;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    RCC_OscInitStruct.PLL2.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL2.PLLSource = RCC_PLL12SOURCE_HSE;
    RCC_OscInitStruct.PLL2.PLLM = 3;
    RCC_OscInitStruct.PLL2.PLLN = 66;
    RCC_OscInitStruct.PLL2.PLLP = 2;
    RCC_OscInitStruct.PLL2.PLLQ = 1;
    RCC_OscInitStruct.PLL2.PLLR = 1;
    RCC_OscInitStruct.PLL2.PLLFRACV = 0x1400;
    RCC_OscInitStruct.PLL2.PLLMODE = RCC_PLL_FRACTIONAL;
    RCC_OscInitStruct.PLL3.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL3.PLLSource = RCC_PLL3SOURCE_HSE;
    RCC_OscInitStruct.PLL3.PLLM = 2;
    RCC_OscInitStruct.PLL3.PLLN = 34;
    RCC_OscInitStruct.PLL3.PLLP = 2;
    RCC_OscInitStruct.PLL3.PLLQ = 17;
    RCC_OscInitStruct.PLL3.PLLR = 37;
    RCC_OscInitStruct.PLL3.PLLRGE = RCC_PLL3IFRANGE_1;
    RCC_OscInitStruct.PLL3.PLLFRACV = 6660;
    RCC_OscInitStruct.PLL3.PLLMODE = RCC_PLL_FRACTIONAL;
    RCC_OscInitStruct.PLL4.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL4.PLLSource = RCC_PLL4SOURCE_HSE;
    RCC_OscInitStruct.PLL4.PLLM = 4;
    RCC_OscInitStruct.PLL4.PLLN = 99;
    RCC_OscInitStruct.PLL4.PLLP = 6;
    RCC_OscInitStruct.PLL4.PLLQ = 8;
    RCC_OscInitStruct.PLL4.PLLR = 8;
    RCC_OscInitStruct.PLL4.PLLRGE = RCC_PLL4IFRANGE_0;
    RCC_OscInitStruct.PLL4.PLLFRACV = 0;
    RCC_OscInitStruct.PLL4.PLLMODE = RCC_PLL_INTEGER;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** RCC Clock Config
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_ACLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
                                | RCC_CLOCKTYPE_PCLK3 | RCC_CLOCKTYPE_PCLK4
                                | RCC_CLOCKTYPE_PCLK5;
    RCC_ClkInitStruct.AXISSInit.AXI_Clock = RCC_AXISSOURCE_PLL2;
    RCC_ClkInitStruct.AXISSInit.AXI_Div = RCC_AXI_DIV1;
    RCC_ClkInitStruct.MCUInit.MCU_Clock = RCC_MCUSSOURCE_PLL3;
    RCC_ClkInitStruct.MCUInit.MCU_Div = RCC_MCU_DIV1;
    RCC_ClkInitStruct.APB4_Div = RCC_APB4_DIV2;
    RCC_ClkInitStruct.APB5_Div = RCC_APB5_DIV4;
    RCC_ClkInitStruct.APB1_Div = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2_Div = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB3_Div = RCC_APB3_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Set the HSE division factor for RTC clock
    */
    __HAL_RCC_RTC_HSEDIV(24);
}

static void PeriphCommonClock_Config(void)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

    /** Initializes the common periph clock
    */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_CKPER;
    PeriphClkInit.CkperClockSelection = RCC_CKPERCLKSOURCE_HSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}


static void MX_GPIO_Init(void)
{

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

}





/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    /* USER CODE BEGIN Callback 0 */

    /* USER CODE END Callback 0 */
    if (htim->Instance == TIM2)
    {
        HAL_IncTick();
    }
    /* USER CODE BEGIN Callback 1 */

    /* USER CODE END Callback 1 */
}




/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    return;
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    while (1)
    {
        __NOP();
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
     /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
