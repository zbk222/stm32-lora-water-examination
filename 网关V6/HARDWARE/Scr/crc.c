
#include "stm32g0xx_hal.h"    
#include "crc.h"              

CRC_HandleTypeDef crc;       //crc功能总控结构体

/*-------------------------------------------------*/
/*函数名：CRC16_Modbus模式初始化                   */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void CRC16_ModbusInit(void)
{
	crc.Instance = CRC;                                                        //配置CRC
	crc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;                //不使用默认多项式
	crc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_DISABLE;                 //不使用默认初始值
	crc.Init.GeneratingPolynomial = 0x8005;                                    //设置多项式
	crc.Init.CRCLength = CRC_POLYLENGTH_16B;                                   //CRC长度 16
	crc.Init.InitValue = 0xFFFF;                                               //设置初始值
	crc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_BYTE;            //输入数据反转
	crc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;        //输出数据反转
	crc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;                          //输入数据格式长度 1个字节
	HAL_CRC_Init(&crc);                                                        //设置
}
/*-------------------------------------------------*/
/*函数名：CRC 的底层初始化                         */
/*参  数：crc：CRC配置总控结构体                   */
/*返回值：无                                       */
/*说  明：此函数会被HAL_CRC_Init()回调调用         */
/*-------------------------------------------------*/
void HAL_CRC_MspInit(CRC_HandleTypeDef* crc)
{
	if(crc->Instance==CRC){           //如果配置crc
		__HAL_RCC_CRC_CLK_ENABLE();   //开crc时钟
	}
}
