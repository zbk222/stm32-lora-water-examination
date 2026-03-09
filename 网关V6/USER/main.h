/*-----------------------------------------------------*/
/*              超子说物联网STM32系列开发板            */
/*-----------------------------------------------------*/
/*                                                     */
/*                程序中各种配置信息头文件             */
/*                                                     */
/*-----------------------------------------------------*/

#ifndef __MAIN_H
#define __MAIN_H

#include "stdint.h"

#define SUN_NUMBER    3    //使用几个节点板

#define MQTT_SERVER        "mqtts.heclouds.com"                                                                         //服务器域名
#define MQTT_PORT          1883                                                                                         //服务器端口号

#define UNIX               "1861891199"                                                                                 //token运算时的过期UNIX时间戳

#define GW_PRODUCTID       "WuSY7240x2"                                                                                 //网关产品ID
#define GW_DEVICENAME      "GW001"                                                                                      //网关设备名称
#define GW_DEVICESECRET    "NEo1Q0NmbVc3aXZZQ2c5d280R21wUVVyTlU5ZWlJcXQ="                                               //网关设备密钥
#define SUB_PRODUCTID      "t3N8uOdwy8"                                                                                 //子设备产品ID
#define SUB1_PDEVICENAME   "D001"                                                                                       //子设备1设备名称
#define SUB2_PDEVICENAME   "D002"                                                                                       //子设备2设备名称
#define SUB3_PDEVICENAME   "D003"                                                                                       //子设备3设备名称

#define ATTRIBUTE1         "PowerSwitch_1"                                       //功能属性1标识符，标识符名称必须和服务器后台设置的完全一样，大小写也必须一样
#define ATTRIBUTE2         "PowerSwitch_2"                                       //功能属性2标识符，标识符名称必须和服务器后台设置的完全一样，大小写也必须一样
#define ATTRIBUTE3         "PowerSwitch_3"                                       //功能属性3标识符，标识符名称必须和服务器后台设置的完全一样，大小写也必须一样
#define ATTRIBUTE4         "PowerSwitch_4"                                       //功能属性4标识符，标识符名称必须和服务器后台设置的完全一样，大小写也必须一样
#define ATTRIBUTE5         "temperature"                                         //功能属性5标识符，标识符名称必须和服务器后台设置的完全一样，大小写也必须一样
#define ATTRIBUTE6         "humidity"                                            //功能属性6标识符，标识符名称必须和服务器后台设置的完全一样，大小写也必须一样
#define ATTRIBUTE7         "lightlux"                                            //功能属性7标识符，标识符名称必须和服务器后台设置的完全一样，大小写也必须一样
#define ATTRIBUTE8         "ADC_CH1"                                             //功能属性8标识符，标识符名称必须和服务器后台设置的完全一样，大小写也必须一样
#define ATTRIBUTE9         "ADC_CH2"                                             //功能属性9标识符，标识符名称必须和服务器后台设置的完全一样，大小写也必须一样
#define ATTRIBUTE10        "ADC_CH3"                                             //功能属性10标识符，标识符名称必须和服务器后台设置的完全一样，大小写也必须一样

/*---------------------------------------------------------------*/
/*-------------------用于各种系统参数的结构体--------------------*/
/*---------------------------------------------------------------*/
typedef struct{
    uint32_t SysEventFlag;                                                       //发生各种事件的标志变量
	uint32_t PingTimer;                                                          //用于记录发送PING数据包的计时器 	
}Sys_CB;
#define SYS_STRUCT_LEN         sizeof(Sys_CB)                                    //用于各种系统参数的 Sys_CB结构体 长度 
	
/*---------------------------------------------------------------*/
/*-----------------------系统事件发生标志定义--------------------*/
/*---------------------------------------------------------------*/
#define CONNECT_MQTT          ((uint32_t)0x00000001)          				     //连接上MQTT服务器事件
#define CONNECT_WIFI          ((uint32_t)0x00000002)          				     //WiFi模块连接上服务器事件
#define CONNECT_CAT1          ((uint32_t)0x00000004)          				     //4G Cat1模块连接上服务器事件
#define CONNECT_PING          ((uint32_t)0x00000008)          				     //需要发送MQTT协议PING保活报文事件

//------------各种外部变量声明，便于其他源文件调用变量-----------//
extern Sys_CB          SysCB;                                                    //外部变量声明，用于各种系统参数的结构体
extern char DeviceNameBuff[SUN_NUMBER+1][64];                                    //外部变量声明，设备名称数组

#endif
