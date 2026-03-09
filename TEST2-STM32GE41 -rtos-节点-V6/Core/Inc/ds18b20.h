#ifndef __DS18B20_H
#define __DS18B20_H  
#include "stm32g4xx_hal.h"
#include "FreeRTOS.h"
//IO方向设置（CRL寄存器对应引脚0~7,CRH寄存器对应引脚8~15）
//DS18B20_GPIO_PORT->CRH&=0xFFFFFFF0为PA8引脚输出模式对应的寄存器清空
//DS18B20_GPIO_PORT->CRH|=0x00000008将(CNF8[1:0]设置为10:上拉/下拉输入模式，MODE8[1;0]设置为00:输入模式)
//DS18B20_GPIO_PORT->CRH|=0x00000003将(CNF8[1:0]设置为00:通用推挽输出模式 ，MODE8[1;0]设置为11:最大50MHZ)
//IO方向设置
#define DS18B20_IO_IN()  { \
    GPIO_InitTypeDef GPIO_InitStruct = {0}; \
    GPIO_InitStruct.Pin = GPIO_PIN_14; \
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT; \
    GPIO_InitStruct.Pull = GPIO_PULLUP; \
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); \
}

#define DS18B20_IO_OUT() { \
    GPIO_InitTypeDef GPIO_InitStruct = {0}; \
    GPIO_InitStruct.Pin = GPIO_PIN_14; \
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; \
    GPIO_InitStruct.Pull = GPIO_PULLUP; \
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; \
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); \
}
////IO��������											   
#define	DS18B20_DQ_OUT(x) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, x) //���ݶ˿�	PB14
#define	DS18B20_DQ_IN   HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14)//���ݶ˿�	PB14

extern short temperature;//�¶�
extern uint16_t tempMax;//�¶�����
extern uint16_t tempMin;//�¶�����
   	
uint8_t DS18B20_Init(void);			//��ʼ��DS18B20
short DS18B20_Get_Temp(void);	//��ȡ�¶�
void DS18B20_Start(void);		//��ʼ�¶�ת��
void DS18B20_Write_Byte(uint8_t dat);//д��һ���ֽ�
uint8_t DS18B20_Read_Byte(void);		//����һ���ֽ�
uint8_t DS18B20_Read_Bit(void);		//����һ��λ
uint8_t DS18B20_Check(void);			//����Ƿ����DS18B20
void DS18B20_Rst(void);			//��λDS18B20  
//void DisplayTemperature(void);//��ʾ�¶Ⱥ���
#endif















