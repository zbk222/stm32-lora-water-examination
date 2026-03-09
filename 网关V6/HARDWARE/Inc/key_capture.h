/*-------------------------------------------------*/
/*            超子说物联网STM32系列开发板          */
/*-------------------------------------------------*/
/*                                                 */
/*             实现按键捕获功能的头文件            */
/*                                                 */
/*-------------------------------------------------*/
#ifndef __KEY_CAPTURE_H
#define __KEY_CAPTURE_H	

#include "stdint.h"          
#include "stm32g0xx_hal_tim.h"

extern TIM_HandleTypeDef tim14;                                           //外部变量声明，定时器总控结构体
extern uint8_t key_flag;                                                  //外部变量声明，按键标志位

void KEY_Capture_Init(void);                                              //函数声明，按键捕获初始化程序，判断长按短按

#endif
