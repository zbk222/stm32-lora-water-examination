/*-------------------------------------------------*/
/*            超子说物联网STM32系列开发板          */
/*-------------------------------------------------*/
/*                                                 */
/*              操作WiFi功能的头文件               */
/*                                                 */
/*-------------------------------------------------*/

#ifndef __WiFi_H
#define __WiFi_H

#include "usart.h"	    
#include "stdint.h"

#define CAT1_TYPE        3          //1:Air780模块  2：Air724模块   3：EC800M模块

#define PACK_MAX_SIZE    1024

#define WIFI_RESET(x)    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_0, (GPIO_PinState)x)    //PF0控制WiFi模块的复位

#define CAT1_POWER(x)    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, (GPIO_PinState)x)   //PA15控制4G Cat1模块复位IO
#define CAT1_POWER_STA   HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)                       //PA0读取电平状态,可以判断4G Cat1模块是开机还是关机状态
#define CAT1_NET_STA     HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1)                       //PA1读取电平状态,可以判断4G Cat1模块网络注册状态

#define PACK_NUM 6                                          //可以记录的最大数据包的数量
typedef struct{       
	uint16_t Data_Packsta;                                  //分包状态：0表示数据无分包，1表示有数据分包
	uint16_t Data_totle;                                    //用于记录已经处理的数据的长度
	uint16_t Data_lenth[PACK_NUM];                          //用于记录整包数据的长度 
	uint16_t Data_num;                                      //用于记录有几个数据包
	uint16_t Data_Packlen[PACK_NUM];                        //用于记录数据包数据长度           
	char     Data_Packbuff[PACK_NUM][PACK_MAX_SIZE];        //用于记录一个整包的数据缓冲区
}Pack_CB;                                                   //整包数据控制结构体
#define PACK_CB_LEN         sizeof(Pack_CB)                 //整包数据控制结构体占用的字节量 

void WiFi_Cat1_InitGPIO(void);                              //函数说明，初始化WiFi 4G Cat1模块 控制IO                
void WiFi_Reset(void);                                      //函数说明，复位WiFi模块                           
void Cat1_Reset(void);                                      //函数说明，复位4G Cat1模块 
void WiFi_ProcessData(uint8_t *, uint16_t);                 //函数说明，处理WiFi模块的数据
void Cat1_ProcessData(uint8_t *, uint16_t);                 //函数说明，处理4G Cat1模块的数据
void Server_ProcessData(uint8_t *, uint16_t);               //函数说明，处理服务器的数据  
void WiFi_Cat1_ActiveEvent(void);                           //函数说明，WiFi模块 or 4G Cat1模块主动事件
void WiFi_Cat1_SubOnline(char,char);                        //函数说明，子设备上下线
void WiFi_Cat1_SubDataPost(unsigned char *);                //函数说明，子设备数据上传

#endif


