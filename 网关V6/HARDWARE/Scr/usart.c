
#include "stm32g0xx_hal.h"    
#include "usart.h"      
#include "timer.h"  
#include "stdio.h"            
#include "stdarg.h"		   
#include "string.h" 

Usart_CB U1_CB;                             //串口1控制结构体
uint8_t  U1_TxBuff[U1_TXBUFF_SIZE];         //串口1发送缓冲区 
uint8_t  U1_RxBuff[U1_RXBUFF_SIZE];         //串口1接收缓冲区 
 
Usart_CB U2_CB;                             //串口2控制结构体
uint8_t  U2_TxBuff[U2_TXBUFF_SIZE];         //串口2发送缓冲区 
uint8_t  U2_RxBuff[U2_RXBUFF_SIZE];         //串口2接收缓冲区 
 
Usart_CB U3_CB;                             //串口3控制结构体
uint8_t  U3_TxBuff[U3_TXBUFF_SIZE];         //串口3发送缓冲区 
uint8_t  U3_RxBuff[U3_RXBUFF_SIZE];         //串口3接收缓冲区

Usart_CB U4_CB;                             //串口4控制结构体
uint8_t  U4_TxBuff[U4_TXBUFF_SIZE];         //串口4发送缓冲区 
uint8_t  U4_RxBuff[U4_RXBUFF_SIZE];         //串口4接收缓冲区 

