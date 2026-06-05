/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "quadspi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <string.h>
#include "wcp.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define WCP_REG_RX_COUNT_ADDR    4U
#define WCP_POLL_INTERVAL_MS     1U
#define WCP_RX_WAIT_TIMEOUT_MS   200U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

static void WCP_DumpAllRegisters(void);
static wcp_err_t WCP_ReadRegisterU32(uint32_t reg_addr, uint32_t *value);
static wcp_err_t WCP_WaitRxCountIncrement(uint32_t previous_count, uint32_t *new_count);
static void WCP_RunTestProgram(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_QUADSPI_Init();
  /* USER CODE BEGIN 2 */

  wcp_init();

  /* USER CODE END 2 */

  /* Initialize leds */
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_YELLOW);
  BSP_LED_Init(LED_RED);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    WCP_RunTestProgram();
    HAL_Delay(1000);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

static void WCP_DumpAllRegisters(void)
{
  uint8_t reg_data[4];
  uint32_t reg_value;

  printf("WCP REGISTER DUMP START\r\n");

  for (uint32_t reg_addr = 0; reg_addr < 20U; reg_addr += 4U)
  {
    memset(reg_data, 0, sizeof(reg_data));

    wcp_err_t reg_status = wcp_read_reg(reg_addr, reg_data, (uint16_t)sizeof(reg_data));
    if (reg_status == WCP_ERR_NONE)
    {
      reg_value = ((uint32_t)reg_data[0]) |
                  ((uint32_t)reg_data[1] << 8) |
                  ((uint32_t)reg_data[2] << 16) |
                  ((uint32_t)reg_data[3] << 24);

      printf("REG[0x%02lX] = 0x%08lX (%lu)\r\n",
             (unsigned long)reg_addr,
             (unsigned long)reg_value,
             (unsigned long)reg_value);
    }
    else
    {
      printf("REG[0x%02lX] READ FAILED (%d)\r\n", (unsigned long)reg_addr, (int)reg_status);
    }
  }

  printf("WCP REGISTER DUMP END\r\n");
}

static wcp_err_t WCP_ReadRegisterU32(uint32_t reg_addr, uint32_t *value)
{
  uint8_t reg_data[4];
  wcp_err_t status;

  if (value == NULL)
  {
    return WCP_ERR_INVALID_ARG;
  }

  status = wcp_read_reg(reg_addr, reg_data, (uint16_t)sizeof(reg_data));
  if (status != WCP_ERR_NONE)
  {
    return status;
  }

  *value = ((uint32_t)reg_data[0]) |
           ((uint32_t)reg_data[1] << 8) |
           ((uint32_t)reg_data[2] << 16) |
           ((uint32_t)reg_data[3] << 24);

  return WCP_ERR_NONE;
}

static wcp_err_t WCP_WaitRxCountIncrement(uint32_t previous_count, uint32_t *new_count)
{
  uint32_t current_count = previous_count;

  if (new_count == NULL)
  {
    return WCP_ERR_INVALID_ARG;
  }

  for (uint32_t elapsed = 0U; elapsed < WCP_RX_WAIT_TIMEOUT_MS; elapsed += WCP_POLL_INTERVAL_MS)
  {
    wcp_err_t status = WCP_ReadRegisterU32(WCP_REG_RX_COUNT_ADDR, &current_count);
    if (status != WCP_ERR_NONE)
    {
      return status;
    }

    if (current_count != previous_count)
    {
      *new_count = current_count;
      return WCP_ERR_NONE;
    }

    HAL_Delay(WCP_POLL_INTERVAL_MS);
  }

  *new_count = current_count;
  return WCP_ERR_TIMEOUT;
}

static void WCP_RunTestProgram(void)
{
  uint8_t tx_buffer[64];
  uint8_t expected_rx_buffer[64];
  uint8_t rx_buffer[64] = {0};
  uint16_t rx_len = (uint16_t)sizeof(rx_buffer);
  uint32_t rx_count_before = 0U;
  uint32_t rx_count_after = 0U;

  wcp_err_t reg_status = WCP_ReadRegisterU32(WCP_REG_RX_COUNT_ADDR, &rx_count_before);
  if (reg_status != WCP_ERR_NONE)
  {
    BSP_LED_On(LED_RED);
    printf("WCP TEST RX_COUNT PRE-READ FAILED (%d)\r\n", (int)reg_status);
    return;
  }

  tx_buffer[0] = 'T';
  tx_buffer[1] = 'E';
  tx_buffer[2] = 'S';
  tx_buffer[3] = 'T';
  for (uint32_t i = 4U; i < sizeof(tx_buffer); ++i)
  {
    tx_buffer[i] = (uint8_t)i;
  }

  expected_rx_buffer[0] = 'O';
  expected_rx_buffer[1] = 'K';
  expected_rx_buffer[2] = '6';
  expected_rx_buffer[3] = '4';
  for (uint32_t i = 4U; i < sizeof(expected_rx_buffer); ++i)
  {
    expected_rx_buffer[i] = (uint8_t)(0xA0U + (uint8_t)i);
  }

  wcp_err_t tx_status = wcp_write_data(tx_buffer, (uint16_t)sizeof(tx_buffer));
  WCP_DumpAllRegisters();
  if (tx_status != WCP_ERR_NONE)
  {
    BSP_LED_On(LED_RED);
    printf("WCP TEST TX FAILED (%d)\r\n", (int)tx_status);
    return;
  }

  wcp_err_t wait_status = WCP_WaitRxCountIncrement(rx_count_before, &rx_count_after);
  if (wait_status != WCP_ERR_NONE)
  {
    BSP_LED_On(LED_RED);
    printf("WCP TEST WAIT RX_COUNT TIMEOUT/FAIL (%d), before=%lu after=%lu\r\n",
           (int)wait_status,
           (unsigned long)rx_count_before,
           (unsigned long)rx_count_after);
    return;
  }

  wcp_err_t rx_status = wcp_read_data(rx_buffer, rx_len);
  WCP_DumpAllRegisters();
  if (rx_status != WCP_ERR_NONE)
  {
    BSP_LED_On(LED_RED);
    printf("WCP TEST RX FAILED (%d)\r\n", (int)rx_status);
    return;
  }

  if (memcmp(rx_buffer, expected_rx_buffer, sizeof(expected_rx_buffer)) == 0)
  {
    BSP_LED_Off(LED_RED);
    BSP_LED_Toggle(LED_GREEN);
    printf("WCP TEST PASS (fixed 64-byte response)\r\n");
  }
  else
  {
    uint32_t mismatch_index = 0U;
    while (mismatch_index < sizeof(expected_rx_buffer) &&
           rx_buffer[mismatch_index] == expected_rx_buffer[mismatch_index])
    {
      ++mismatch_index;
    }

    BSP_LED_On(LED_RED);
    if (mismatch_index < sizeof(expected_rx_buffer))
    {
      printf("WCP TEST INVALID 64-byte RESPONSE idx=%lu rx=0x%02X exp=0x%02X\r\n",
             (unsigned long)mismatch_index,
             rx_buffer[mismatch_index],
             expected_rx_buffer[mismatch_index]);
    }
    else
    {
      printf("WCP TEST INVALID 64-byte RESPONSE (unknown diff)\r\n");
    }
  }
}

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
