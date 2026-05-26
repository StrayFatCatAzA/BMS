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
#include "cmsis_os.h"
#include "can.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
const uint8_t cell_id_index_map[9] = {
    1, 2,
    5, 6, 7,
    10, 11, 12,
    15};
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_CAN_Init();
  /* USER CODE BEGIN 2 */
  drv_led_init();

  drv_led_set_state(DRV_LED_ON);

  if (bsp_uart1_init() != UART_STATE_OK)
  {
    bsp_uart1_printf("uart init failed\r\n");
  }

  bsp_uart1_printf("uart init done\r\n");

  if (bsp_iic_soft_init() != IIC_OK)
  {
    bsp_uart1_printf("iic init failed\r\n");
  }

  bsp_uart1_printf("iic init done\r\n");

  if (bq76940_init() != BQ76940_STATE_OK)
  {
    bsp_uart1_printf("bq76940 init failed\r\n");
  }

  bsp_uart1_printf("bq76940 init done\r\n");

  // bq76940_static_test();
  const uint16_t cell_number = 9;
  uint16_t battery_voltage = 0;
  uint16_t cells_voltage[cell_number];
  uint16_t gain = 0;
  int8_t offset = 0;
  int16_t ex_temp = 0;

  bq76940_set_voltage_collection(BQ76940_ENABLE);
  bq76940_set_temperature_collection(BQ76940_ENABLE);
  bq76940_set_current_collection(BQ76940_ENABLE);

  bq76940_get_battery_voltage(&battery_voltage, cell_number);

  bq76940_get_cell_voltage(0, &cells_voltage[0]);
  bq76940_get_cell_voltage(1, &cells_voltage[1]);
  bq76940_get_cell_voltage(4, &cells_voltage[2]);
  bq76940_get_cell_voltage(5, &cells_voltage[3]);
  bq76940_get_cell_voltage(6, &cells_voltage[4]);
  bq76940_get_cell_voltage(9, &cells_voltage[5]);
  bq76940_get_cell_voltage(10, &cells_voltage[6]);
  bq76940_get_cell_voltage(11, &cells_voltage[7]);
  bq76940_get_cell_voltage(14, &cells_voltage[8]);

  bq76940_get_calibration(&gain, &offset);
  printf("gain %d offset: %d \r\n", gain, offset);

  printf("Battery voltage: %d.%03d V\r\n", battery_voltage / 1000, battery_voltage % 1000);

  printf("cell %d voltage: %d.%03d V\r\n", 1, cells_voltage[0] / 1000, cells_voltage[0] % 1000);
  printf("cell %d voltage: %d.%03d V\r\n", 2, cells_voltage[1] / 1000, cells_voltage[1] % 1000);
  printf("cell %d voltage: %d.%03d V\r\n", 3, cells_voltage[2] / 1000, cells_voltage[2] % 1000);
  printf("cell %d voltage: %d.%03d V\r\n", 4, cells_voltage[3] / 1000, cells_voltage[3] % 1000);
  printf("cell %d voltage: %d.%03d V\r\n", 5, cells_voltage[4] / 1000, cells_voltage[4] % 1000);
  printf("cell %d voltage: %d.%03d V\r\n", 6, cells_voltage[5] / 1000, cells_voltage[5] % 1000);
  printf("cell %d voltage: %d.%03d V\r\n", 7, cells_voltage[6] / 1000, cells_voltage[6] % 1000);
  printf("cell %d voltage: %d.%03d V\r\n", 8, cells_voltage[7] / 1000, cells_voltage[7] % 1000);
  printf("cell %d voltage: %d.%03d V\r\n", 9, cells_voltage[8] / 1000, cells_voltage[8] % 1000);

  bq76940_get_external_temperature(&ex_temp);

  printf("external temmperature: %.1f \r\n", ex_temp * 0.1f);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize(); /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM4 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4)
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
