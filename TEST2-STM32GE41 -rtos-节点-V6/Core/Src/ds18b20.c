#include "ds18b20.h"
#include <stdio.h>	
#include "FreeRTOS.h"
#include "task.h"
uint16_t tempMax=373;//???????
uint16_t tempMin=150;//???????
short temperature=0;//???
void Delay_us(uint32_t us);
//??��DS18B20
void DS18B20_Rst(void)	   
{                 
	DS18B20_IO_OUT(); //SET PA0 OUTPUT
  DS18B20_DQ_OUT(GPIO_PIN_RESET); // ????DQ
  Delay_us(750);    // ????750us
  DS18B20_DQ_OUT(GPIO_PIN_SET); // DQ=1 
	Delay_us(15);     //15US
}
//???DS18B20????
//????1:��???DS18B20?????
//????0:????
uint8_t DS18B20_Check(void) 	   
{   
	uint8_t retry=0;
	DS18B20_IO_IN();//SET PA0 INPUT	 
    while (DS18B20_DQ_IN && retry < 200)
	{
		retry++;
		Delay_us(1);
	};	 
	if(retry>=200)return 1;
	else retry=0;
    while (!DS18B20_DQ_IN && retry < 240)
	{
		retry++;
		Delay_us(1);
	};
	if(retry>=240)return 1;	    
	return 0;
}
//??DS18B20??????��
//???????1/0
uint8_t DS18B20_Read_Bit(void) 			 // read one bit
{
    uint8_t data;
	DS18B20_IO_OUT();//SET PA0 OUTPUT
    DS18B20_DQ_OUT(GPIO_PIN_RESET); 
	Delay_us(2);
    DS18B20_DQ_OUT(GPIO_PIN_SET); 
	DS18B20_IO_IN();//SET PA0 INPUT
	Delay_us(12);
	if(DS18B20_DQ_IN)data=1;//????1
    else data=0;//????0	 
    Delay_us(50);           
    return data;
}
//??DS18B20?????????
//?????????????????
uint8_t DS18B20_Read_Byte(void)    // read one byte
{        
    uint8_t i,j,dat;
    dat=0;
	 for (i=1;i<=8;i++) 
	 {
        j=DS18B20_Read_Bit();  
        dat=(j<<7)|(dat>>1);
   }						    
    return dat;
}
//��???????DS18B20
//dat???��??????
void DS18B20_Write_Byte(uint8_t dat)     
 {             
    uint8_t j;
    uint8_t testb;
	DS18B20_IO_OUT();//SET PA0 OUTPUT;
    for (j=1;j<=8;j++) 
	{
        testb=dat&0x01;
        dat=dat>>1;
        if (testb) 
        {
            DS18B20_DQ_OUT(GPIO_PIN_RESET); // Write 1
            Delay_us(2);                            
            DS18B20_DQ_OUT(GPIO_PIN_SET);
            Delay_us(60);             
        }
        else 
        {
            DS18B20_DQ_OUT(GPIO_PIN_RESET); // Write 0
            Delay_us(60);             
            DS18B20_DQ_OUT(GPIO_PIN_SET);
            Delay_us(2);                          
        }
    }
}
//?????????
void DS18B20_Start(void)// ds1820 start convert
{   						               
    DS18B20_Rst();	   
		DS18B20_Check();	 
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0x44);// convert
} 
//?????DS18B20??IO?? DQ ?????DS?????
//????1:??????
//????0:????    	 
uint8_t DS18B20_Init(void)
{
 	// GPIO_InitTypeDef  GPIO_InitStructure;
 	
 	// RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //???PORTA????? 
	
 	// GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;				//PORTA11 ???????
 	// GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		  
 	// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	// GPIO_Init(GPIOB, &GPIO_InitStructure);

 	// GPIO_SetBits(GPIOB,GPIO_Pin_6);    //???1
HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
	DS18B20_Rst();

	if (DS18B20_Check()==1)
    {
        printf("DS18B20 Not Found!\r\n");
        return 1;
    }
     return 0;
}  
//??ds18b20???????
//?????0.1C
//??????????? ??-550~1250?? 
short DS18B20_Get_Temp(void)
{
    uint8_t temp;
    uint8_t TL,TH;
	short tem;
    DS18B20_Start ();                    // ds1820 start convert
    vTaskDelay(pdMS_TO_TICKS(800));      // wait conversion complete (>=750ms for 12-bit)
    DS18B20_Rst();
    DS18B20_Check();	 
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0xbe);// convert	    
    TL=DS18B20_Read_Byte(); // LSB   
    TH=DS18B20_Read_Byte(); // MSB  
	    	  
    if(TH>7)
    {
        TH=~TH;
        TL=~TL; 
        temp=0;//??????  
    }else temp=1;//??????	  	  
    tem=TH; //?????��
    tem<<=8;    
    tem+=TL;//?????��
    tem=(float)tem*0.625f;//???     
	if(temp)return tem; //????????
	else return -tem;    
} 
 
/*
short magnitude = ((TH & 0x07) << 8) | TL;  // ???12��
float temperature;

if (TH & 0xF8) {  // ??5��????????????
    temperature = -magnitude * 0.0625;
} else {
    temperature = magnitude * 0.0625;
}
*/

void Delay_us(uint32_t udelay)
{
    __IO uint32_t Delay = udelay * 21U;
    while (Delay--) { __NOP(); }
}
