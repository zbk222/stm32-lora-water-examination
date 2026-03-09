/*-------------------------------------------------*/
/*            超子说物联网STM32系列开发板            */
/*-------------------------------------------------*/
/*                                                 */
/*            实现定时器功能的头文件                */
/*                                                 */
/*-------------------------------------------------*/

#ifndef _TIMER_H
#define _TIMER_H

#include "stdint.h"          
#include "stm32g0xx_hal_tim.h"

extern TIM_HandleTypeDef tim3;                   //外部变量声明，定时器3总控结构体
extern TIM_HandleTypeDef tim6;                   //外部变量声明，定时器6总控结构体
extern uint8_t tim6_flag;                        //外部变量声明，定时器6标志位

void TIM3_Init(uint16_t, uint16_t);              //函数声明，定时器3初始化
void TIM6_Init(uint16_t, uint16_t);              //函数声明，定时器6初始化

#endif
