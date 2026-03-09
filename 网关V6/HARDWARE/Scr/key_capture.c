
#include "stm32g0xx_hal.h"    
#include "main.h"             
#include "clock.h"            
#include "key_capture.h"     
#include "usart.h"            
#include "wifi_cat1.h"	          

TIM_HandleTypeDef tim14;    //定时器总控结构体
uint8_t key_flag;           //按键标志位

/*-------------------------------------------------*/
/*函数名：按键捕获初始化程序，判断按键时间         */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/	   
void KEY_Capture_Init(void)
{
	TIM_IC_InitTypeDef ConfigIC;                               //定义一个配置捕获功能的控制结构体
	
	tim14.Instance = TIM14;                                    //使用定时器14
	tim14.Init.Prescaler = 64000-1;                            //设置预分频值 64m/64000 = 1000hz 计1个数1ms
	tim14.Init.Period = 0xFFFF;                                //周期重载值
	tim14.Init.CounterMode = TIM_COUNTERMODE_UP;               //向上计数
	tim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;         //1分频
	HAL_TIM_IC_Init(&tim14);                                   //设置定时器
	
	ConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;   //捕获上下沿
	ConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;           //直连IO
	ConfigIC .ICPrescaler = TIM_ICPSC_DIV1;                     //1分频
	HAL_TIM_IC_ConfigChannel(&tim14,&ConfigIC,TIM_CHANNEL_1);  //配置定时器通道1
	HAL_TIM_IC_Start_IT(&tim14, TIM_CHANNEL_1);                //使能定时器通道1的捕获中断
}
/*-------------------------------------------------*/
/*函数名：配置定时器的底层硬件                     */
/*参  数：htim：定时器总控结构体                   */
/*返回值：无                                       */
/*说  明：被HAL_TIM_IC_ConfigChannel函数调用       */
/*-------------------------------------------------*/	 
void HAL_TIM_IC_MspInit(TIM_HandleTypeDef* htim)
{
	GPIO_InitTypeDef GPIO_InitStruct;            	  //定义一个配置IO的总控结构体
	
	if(htim->Instance==TIM14){                   	  //如果是定时器14
		__HAL_RCC_TIM14_CLK_ENABLE();             	  //使能定时器时钟
		__HAL_RCC_GPIOC_CLK_ENABLE();			      //使能GPIOC时钟
		
		GPIO_InitStruct.Pin = GPIO_PIN_12;            //准备配置PC12
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;       //复用推挽
		GPIO_InitStruct.Pull = GPIO_PULLUP;           //开启内部上拉
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM14;   //复用为定时器14通道1
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);       //配置PC12
		
		HAL_NVIC_SetPriority(TIM14_IRQn, 0, 0);       //设置定时器中断
		HAL_NVIC_EnableIRQ(TIM14_IRQn);               //使能定时器中断
	}
}
/*-------------------------------------------------*/
/*函数名：定时器中断回调函数                       */
/*参  数：htim_ic：定时器总控结构体                */
/*返回值：无                                       */
/*说  明：被HAL_TIM_IRQHandler函数调用             */
/*-------------------------------------------------*/
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM14){                 	                           		//如果是定时器14
		if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_12) == 0){                           //如果当前是低电平，进入if
			__HAL_TIM_DISABLE(htim);                                        	//先关闭定时器的计数    
			__HAL_TIM_SET_COUNTER(htim,0);                                  	//设置计数器值=0
			__HAL_TIM_ENABLE(htim);                                         	//再打开定时器计数，从0开始计数
		}else{                                                             	    //反之是高电平，进入else
			__HAL_TIM_DISABLE(htim);                                      	    //关定时器计数
			if(HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1)>=3000){           //读取计数器数值，大于3000，进入if					
				key_flag = 1;                                                   //key_flag置1									
		    }
		}
	}  
}