/*-------------------------------------------------*/
/*函数名：初始化串口1                              */
/*参  数：bound：波特率                            */
/*返回值：无                                       */
/*-------------------------------------------------*/
void U1_Init(uint32_t bound)
{	
	U1_CB.USART_Handler.Instance=USART1;					        //使用串口1
	U1_CB.USART_Handler.Init.BaudRate=bound;				        //设置波特率
	U1_CB.USART_Handler.Init.WordLength=UART_WORDLENGTH_8B;         //设置字长为8位数据格式
	U1_CB.USART_Handler.Init.StopBits=UART_STOPBITS_1;	            //设置1个停止位
	U1_CB.USART_Handler.Init.Parity=UART_PARITY_NONE;		        //设置无奇偶校验位
	U1_CB.USART_Handler.Init.Mode=UART_MODE_TX_RX;		            //设置收发模式
	U1_CB.USART_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;         //设置无硬件流控
	HAL_UART_Init(&U1_CB.USART_Handler);			                //设置串口1
	
	U1_CB.RxIN  = &U1_CB.se_RxBuff[0];                              //接收缓冲区 IN指针  指向se指针对结构体数组0号成员  
	U1_CB.RxOUT = &U1_CB.se_RxBuff[0];                              //接收缓冲区 OUT指针 指向se指针对结构体数组0号成员  
    U1_CB.RxIN->s = U1_RxBuff;                                      //接收缓冲区 IN指针  当前指向的se指针对结构体数组成员中的s指向串口1接收缓冲区的起始位置，为接收DMA指明储存位置
	U1_CB.RxCounter = 0;                                            //累计 接收缓冲区 已经存放的数据量 为0
	
	HAL_UARTEx_ReceiveToIdle_DMA(&U1_CB.USART_Handler,U1_CB.RxIN->s,U1_RXMAX_SIZE);    //开启DMA接收 存放在U1_CB.RxIN->s指明的位置 传输量为U1_RXMAX_SIZE
}
/*-------------------------------------------------*/
/*函数名：初始化串口2                              */
/*参  数：bound：波特率                            */
/*返回值：无                                       */
/*-------------------------------------------------*/
void U2_Init(uint32_t bound)
{	
	U2_CB.USART_Handler.Instance=USART2;					        //使用串口2
	U2_CB.USART_Handler.Init.BaudRate=bound;				        //设置波特率
	U2_CB.USART_Handler.Init.WordLength=UART_WORDLENGTH_8B;         //设置字长为8位数据格式
	U2_CB.USART_Handler.Init.StopBits=UART_STOPBITS_1;	            //设置一个停止位
	U2_CB.USART_Handler.Init.Parity=UART_PARITY_NONE;		        //设置无奇偶校验位
	U2_CB.USART_Handler.Init.Mode=UART_MODE_TX_RX;		            //设置收发模式
	U2_CB.USART_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;         //设置无硬件流控
	HAL_UART_Init(&U2_CB.USART_Handler);			                //设置串口2
		
	U2_CB.TxIN  = &U2_CB.se_TxBuff[0];           	                //发送缓冲区 IN指针  指向se指针对结构体数组0号成员   
	U2_CB.TxOUT = &U2_CB.se_TxBuff[0];              	            //发送缓冲区 OUT指针 指向se指针对结构体数组0号成员 
    U2_CB.TxIN->s = U2_TxBuff;	               	                    //发送缓冲区 IN指针  当前指向的se指针对结构体数组成员中的s指向串口2发送缓冲区的起始位置，为向发送缓冲区存放数据指明位置
	U2_CB.TxCounter = 0;                                      	    //累计 发送冲区 已经存放的数据量 为0
	U2_CB.TxCpltflag = 0;                                     	    //DMA发送状态 0：空闲
		
	U2_CB.RxIN  = &U2_CB.se_RxBuff[0];          	                //接收缓冲区 IN指针  指向se指针对结构体数组0号成员   
	U2_CB.RxOUT = &U2_CB.se_RxBuff[0];             	                //接收缓冲区 OUT指针 指向se指针对结构体数组0号成员    
    U2_CB.RxIN->s = U2_RxBuff;                  	                //接收缓冲区 IN指针  当前指向的se指针对结构体数组成员中的s指向串口2接收缓冲区的起始位置，为接收DMA指明储存位置
	U2_CB.RxCounter = 0;                                            //累计 接收缓冲区 已经存放的数据量 为0
	
	HAL_UARTEx_ReceiveToIdle_DMA(&U2_CB.USART_Handler,U2_CB.RxIN->s,U2_RXMAX_SIZE);    //开启DMA接收 存放在U2_CB.RxIN->s指明的位置 传输量为U2_RXMAX_SIZE
}
/*-------------------------------------------------*/
/*函数名：初始化串口3                              */
/*参  数：bound：波特率                            */
/*返回值：无                                       */
/*-------------------------------------------------*/
void U3_Init(uint32_t bound)
{	
	U3_CB.USART_Handler.Instance=USART3;					        //使用串口3
	U3_CB.USART_Handler.Init.BaudRate=bound;				        //设置波特率
	U3_CB.USART_Handler.Init.WordLength=UART_WORDLENGTH_8B;         //设置字长为8位数据格式
	U3_CB.USART_Handler.Init.StopBits=UART_STOPBITS_1;	            //设置一个停止位
	U3_CB.USART_Handler.Init.Parity=UART_PARITY_NONE;		        //设置无奇偶校验位
	U3_CB.USART_Handler.Init.Mode=UART_MODE_TX_RX;		            //设置收发模式
	U3_CB.USART_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;         //设置无硬件流控
	HAL_UART_Init(&U3_CB.USART_Handler);			                //设置串口3
		
	U3_CB.TxIN  = &U3_CB.se_TxBuff[0];           	                //发送缓冲区 IN指针  指向se指针对结构体数组0号成员   
	U3_CB.TxOUT = &U3_CB.se_TxBuff[0];              	            //发送缓冲区 OUT指针 指向se指针对结构体数组0号成员 
    U3_CB.TxIN->s = U3_TxBuff;	               	                    //发送缓冲区 IN指针  当前指向的se指针对结构体数组成员中的s指向串口3发送缓冲区的起始位置，为向发送缓冲区存放数据指明位置
	U3_CB.TxCounter = 0;                                      	    //累计 发送冲区 已经存放的数据量 为0
	U3_CB.TxCpltflag = 0;                                     	    //DMA发送状态 0：空闲
		
	U3_CB.RxIN  = &U3_CB.se_RxBuff[0];          	                //接收缓冲区 IN指针  指向se指针对结构体数组0号成员   
	U3_CB.RxOUT = &U3_CB.se_RxBuff[0];             	                //接收缓冲区 OUT指针 指向se指针对结构体数组0号成员    
    U3_CB.RxIN->s = U3_RxBuff;                  	                //接收缓冲区 IN指针  当前指向的se指针对结构体数组成员中的s指向串口3接收缓冲区的起始位置，为接收DMA指明储存位置
	U3_CB.RxCounter = 0;                                            //累计 接收缓冲区 已经存放的数据量 为0
	
	HAL_UARTEx_ReceiveToIdle_DMA(&U3_CB.USART_Handler,U3_CB.RxIN->s,U3_RXMAX_SIZE);    //开启DMA接收 存放在U3_CB.RxIN->s指明的位置 传输量为U3_RXMAX_SIZE
}
/*-------------------------------------------------*/
/*函数名：初始化串口4                              */
/*参  数：bound：波特率                            */
/*返回值：无                                       */
/*-------------------------------------------------*/
void U4_Init(uint32_t bound)
{	
	U4_CB.USART_Handler.Instance=USART4;					        //使用串口4
	U4_CB.USART_Handler.Init.BaudRate=bound;				        //设置波特率
	U4_CB.USART_Handler.Init.WordLength=UART_WORDLENGTH_8B;         //设置字长为8位数据格式
	U4_CB.USART_Handler.Init.StopBits=UART_STOPBITS_1;	            //设置一个停止位
	U4_CB.USART_Handler.Init.Parity=UART_PARITY_NONE;		        //设置无奇偶校验位
	U4_CB.USART_Handler.Init.Mode=UART_MODE_TX_RX;		            //设置收发模式
	U4_CB.USART_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;         //设置无硬件流控
	HAL_UART_Init(&U4_CB.USART_Handler);			                //设置串口4
		
	U4_CB.TxIN  = &U4_CB.se_TxBuff[0];           	                //发送缓冲区 IN指针  指向se指针对结构体数组0号成员   
	U4_CB.TxOUT = &U4_CB.se_TxBuff[0];              	            //发送缓冲区 OUT指针 指向se指针对结构体数组0号成员 
    U4_CB.TxIN->s = U4_TxBuff;	               	                    //发送缓冲区 IN指针  当前指向的se指针对结构体数组成员中的s指向串口4发送缓冲区的起始位置，为向发送缓冲区存放数据指明位置
	U4_CB.TxCounter = 0;                                      	    //累计 发送冲区 已经存放的数据量 为0
	U4_CB.TxCpltflag = 0;                                     	    //DMA发送状态 0：空闲
		
	U4_CB.RxIN  = &U4_CB.se_RxBuff[0];          	                //接收缓冲区 IN指针  指向se指针对结构体数组0号成员   
	U4_CB.RxOUT = &U4_CB.se_RxBuff[0];             	                //接收缓冲区 OUT指针 指向se指针对结构体数组0号成员    
    U4_CB.RxIN->s = U4_RxBuff;                  	                //接收缓冲区 IN指针  当前指向的se指针对结构体数组成员中的s指向串口4接收缓冲区的起始位置，为接收DMA指明储存位置
	U4_CB.RxCounter = 0;                                            //累计 接收缓冲区 已经存放的数据量 为0
	
	HAL_UARTEx_ReceiveToIdle_DMA(&U4_CB.USART_Handler,U4_CB.RxIN->s,U4_RXMAX_SIZE);    //开启DMA接收 存放在U4_CB.RxIN->s指明的位置 传输量为U4_RXMAX_SIZE
}
/*-------------------------------------------------*/
/*函数名：串口1的底层初始化                        */
/*参  数：huart：串口配置总控结构体                */
/*返回值：无                                       */
/*说  明：此函数会被HAL_UART_Init()回调调用        */
/*-------------------------------------------------*/
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{  
	GPIO_InitTypeDef GPIO_Initure;                                                //GPIO口总控结构体
	
	if(huart->Instance==USART1){                                                  //如果是串口1，进入if分支
		__HAL_RCC_GPIOA_CLK_ENABLE();			                                  //使能GPIOA时钟
		__HAL_RCC_USART1_CLK_ENABLE();			                                  //使能串口1时钟
		__HAL_RCC_DMA1_CLK_ENABLE();                                              //使能DMA1时钟
		
		GPIO_Initure.Pin = GPIO_PIN_9|GPIO_PIN_10;                                //准备设置PA9 10
		GPIO_Initure.Mode = GPIO_MODE_AF_PP;                                      //复用功能
		GPIO_Initure.Pull = GPIO_PULLUP;                                          //上拉
		GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;                                //快速	
		GPIO_Initure.Alternate = GPIO_AF1_USART1;                                 //PA9 10复用为串口1的TXD RXD功能
		HAL_GPIO_Init(GPIOA, &GPIO_Initure);                                      //设置PA9 10   

        HAL_NVIC_SetPriority(USART1_IRQn,2,0);                                    //设置串口1中断优先级
		HAL_NVIC_EnableIRQ(USART1_IRQn);                                          //使能串口1的中断
		
		U1_CB.USART_RxDMA.Instance                 = DMA1_Channel1;               //DMA1通道1
		U1_CB.USART_RxDMA.Init.Request             = DMA_REQUEST_USART1_RX;       //串口1接收
		U1_CB.USART_RxDMA.Init.Direction           = DMA_PERIPH_TO_MEMORY;        //外设到存储区
		U1_CB.USART_RxDMA.Init.PeriphInc           = DMA_PINC_DISABLE;            //外设不递增
		U1_CB.USART_RxDMA.Init.MemInc              = DMA_MINC_ENABLE;             //存储区递增
		U1_CB.USART_RxDMA.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;         //按字节存取
		U1_CB.USART_RxDMA.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;         //按字节存取
		U1_CB.USART_RxDMA.Init.Mode                = DMA_NORMAL;                  //正常模式
		U1_CB.USART_RxDMA.Init.Priority            = DMA_PRIORITY_HIGH;           //高优先级
		HAL_DMA_Init(&U1_CB.USART_RxDMA);                                         //配置
		__HAL_LINKDMA(huart, hdmarx, U1_CB.USART_RxDMA);                          //和串口1 DMA接收连接
		
		HAL_NVIC_SetPriority(DMA1_Channel1_IRQn,2,0);                             //设置DMA1通道1中断优先级
		HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);                                   //使能DMA1通道1中断	
		 
	}else if(huart->Instance==USART2){                                            //如果是串口2，进入else if分支
		__HAL_RCC_GPIOA_CLK_ENABLE();			                                  //使能GPIOA时钟
		__HAL_RCC_USART2_CLK_ENABLE();			                                  //使能串口2时钟
		__HAL_RCC_DMA1_CLK_ENABLE();                                              //使能DMA1时钟
		
		GPIO_Initure.Pin = GPIO_PIN_2|GPIO_PIN_3;                                 //准备设置PA2 3
		GPIO_Initure.Mode = GPIO_MODE_AF_PP;                                      //复用功能
		GPIO_Initure.Pull = GPIO_PULLUP;                                          //上拉
		GPIO_Initure.Alternate = GPIO_AF1_USART2;                                 //PA2 3复用为串口2的TXD RXD功能
		HAL_GPIO_Init(GPIOA, &GPIO_Initure);                                      //设置
		
		HAL_NVIC_SetPriority(USART2_IRQn,0,2);                                    //设置串口2中断优先级
		HAL_NVIC_EnableIRQ(USART2_IRQn);                                          //使能串口2的中断
		
		U2_CB.USART_TxDMA.Instance                 = DMA1_Channel2;               //DMA1通道2
		U2_CB.USART_TxDMA.Init.Request             = DMA_REQUEST_USART2_TX;       //串口2发送
		U2_CB.USART_TxDMA.Init.Direction           = DMA_MEMORY_TO_PERIPH;        //存储区到外设
		U2_CB.USART_TxDMA.Init.PeriphInc           = DMA_PINC_DISABLE;            //外设不递增
		U2_CB.USART_TxDMA.Init.MemInc              = DMA_MINC_ENABLE;             //存储区递增
		U2_CB.USART_TxDMA.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;         //按字节存取
		U2_CB.USART_TxDMA.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;         //按字节存取
		U2_CB.USART_TxDMA.Init.Mode                = DMA_NORMAL;                  //正常模式
		U2_CB.USART_TxDMA.Init.Priority            = DMA_PRIORITY_HIGH;           //高优先级
		HAL_DMA_Init(&U2_CB.USART_TxDMA);                                         //配置
		__HAL_LINKDMA(huart, hdmatx, U2_CB.USART_TxDMA);                          //和串口2 DMA发送连接		
		
		U2_CB.USART_RxDMA.Instance                 = DMA1_Channel3;               //DMA1通道3
		U2_CB.USART_RxDMA.Init.Request             = DMA_REQUEST_USART2_RX;       //串口2接收
		U2_CB.USART_RxDMA.Init.Direction           = DMA_PERIPH_TO_MEMORY;        //外设到存储区
		U2_CB.USART_RxDMA.Init.PeriphInc           = DMA_PINC_DISABLE;            //外设不递增
		U2_CB.USART_RxDMA.Init.MemInc              = DMA_MINC_ENABLE;             //存储区递增
		U2_CB.USART_RxDMA.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;         //按字节存取
		U2_CB.USART_RxDMA.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;         //按字节存取
		U2_CB.USART_RxDMA.Init.Mode                = DMA_NORMAL;                  //正常模式
		U2_CB.USART_RxDMA.Init.Priority            = DMA_PRIORITY_HIGH;           //高优先级
		HAL_DMA_Init(&U2_CB.USART_RxDMA);                                         //配置
		__HAL_LINKDMA(huart, hdmarx, U2_CB.USART_RxDMA);                          //和串口2 DMA接收连接
		
		HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn,2,0);                           //设置DMA1通道2/3中断优先级
		HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);                                 //使能DMA1通道2/3中断		
		
	}else if(huart->Instance==USART3){                                            //如果是串口3，进入else if分支
		__HAL_RCC_GPIOB_CLK_ENABLE();			                                  //使能GPIOB时钟
		__HAL_RCC_USART3_CLK_ENABLE();			                                  //使能串口3时钟
		__HAL_RCC_DMA1_CLK_ENABLE();                                              //使能DMA1时钟
		
		GPIO_Initure.Pin = GPIO_PIN_8|GPIO_PIN_9;                                 //准备设置PB8 9
		GPIO_Initure.Mode = GPIO_MODE_AF_PP;                                      //复用功能
		GPIO_Initure.Pull = GPIO_PULLUP;                                          //上拉
		GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;                                //快速	
		GPIO_Initure.Alternate = GPIO_AF4_USART3;                                 //PB8 9复用为串口3的TXD RXD功能
		HAL_GPIO_Init(GPIOB, &GPIO_Initure);                                      //设置PB8 9
		
		HAL_NVIC_SetPriority(USART3_4_IRQn,2,0);                                  //设置串口3中断优先级
		HAL_NVIC_EnableIRQ(USART3_4_IRQn);                                        //使能串口3的中断
		
		U3_CB.USART_TxDMA.Instance                 = DMA1_Channel4;               //DMA1通道4
		U3_CB.USART_TxDMA.Init.Request             = DMA_REQUEST_USART3_TX;       //串口3发送
		U3_CB.USART_TxDMA.Init.Direction           = DMA_MEMORY_TO_PERIPH;        //存储区到外设
		U3_CB.USART_TxDMA.Init.PeriphInc           = DMA_PINC_DISABLE;            //外设不递增
		U3_CB.USART_TxDMA.Init.MemInc              = DMA_MINC_ENABLE;             //存储区递增
		U3_CB.USART_TxDMA.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;         //按字节存取
		U3_CB.USART_TxDMA.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;         //按字节存取
		U3_CB.USART_TxDMA.Init.Mode                = DMA_NORMAL;                  //正常模式
		U3_CB.USART_TxDMA.Init.Priority            = DMA_PRIORITY_HIGH;           //高优先级
		HAL_DMA_Init(&U3_CB.USART_TxDMA);                                         //配置
		__HAL_LINKDMA(huart, hdmatx, U3_CB.USART_TxDMA);                          //和串口3 DMA发送连接		
		
		U3_CB.USART_RxDMA.Instance                 = DMA1_Channel5;               //DMA1通道5
		U3_CB.USART_RxDMA.Init.Request             = DMA_REQUEST_USART3_RX;       //串口3接收
		U3_CB.USART_RxDMA.Init.Direction           = DMA_PERIPH_TO_MEMORY;        //外设到存储区
		U3_CB.USART_RxDMA.Init.PeriphInc           = DMA_PINC_DISABLE;            //外设不递增
		U3_CB.USART_RxDMA.Init.MemInc              = DMA_MINC_ENABLE;             //存储区递增
		U3_CB.USART_RxDMA.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;         //按字节存取
		U3_CB.USART_RxDMA.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;         //按字节存取
		U3_CB.USART_RxDMA.Init.Mode                = DMA_NORMAL;                  //正常模式
		U3_CB.USART_RxDMA.Init.Priority            = DMA_PRIORITY_HIGH;           //高优先级
		HAL_DMA_Init(&U3_CB.USART_RxDMA);                                         //配置
		__HAL_LINKDMA(huart, hdmarx, U3_CB.USART_RxDMA);                          //和串口3 DMA接收连接
		
		HAL_NVIC_SetPriority(DMA1_Ch4_7_DMAMUX1_OVR_IRQn,2,0);                    //设置DMA1通道4-7中断优先级
		HAL_NVIC_EnableIRQ(DMA1_Ch4_7_DMAMUX1_OVR_IRQn);                          //使能DMA1通道4-7中断
		
	}else if(huart->Instance==USART4){                                            //如果是串口4，进入else if分支
		__HAL_RCC_GPIOC_CLK_ENABLE();			                                  //使能GPIOC时钟
		__HAL_RCC_USART4_CLK_ENABLE();			                                  //使能串口4时钟
		__HAL_RCC_DMA1_CLK_ENABLE();                                              //使能DMA1时钟
		
		GPIO_Initure.Pin = GPIO_PIN_10|GPIO_PIN_11;                               //准备设置PC10 11
		GPIO_Initure.Mode = GPIO_MODE_AF_PP;                                      //复用功能
		GPIO_Initure.Pull = GPIO_PULLUP;                                          //上拉
		GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;                                //快速	
		GPIO_Initure.Alternate = GPIO_AF1_USART4;                                 //PC10 11复用为串口4的TXD RXD功能
		HAL_GPIO_Init(GPIOC, &GPIO_Initure);                                      //设置
		
		HAL_NVIC_SetPriority(USART3_4_IRQn,0,2);                                  //设置串口4中断优先级
		HAL_NVIC_EnableIRQ(USART3_4_IRQn);                                        //使能串口4的中断
		
		U4_CB.USART_TxDMA.Instance                 = DMA1_Channel6;               //DMA1通道6
		U4_CB.USART_TxDMA.Init.Request             = DMA_REQUEST_USART4_TX;       //串口4发送
		U4_CB.USART_TxDMA.Init.Direction           = DMA_MEMORY_TO_PERIPH;        //存储区到外设
		U4_CB.USART_TxDMA.Init.PeriphInc           = DMA_PINC_DISABLE;            //外设不递增
		U4_CB.USART_TxDMA.Init.MemInc              = DMA_MINC_ENABLE;             //存储区递增
		U4_CB.USART_TxDMA.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;         //按字节存取
		U4_CB.USART_TxDMA.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;         //按字节存取
		U4_CB.USART_TxDMA.Init.Mode                = DMA_NORMAL;                  //正常模式
		U4_CB.USART_TxDMA.Init.Priority            = DMA_PRIORITY_HIGH;           //高优先级
		HAL_DMA_Init(&U4_CB.USART_TxDMA);                                         //配置
		__HAL_LINKDMA(huart, hdmatx, U4_CB.USART_TxDMA);                          //和串口4 DMA发送连接		
		
		U4_CB.USART_RxDMA.Instance                 = DMA1_Channel7;               //DMA1通道7
		U4_CB.USART_RxDMA.Init.Request             = DMA_REQUEST_USART4_RX;       //串口4接收
		U4_CB.USART_RxDMA.Init.Direction           = DMA_PERIPH_TO_MEMORY;        //外设到存储区
		U4_CB.USART_RxDMA.Init.PeriphInc           = DMA_PINC_DISABLE;            //外设不递增
		U4_CB.USART_RxDMA.Init.MemInc              = DMA_MINC_ENABLE;             //存储区递增
		U4_CB.USART_RxDMA.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;         //按字节存取
		U4_CB.USART_RxDMA.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;         //按字节存取
		U4_CB.USART_RxDMA.Init.Mode                = DMA_NORMAL;                  //正常模式
		U4_CB.USART_RxDMA.Init.Priority            = DMA_PRIORITY_HIGH;           //高优先级
		HAL_DMA_Init(&U4_CB.USART_RxDMA);                                         //配置
		__HAL_LINKDMA(huart, hdmarx, U4_CB.USART_RxDMA);                          //和串口4 DMA接收连接
		
		HAL_NVIC_SetPriority(DMA1_Ch4_7_DMAMUX1_OVR_IRQn,2,0);                    //设置DMA1通道4-7中断优先级
		HAL_NVIC_EnableIRQ(DMA1_Ch4_7_DMAMUX1_OVR_IRQn);                          //使能DMA1通道4-7中断		
	}
}
/*-------------------------------------------------*/
/*函数名：串口1 printf函数                         */
/*参  数：fmt,...  格式化输出字符串和参数          */
/*返回值：无                                       */
/*-------------------------------------------------*/
void u1_printf(char* fmt,...) 
{  
	uint16_t i;    	
	va_list ap;
	va_start(ap,fmt);
	vsprintf((char *)U1_TxBuff,fmt,ap);
	va_end(ap);
	
	for(i = 0;i < strlen((const char*)U1_TxBuff);i ++){				         //利用for循环，一个字节，一个字节的发送
		while(__HAL_UART_GET_FLAG(&U1_CB.USART_Handler,UART_FLAG_TXE)!=SET); //等待串口发送寄存器空	
		U1_CB.USART_Handler.Instance->TDR = U1_TxBuff[i];                    //把需要发送的字节填充到寄存器，启动发送		
	}
	while(__HAL_UART_GET_FLAG(&U1_CB.USART_Handler,UART_FLAG_TC)!=SET);	     //等待串口发送完最后一个字节，才能退出函数		
}
/*-------------------------------------------------*/
/*函数名：串口2 printf函数                         */
/*参  数：fmt,...  格式化输出字符串和参数          */
/*返回值：无                                       */
/*-------------------------------------------------*/
void u2_printf(char* fmt,...) 
{  
	uint16_t i;    	
	va_list ap;
	va_start(ap,fmt);
	vsprintf((char *)U2_TxBuff,fmt,ap);
	va_end(ap);
	
	for(i = 0;i < strlen((const char*)U2_TxBuff);i ++){				         //利用for循环，一个字节，一个字节的发送
		while(__HAL_UART_GET_FLAG(&U2_CB.USART_Handler,UART_FLAG_TXE)!=SET); //等待串口发送寄存器空
		U2_CB.USART_Handler.Instance->TDR = U2_TxBuff[i];                    //把需要发送的字节填充到寄存器，启动发送
	}
	while(__HAL_UART_GET_FLAG(&U2_CB.USART_Handler,UART_FLAG_TC)!=SET);	     //等待串口发送完最后一个字节，才能退出函数		
}
/*-------------------------------------------------*/
/*函数名：串口4 printf函数                         */
/*参  数：fmt,...  格式化输出字符串和参数          */
/*返回值：无                                       */
/*-------------------------------------------------*/
void u4_printf(char* fmt,...) 
{  
	uint16_t i;    	
	va_list ap;
	va_start(ap,fmt);
	vsprintf((char *)U4_TxBuff,fmt,ap);
	va_end(ap);
	
	for(i = 0;i < strlen((const char*)U4_TxBuff);i ++){				         //利用for循环，一个字节，一个字节的发送
		while(__HAL_UART_GET_FLAG(&U4_CB.USART_Handler,UART_FLAG_TXE)!=SET); //等待串口发送寄存器空	
		U4_CB.USART_Handler.Instance->TDR = U4_TxBuff[i];                    //把需要发送的字节填充到寄存器，启动发送		
	}
	while(__HAL_UART_GET_FLAG(&U4_CB.USART_Handler,UART_FLAG_TC)!=SET);	     //等待串口发送完最后一个字节，才能退出函数			
}
/*-------------------------------------------------*/
/*函数名：串口1直接发送数据                        */
/*参  数：data：数据                               */
/*参  数：len：数据量                              */
/*返回值：无                                       */
/*-------------------------------------------------*/
void u1_TxData(uint8_t *data, uint16_t len)
{  
	uint16_t i;                                                               
	
	for(i=0;i<len;i++){	                                                     //利用for循环，一个字节，一个字节的发送	
		while(__HAL_UART_GET_FLAG(&U1_CB.USART_Handler,UART_FLAG_TXE)!=SET); //等待串口发送寄存器空
		U1_CB.USART_Handler.Instance->TDR = data[i];                         //把需要发送的字节填充到寄存器，启动发送					
	}
	while(__HAL_UART_GET_FLAG(&U1_CB.USART_Handler,UART_FLAG_TC)!=SET);      //等待串口发送完最后一个字节，才能退出函数			
}
/*-------------------------------------------------*/
/*函数名：串口2直接发送数据                        */
/*参  数：data：数据                               */
/*参  数：len：数据量                              */
/*返回值：无                                       */
/*-------------------------------------------------*/
void u2_TxData(uint8_t *data, uint16_t len)
{  
	uint16_t i;                                                               
	
	for(i=0;i<len;i++){	                                                     //利用for循环，一个字节，一个字节的发送	
		while(__HAL_UART_GET_FLAG(&U2_CB.USART_Handler,UART_FLAG_TXE)!=SET); //等待串口发送寄存器空
		U2_CB.USART_Handler.Instance->TDR = data[i];                         //把需要发送的字节填充到寄存器，启动发送			
	}
	while(__HAL_UART_GET_FLAG(&U2_CB.USART_Handler,UART_FLAG_TC)!=SET);      //等待串口发送完最后一个字节，才能退出函数		
}
/*-------------------------------------------------*/
/*函数名：串口3直接发送数据                        */
/*参  数：data：数据                               */
/*参  数：len：数据量                              */
/*返回值：无                                       */
/*-------------------------------------------------*/
void u3_TxData(uint8_t *data, uint16_t len)
{  
	uint16_t i;                                                               
	
	for(i=0;i<len;i++){	                                                     //利用for循环，一个字节，一个字节的发送
		while(__HAL_UART_GET_FLAG(&U3_CB.USART_Handler,UART_FLAG_TXE)!=SET); //等待串口发送寄存器空
		U3_CB.USART_Handler.Instance->TDR = data[i];                         //把需要发送的字节填充到寄存器，启动发送		
	}
	while(__HAL_UART_GET_FLAG(&U3_CB.USART_Handler,UART_FLAG_TC)!=SET);      //等待串口发送完最后一个字节，才能退出函数				
}
/*-------------------------------------------------*/
/*函数名：串口4直接发送数据                        */
/*参  数：data：数据                               */
/*参  数：len：数据量                              */
/*返回值：无                                       */
/*-------------------------------------------------*/
void u4_TxData(uint8_t *data, uint16_t len)
{  
	uint16_t i;                                                               
	
	for(i=0;i<len;i++){	                                                     //利用for循环，一个字节，一个字节的发送
		while(__HAL_UART_GET_FLAG(&U4_CB.USART_Handler,UART_FLAG_TXE)!=SET); //等待串口发送寄存器空
		U4_CB.USART_Handler.Instance->TDR = data[i];                         //把需要发送的字节填充到寄存器，启动发送				
	}
	while(__HAL_UART_GET_FLAG(&U4_CB.USART_Handler,UART_FLAG_TC)!=SET);      //等待串口发送完最后一个字节，才能退出函数			
}
/*----------------------------------------------------------*/
/*函数名：向串口2发送缓冲区添加数据                         */
/*参  数：data：数据                                        */
/*参  数：datalen：数据长度                                 */
/*返回值：无                                                */
/*----------------------------------------------------------*/
void u2_TxDataBuf(uint8_t *data, uint32_t datalen)
{
	if(U2_TXBUFF_SIZE - U2_CB.TxCounter < datalen){                          //剩余空闲长度不够存放本次的数据，进入if,发送缓冲区回卷
		U2_CB.TxIN->s = U2_TxBuff;                                           //回卷了，IN指向的s标记本次存放数据的起始位置为发送缓冲区的起始位置
		U2_CB.TxCounter = 0;                                                 //回卷了，累计发送缓冲区已经存放的数据量清零				
	}
	memcpy(U2_CB.TxIN->s,data,datalen);	                                     //从IN指向的s位置拷贝数据       
    U2_CB.TxCounter += datalen;	                                             //计算累计发送缓冲区已经存放的数据量
	U2_CB.TxIN->e = &U2_TxBuff[U2_CB.TxCounter-1];                           //IN指向的e标记本次存放数据的结束位置		
				
	U2_CB.TxIN++;                                                            //发送缓冲区 IN指针后移 一个成员
	if(U2_CB.TxIN == &U2_CB.se_TxBuff[TX_NUM-1])                             //如果后移到se指针对结构体数组的最后一个成员，进入if，需要回卷
		U2_CB.TxIN = &U2_CB.se_TxBuff[0];                                    //回卷了 发送缓冲区 IN指针 指向se指针对结构体数组的0号成员
	U2_CB.TxIN->s = &U2_TxBuff[U2_CB.TxCounter];                             //IN指向的s标记下一次存放数据的起始位置			
}
/*----------------------------------------------------------*/
/*函数名：向串口3发送缓冲区添加数据                         */
/*参  数：data：数据                                        */
/*参  数：datalen：数据长度                                 */
/*返回值：无                                                */
/*----------------------------------------------------------*/
void u3_TxDataBuf(uint8_t *data, uint32_t datalen)
{
	if(U3_TXBUFF_SIZE - U3_CB.TxCounter < datalen){                          //剩余空闲长度不够存放本次的数据，进入if,发送缓冲区回卷
		U3_CB.TxIN->s = U3_TxBuff;                                           //回卷了，IN指向的s标记本次存放数据的起始位置为发送缓冲区的起始位置
		U3_CB.TxCounter = 0;                                                 //回卷了，累计发送缓冲区已经存放的数据量清零				
	}
	memcpy(U3_CB.TxIN->s,data,datalen);	                                     //从IN指向的s位置拷贝数据       
    U3_CB.TxCounter += datalen;	                                             //计算累计发送缓冲区已经存放的数据量
	U3_CB.TxIN->e = &U3_TxBuff[U3_CB.TxCounter-1];                           //IN指向的e标记本次存放数据的结束位置		
				
	U3_CB.TxIN++;                                                            //发送缓冲区 IN指针后移 一个成员
	if(U3_CB.TxIN == &U3_CB.se_TxBuff[TX_NUM-1])                             //如果后移到se指针对结构体数组的最后一个成员，进入if，需要回卷
		U3_CB.TxIN = &U3_CB.se_TxBuff[0];                                    //回卷了 发送缓冲区 IN指针 指向se指针对结构体数组的0号成员
	U3_CB.TxIN->s = &U3_TxBuff[U3_CB.TxCounter];                             //IN指向的s标记下一次存放数据的起始位置			
}
/*----------------------------------------------------------*/
/*函数名：向串口4发送缓冲区添加数据                         */
/*参  数：data：数据                                        */
/*参  数：datalen：数据长度                                 */
/*返回值：无                                                */
/*----------------------------------------------------------*/
void u4_TxDataBuf(uint8_t *data, uint32_t datalen)
{
	if(U4_TXBUFF_SIZE - U4_CB.TxCounter < datalen){                          //剩余空闲长度不够存放本次的数据，进入if,发送缓冲区回卷
		U4_CB.TxIN->s = U4_TxBuff;                                           //回卷了，IN指向的s标记本次存放数据的起始位置为发送缓冲区的起始位置
		U4_CB.TxCounter = 0;                                                 //回卷了，累计发送缓冲区已经存放的数据量清零				
	}
	memcpy(U4_CB.TxIN->s,data,datalen);	                                     //从IN指向的s位置拷贝数据       
    U4_CB.TxCounter += datalen;	                                             //计算累计发送缓冲区已经存放的数据量
	U4_CB.TxIN->e = &U4_TxBuff[U4_CB.TxCounter-1];                           //IN指向的e标记本次存放数据的结束位置		
				
	U4_CB.TxIN++;                                                            //发送缓冲区 IN指针后移 一个成员
	if(U4_CB.TxIN == &U4_CB.se_TxBuff[TX_NUM-1])                             //如果后移到se指针对结构体数组的最后一个成员，进入if，需要回卷
		U4_CB.TxIN = &U4_CB.se_TxBuff[0];                                    //回卷了 发送缓冲区 IN指针 指向se指针对结构体数组的0号成员
	U4_CB.TxIN->s = &U4_TxBuff[U4_CB.TxCounter];                             //IN指向的s标记下一次存放数据的起始位置			
}
/*----------------------------------------------------------*/
/*函数名：空闲中断产生 接收完成回调函数                     */
/*参  数：huart：串口总控结构体                             */
/*返回值：无                                                */
/*----------------------------------------------------------*/
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{	
	if((huart->Instance==USART1)&&(huart->RxState == HAL_UART_STATE_READY)){               //如果是串口1 且 当前接收状态是READY 进入if，表示接收完成 
		U1_CB.RxCounter += (Size+1);                                                       //累计接收缓冲区已经存放的数据量，我们多加一个字节，放入0，作为我们自定的结束符
		U1_CB.RxIN->e = &U1_RxBuff[U1_CB.RxCounter - 1];   		                           //IN指向的e标记结尾位置
		*U1_CB.RxIN->e = 0;                                                                //结尾位置放入0，作为我们自定的结束符 
		U1_CB.RxIN++;                                                                      //IN指针后移一个成员
		if(U1_CB.RxIN==&U1_CB.se_RxBuff[RX_NUM-1])                                         //如果后移到se指针对结构体数组的最后一个成员，进入if，需要回卷
			U1_CB.RxIN = &U1_CB.se_RxBuff[0];                                              //回卷了 接收缓冲区 IN指针 指向se指针对结构体数组的0号成员   	
		if(U1_RXBUFF_SIZE - U1_CB.RxCounter >= U1_RXMAX_SIZE){                             //接收缓冲区总长度 - 累计接收缓冲区已经存放的数据量 = 剩余空闲长度，只有大于等于U1_RXMAX_SIZE时，才能进入if
			U1_CB.RxIN->s = &U1_RxBuff[U1_CB.RxCounter];                                   //IN指向的s标记下次接收的起始位置	
		}else{                                                                             //反之，剩余空闲长度不够U1_RXMAX_SIZE时，进入else，接收缓冲区回卷
			U1_CB.RxCounter = 0;                                                           //回卷了，累计接收缓冲区已经存放的数据量清零
			U1_CB.RxIN->s = U1_RxBuff;                                                     //回卷了，IN指向的s标记本次存放数据的起始位置为接收缓冲区的起始位置			
		}
		HAL_UARTEx_ReceiveToIdle_DMA(&U1_CB.USART_Handler,U1_CB.RxIN->s,U1_RXMAX_SIZE);    //开启DMA，不管上方进的if还是else，都是通过IN指向的s标记本次存放数据的起始位置，所以把IN指向的s标记的位置告诉DMA
		
	}else if((huart->Instance==USART2)&&(huart->RxState == HAL_UART_STATE_READY)){         //如果是串口2 且 当前接收状态是READY 进入if，表示接收完成 
		U2_CB.RxCounter += (Size+1);                                                       //累计接收缓冲区已经存放的数据量，我们多加一个字节，放入0，作为我们自定的结束符
		U2_CB.RxIN->e = &U2_RxBuff[U2_CB.RxCounter - 1];   		                           //IN指向的e标记结尾位置	
		*U2_CB.RxIN->e = 0;                                                                //结尾位置放入0，作为我们自定的结束符 
		U2_CB.RxIN++;                                                                      //IN指针后移一个成员
		if(U2_CB.RxIN == &U2_CB.se_RxBuff[RX_NUM-1])                                       //如果后移到se指针对结构体数组的最后一个成员，进入if，需要回卷
			U2_CB.RxIN = &U2_CB.se_RxBuff[0];                                              //回卷了 接收缓冲区 IN指针 指向se指针对结构体数组的0号成员   		
		if(U2_RXBUFF_SIZE - U2_CB.RxCounter >= U2_RXMAX_SIZE){                             //接收缓冲区总长度 - 累计接收缓冲区已经存放的数据量 = 剩余空闲长度，只有大于等于U2_RXMAX_SIZE时，才能进入if
			U2_CB.RxIN->s = &U2_RxBuff[U2_CB.RxCounter];                                   //IN指向的s标记下次接收的起始位置		
		}else{                                                                             //反之，剩余空闲长度不够U2_RXMAX_SIZE时，进入else，接收缓冲区回卷
			U2_CB.RxCounter = 0;                                                           //回卷了，累计接收缓冲区已经存放的数据量清零
			U2_CB.RxIN->s = U2_RxBuff;                                                     //回卷了，IN指向的s标记本次存放数据的起始位置为接收缓冲区的起始位置	
		}
		HAL_UARTEx_ReceiveToIdle_DMA(&U2_CB.USART_Handler,U2_CB.RxIN->s,U2_RXMAX_SIZE);    //开启DMA，不管上方进的if还是else，都是通过IN指向的s标记本次存放数据的起始位置，所以把IN指向的s标记的位置告诉DMA
	}else if((huart->Instance==USART3)&&(huart->RxState == HAL_UART_STATE_READY)){         //如果是串口3 且 当前接收状态是READY 进入if，表示接收完成 
		U3_CB.RxCounter += (Size+1);                                                       //累计接收缓冲区已经存放的数据量，我们多加一个字节，放入0，作为我们自定的结束符
		U3_CB.RxIN->e = &U3_RxBuff[U3_CB.RxCounter - 1];   		                           //IN指向的e标记结尾位置	
		*U3_CB.RxIN->e = 0;                                                                //结尾位置放入0，作为我们自定的结束符 
		U3_CB.RxIN++;                                                                      //IN指针后移一个成员
		if(U3_CB.RxIN == &U3_CB.se_RxBuff[RX_NUM-1])                                       //如果后移到se指针对结构体数组的最后一个成员，进入if，需要回卷
			U3_CB.RxIN = &U3_CB.se_RxBuff[0];                                              //回卷了 接收缓冲区 IN指针 指向se指针对结构体数组的0号成员   		
		if(U3_RXBUFF_SIZE - U3_CB.RxCounter >= U3_RXMAX_SIZE){                             //接收缓冲区总长度 - 累计接收缓冲区已经存放的数据量 = 剩余空闲长度，只有大于等于U3_RXMAX_SIZE时，才能进入if
			U3_CB.RxIN->s = &U3_RxBuff[U3_CB.RxCounter];                                   //IN指向的s标记下次接收的起始位置		
		}else{                                                                             //反之，剩余空闲长度不够U3_RXMAX_SIZE时，进入else，接收缓冲区回卷
			U3_CB.RxCounter = 0;                                                           //回卷了，累计接收缓冲区已经存放的数据量清零
			U3_CB.RxIN->s = U3_RxBuff;                                                     //回卷了，IN指向的s标记本次存放数据的起始位置为接收缓冲区的起始位置	
		}
		U3_CB.TxCpltflag &=~ TX_STA_ACK;                                                   //接收到节点回复的数据，清除相应状态标记
		HAL_TIM_Base_Stop_IT(&tim3);                                                       //接收到节点回复的数据，关闭定时器3
		__HAL_TIM_SET_COUNTER(&tim3,0);                                                    //接收到节点回复的数据，定时器计数器清零								
		HAL_UARTEx_ReceiveToIdle_DMA(&U3_CB.USART_Handler,U3_CB.RxIN->s,U3_RXMAX_SIZE);    //开启DMA，不管上方进的if还是else，都是通过IN指向的s标记本次存放数据的起始位置，所以把IN指向的s标记的位置告诉DMA
	}else if((huart->Instance==USART4)&&(huart->RxState == HAL_UART_STATE_READY)){         //如果是串口4 且 当前接收状态是READY 进入if，表示接收完成 
		U4_CB.RxCounter += (Size+1);                                                       //累计接收缓冲区已经存放的数据量，我们多加一个字节，放入0，作为我们自定的结束符
		U4_CB.RxIN->e = &U4_RxBuff[U4_CB.RxCounter - 1];   		                           //IN指向的e标记结尾位置	
		*U4_CB.RxIN->e = 0;                                                                //结尾位置放入0，作为我们自定的结束符 
		U4_CB.RxIN++;                                                                      //IN指针后移一个成员
		if(U4_CB.RxIN == &U4_CB.se_RxBuff[RX_NUM-1])                                       //如果后移到se指针对结构体数组的最后一个成员，进入if，需要回卷
			U4_CB.RxIN = &U4_CB.se_RxBuff[0];                                              //回卷了 接收缓冲区 IN指针 指向se指针对结构体数组的0号成员   		
		if(U4_RXBUFF_SIZE - U4_CB.RxCounter >= U4_RXMAX_SIZE){                             //接收缓冲区总长度 - 累计接收缓冲区已经存放的数据量 = 剩余空闲长度，只有大于等于U4_RXMAX_SIZE时，才能进入if
			U4_CB.RxIN->s = &U4_RxBuff[U4_CB.RxCounter];                                   //IN指向的s标记下次接收的起始位置		
		}else{                                                                             //反之，剩余空闲长度不够U4_RXMAX_SIZE时，进入else，接收缓冲区回卷
			U4_CB.RxCounter = 0;                                                           //回卷了，累计接收缓冲区已经存放的数据量清零
			U4_CB.RxIN->s = U4_RxBuff;                                                     //回卷了，IN指向的s标记本次存放数据的起始位置为接收缓冲区的起始位置	
		}
		HAL_UARTEx_ReceiveToIdle_DMA(&U4_CB.USART_Handler,U4_CB.RxIN->s,U4_RXMAX_SIZE);    //开启DMA，不管上方进的if还是else，都是通过IN指向的s标记本次存放数据的起始位置，所以把IN指向的s标记的位置告诉DMA
	}	
}
/*----------------------------------------------------------*/
/*函数名：串口数据发送完成回调函数                          */
/*参  数：huart：串口总控结构体                             */
/*返回值：无                                                */
/*----------------------------------------------------------*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART2){              //如果是串口2，进入if分支
		U2_CB.TxCpltflag &=~ TX_STA_DMA;      //DMA发送完成，清除相应状态标记
	}else if(huart->Instance==USART3){        //如果是串口3，进入if分支
		U3_CB.TxCpltflag &=~ TX_STA_DMA;      //DMA发送完成，清除相应状态标记
	}else if(huart->Instance==USART4){        //如果是串口4，进入if分支
		U4_CB.TxCpltflag &=~ TX_STA_DMA;      //DMA发送完成，清除相应状态标记
	}
}
/*----------------------------------------------------------*/
/*函数名：串口错误回调函数                                  */
/*参  数：huart：串口总控结构体                             */
/*返回值：无                                                */
/*----------------------------------------------------------*/
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART2){                                                           //如果是串口2，进入if分支
		HAL_UARTEx_ReceiveToIdle_DMA(&U2_CB.USART_Handler,U2_CB.RxIN->s,U2_RXMAX_SIZE);    //重新开启DMA接收 存放在U2_CB.RxIN->s指明的位置 传输量为U2_RXMAX_SIZE
	}else if(huart->Instance==USART4){                                                     //如果是串口4，进入if分支
		HAL_UARTEx_ReceiveToIdle_DMA(&U4_CB.USART_Handler,U4_CB.RxIN->s,U4_RXMAX_SIZE);    //重新开启DMA接收 存放在U4_CB.RxIN->s指明的位置 传输量为U4_RXMAX_SIZE
	}
}
