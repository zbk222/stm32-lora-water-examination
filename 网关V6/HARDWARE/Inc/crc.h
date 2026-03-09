/*-------------------------------------------------*/
/*           超子说物联网STM32系列开发板           */
/*-------------------------------------------------*/
/*                                                 */
/*                实现CRC功能的头文件              */
/*                                                 */
/*-------------------------------------------------*/

#ifndef __CRC_H
#define __CRC_H

extern CRC_HandleTypeDef crc;        //外部变量声明，crc功能总控结构体

void CRC16_XmodemInit(void);         //函数声明，CRC16_Xmodem模式初始化
void CRC16_ModbusInit(void);         //函数声明，CRC16_Modbus模式初始化

#endif


