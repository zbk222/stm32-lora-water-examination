
#include "stm32g0xx_hal.h"    
#include "main.h"            
#include "clock.h"  
#include "usart.h"            
#include "lora.h"                   
#include "crc.h"   
#include "string.h" 
#include "mqtt.h" 
#include "timer.h" 
#include "wifi_cat1.h"
#include "stdio.h"            
#include "stdarg.h"   
#include "string.h" 

LoRaCB lora;                             //LoRa struct 

LoRaParameter LoRaSetData = {            //LoRa config
0x00,                                //AddrH
0x02,                                //AddrL
0x02,                                //NetID
LoRa_9600,                           //Baudrate
LoRa_8N1,                            //Parity
LoRa_19_2,                           //AirSpeed
LoRa_Data240,                        //PacketSize
LoRa_RssiDIS,                        //RSSI Disable
LoRa_SoftDIS,                        //Soft Switch
LoRa_FEC_22DBM,                      //TxPower
LoRa_CH23,                           //Channel
LoRa_RssiByteDIS,                    //RSSI Byte
LoRa_ModePOINT,                      //Point Mode
LoRa_RelayDIS,                       //Relay
LoRa_LBTDIS,                         //LBT
LoRa_WorTX,                          //Wor Mode
LoRa_Wor2000ms,                      //Wor Cycle
0x22,                                //KeyH
0x33,                                //KeyL
};

/*-------------------------------------------------*/
/* Initialize LoRa                                 */
/*-------------------------------------------------*/
void LoRa_Init(void)
{
uint8_t cmd[12];

GPIO_InitTypeDef GPIO_Initure;

__HAL_RCC_GPIOB_CLK_ENABLE();
__HAL_RCC_GPIOF_CLK_ENABLE();

GPIO_Initure.Pin = GPIO_PIN_7;
GPIO_Initure.Mode = GPIO_MODE_INPUT;
GPIO_Initure.Pull = GPIO_NOPULL;
HAL_GPIO_Init(GPIOB, &GPIO_Initure);

GPIO_Initure.Pin = GPIO_PIN_1;
GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_Initure.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOF, &GPIO_Initure);

GPIO_Initure.Pin = GPIO_PIN_5 | GPIO_PIN_6;
GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_Initure.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOB, &GPIO_Initure);

LoRa_PowerOFF;
HAL_Delay(1000);
LoRa_PowerON;
while(LoRa_AUX!=1);
LoRa_MODE2;
lora.sta = 0;
HAL_Delay(2000);
U3_Init(9600);

cmd[0] = 0xC0;
cmd[1] = 0x00;
cmd[2] = 0x09;
cmd[3] = LoRaSetData.LoRa_AddrH;
cmd[4] = LoRaSetData.LoRa_AddrL;
cmd[5] = LoRaSetData.LoRa_NetID;
cmd[6] = LoRaSetData.LoRa_Baudrate | LoRaSetData.LoRa_UartMode | LoRaSetData.LoRa_airvelocity;
cmd[7] = LoRaSetData.LoRa_DataLen | LoRaSetData.LoRa_Rssi | LoRa_SoftDIS | LoRaSetData.LoRa_TxPower;
cmd[8] = LoRaSetData.LoRa_CH;
cmd[9] = LoRaSetData.LoRa_RssiByte | LoRaSetData.LoRa_DateMode | LoRaSetData.LoRa_Relay | LoRaSetData.LoRa_LBT | LoRaSetData.LoRa_WORmode | LoRaSetData.LoRa_WORcycle;
cmd[10] = LoRaSetData.LoRa_KeyH;
cmd[11] = LoRaSetData.LoRa_KeyL;
u3_TxDataBuf(cmd,12);
}

/*-------------------------------------------------*/
/* Check LoRa Configuration                        */
/*-------------------------------------------------*/
void LoRa_ConfigData(uint8_t *data, uint16_t data_len)
{
if((data_len == 12)&&(data[0] == 0xC1)&&(data[1] == 0x00)&&(data[2] == 0x09)){
while(LoRa_AUX!=1);
LoRa_MODE0;
lora.sta = 1;
lora.timer = 0;
lora.counter = 0;
HAL_Delay(200);
u1_printf("Lora config success\r\n");
        // Printf details omitted for brevity
}
}

/*-------------------------------------------------*/
/* Old LoRa_TransData (Deprecated)                 */
/*-------------------------------------------------*/
void LoRa_TransData_Old(uint8_t *data, uint16_t data_len)
{
    // Old implementation content for reference or unused
    // We keep it empty or commented out to avoid conflict if called
}

/*-------------------------------------------------*/
/* New Implementation: LoRa_TransData              */
/* Protocol: [FF][FE][ID][Data 16][T1][T2]         */
/* Length: 21 bytes                                */
/*-------------------------------------------------*/
void LoRa_TransData(uint8_t *data, uint16_t data_len)
{
uint8_t id;

if(data_len == 21) {
if((data[0] == 0xFF) && (data[1] == 0xFE)) {
id = data[2]; 

if(id >= 1 && id <= SUN_NUMBER) {
lora.timeout[id] = HAL_GetTick(); // ?? timeout ????????????

if(lora.online[id] == 0) {
u1_printf("Recv ID:%d, Go Online\r\n", id);
WiFi_Cat1_SubOnline(id, 1);
} else {
u1_printf("Recv ID:%d, Refresh Heartbeat\r\n", id);
}
} else {
u1_printf("Unknown ID: %d\r\n", id);
}
} else {
u1_printf("Header Error: %02X %02X\r\n", data[0], data[1]);
}
} else {
/* ???? main.c ???? handled=1 ??????????? */
// u1_printf("Len Error: %d (Exp 21)\r\n", data_len);
}
}

/*-------------------------------------------------*/
/* Old LoRa_ActiveEvent (Deprecated)               */
/*-------------------------------------------------*/
void LoRa_ActiveEvent_Old(void)
{
    // Old implementation removed to avoid conflicts
}

/*-------------------------------------------------*/
/* 函数名：发送轮询帧                               */
/* 参  数：id：设备ID                               */
/* 返回值：无                                      */
/*-------------------------------------------------*/
void LoRa_SendPoll(uint8_t id)
{
	uint8_t cmd[7] = {0x00, 0x01, 0x17, 0xFF, 0x00, 0x01, 0xFE};
	cmd[4] = id;
	u3_TxData(cmd, 7);
}

/*-------------------------------------------------*/
/* New Implementation: LoRa_ActiveEvent            */
/* Check 30s timeout                               */
/*-------------------------------------------------*/
void LoRa_ActiveEvent(void)
{
uint8_t i;

// ???MQTT ? ?1s ??????
if((SysCB.SysEventFlag&CONNECT_MQTT)&&((HAL_GetTick() - lora.timer) >= 1000)&&(lora.sta == 1)){
lora.timer = HAL_GetTick();

for(i=1; i<=SUN_NUMBER; i++){
if(lora.online[i] == 1){ 
// 15s超时
if((HAL_GetTick() - lora.timeout[i]) > 15000){ 
u1_printf("ID:%d Timeout 15s, Go Offline\r\n", i);
WiFi_Cat1_SubOnline(i, 0); 
lora.online[i] = 0;
}
}
}
}
}
