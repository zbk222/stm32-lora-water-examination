#ifndef __LORA_H__
#define __LORA_H__

#include "stm32g4xx_hal.h"
#include "usart.h"
#include "sensor.h"
#include "FreeRTOS.h"
#include "semphr.h" 
/*lora使用定点传输
模块地址：节点：0x01 网关：0x02
信道：0X01
网络ID：0X04。
数据传输格式：地址高字节，地址低字节，信道，帧头1(0XFF)，帧头2(0XFE)，设备id（0X01-0X03）,数据内容（先后顺序是结构体中tds，
浊度，温度，ph），帧尾1，帧尾2。
当M0=0，M1=0时，模块工作在传输模式下，波特率为9600，无校验，1个停止位。
当M0=0,M1=1,时，模块工作在配置模式下，波特率为9600，无校验，1个停止位。
*/
/*
v4更新。更新接收链路。网关板会将从云平台接收到的ph和浊度的阈值通过lora下发给节点板，节点板解析数据帧并写入阈值结构体中。
数据帧为：0XFF(帧头) 0X01(设备码) (PH阈值)4字节 (浊度阈值)4字节 0XFD(帧尾)
*/ 
#define LORA_ADDRESS_NODE_HIGH    0x00
#define LORA_ADDRESS_NODE_LOW     0x02
#define LORA_XINDAO 0x17
#define LORA_id 0x01
#define LORA_zhentou1 0XFF
#define LORA_zhentou2 0XFE
#define LORA_zhwei1 0XFD
#define LORA_zhwei2 0XFC
#define LORA_M0(x) HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,(GPIO_PinState)x)
#define LORA_M1(x) HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,(GPIO_PinState)x)
#define LORA_AUX  HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_8)
// typedef struct uart1_buffer{
//  uint8_t buffer[256];
//      int32_t rd;
//      int32_t wr;
// } uart1_buffer;
// extern uart1_buffer lora_uart1_buffer;
extern SemaphoreHandle_t xUart1TxDoneSem;

int lora_init(void);
void lora_send_data(void);
void lora_recv(void);



#endif
