
#include "lora.h"
#include "semphr.h"
#include "task.h"
#include <string.h>
#include <stdio.h>



 extern uint8_t uart1_rx_buf[256]; /* 引用接收buffer */
extern SemaphoreHandle_t xMutex;
extern SemaphoreHandle_t xUart1TxDoneSem;
extern SemaphoreHandle_t xBinarySemaphore1; /* 引用发送任务信号量 */
/* 调试日志开关：0 关闭，1 打印 */
#ifndef LORA_DEBUG
#define LORA_DEBUG 0
#endif

int lora_init(void)
{
// uint32_t wait_time_ms = 0;
//     const uint32_t timeout_ms = 1000; // 超时时间1秒
//  LORA_M0(0);//M0=0
//  LORA_M1(1);
//  vTaskDelay(pdMS_TO_TICKS(2000));
//  char cmd[] = "AT+RESET";
//      HAL_UART_Transmit(&huart1, (uint8_t*)cmd, strlen(cmd), 100);
//      vTaskDelay(pdMS_TO_TICKS(2000)); // 初始延时，避免忙等
    LORA_M0(0);//M0=0
    LORA_M1(0);//M1=0
    vTaskDelay(pdMS_TO_TICKS(2000));
    // 5. (强烈建议) 在此处添加模块参数一致性检查或配置
    // 例如：使用AT指令读取当前信道、速率，与预设值对比
    // if (!lora_check_parameters()) {
    //     return 0;
    // }
   // printf("LoRa Module Initialized Successfully.\n");
    return 1; // 初始化成功
}





/* 全局静态缓冲，避免 IT 发送期间被局部栈复用覆盖 */
static uint8_t lora_tx_buf[32];
static size_t  lora_tx_len = 0;

void lora_send_data(void)
{
    /* 组包缓冲区（含边界防护） */
    size_t index = 0;

    /* 固定字段 */
    lora_tx_buf[index++] = LORA_ADDRESS_NODE_HIGH;
    lora_tx_buf[index++] = LORA_ADDRESS_NODE_LOW;
    lora_tx_buf[index++] = LORA_XINDAO;
    lora_tx_buf[index++] = LORA_zhentou1;
    lora_tx_buf[index++] = LORA_zhentou2;
    lora_tx_buf[index++] = LORA_id;
    /* 传感数据（float 按内存字节打包） */
    float sensorValues[4] = {
        sensorData.tds_Value,
        sensorData.TU_Value,
        sensorData.temp,
        sensorData.pH_Value
    };

    for (int i = 0; i < 4; i++) {
        /* 采用 memcpy 避免别名与未对齐问题 */
        uint8_t fb[4];
        memcpy(fb, &sensorValues[i], 4);
        for (int j = 0; j < 4; j++) {
            if (index < sizeof(lora_tx_buf)) {
                lora_tx_buf[index++] = fb[j];
            }
        }
    }

    /* 帧尾 */
    if (index + 2 <= sizeof(lora_tx_buf)) {
        lora_tx_buf[index++] = LORA_zhwei1;
        lora_tx_buf[index++] = LORA_zhwei2;
    }
    lora_tx_len = index;
#if LORA_DEBUG
    // printf("LoRa Sending Data: ");
    // for (size_t i = 0; i < lora_tx_len; i++) {
    //     printf("%02X ", lora_tx_buf[i]);
    // }
    // printf("\r\n");
#endif
    /* 等待 AUX 空闲，加入超时，避免无限阻塞 */
    TickType_t start = xTaskGetTickCount();
    while (LORA_AUX == GPIO_PIN_RESET) {
        vTaskDelay(pdMS_TO_TICKS(5));
        if ((xTaskGetTickCount() - start) > pdMS_TO_TICKS(500)) {
            // printf("LoRa AUX Busy Timeout!\n");
            break; /* 超过 500ms 直接发送 */
        }
    }

    /* 非阻塞发送：使用 IT 方式 + 完成信号量，保证缓冲不被复用 */
    if (xSemaphoreTake(xMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        /* 若串口忙，等待短时间后重试/返回 */
        if (huart1.gState != HAL_UART_STATE_READY) {
            vTaskDelay(pdMS_TO_TICKS(2));
        }
        if (HAL_UART_Transmit_IT(&huart1, lora_tx_buf, lora_tx_len) == HAL_OK) {
            /* 等待发送完成，超时保护 */
            if (xSemaphoreTake(xUart1TxDoneSem, pdMS_TO_TICKS(200)) != pdTRUE) {
#if LORA_DEBUG
                // printf("LoRa TX timeout\r\n");
#endif
            }
        } else {
#if LORA_DEBUG
            // printf("LoRa TX start failed\r\n");
#endif
        }
        xSemaphoreGive(xMutex);
    } else {
        /* 避免频繁输出导致任务栈压力 */
#if LORA_DEBUG
        // printf("LoRa UART busy (mutex)\r\n");
#endif
    }
     
    /* 可选调试：日志走 USART2 的 fputc，避免与 LoRa 串口复用 */
}
void lora_recv()
{/* 请求帧判断:  ff 01 01 fe */
  if (uart1_rx_buf[0] == 0xFF && uart1_rx_buf[1] == 0x01 && 
      uart1_rx_buf[2] == 0x01 && uart1_rx_buf[3] == 0xFE)
  {
      /* 收到请求帧，给出信号量触发发送任务 */
      xSemaphoreGive(xBinarySemaphore1);
  }
  /* 阈值设置帧判断: FF 01 ... FD */
  else 
  if (uart1_rx_buf[0] == 0xFF && uart1_rx_buf[1] == 0x01 && uart1_rx_buf[10] == 0xFD)
  {
      float new_ph_th, new_tu_th;
      /* 使用 memcpy 避免对齐问题 */
      memcpy(&new_ph_th, &uart1_rx_buf[2], 4);
      memcpy(&new_tu_th, &uart1_rx_buf[6], 4);
      
      /* 安全保护：防止 Mutex 未创建时访问导致 Crash */
      if (xMutex != NULL) {
          xSemaphoreTake(xMutex, portMAX_DELAY);
          sensorThresholds.pH_threshold = new_ph_th;
          sensorThresholds.TU_threshold = new_tu_th;
          xSemaphoreGive(xMutex);
      } else {
          /* Fallback: 如果没有 Mutex，直接赋值（不推荐但比死机好） */
          sensorThresholds.pH_threshold = new_ph_th;
          sensorThresholds.TU_threshold = new_tu_th;
      }
      
      // printf("[LoRa Rx] Updated Thresholds: PH=%.2f, TU=%.2f\r\n", new_ph_th, new_tu_th);
  }

}
