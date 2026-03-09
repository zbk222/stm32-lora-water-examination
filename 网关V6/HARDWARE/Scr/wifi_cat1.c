
#include "stm32g0xx_hal.h" 
#include "main.h"
#include "wifi_cat1.h"	          
#include "clock.h"	         
#include "usart.h"	                                      
#include "lora.h"	          
#include "crc.h"              
#include "mqtt.h"  
#include "string.h" 
#include "timer.h" 
#include "key_capture.h" 
#include "math.h" 
#include "stdio.h"            
#include "stdarg.h"		   
#include "string.h" 

// 全局变量，确保阈值数据持久保存
volatile float Ph_Threshold[SUN_NUMBER+1]={0}; // 用于保存各子设备PH阈值
volatile float TU_Threshold[SUN_NUMBER+1]={0}; // 用于保存各子设备浊度阈值

Pack_CB pack;              //WiFi模块 or 4G Cat1模块，整包数据控制结构体

/*-------------------------------------------------*/
/*函数名：初始化WiFi 4G Cat1模块 控制IO            */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void WiFi_Cat1_InitGPIO(void)
{
	GPIO_InitTypeDef GPIO_Initure;
    	
	__HAL_RCC_GPIOF_CLK_ENABLE();                //使能端口F时钟	
	GPIO_Initure.Pin=GPIO_PIN_0;                 //设置PF0
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;       //推免输出
    GPIO_Initure.Speed = GPIO_SPEED_FREQ_LOW;    //低速模式	
	HAL_GPIO_Init(GPIOF,&GPIO_Initure);          //设置PF0，控制WiFi模块复位
	
	__HAL_RCC_GPIOA_CLK_ENABLE();                //使能端口A时钟	
	GPIO_Initure.Pin=GPIO_PIN_15;                //设置PA15
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;       //推免输出
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);          //设置PA15，控制4G Cat1模块开机或者关机
	
	GPIO_Initure.Pin=GPIO_PIN_0 | GPIO_PIN_1;    //设置PA0 1
    GPIO_Initure.Mode=GPIO_MODE_INPUT;           //输入模式
	GPIO_Initure.Pull = GPIO_PULLUP;             //使能上拉
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);          //设置PA0，控制读取4G Cat1模块开关机状态  设置PA1，控制读取4G Cat1模块网络注册状态
}
/*-------------------------------------------------*/
/*函数名：复位WiFi模块                             */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void WiFi_Reset(void)
{
	WIFI_RESET(0);                               //复位IO拉低电平
	HAL_Delay(500);                              //延时
	WIFI_RESET(1);                               //复位IO拉高电平
	HAL_Delay(500);                              //延时
}
/*-------------------------------------------------*/
/*函数名：复位4G Cat1模块                          */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void Cat1_Reset(void)
{
	if(CAT1_POWER_STA==1){   							       		      //如果目前处于关机状态，进入该分支		
		u1_printf("\r\n目前4G Cat1模块处于关机状态，准备开机\r\n");       //串口输出信息
		CAT1_POWER(1);                                         		      //拉高
		HAL_Delay(1500);                                       		      //延时
		CAT1_POWER(0);										   		      //开机成功了，拉低
	}else{                                                     		      //反之表示目前处于开机状态，进入该分支
		u1_printf("\r\n目前4G Cat1模块处于开机状态，准备重启\r\n");       //串口输出信息		
		CAT1_POWER(1);                                         		      //先拉高
		HAL_Delay(1500);                                       		      //延时
		CAT1_POWER(0);										   		      //关机成功了，拉低		
		u1_printf("\r\n关机成功，准备开机\r\n");                          //串口输出信息
		HAL_Delay(6000);                                        		  //延时
		CAT1_POWER(1);                                         		      //拉高
		HAL_Delay(1500);                                       		      //延时
		CAT1_POWER(0);										   		      //开机成功了，拉低
	}
	u1_printf("开机成功，请等待4G Cat1模块注册上网络... ...\r\n");        //串口输出信息
		
	if(CAT1_TYPE == 2){                                                   //如果是Air724模块，进入分支
		u1_printf("\r\n使用Air724模块\r\n");            		          //串口输出信息			
		while(1){                                                  		  //等待注册上网络
		HAL_Delay(100);                                        		      //延时
		u1_printf(".");                                        		      //串口输出信息	
		if(CAT1_NET_STA == 0)                                  		      //CAT1_NET_STA 引脚变成低电平 表示注册上网络
			break;       						               		      //主动跳出while循环
		}
		u1_printf("\r\n4G Cat1模块已经注册上网络\r\n");            		  //串口输出信息
		HAL_Delay(1000);                                                  //间隔
		u2_printf("AT+CIPMUX=0\r\n");                             		  //发送指令关闭多路连接			
	}	
}
/*-------------------------------------------------*/
/*函数名：处理WiFi模块的数据                       */
/*参  数：data ：数据                              */
/*参  数：datalen ：数据长度                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void WiFi_ProcessData(uint8_t *data, uint16_t data_len)
{	

	/*----------------------------------------------*/
	/*                  处理分包                    */
	/*----------------------------------------------*/
	if((data[data_len-1] != '\n')&&(data[data_len-1] != '>')){                   //命令都以\n结尾，如果最后1个数据不是\n，说明有分包，本次只是一部分，还不完整 特殊情况就是进入透传时 结尾是>
		memcpy(&pack.Data_Packbuff[0][pack.Data_Packlen[0]],data,data_len);      //拷贝本次接收的数据，注意本次数据还不完整，只有一部分
		pack.Data_Packlen[0] += data_len;                                        //累计本次接收的数据量
		pack.Data_Packsta = 1;                                                   //pack.Data_Packsta 置1，表示有分包了
		return;                                                                  //直接返回，等下包数据
	}else{                                                                       //命令都以\n结尾，如果最后1个数据是\n，有2种情况，分别对应下方if和else分支
		if(pack.Data_Packsta == 1){                                              //pack.Data_Packsta 等于1，说明有分包情况存在，本次是最后一个分包，加上前面已经接收的数据后，整个数据包就完整了
			pack.Data_Packsta = 0;                                               //清除 pack.Data_Packsta，因为本次接收后，数据包就完整了
			memcpy(&pack.Data_Packbuff[0][pack.Data_Packlen[0]],data,data_len);  //拷贝本次数据
			pack.Data_Packlen[0] += data_len;                                    //累计本次接收的数据量，到此完整的数据包长度，就统计到 pack.Data_Packlen 中了
		}else{                                                                   //pack.Data_Packsta 不等于1，说明没有分包，本次接收的数据就是一个完整的数据包
			memcpy(pack.Data_Packbuff[0],data,data_len);	                     //拷贝本次数据
			pack.Data_Packlen[0] = data_len;                                     //记录本次数据包长度
		}
	}	
	/*----------------------------------------------*/
	/*                 连接上服务器                 */
	/*----------------------------------------------*/
	if(strstr(pack.Data_Packbuff[0],"ERROR")){                                  //接收到数据ERROR
		u1_printf("WiFi模块出错，重启\r\n");                                    //串口输出信息
		NVIC_SystemReset();                                                     //重启	
	}
	/*----------------------------------------------*/
	/*                设置STA模式成功               */
	/*----------------------------------------------*/
	if(strstr(pack.Data_Packbuff[0],"AT+CWMODE=1\r\r\n")){                      //接收到数据 AT+CWMODE=1
		u1_printf("WiFi模块设置STA模式成功\r\n");                               //串口输出信息
		u4_printf("AT+CWSTARTSMART\r\n");                                       //发送wifi配网命令
	}
	/*----------------------------------------------*/
	/*                 使用微信配网                 */
	/*----------------------------------------------*/
	if(strstr(pack.Data_Packbuff[0],"AT+CWSTARTSMART\r\r\n")){                   //接收到数据AT+CWSTARTSMART
		u1_printf("请使用微信Airkiss对WiFi模块配网\r\n");                        //串口输出信息
		u4_printf("AT+CWAUTOCONN=1\r\n");                                        //发送自动连接路由器指令
	}
	/*----------------------------------------------*/
	/*               WiFi连接上路由器               */
	/*----------------------------------------------*/
	if(strstr(pack.Data_Packbuff[0],"WIFI GOT IP\r\n")){                         //接收到数据WIFI GOT IP
		u1_printf("wifi模块连接上路由器\r\n");                                   //串口输出信息		
		u4_printf("AT+CIPMUX=0\r\n");                                            //发送关闭多路连接指令		
	}
	/*----------------------------------------------*/
	/*                   注册上网络                 */
	/*----------------------------------------------*/
	if(strstr(pack.Data_Packbuff[0],"AT+CIPMUX=0\r\r\n")){                       //接收到数据AT+CIPMUX=0
		u1_printf("WiFi模块关闭多路连接成功\r\n");                               //串口输出信息
		u4_printf("AT+CIPMODE=1\r\n");                                           //发送透传模式指令		
	}
	/*----------------------------------------------*/
	/*                 透传模式成功                 */
	/*----------------------------------------------*/
	if(strstr(pack.Data_Packbuff[0],"AT+CIPMODE=1\r\r\n")){                       //接收到数据AT+CIPMODE=1
		u1_printf("WiFi模块透传模式成功，等待逻连接服务器\r\n");                  //串口输出信息
		u4_printf("AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",MQTT_SERVER,MQTT_PORT);     //发送连接MQTT服务器指令			
	}
	/*----------------------------------------------*/
	/*                 连接上服务器                 */
	/*----------------------------------------------*/
	if(strstr(pack.Data_Packbuff[0],"CONNECT\r\n\r\nOK\r\n")){                    //接收到数据CONNECT\r\n
		u1_printf("WiFi模块连接上服务器\r\n");                                    //串口输出信息
		if(CAT1_POWER_STA==0){                                                    //如果4G Cat1已开机了，进入if
			SysCB.SysEventFlag = 0;                                               //清除SysCB.SysEventFlag
			u1_printf("切换WiFi模块连接服务器\r\n");                              //串口输出信息
			u1_printf("将4G Cat1模块关机\r\n");                                   //串口输出信息		
			CAT1_POWER(1);                                         		          //先拉高
			HAL_Delay(1500);                                       		          //延时
			CAT1_POWER(0);										   		          //拉低关机	
			while(CAT1_POWER_STA==0);                                             //等到CAT1_POWER_STA变成低电平
			u1_printf("4G Cat1模块关机成功\r\n");                                 //串口输出信息
		}
		u4_printf("AT+CIPSEND\r\n");                                              //发送进入透传指令	
	}
	/*----------------------------------------------*/
	/*                 进入透传成功                 */
	/*----------------------------------------------*/
	if(strstr(pack.Data_Packbuff[0],"\r\nOK\r\n\r\n>")){                          //接收到数据\r\nOK\r\n\r\n>
		u1_printf("WiFi模块进入透传成功\r\n");                                    //串口输出信息
		SysCB.PingTimer = HAL_GetTick();                                          //发送PING报文计时时间记录当前时间							
		lora.timer = HAL_GetTick();                                               //LoRa发送数据计时时间记录当前时间值
		SysCB.SysEventFlag |= CONNECT_MQTT;        	                              //WiFi模块连接上MQTT服务器事件
		SysCB.SysEventFlag |= CONNECT_WIFI;        	                              //连接上服务器事件发生		
		HAL_TIM_Base_Stop_IT(&tim6);                                              //关闭定时器6		
		U4_CB.TxIN  = &U4_CB.se_TxBuff[0];           	                          //发送缓冲区 IN指针  指向se指针对结构体数组0号成员   
		U4_CB.TxOUT = &U4_CB.se_TxBuff[0];              	                      //发送缓冲区 OUT指针 指向se指针对结构体数组0号成员 
		U4_CB.TxIN->s = U4_TxBuff;	               	                              //发送缓冲区 IN指针  当前指向的se指针对结构体数组成员中的s指向串口4发送缓冲区的起始位置，为向发送缓冲区存放数据指明位置
		U4_CB.TxCounter = 0;                                      	              //累计 发送冲区 已经存放的数据量 为0
		MQTT_Connect(60);                                                         //构建MQTT协议CONNECT报文
		u4_TxDataBuf(mqtt.buff,mqtt.lenth);                                       //加入WiFi模块串口4发送缓冲区
	}
	/*----------------------------------------------*/
	/*              清除pack结构体                  */
	/*----------------------------------------------*/
	memset(&pack,0,PACK_CB_LEN);                                                   //清除pack结构体	 
}
/*-------------------------------------------------*/
/*函数名：处理4G Cat1模块的数据                    */
/*参  数：data ：数据                              */
/*参  数：datalen ：数据长度                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void Cat1_ProcessData(uint8_t *data, uint16_t data_len)
{	
	/*----------------------------------------------*/
	/*                  处理分包                    */
	/*----------------------------------------------*/
	if(data[data_len-1] != '\n'){                							     //命令都以\n结尾，如果最后1个数据不是\n，说明有分包，本次只是一部分，还不完整
		memcpy(&pack.Data_Packbuff[0][pack.Data_Packlen[0]],data,data_len);      //拷贝本次接收的数据，注意本次数据还不完整，只有一部分
		pack.Data_Packlen[0] += data_len;                                        //累计本次接收的数据量
		pack.Data_Packsta = 1;                                                   //pack.Data_Packsta 置1，表示有分包了
		return;                                                                  //直接返回，等下包数据
	}else{                                                                       //命令都以\n结尾，如果最后1个数据是\n，有2种情况，分别对应下方if和else分支
		if(pack.Data_Packsta == 1){                                              //pack.Data_Packsta 等于1，说明有分包情况存在，本次是最后一个分包，加上前面已经接收的数据后，整个数据包就完整了
			pack.Data_Packsta = 0;                                               //清除 pack.Data_Packsta，因为本次接收后，数据包就完整了
			memcpy(&pack.Data_Packbuff[0][pack.Data_Packlen[0]],data,data_len);  //拷贝本次数据
			pack.Data_Packlen[0] += data_len;                                    //累计本次接收的数据量，到此完整的数据包长度，就统计到 pack.Data_Packlen 中了
		}else{                                                                   //pack.Data_Packsta 不等于1，说明没有分包，本次接收的数据就是一个完整的数据包
			memcpy(pack.Data_Packbuff[0],data,data_len);	                     //拷贝本次数据
			pack.Data_Packlen[0] = data_len;                                     //记录本次数据包长度
		}
	}
	/*----------------------------------------------*/
	/*                  注册上网络                  */
	/*----------------------------------------------*/
	if(strstr(pack.Data_Packbuff[0],"+CGREG: 1\r\n")){                           //接收到数据+CGREG: 1
		u1_printf("\r\n4G Cat1模块已经注册上网络\r\n");            		         //串口输出信息
		HAL_Delay(1000);                                                         //间隔
		if(CAT1_TYPE == 1){                                                      //如果是Air780模块，进入分支
			u1_printf("\r\n使用Air780模块\r\n");            		             //串口输出信息
			u2_printf("AT+CIPMUX=0\r\n");                             		     //发送指令关闭多路连接	
		}else if(CAT1_TYPE == 3){                                                //如果是EC800M模块，进入分支
			u1_printf("\r\n使用EC800M模块\r\n");            		             //串口输出信息			
			u2_printf("AT+QIOPEN=1,0,\"TCP\",\"%s\",%d,0,2\r\n",MQTT_SERVER,MQTT_PORT);   //发送连接OTA服务器指令	
		}			
	}		
	/*----------------------------------------------*/
	/*                关闭多路连接成功              */
	/*----------------------------------------------*/
	if(strstr(pack.Data_Packbuff[0],"AT+CIPMUX=0\r\n")){                            //接收到数据AT+CIPMUX=0
		u1_printf("4G Cat1模块关闭多路连接成功\r\n");                               //串口输出信息
		u2_printf("AT+CIPMODE=1\r\n");                                              //发送透传模式指令		
	}
	/*----------------------------------------------*/
	/*                 透传模式成功                 */
	/*----------------------------------------------*/
	if(strstr(pack.Data_Packbuff[0],"AT+CIPMODE=1\r\n")){                            //接收到数据AT+CIPMODE=1
		u1_printf("4G Cat1模块透传模式成功\r\n");                                    //串口输出信息
		u2_printf("AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",MQTT_SERVER,MQTT_PORT);        //发送连接MQTT服务器指令			
	}
	/*----------------------------------------------*/
	/*               连接服务器失败                 */
	/*----------------------------------------------*/
	if(strstr(pack.Data_Packbuff[0],"CONNECT FAIL\r\n")){                           //接收到数据CONNECT FAIL
		u1_printf("4G Cat1模块连接服务器失败，准备重启\r\n");                       //串口输出信息
		NVIC_SystemReset();                                                         //重启 
	}
	/*----------------------------------------------*/
	/*                 连接上服务器                 */
	/*----------------------------------------------*/
	if(strstr(pack.Data_Packbuff[0],"CONNECT\r\n")){                                //接收到数据CONNECT
		u1_printf("4G Cat1模块连接上服务器\r\n");                                   //串口输出信息
		SysCB.PingTimer = HAL_GetTick();                                            //发送PING报文计时时间记录当前时间							
		lora.timer = HAL_GetTick();                                                 //LoRa发送数据计时时间记录当前时间值
		SysCB.SysEventFlag |= CONNECT_MQTT;        	                                //连接上MQTT服务器事件发生
		SysCB.SysEventFlag |= CONNECT_CAT1;        	                                //4G Cat1模块连接上服务器事件	
		U2_CB.TxIN  = &U2_CB.se_TxBuff[0];           	                            //发送缓冲区 IN指针  指向se指针对结构体数组0号成员   
		U2_CB.TxOUT = &U2_CB.se_TxBuff[0];              	                        //发送缓冲区 OUT指针 指向se指针对结构体数组0号成员 
		U2_CB.TxIN->s = U2_TxBuff;	               	                                //发送缓冲区 IN指针  当前指向的se指针对结构体数组成员中的s指向串口2发送缓冲区的起始位置，为向发送缓冲区存放数据指明位置
		U2_CB.TxCounter = 0;                                      	                //累计 发送冲区 已经存放的数据量 为0
		MQTT_Connect(60);                                                           //构建MQTT协议CONNECT报文
		u2_TxDataBuf(mqtt.buff,mqtt.lenth);                                         //加入WiFi模块串口4发送缓冲区	
	}
	/*----------------------------------------------*/
	/*              清除pack结构体                  */
	/*----------------------------------------------*/
	memset(&pack,0,PACK_CB_LEN);                                                    //清除pack结构体	
}
/*-------------------------------------------------*/
/*函数名：处理服务器的数据                         */
/*参  数：data ：数据                              */
/*参  数：datalen ：数据长度                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void Server_ProcessData(uint8_t *data, uint16_t data_len)
{
    int8_t res,i,j,k;
    unsigned char qs;
    unsigned int messageid;
    int id,code;
    uint8_t tempbuff[20];
    uint32_t temp_crc;
    
    // 阈值变量已由局部静态改为全局变量，此处删除定义

    /*----------------------------------------------*/
    /*      处理可能的接收的报文分包 or 多包        */
    /*----------------------------------------------*/
    pack.Data_totle = 0;                                                                                                                                                            //清空pack.Data_totle
    for(k=0;k<PACK_NUM;k++){    
        if(((((data[pack.Data_totle]&0xf0)>>4)>=1)&&(((data[pack.Data_totle]&0xf0)>>4)<=14))||((pack.Data_Packsta == 1))){                                                          //14个报文 或者 有分包多包的情况，进入if
            if(pack.Data_Packsta == 0){	                                                                                                                                            //刚接收的数据，还未判断分包多包
                for(i=1;i<5;i++){                                                                                                                                                   //利用for循环计算剩余长度几个字节
                    if((data[pack.Data_totle+i]&0x80) == 0x00)                                                                                                                      //轮询直到BIT7不是1时，进入if
                        break;                                                                                                                                                      //跳出for循环，i的值就是剩余长度占用的字节数
                }
                pack.Data_lenth[k] = 0;                                                                                                                                             //清空pack.Data_lenth
                for(j=1;j<=i;j++){                                                                                                                                                  //按剩余长度占用的字节数次数，开始轮询计算剩余长度的值
                    pack.Data_lenth[k] += (data[pack.Data_totle+j]&0x7F)*pow(128,j-1);                                                                                              //128进制开始计算
                }
                pack.Data_lenth[k] += (i+1);                                                                                                                                        //最终还要加上剩余长度占用的字节数和第一个固定报头第一个字节，从而得到整包数据的长度
                pack.Data_totle += pack.Data_lenth[k];                                                                                                                              //已经处理的数据量累加
                if(data_len == pack.Data_totle) {                                                                                                                                   //收到的数据长度等于pack.Data_totle，两种情况进入if，（1）就1个数据包且接收完整 （2）多个数据包且都接收完整了
                    memcpy(pack.Data_Packbuff[k],&data[pack.Data_totle - pack.Data_lenth[k]],pack.Data_lenth[k]);                                                                   //拷贝本次接收的数据
                    pack.Data_Packlen[k] = pack.Data_lenth[k];                                                                                                                      //记录本次接收的数据量
                    pack.Data_num = k;                                                                                                                                              //记录数据包的数量
                    break;                                                                                                                                                          //跳出k的for循环
                }else if(data_len < pack.Data_totle){                                                                                                                               //收到的数据长度小于整包数据的长度，说明有分包，本次只是一部分，还不完整，进入if
                    memcpy(&pack.Data_Packbuff[k][pack.Data_Packlen[k]],&data[pack.Data_totle - pack.Data_lenth[k]],data_len - (pack.Data_totle - pack.Data_lenth[k]));             //拷贝本次接收的数据，注意本次数据还不完整，只有一部分
                    pack.Data_Packlen[k] += data_len - (pack.Data_totle - pack.Data_lenth[k]);                                                                                      //累计本次接收的数据量
                    pack.Data_Packsta = 1;                                                                                                                                          //pack.Data_Packsta 置1，表示有分包了
                    pack.Data_num = k;                                                                                                                                              //记录数据包的数量
                    return;                                                                                                                                                         //直接返回，等下包数据
                }else{                                                                                                                                                              //大于的情况，说明是多包数据同时到来
                    memcpy(pack.Data_Packbuff[k],&data[pack.Data_totle - pack.Data_lenth[k]],pack.Data_lenth[k]);                                                                   //先拷贝一包数据
                    pack.Data_Packlen[k] = pack.Data_lenth[k];                                                                                                                      //记录本次接收的数据量
                }
            }else if(pack.Data_Packsta == 1){                                                                                                                                       //有分包的情况下
                if(data_len == (pack.Data_lenth[pack.Data_num] - pack.Data_Packlen[pack.Data_num])){                                                                                //分包剩余数据正好接收完毕，进入if
                    memcpy(&pack.Data_Packbuff[pack.Data_num][pack.Data_Packlen[pack.Data_num]],data,data_len);                                                                     //拷贝本次数据
                    pack.Data_Packlen[pack.Data_num] += data_len;                                                                                                                   //累计本次接收的数据量
                    break;                                                                                                                                                          //各个数据包都完整了，跳出k的for循环
                }else if(data_len > (pack.Data_lenth[pack.Data_num] - pack.Data_Packlen[pack.Data_num])){                                                                           //本包数据完整了，但是后续还有一个数据包
                    memcpy(&pack.Data_Packbuff[pack.Data_num][pack.Data_Packlen[pack.Data_num]],data,pack.Data_lenth[pack.Data_num] - pack.Data_Packlen[pack.Data_num]);            //拷贝本包数据
                    pack.Data_totle = pack.Data_lenth[pack.Data_num] - pack.Data_Packlen[pack.Data_num];					                                                        //已经处理的数据量累加
                    pack.Data_Packlen[pack.Data_num] += pack.Data_lenth[pack.Data_num] - pack.Data_Packlen[pack.Data_num];                                                          //记录本包数据长度                                                                                                        //累计本次接收的数据量 
                    k = pack.Data_num;                                                                                                                                              //记录k值，进循环
                    pack.Data_Packsta = 0;                                                                                                                                          //pack.Data_Packsta清0 后续可以进if(pack.Data_Packsta == 0)分支
                }else if(data_len < (pack.Data_lenth[pack.Data_num] - pack.Data_Packlen[pack.Data_num])){                                                                           //本包数据还没有接收完整
                    memcpy(&pack.Data_Packbuff[pack.Data_num][pack.Data_Packlen[pack.Data_num]],data,data_len);                                                                     //拷贝本次接收数据
                    pack.Data_Packlen[pack.Data_num] += data_len;                                                                                                                   //累计本次接收的数据量
                    return;                                                                                                                                                         //直接返回，等待下次数据
                }	
            }			
        }
    }
	
	for(j=0;j<=k;j++){ 
		/*----------------------------------------------*/
		/*               处理CONNACK报文                */
		/*----------------------------------------------*/
		res = MQTT_CONNACK((unsigned char *)pack.Data_Packbuff[j],pack.Data_Packlen[j]);
		if(res != -1){                                                                    
			u1_printf("接收到CONNACK报文\r\n");                                          //串口输出信息
			switch(res){	 		                                                     //判断返回码，表示CONNECT报文是否成功
				case 0x00 : u1_printf("CONNECT报文成功\r\n");                            //串口输出信息	
							for(i=0;i<TopicNum;i++){                                     //按需要订阅的Topic数量轮询
								MQTT_SUBSCRIBE(TopicBuff[i],0);                          //构建订阅报文
								if(SysCB.SysEventFlag&CONNECT_WIFI){                     //如果是WiFi模块连接服务器
									u4_TxDataBuf(mqtt.buff,mqtt.lenth);                  //加入WiFi模块串口4发送缓冲区
								}else if(SysCB.SysEventFlag&CONNECT_CAT1){               //如果是4G Cat1模块连接服务器
									u2_TxDataBuf(mqtt.buff,mqtt.lenth);                  //加入4G Cat1模块串口2发送缓冲区
								}
							}
							for(i=1;i<=SUN_NUMBER;i++){                                  //网关刚登陆上时，先向服务器发送所有子设备下线数据
								WiFi_Cat1_SubOnline(i,0);                                //发送子设备下线数据
							}							
							break;                                                       //跳出分支case 0x00                                              
				case 0x01 : u1_printf("连接已拒绝，不支持的协议版本，准备重启\r\n");     //串口输出信息
							NVIC_SystemReset();                                          //重启		
							break;                                                       //跳出分支case 0x01   
				case 0x02 : u1_printf("连接已拒绝，不合格的客户端标识符，准备重启\r\n"); //串口输出信息
							NVIC_SystemReset();                                          //重启		
							break;                                                       //跳出分支case 0x02 
				case 0x03 : u1_printf("连接已拒绝，服务端不可用，准备重启\r\n");         //串口输出信息
							NVIC_SystemReset();                                          //重启		
							break;                                                       //跳出分支case 0x03
				case 0x04 : u1_printf("连接已拒绝，无效的用户名或密码，准备重启\r\n");   //串口输出信息
							NVIC_SystemReset();                                          //重启		
							break;                                                       //跳出分支case 0x04
				case 0x05 : u1_printf("连接已拒绝，未授权，准备重启\r\n");               //串口输出信息
							NVIC_SystemReset();                                          //重启		
							break;                                                       //跳出分支case 0x05 		
				default   : u1_printf("连接已拒绝，未知状态，准备重启\r\n");             //串口输出信息 
							NVIC_SystemReset();                                          //重启		
							break;                                                       //跳出分支default 								
			}
		}
		/*----------------------------------------------*/
		/*              处理PINGRESP报文                */
		/*----------------------------------------------*/
		res = MQTT_PINGRESP((unsigned char *)pack.Data_Packbuff[j],pack.Data_Packlen[j]);
		if(res != -1){                                                                    
			u1_printf("接收到PINGRESP报文\r\n");                                         //串口输出信息
			SysCB.SysEventFlag &=~ CONNECT_PING;                                         //清除需要发送MQTT协议PING保活报文事件		
		}
		/*----------------------------------------------*/
		/*              处理MQTT_SUBACK报文             */
		/*----------------------------------------------*/
		res = MQTT_SUBACK((unsigned char *)pack.Data_Packbuff[j],pack.Data_Packlen[j]);
		if(res != -1){                                                                    
			u1_printf("接收到SUBACK报文\r\n");                                       //串口输出信息
			switch(res){		                                                     //判断返回码，是订阅结果数据，每个topic有一个结果字节			
				case 0x00 :                                                          //返回码0/1/2都表示正确
				case 0x01 :
				case 0x02 : u1_printf("订阅Topic成功\r\n");                          //串口输出信息					           
							break;                                                   //跳出分支                                             
				default   : u1_printf("订阅Topic失败，准备重启\r\n");                //串口输出信息 
							NVIC_SystemReset();                                      //重启		
							break;                                                   //跳出分支 								
			}					
		}
		/*----------------------------------------------*/
		/*         处理服务器推送来的PUBLISH报文        */
		/*----------------------------------------------*/
		res = MQTT_ProcessPUBLISH((unsigned char *)pack.Data_Packbuff[j],pack.Data_Packlen[j],&qs,&messageid);
		if(res != -1){                                                                    		
			if(qs == 0){	
				u1_printf("接收到服务器推送的等级0的Publsh报文Topic：%s\r\n",&mqtt.topic[2]);                            //串口输出信息
				u1_printf("接收到服务器推送的等级0的Publsh报文Data：%s\r\n",&mqtt.data[2]);                              //串口输出信息
				
				if(strstr((char *)&mqtt.topic[2],"thing/sub/login/reply")){                                              //搜索到关键词 thing/sub/login/reply
					if(sscanf((char *)&mqtt.data[2],"{\"id\":\"%d\",\"code\":%d,\"msg\":\"success\"}",&id,&code) == 2){  //提取数据，正确进入if
						if(code == 200){                                                                                 //code码200表示正确，进入if
							u1_printf("子设备%d上线成功\r\n",id);                                                        //串口输出信息
							lora.online[id] = 1;                                                                         //设置上线状态标志位
						}
					}
				}else if(strstr((char *)&mqtt.topic[2],"thing/sub/logout/reply")){                                       //搜索到关键词 thing/sub/logout/reply
					if(sscanf((char *)&mqtt.data[2],"{\"id\":\"%d\",\"code\":%d,\"msg\":\"success\"}",&id,&code) == 2){  //提取数据，正确进入if
						if(code == 200){                                                                                 //code码200表示正确，进入if
							u1_printf("子设备%d下线成功\r\n",id);                                                        //串口输出信息
							lora.online[id] = 0;                                                                         //设置上线状态标志位
						}
					}
				}else if(strstr((char *)&mqtt.topic[2],"thing/sub/property/set")){                                   //搜索到关键词 thing/sub/property/set
					for(i=1;i<=SUN_NUMBER;i++){                                                                      //按子设备数量轮询
						if(strstr((char *)&mqtt.data[2],DeviceNameBuff[i])){                                         //搜索子设备名称						
							if(strstr((char *)&mqtt.data[2],"\"PowerSwitch_1\":false")){                             //如果搜索到关键词 说明服务器下发开关1命令				
								tempbuff[0]=i;                                                                       //从机地址
								tempbuff[1]=0x06;                                                                    //06功能码
								tempbuff[2]=LED_REGISTER/256;                                                        //写LED寄存器
								tempbuff[3]=LED_REGISTER%256;                                                        //写LED寄存器
								tempbuff[4]=0x00;                                                                    //写数据
								tempbuff[5]=(lora.SW_Sta[i]&0x0E);                                                   //写数据		
								temp_crc = HAL_CRC_Calculate(&crc,(uint32_t *)tempbuff,6);                           //计算CRC
								tempbuff[6] = temp_crc/256;                                                          //CRC高字节
								tempbuff[7] = temp_crc%256;                                                          //CRC低字节
								u3_TxDataBuf(tempbuff,8);                                                            //加入LoRa缓冲区
								u1_printf("子设备%d 开关1关闭\r\n",i);                                               //串口输出信息
							}else if(strstr((char *)&mqtt.data[2],"\"PowerSwitch_1\":true")){                        //如果搜索到关键词 说明服务器下发开关1命令				
								tempbuff[0]=i;                                                                       //从机地址
								tempbuff[1]=0x06;                                                                    //06功能码
								tempbuff[2]=LED_REGISTER/256;                                                        //写LED寄存器
								tempbuff[3]=LED_REGISTER%256;                                                        //写LED寄存器
								tempbuff[4]=0x00;                                                                    //写数据
								tempbuff[5]=(lora.SW_Sta[i]|0x01);                                                   //写数据		
								temp_crc = HAL_CRC_Calculate(&crc,(uint32_t *)tempbuff,6);                           //计算CRC
								tempbuff[6] = temp_crc/256;                                                          //CRC高字节
								tempbuff[7] = temp_crc%256;                                                          //CRC低字节
								u3_TxDataBuf(tempbuff,8);                                                            //加入LoRa缓冲区
								u1_printf("子设备%d 开关1打开\r\n",i);                                               //串口输出信息
							}
							
							if(strstr((char *)&mqtt.data[2],"\"PowerSwitch_2\":false")){                             //如果搜索到关键词 说明服务器下发开关2命令				
								tempbuff[0]=i;                                                                       //从机地址
								tempbuff[1]=0x06;                                                                    //06功能码
								tempbuff[2]=LED_REGISTER/256;                                                        //写LED寄存器
								tempbuff[3]=LED_REGISTER%256;                                                        //写LED寄存器
								tempbuff[4]=0x00;                                                                    //写数据
								tempbuff[5]=(lora.SW_Sta[i]&0x0D);                                                   //写数据		
								temp_crc = HAL_CRC_Calculate(&crc,(uint32_t *)tempbuff,6);                           //计算CRC
								tempbuff[6] = temp_crc/256;                                                          //CRC高字节
								tempbuff[7] = temp_crc%256;                                                          //CRC低字节
								u3_TxDataBuf(tempbuff,8);                                                            //加入LoRa缓冲区
								u1_printf("子设备%d 开关2关闭\r\n",i);                                               //串口输出信息
							}else if(strstr((char *)&mqtt.data[2],"\"PowerSwitch_2\":true")){                        //如果搜索到关键词 说明服务器下发开关2命令				
								tempbuff[0]=i;                                                                       //从机地址
								tempbuff[1]=0x06;                                                                    //06功能码
								tempbuff[2]=LED_REGISTER/256;                                                        //写LED寄存器
								tempbuff[3]=LED_REGISTER%256;                                                        //写LED寄存器
								tempbuff[4]=0x00;                                                                    //写数据
								tempbuff[5]=(lora.SW_Sta[i]|0x02);                                                   //写数据		
								temp_crc = HAL_CRC_Calculate(&crc,(uint32_t *)tempbuff,6);                           //计算CRC
								tempbuff[6] = temp_crc/256;                                                          //CRC高字节
								tempbuff[7] = temp_crc%256;                                                          //CRC低字节
								u3_TxDataBuf(tempbuff,8);                                                            //加入LoRa缓冲区
								u1_printf("子设备%d 开关2打开\r\n",i);                                               //串口输出信息
							}
							
							if(strstr((char *)&mqtt.data[2],"\"PowerSwitch_3\":false")){                             //如果搜索到关键词 说明服务器下发开关3命令				
								tempbuff[0]=i;                                                                       //从机地址
								tempbuff[1]=0x06;                                                                    //06功能码
								tempbuff[2]=LED_REGISTER/256;                                                        //写LED寄存器
								tempbuff[3]=LED_REGISTER%256;                                                        //写LED寄存器
								tempbuff[4]=0x00;                                                                    //写数据
								tempbuff[5]=(lora.SW_Sta[i]&0x0B);                                                   //写数据		
								temp_crc = HAL_CRC_Calculate(&crc,(uint32_t *)tempbuff,6);                           //计算CRC
								tempbuff[6] = temp_crc/256;                                                          //CRC高字节
								tempbuff[7] = temp_crc%256;                                                          //CRC低字节
								u3_TxDataBuf(tempbuff,8);                                                            //加入LoRa缓冲区
								u1_printf("子设备%d 开关3关闭\r\n",i);                                               //串口输出信息
							}else if(strstr((char *)&mqtt.data[2],"\"PowerSwitch_3\":true")){                        //如果搜索到关键词 说明服务器下发开关3命令				
								tempbuff[0]=i;                                                                       //从机地址
								tempbuff[1]=0x06;                                                                    //06功能码
								tempbuff[2]=LED_REGISTER/256;                                                        //写LED寄存器
								tempbuff[3]=LED_REGISTER%256;                                                        //写LED寄存器
								tempbuff[4]=0x00;                                                                    //写数据
								tempbuff[5]=(lora.SW_Sta[i]|0x04);                                                   //写数据		
								temp_crc = HAL_CRC_Calculate(&crc,(uint32_t *)tempbuff,6);                           //计算CRC
								tempbuff[6] = temp_crc/256;                                                          //CRC高字节
								tempbuff[7] = temp_crc%256;                                                          //CRC低字节
								u3_TxDataBuf(tempbuff,8);                                                            //加入LoRa缓冲区
								u1_printf("子设备%d 开关3打开\r\n",i);                                               //串口输出信息
							}
							
							if(strstr((char *)&mqtt.data[2],"\"PowerSwitch_4\":false")){                             //如果搜索到关键词 说明服务器下发开关4命令				
								tempbuff[0]=i;                                                                       //从机地址
								tempbuff[1]=0x06;                                                                    //06功能码
								tempbuff[2]=LED_REGISTER/256;                                                        //写LED寄存器
								tempbuff[3]=LED_REGISTER%256;                                                        //写LED寄存器
								tempbuff[4]=0x00;                                                                    //写数据
								tempbuff[5]=(lora.SW_Sta[i]&0x07);                                                   //写数据		
								temp_crc = HAL_CRC_Calculate(&crc,(uint32_t *)tempbuff,6);                           //计算CRC
								tempbuff[6] = temp_crc/256;                                                          //CRC高字节
								tempbuff[7] = temp_crc%256;                                                          //CRC低字节
								u3_TxDataBuf(tempbuff,8);                                                            //加入LoRa缓冲区
								u1_printf("子设备%d 开关4关闭\r\n",i);                                               //串口输出信息
							}else if(strstr((char *)&mqtt.data[2],"\"PowerSwitch_4\":true")){                        //如果搜索到关键词 说明服务器下发开关4命令
								tempbuff[0]=i;                                                                       //从机地址
								tempbuff[1]=0x06;                                                                    //06功能码
								tempbuff[2]=LED_REGISTER/256;                                                        //写LED寄存器
								tempbuff[3]=LED_REGISTER%256;                                                        //写LED寄存器
								tempbuff[4]=0x00;                                                                    //写数据
								tempbuff[5]=(lora.SW_Sta[i]|0x08);                                                   //写数据
								temp_crc = HAL_CRC_Calculate(&crc,(uint32_t *)tempbuff,6);                           //计算CRC
								tempbuff[6] = temp_crc/256;                                                          //CRC高字节
								tempbuff[7] = temp_crc%256;                                                          //CRC低字节
								u3_TxDataBuf(tempbuff,8);                                                            //加入LoRa缓冲区
								u1_printf("子设备%d 开关4打开\r\n",i);                                               //串口输出信息
							}
							
							/*--------------------------------------------------------------------------------*/
							/*                           处 理 Ph 和 浊 度 阈 值 (优化改进版)                  */
							/*--------------------------------------------------------------------------------*/
							{
								uint8_t update_threshold = 0;
								char *ptr = NULL;  
								typedef union {
									float fVal;
									uint8_t bVal[4];
								} Float2Byte;
								Float2Byte u_ph, u_tu; // 定义联合体用于转换
								
								// 优化：只搜索字段名，不强制匹配前后引号，防止格式差异导致匹配失败
								if((ptr = strstr((char *)&mqtt.data[2], "ph_Threshold")) != NULL) {
									char *val_ptr = strchr(ptr, ':');     // 找到字段后的冒号
									if(val_ptr != NULL) {
										sscanf(val_ptr + 1, "%f", &Ph_Threshold[i]); // 解析冒号后的数值
										update_threshold = 1;										
									}
								}

								if((ptr = strstr((char *)&mqtt.data[2], "TU_Threshold")) != NULL) {
									char *val_ptr = strchr(ptr, ':');     // 找到字段后的冒号
									if(val_ptr != NULL) {
										sscanf(val_ptr + 1, "%f", &TU_Threshold[i]); // 解析冒号后的数值
										update_threshold = 1;										
									}
								}

								if(update_threshold == 1){
									// 使用联合体进行明确的字节分配
									u_ph.fVal = Ph_Threshold[i];
									u_tu.fVal = TU_Threshold[i];

									// 增加帧头 00 01 17
									tempbuff[0] = 0x00;
									tempbuff[1] = 0x01;
									tempbuff[2] = 0x17;

									tempbuff[3] = 0xFF;                         //帧头 0xFF
									tempbuff[4] = i;                            //节点ID (0x01-0x03)
									
									// 显式赋值，STM32是小端模式，直接按字节序存入
									tempbuff[5] = u_ph.bVal[0];
									tempbuff[6] = u_ph.bVal[1];
									tempbuff[7] = u_ph.bVal[2];
									tempbuff[8] = u_ph.bVal[3];
									
									tempbuff[9] = u_tu.bVal[0];
									tempbuff[10] = u_tu.bVal[1];
									tempbuff[11] = u_tu.bVal[2];
									tempbuff[12] = u_tu.bVal[3];
									
									tempbuff[13] = 0xFD;                        //帧尾 0xFD
									
									// 调试打印，确保数据转换正确
									u1_printf("解析数据: PH=%.2f TU=%.2f\r\n", u_ph.fVal, u_tu.fVal);
									// 打印HEX值用于核对
									u1_printf("HEX DEBUG: PH=%02X%02X%02X%02X TU=%02X%02X%02X%02X\r\n", 
										u_ph.bVal[0], u_ph.bVal[1], u_ph.bVal[2], u_ph.bVal[3],
										u_tu.bVal[0], u_tu.bVal[1], u_tu.bVal[2], u_tu.bVal[3]);

									u3_TxDataBuf(tempbuff, 14);                 //发送14字节数据到LoRa串口
									u1_printf("网关下发阈值数据帧: 00 01 17 FF %02X ... FD\r\n", i); //串口打印调试信息
								}
							}
						}	
					}										
				}
			}			
		}
	}
	/*----------------------------------------------*/
	/*              清除pack结构体                  */
	/*----------------------------------------------*/
	memset(&pack,0,PACK_CB_LEN);                                                                                //清除pack结构体	 
}
/*-------------------------------------------------*/
/*函数名：子设备上下线                             */
/*参  数：sub_num ：那个子设备                     */
/*参  数：mode ：0 下线 1：上线                    */
/*返回值：无                                       */
/*-------------------------------------------------*/
void WiFi_Cat1_SubOnline(char sub_num,char mode)
{	
	char temptopic[64];                                                                     //用于构建发送topic的缓冲区
	char tempdata[128];                                                                     //用于构建发送topic的缓冲区

	memset(temptopic,0,64);                                                                 //清空临时缓冲区	
	memset(tempdata,0,128);                                                                 //清空临时缓冲区	
	if(mode == 0){
		sprintf(temptopic,"$sys/%s/%s/thing/sub/logout",GW_PRODUCTID,GW_DEVICENAME);        //构建下线topic					
	}else{
		sprintf(temptopic,"$sys/%s/%s/thing/sub/login",GW_PRODUCTID,GW_DEVICENAME);         //构建上线topic						
	}
	sprintf((char *)tempdata,"{\"id\": \"%d\",\"version\": \"1.0\",\"params\": {\"productID\": \"%s\", \"deviceName\": \"%s\"}}",sub_num,SUB_PRODUCTID,DeviceNameBuff[sub_num]);  //构建子设备上线数据				
	MQTT_PUBLISH0(0,temptopic,(unsigned char *)tempdata,strlen(tempdata));	                //使用等级0的PUBLISH报文发送到服务器
	if(SysCB.SysEventFlag&CONNECT_WIFI){                                                    //如果是WiFi模块连接服务器
		u4_TxDataBuf(mqtt.buff,mqtt.lenth);                                                 //加入WiFi模块串口4发送缓冲区
	}else if(SysCB.SysEventFlag&CONNECT_CAT1){                                              //如果是4G Cat1模块连接服务器
		u2_TxDataBuf(mqtt.buff,mqtt.lenth);                                                 //加入4G Cat1模块串口2发送缓冲区
	}
}
/*-------------------------------------------------*/
/*函数名：子设备数据上传                           */
/*参  数：postdata ：上传的数据                    */
/*返回值：无                                       */
/*-------------------------------------------------*/
void WiFi_Cat1_SubDataPost(unsigned char * postdata)
{	  
	char temptopic[64];                                                             //用于构建发送topic的缓冲区

	memset(temptopic,0,64);                                                         //清空临时缓冲区	
	sprintf(temptopic,"$sys/%s/%s/thing/pack/post",GW_PRODUCTID,GW_DEVICENAME);     //构建topic	                                                     				
	MQTT_PUBLISH0(0,temptopic,(unsigned char *)postdata,strlen((char *)postdata));	//使用等级0的PUBLISH报文发送到服务器
	if(SysCB.SysEventFlag&CONNECT_WIFI){                                            //如果是WiFi模块连接服务器
		u4_TxDataBuf(mqtt.buff,mqtt.lenth);                                         //加入WiFi模块串口4发送缓冲区
	}else if(SysCB.SysEventFlag&CONNECT_CAT1){                                      //如果是4G Cat1模块连接服务器
		u2_TxDataBuf(mqtt.buff,mqtt.lenth);                                         //加入4G Cat1模块串口2发送缓冲区
	}	
}
/*-------------------------------------------------*/
/*函数名：WiFi模块 or 4G Cat1模块主动事件          */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void WiFi_Cat1_ActiveEvent(void)
{			
	/*---------------------------------------------*/
	/*            WiFi Airkiss配网事件             */
	/*---------------------------------------------*/
	if(key_flag == 1){                                                       //按键启动配网，传输路由器密码给WiFi模块		
		key_flag = 0;                                                        //清除按键标志位
		u1_printf("准备WiFi配网\r\n");                                       //串口提示数据		
		HAL_TIM_Base_Stop_IT(&tim6);                                         //关闭定时器6		
		if(SysCB.SysEventFlag&CONNECT_WIFI)                                  //如果WiFi连接上服务器事件发生，进入if
			SysCB.SysEventFlag = 0;                                          //清除标志位
		WiFi_Reset();                                                        //WiFi复位		
		u4_printf("AT+CWMODE=1\r\n");                                        //发送设置STA模式命令启动配网		
	}	
	/*---------------------------------------------*/
	/*              发送PING报文时间到             */
	/*---------------------------------------------*/
	if((SysCB.SysEventFlag&CONNECT_MQTT)&&(HAL_GetTick() - SysCB.PingTimer >=15000)){      //连接上MQTT服务器 且 间隔时间构15s                         
		SysCB.PingTimer = HAL_GetTick();                                     //发送PING报文计时时间记录当前时间
	    u1_printf("需要发送PINGREQ报文\r\n");                                //串口提示数据
		if(SysCB.SysEventFlag&CONNECT_PING){                                 //如果CONNECT_PING事件没有被清除，进入if分支
			u1_printf("连接掉线，重启\r\n");                                 //串口输出信息
			NVIC_SystemReset();                                              //重启 
		}else{                                                               //如果CONNECT_PING事件被清除，进入else分支
			SysCB.SysEventFlag |= CONNECT_PING;                              //需要发送MQTT协议PING保活报文事件发生
			MQTT_PINGREQ();                                                  //构建MQTT协议PINGREQ报文
			if(SysCB.SysEventFlag&CONNECT_WIFI){                             //如果是WiFi模块连接服务器
				u4_TxDataBuf(mqtt.buff,mqtt.lenth);                          //加入WiFi模块串口4发送缓冲区
			}else if(SysCB.SysEventFlag&CONNECT_CAT1){                       //如果是4G Cat1模块连接服务器
				u2_TxDataBuf(mqtt.buff,mqtt.lenth);                          //加入4G Cat1模块串口2发送缓冲区
			}		
		}
	}
	/*---------------------------------------------*/
	/*             需要切换成4G Cat1联网           */
	/*---------------------------------------------*/
	if(tim6_flag == 1){                                                      //需要切换成4G Cat1联网                     
		tim6_flag = 0;                                                       //清除tim6_flag标志位
		u1_printf("需要切换成4G Cat1联网\r\n");                              //串口提示数据
		Cat1_Reset();                                                        //复位4G Cat1模块	
	}
}
