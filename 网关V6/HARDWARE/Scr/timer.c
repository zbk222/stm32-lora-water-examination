
#include "stm32g0xx_hal.h"  
#include "main.h"         
#include "timer.h"         
#include "usart.h"         

TIM_HandleTypeDef tim3;    //定时器3总控结构体
TIM_HandleTypeDef tim6;    //定时器6总控结构体
uint8_t tim6_flag;         //定时器6标志位

/*-------------------------------------------------*/
/*函数名：定时器3初始化                            */
/*参  数：arr：自动重装值                          */
/*参  数：psc：时钟预分频数                        */
/*返回值：无                                       */
/*-------------------------------------------------*/
void TIM3_Init(uint16_t arr, uint16_t psc)
{
	tim3.Instance = TIM3;                                         //设置定时器3
	tim3.Init.Prescaler = psc-1;                                  //设置定时器预分频数
	tim3.Init.Period = arr-1;                                     //设置自动重装载值
	tim3.Init.CounterMode = TIM_COUNTERMODE_UP;                   //向上计数模式
	tim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; //不使用预装载
	HAL_TIM_Base_Init(&tim3);                                     //设置
	__HAL_TIM_CLEAR_IT(&tim3, TIM_IT_UPDATE);                     //清除中断标志位
}
/*-------------------------------------------------*/
/*函数名：定时器6初始化                            */
/*参  数：arr：自动重装值                          */
/*参  数：psc：时钟预分频数                        */
/*返回值：无                                       */
/*-------------------------------------------------*/
void TIM6_Init(uint16_t arr, uint16_t psc)
{
	tim6.Instance = TIM6;                                         //设置定时器6
	tim6.Init.Prescaler = psc-1;                                  //设置定时器预分频数
	tim6.Init.Period = arr-1;                                     //设置自动重装载值
	tim6.Init.CounterMode = TIM_COUNTERMODE_UP;                   //向上计数模式
	tim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; //不使用预装载
	HAL_TIM_Base_Init(&tim6);                                     //设置
	__HAL_TIM_CLEAR_IT(&tim6, TIM_IT_UPDATE);                     //清除中断标志位
	HAL_TIM_Base_Start_IT(&tim6);                                 //启动定时器6	
	
}
/*-------------------------------------------------*/
/*函数名：定时器底层驱动，开启时钟，设置中断优先级 */
/*参  数：htim：定时器控制总控结构体指针           */
/*返回值：无                                       */
/*说  明：此函数会被HAL_TIM_Base_Init()函数调用    */
/*-------------------------------------------------*/
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM3){	                               //如果是定时器3，进入if	分支	
		__HAL_RCC_TIM3_CLK_ENABLE();                           //使能TIM3时钟
		HAL_NVIC_SetPriority(TIM3_IRQn,1,3);                   //设置中断优先级
		HAL_NVIC_EnableIRQ(TIM3_IRQn);                         //开启ITM3中断   
	}else if(htim->Instance==TIM6){	                           //如果是配置定时器6，进入if		
		__HAL_RCC_TIM6_CLK_ENABLE();                           //使能TIM6时钟
		HAL_NVIC_SetPriority(TIM6_IRQn,1,3);                   //设置中断优先级，抢占优先级1，子优先级3（G0系列子优先级无效）
		HAL_NVIC_EnableIRQ(TIM6_IRQn);                         //开启ITM6中断   
	}
}
/*---------------------------------------------------*/
/*函数名：定时器3定时任务，回调函数                  */
/*参  数：htim：定时器控制总控结构体指针             */
/*返回值：无                                         */
/*说  明：此函数会被HAL_TIM_IRQHandler()中断函数调用 */
/*---------------------------------------------------*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM3){                     //如果是定时器3，进入if分支
		HAL_TIM_Base_Stop_IT(htim);               //关闭定时器3
		U3_CB.TxCpltflag &=~ TX_STA_ACK;          //接收超时，清除相应状态标记	
	}else if(htim->Instance==TIM6){               //如果是定时器6，进入else if分支
		HAL_TIM_Base_Stop_IT(htim);               //关闭定时器6
		tim6_flag = 1;                            //需要切换成4G Cat1联网
	}	
}
