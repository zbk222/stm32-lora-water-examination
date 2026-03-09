/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include "sensor.h"
#include "lora.h"
#include <stdio.h>
TaskHandle_t InitTaskHandle;
SemaphoreHandle_t xMutex;
SemaphoreHandle_t xBinarySemaphore1;
SemaphoreHandle_t xBinarySemaphore2;
SemaphoreHandle_t xUart1TxDoneSem;
SemaphoreHandle_t xLoraRxSem; /* LoRa 接收信号量 */
uint8_t init_flag=0;
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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */


/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void init_task(void *argument);
void lora_send_task(void *argument);
void lora_rx_task(void *argument);
void sensor_task(void *argument);
void oled_display_task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  /* 
   * 【关键修复】优先创建同步对象 (Mutex/Semaphore)
   * 必须在任务创建前初始化，否则如果任务运行过快可能访问到空指针导致死机
   */
  xMutex = xSemaphoreCreateMutex();
  
  xBinarySemaphore1 = xSemaphoreCreateBinary();
  xBinarySemaphore2 = xSemaphoreCreateBinary();
  xUart1TxDoneSem   = xSemaphoreCreateBinary();
  xLoraRxSem        = xSemaphoreCreateBinary();
  
  /* 必须检查 Mutex 是否创建成功 */
  if (xMutex == NULL || xLoraRxSem == NULL) {
      // 致命错误：内存不足，无法继续
      // 可以在这里点亮一个红色LED死循环
      // HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
      // while(1);
  }

  /* 置为初始无信号态 */
  if (xUart1TxDoneSem) { xSemaphoreTake(xUart1TxDoneSem, 0); }
  
  BaseType_t r;
  /* InitTask */
  r = xTaskCreate( init_task, "TaskName", 256, NULL, 2, &InitTaskHandle );

  BaseType_t rLora, rSensor, rOled, rLoraRx;
  /* 
   * 堆空间优化 (Heap = 13KB)：
   * 确保有足够空间创建 Mutex 和 Semaphores
   */
  rLora   = xTaskCreate( lora_send_task,   "lora_send_data_task", 256, NULL, 2, NULL );
  /* 增加 LoraRx 栈到 512，防止 memcpy 或 HAL 操作溢出 */
  rLoraRx = xTaskCreate( lora_rx_task,     "lora_rx_task",        512, NULL, 3, NULL ); 
  rSensor = xTaskCreate( sensor_task,      "sensor_task",         512, NULL, 2, NULL );
  rOled   = xTaskCreate( oled_display_task,"oled_display_task",    256, NULL, 2, NULL );
  
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  // 移至最上方创建
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  // 移至最上方创建
  /* USER CODE END RTOS_SEMAPHORES */
sensorThresholds.pH_threshold = 8.0f; // 默认值
  sensorThresholds.TU_threshold = 100.0f; // 默认值
  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */


  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_init_task */
/**
  * @brief  Function implementing the init_task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_init_task */
void init_task(void *argument)
{
  /* USER CODE BEGIN init_task */
  /* Init peripherals once */
  OLED_Init();
  OLED_ShowString(16,0,"temp:",OLED_8X16);
  OLED_ShowString(16,16,"ph:",OLED_8X16);
  OLED_ShowString(16,32,"tds:",OLED_8X16);
  OLED_ShowString(16,48,"TUR:",OLED_8X16);
  OLED_Update();
  sensor_init();

  /* Infinite loop */
  for(;;)
  {
    init_flag=lora_init();

    /* 删除自身任务：使用 NULL 更安全，避免句柄不一致 */
    if(init_flag==1)
    {
      vTaskDelete(NULL);
    }
  
    /* Retry delay if init failed - avoid tight loops */
    vTaskDelay(pdMS_TO_TICKS(1000));

  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void lora_send_task(void *argument)
{
  /* USER CODE BEGIN lora_send_task */
  /* Infinite loop */
  for(;;)
  {
    /* 每轮等待一次信号量，避免永久停在内部 while */
    if (xSemaphoreTake(xBinarySemaphore1, portMAX_DELAY) == pdTRUE)
    {
      if(init_flag==1)
      {
        lora_send_data();
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));


  }
}

void lora_rx_task(void *argument)
{
  /* 在任务开始前启动 DMA 接收，确保信号量已创建且 Scheduler 已运行 */
  extern UART_HandleTypeDef huart1;
  extern uint8_t uart1_rx_buf[256];
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart1_rx_buf, 256);

  /* 初始化 sensorThresholds，避免使用未初始化值 */
  

  for(;;)
  {
    if (xSemaphoreTake(xLoraRxSem, portMAX_DELAY) == pdTRUE)
    {
      lora_recv();
      
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void sensor_task(void *argument)
{
  /* USER CODE BEGIN sensor_task */
  /* Infinite loop */
  for(;;)
  {
    temp_read();
    tds_value();
    ph_value();
    TU_Value_Conversion();
    // printf("ADC CH1=%u, CH2=%u, CH3=%u\r\n", adc_value[0], adc_value[1], adc_value[2]);
    if(sensorData.temp>40||sensorData.pH_Value>sensorThresholds.pH_threshold||sensorData.tds_Value>500||sensorData.TU_Value>sensorThresholds.TU_threshold)
    {
      HAL_GPIO_WritePin(gpio_buzzer_GPIO_Port,gpio_buzzer_Pin,GPIO_PIN_RESET);
    }
    else
    {
      HAL_GPIO_WritePin(gpio_buzzer_GPIO_Port,gpio_buzzer_Pin,GPIO_PIN_SET);
    }
    /* xSemaphoreGive(xBinarySemaphore1); //发送信号量 - 已改为由串口接收中断触发 */
    xSemaphoreGive(xBinarySemaphore2); //发送信号量
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}
void oled_display_task(void *argument)
{
  /* USER CODE BEGIN oled_display_task */
  /* Infinite loop */
  for(;;)
  {
    if (xSemaphoreTake(xBinarySemaphore2, portMAX_DELAY) == pdTRUE)//判断信号量是否得到
    {
OLED_ShowFloatNum(72,0,sensorData.temp,2,1,OLED_8X16);
OLED_ShowFloatNum(72,16,sensorData.pH_Value,2,1,OLED_8X16);
OLED_ShowFloatNum(72,32,sensorData.tds_Value,4,1,OLED_8X16);
OLED_ShowFloatNum(72,48,sensorData.TU_Value,4,1,OLED_8X16);
OLED_Update();
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
/* USER CODE END Application */
/* Diagnostic hooks for FreeRTOS (stack overflow / malloc failure) */
#if (configCHECK_FOR_STACK_OVERFLOW > 0)
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  (void)xTask;
  // printf("[RTOS] Stack overflow: %s\r\n", pcTaskName);
  taskDISABLE_INTERRUPTS();
  for(;;){}
}
#endif

#if (configUSE_MALLOC_FAILED_HOOK == 1)
void vApplicationMallocFailedHook(void)
{
  // printf("[RTOS] Malloc failed! free=%lu min=%lu\r\n",
  //        (unsigned long)xPortGetFreeHeapSize(),
  //        (unsigned long)xPortGetMinimumEverFreeHeapSize());
  taskDISABLE_INTERRUPTS();
  for(;;){}
}
#endif

