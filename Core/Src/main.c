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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define ACCEL_SCALE 16384.0  // Correct for MPU6050 +/- 2g range
#define THRESHOLD   1.2      // 2.5g (standard impact threshold)
#define TILT_ANGLE  40 // Original 60
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c3;

UART_HandleTypeDef huart1;

osThreadId defaultTaskHandle;
osTimerId myTimer01Handle;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C3_Init(void);
static void MX_USART1_UART_Init(void);
void StartDefaultTask(void const * argument);
void Callback01(void const * argument);

/* USER CODE BEGIN PFP */
/*================ TASK PROTOTYPES ==================*/
void Task1(void *arg);
void Task2(void *arg);
void Task3(void *arg);
void Timer_Task1(TimerHandle_t xTimer);

/*================ MPU PROTOTYPES ==================*/
void MPU6050_Init(void);
void MPU6050_Read(void);
#define MPU6050_ADDR (0x68 << 1)
uint8_t data[14];
int16_t g_Ax, g_Ay, g_Az,Gx,Gy,Gz;
char uart_buf[100];

/* =============== PRINTF ============================*/
//To use printf()
int _write(int file, char *ptr, int len) {
	HAL_UART_Transmit(&huart1, (uint8_t*) ptr, (uint16_t) len, HAL_MAX_DELAY);
	return len;
}

/* =============== HANDLES ============================*/
TimerHandle_t TimerHandle1;
TaskHandle_t Task1_Handle, Task2_Handle, Task3_Handle;
QueueHandle_t alertQueue;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void MPU6050_Init(void)
{
  uint8_t check;
  uint8_t data;

  // Check WHO_AM_I
  HAL_I2C_Mem_Read(&hi2c3, MPU6050_ADDR, 0x75, 1, &check, 1, HAL_MAX_DELAY);

  // Wake up MPU6050
  data = 0;
  HAL_I2C_Mem_Write(&hi2c3, MPU6050_ADDR, 0x6B, 1, &data, 1, HAL_MAX_DELAY);

  // Set accel ±2g
  data = 0x00;
  HAL_I2C_Mem_Write(&hi2c3, MPU6050_ADDR, 0x1C, 1, &data, 1, HAL_MAX_DELAY);

  // Set gyro ±250 dps
  data = 0x00;
  HAL_I2C_Mem_Write(&hi2c3, MPU6050_ADDR, 0x1B, 1, &data, 1, HAL_MAX_DELAY);
}
void MPU6050_Read(void)
{
  HAL_I2C_Mem_Read(&hi2c3, MPU6050_ADDR, 0x3B, 1, data, 14, HAL_MAX_DELAY);

  g_Ax = (int16_t)(data[0] << 8 | data[1]);
  g_Ay = (int16_t)(data[2] << 8 | data[3]);
  g_Az = (int16_t)(data[4] << 8 | data[5]);

  Gx = (int16_t)(data[8] << 8 | data[9]);
  Gy = (int16_t)(data[10] << 8 | data[11]);
  Gz = (int16_t)(data[12] << 8 | data[13]);
}
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
  MX_I2C3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  MPU6050_Init();

  /*================= SOFTWARE TMR CREATIONS ====================*/
  TimerHandle1 = xTimerCreate("Timerpc", pdMS_TO_TICKS(20), pdTRUE, 0,
  			Timer_Task1);
  /*================= TASK CREATIONS ====================*/
  alertQueue = xQueueCreate(5, sizeof(uint8_t));
  xTaskCreate(Task1, "Task1_pc", 128, NULL, 1, &Task1_Handle);
  xTaskCreate(Task2, "Task2_pc", 128, NULL, 2, &Task2_Handle);
  xTaskCreate(Task3, "Task3_pc", 128, NULL, 2, &Task3_Handle);

	//Start the software timer
	xTimerStart(TimerHandle1, 0);

	//Start the scheduler
	vTaskStartScheduler();

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* definition and creation of myTimer01 */
  osTimerDef(myTimer01, Callback01);
  myTimer01Handle = osTimerCreate(osTimer(myTimer01), osTimerPeriodic, NULL);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.ClockSpeed = 100000;
  hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pins : PG13 PG14 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void Timer_Task1(TimerHandle_t xTimer){
	MPU6050_Read();
}
void Task1(void *arg) {
	for (;;) {
		sprintf(uart_buf, "Ax=%d Ay=%d Az=%d | Gx=%d Gy=%d Gz=%d\r\n", g_Ax, g_Ay, g_Az, Gx, Gy, Gz);

		printf("%s\r\n",uart_buf);


		vTaskDelay(pdMS_TO_TICKS(5000));

	}
}
void Task2(void *arg) {
	for (;;) {
        // 1. Calculate Resultant Acceleration Magnitude
		float aTotal = sqrtf((float)g_Ax * g_Ax + (float)g_Ay * g_Ay + (float)g_Az * g_Az) / ACCEL_SCALE;

		// 2. Only check for Tilt IF an Impact (Threshold) is detected first
		if (aTotal > THRESHOLD) {

			// Add a very tiny delay to let the sensor stabilize after impact
			vTaskDelay(pdMS_TO_TICKS(10));

			float pitch = atan2f(g_Ax, sqrtf(g_Ay * g_Ay + g_Az * g_Az)) * 180.0f / M_PI;
			float roll  = atan2f(g_Ay, sqrtf(g_Ax * g_Ax + g_Az * g_Az)) * 180.0f / M_PI;

			// 3. Confirm the device is actually tilted AFTER the impact
			if (fabs(pitch) > TILT_ANGLE || fabs(roll) > TILT_ANGLE) {
		//		uint8_t alert = 1;
				HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_14);

				xTaskNotifyGive(Task3_Handle); // Notify UART task

			//	xQueueSend(alertQueue, &alert, 0);
			}
		}
		vTaskDelay(pdMS_TO_TICKS(50));

	}
}
void Task3(void *arg) {
	for (;;) {
		//uint8_t alert=0;

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		//printf("NOTIFIED\r\n");
	    //xQueueReceive(alertQueue, &alert,portMAX_DELAY);
	    //printf("received\r\n");

	 //   if(alert==1){
	    	printf("FALL DETECTED\r\n");
	    	HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13);
	   // }

	}
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/* Callback01 function */
void Callback01(void const * argument)
{
  /* USER CODE BEGIN Callback01 */

  /* USER CODE END Callback01 */
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
