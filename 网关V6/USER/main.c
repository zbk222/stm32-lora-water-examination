#include "stm32g0xx_hal.h"    
#include "main.h"
#include "clock.h"            
#include "usart.h"            
#include "lora.h"	         
#include "crc.h"   
#include "timer.h"         
#include "wifi_cat1.h"
#include "key_capture.h"  
#include "mqtt.h"  
#include "string.h"
#include "stdio.h"

Sys_CB  SysCB;                                               //ï¿½ï¿½ï¿½Ú¸ï¿½ï¿½ï¿½ÏµÍ³ï¿½ï¿½ï¿½ï¿½ï¿½Ä½á¹¹ï¿½ï¿½
uint8_t state=0;
uint32_t Poll_Timer = 0;
uint8_t Poll_ID = 1;
char DeviceNameBuff[SUN_NUMBER+1][64]=                       //ï¿½è±¸ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
{
GW_DEVICENAME,	                                             //ï¿½ï¿½ï¿½ï¿½
SUB1_PDEVICENAME,                                            //ï¿½ï¿½ï¿½è±¸1
SUB2_PDEVICENAME,                                            //ï¿½ï¿½ï¿½è±¸2
SUB3_PDEVICENAME,                                            //ï¿½ï¿½ï¿½è±¸3
};

int main(void) 
{    		
	uint16_t i;

	HAL_Init();                                              //ï¿½ï¿½Ê¼ï¿½ï¿½HAL	
	CLock_Init();                                            //ï¿½ï¿½Ê¼ï¿½ï¿½Ê±ï¿½ï¿½
	CRC16_ModbusInit();                                      //ï¿½ï¿½Ê¼ï¿½ï¿½CRC ModbusÄ£Ê½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ê½0x8005ï¿½ï¿½ï¿½ï¿½Ê¼Öµ0xFFFF,ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ý¾ï¿½ï¿½ï¿½×?
	U1_Init(921600);                                         //ï¿½ï¿½Ê¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½1	
	U4_Init(921600);                                         //ï¿½ï¿½Ê¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½4ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½WiFiÄ£ï¿½ï¿½				
	U2_Init(921600);                                   //ï¿½ï¿½Ê¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½2ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½4G Cat1Ä£ï¿½ï¿½		
	WiFi_Cat1_InitGPIO();                                    //ï¿½ï¿½Ê¼ï¿½ï¿½ï¿½ï¿½Ê¼ï¿½ï¿½WiFi 4G Cat1Ä£ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½IO  
	MQTT_Init();                               //ï¿½ï¿½Ê¼ï¿½ï¿½MQTTï¿½ï¿½ï¿½ï¿½
	KEY_Capture_Init();                                      //ï¿½ï¿½ï¿½ñ°´¼ï¿½ï¿½ï¿½Ê¼ï¿½ï¿½,ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½WiFiï¿½ï¿½ï¿½ï¿½	
	TIM3_Init(2000,64000);                                   //ï¿½ï¿½Ê±ï¿½ï¿½3ï¿½ï¿½Ê±2sï¿½ï¿½ï¿½ï¿½ï¿½ï¿½LoRaï¿½È´ï¿½ï¿½ï¿½ï¿½è±¸ï¿½Ø¸ï¿½ï¿½ï¿½Ê±ï¿½ï¿½Ê±
	TIM6_Init(200,64000); 
	LoRa_Init();	//ï¿½ï¿½Î»WiFiÄ£ï¿½ï¿½               //ï¿½ï¿½Ê±ï¿½ï¿½6ï¿½ï¿½Ê±20sï¿½ï¿½ï¿½ï¿½ï¿½ï¿½WiFiï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ê±ï¿½ï¿½Ê±			                                     //ï¿½ï¿½Ê¼ï¿½ï¿½LoRaÄ£ï¿½ï¿½
	WiFi_Reset(); 
  
	
	while(1){ 	
		
		/*-----------------------------------------------------------------------------------------*/  
		/*                             ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ 3 ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½                             */
		/*-----------------------------------------------------------------------------------------*/
if(U3_CB.RxOUT != U3_CB.RxIN){                                                                                    //´®¿Ú3½ÓÊÕ»º³å OUTÖ¸Õë ºÍ INÖ¸Õë²»µÈ£¬½øif£¬ËµÃ÷´®¿Ú3½ÓÊÕ»º³åÇøÓÐÊý¾Ý£¬ÐèÒª²é¿´	   																																																																												
			u1_printf("LoRaÄ£¿é±¾´Î½ÓÊÕ%d×Ö½ÚÊý¾Ý\r\n",U3_CB.RxOUT->e - U3_CB.RxOUT->s);                                  //´òÓ¡LoRa±¾´Î½ÓÊÕµÄÊý¾Ý³¤¶È
			for(i=0;i<(U3_CB.RxOUT->e - U3_CB.RxOUT->s);i++)                                                              //Ñ­»·Êä³ö½ÓÊÕµÄÊý¾Ý
				u1_printf("%02x ",U3_CB.RxOUT->s[i]);                                                                     //16½øÖÆÊä³ö
			u1_printf("\r\n\r\n");                                                                                        //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?2ï¿½ï¿½ï¿½Ø³ï¿½ï¿½ï¿½ï¿½ï¿½
			/* ï¿½ï¿½ï¿½È³ï¿½ï¿½Ô½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½è±¸21ï¿½Ö½Ú´ï¿½ï¿½ï¿½Ö¡ï¿½ï¿½ï¿½Ï±ï¿½ï¿½Ä¸ï¿½floatï¿½ï¿½ï¿½ï¿½Æ½Ì¨ */
			{
				uint8_t *buf = U3_CB.RxOUT->s;
				uint16_t len = U3_CB.RxOUT->e - U3_CB.RxOUT->s;
				int handled = 0;
				if(len >= 21){
					/* ï¿½ï¿½ï¿½ï¿½Ö¡Í· 0xFF 0xFE */
					int start = -1;
					for(int k=0;k<=len-2;k++){
						if(buf[k] == 0xFF && buf[k+1] == 0xFE){ start = k; break; }
					}
					if(start >= 0){
						/* Ô¤ï¿½Ú¸ï¿½Ê½: [FF][FE][DeviceID][16ï¿½Ö½ï¿½ï¿½ï¿½ï¿½ï¿½][Î²1][Î²2] */
						uint8_t dev_id = buf[start+2];
						int data_pos = start + 3; /* ï¿½ï¿½ï¿½ï¿½ï¿½è±¸IDï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ê? */
						
						/* ï¿½ï¿½ï¿½ï¿½Ö¡Î²Ð£ï¿½ï¿½: 0xFD 0xFC */
						if(len - data_pos >= 18 && buf[data_pos+16] == 0xFD && buf[data_pos+17] == 0xFC){
							float tds, tur, temp, ph;
							/* ï¿½Ô±ï¿½ï¿½ï¿½ï¿½Ö½ï¿½ï¿½ï¿½Ö±ï¿½ï¿½È¡ï¿½ï¿½4ï¿½ï¿½floatï¿½ï¿½ï¿½ï¿½ï¿½è·¢ï¿½Í¶ï¿½ÎªIEEE754Ð¡ï¿½ï¿½ */
							memcpy(&tds,  &buf[data_pos + 0],  4);
							memcpy(&tur,  &buf[data_pos + 4],  4);
							memcpy(&temp, &buf[data_pos + 8],  4);
							memcpy(&ph,   &buf[data_pos + 12], 4);
							
							char *pDeviceName = SUB1_PDEVICENAME;
							if(dev_id >= 1 && dev_id <= 3) {
								pDeviceName = DeviceNameBuff[dev_id];
							}
											
							char json[512];
							/* ï¿½Î¿ï¿½LoRa_TransDataï¿½Ðµï¿½Gateway Batch Uploadï¿½ï¿½Ê½ï¿½ï¿½ï¿½ï¿½JSON */
							/* ï¿½ï¿½Ê½: {"id":"1","version":"1.0","params":[{"identity":{"productID":"...","deviceName":"..."},"properties":{"key":{"value":val},...}}]} */
							/* 
								OneNETï¿½ï¿½ï¿½ï¿½ï¿½ï¿½2254: float not conform step (ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½)
								TDS: 0.01 (%.2f)
								Turbidity: 0.1 (%.1f)
								Temperature: 0.1 (%.1f)
								PH: 0.1 (%.1f) 
							*/
							snprintf(json, sizeof(json), 
								"{\"id\":\"1\",\"version\":\"1.0\",\"params\":[{\"identity\":{\"productID\":\"%s\",\"deviceName\":\"%s\"},\"properties\":{\"tds\":{\"value\":%.2f},\"turbidity\":{\"value\":%.1f},\"temperuature\":{\"value\":%.1f},\"ph\":{\"value\":%.1f}}}]}",
								SUB_PRODUCTID, pDeviceName, tds, tur, temp, ph);
											
							/* Í¨ï¿½ï¿½ï¿½ï¿½ï¿½è±¸ï¿½ï¿½ï¿½ï¿½ï¿½Ï±ï¿½ï¿½Ó¿Ú£ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½JSONï¿½ï¿½ï¿½ï¿½ */
							WiFi_Cat1_SubDataPost((unsigned char*)json);
							
							/* Ë¢ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ß¼ï¿½ï¿? */
							if(dev_id >= 1 && dev_id <= SUN_NUMBER){
								lora.timeout[dev_id] = HAL_GetTick();
								if(lora.online[dev_id] == 0){
									u1_printf("ID:%d ÉÏÏß\r\n", dev_id);
									WiFi_Cat1_SubOnline(dev_id, 1);
									lora.online[dev_id] = 1;
								}
							}

							handled = 1;
						}
					}
				}
				if(!handled){
					if(lora.sta == 0)                                                                                             //lora.staï¿½ï¿½ï¿½ï¿½0ï¿½ï¿½ï¿½ï¿½Ê¾ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½×´Ì¬                              
						LoRa_ConfigData(U3_CB.RxOUT->s,U3_CB.RxOUT->e - U3_CB.RxOUT->s);                                          //ï¿½ï¿½ï¿½ï¿½LoRaï¿½ï¿½ï¿½ï¿½×´Ì¬ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
					else if(lora.sta == 1)                                                                                        //lora.staï¿½ï¿½ï¿½ï¿½1ï¿½ï¿½ï¿½ï¿½Ê¾ï¿½ï¿½ï¿½Ú´ï¿½ï¿½ï¿½×´Ì¬ 
						LoRa_TransData(U3_CB.RxOUT->s,U3_CB.RxOUT->e - U3_CB.RxOUT->s);                                           //ï¿½ï¿½ï¿½ï¿½LoRaï¿½ï¿½ï¿½ï¿½×´Ì¬ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
				}
			}
			U3_CB.RxOUT ++;                                                                                               //ï¿½ï¿½ï¿½ï¿½3ï¿½ï¿½ï¿½Õ»ï¿½ï¿½ï¿½ï¿½ï¿½ OUTÖ¸ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½Ô?
			if(U3_CB.RxOUT == &U3_CB.se_RxBuff[RX_NUM-1])                                                                 //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Æµï¿½seÖ¸ï¿½ï¿½Ô½á¹¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½Ô±Î»ï¿½Ã£ï¿½ï¿½ï¿½ï¿½ï¿½ifï¿½ï¿½ï¿½ï¿½Òªï¿½Ø¾ï¿½
				U3_CB.RxOUT = &U3_CB.se_RxBuff[0];                                                                        //ï¿½ï¿½ï¿½ï¿½3ï¿½ï¿½ï¿½Õ»ï¿½ï¿½ï¿½ï¿½ï¿½ OUTÖ¸ï¿½ï¿½Ö¸ï¿½ï¿½ seÖ¸ï¿½ï¿½Ô½á¹¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?0ï¿½Å³ï¿½Ô± 
		}
		/*-----------------------------------------------------------------------------------------*/  
		/*                              ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ 3 ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½                               */
		/*-----------------------------------------------------------------------------------------*/
		if((U3_CB.TxOUT != U3_CB.TxIN)&&(U3_CB.TxCpltflag == 0)&&(LoRa_AUX == 1)){                                        //´®¿Ú3·¢ËÍ»º³å OUTÖ¸Õë ºÍ INÖ¸Õë²»µÈ ÇÒ ·¢ËÍ¿ÕÏÐ£¨DMA·¢ËÍ¿ÕÏÐ£© ÇÒ LoRa_AUXÊÇ¸ßµçÆ½£¨Ä£¿é¿ÕÏÐ£©£¬ËµÃ÷·¢ËÍ»º³åÇøÓÐÊý¾Ý£¬ÇÒ¿ÉÒÔ·¢ËÍ         		             					   						
		    u1_printf("LoRaÄ£¿é±¾´Î·¢ËÍ%d×Ö½ÚÊý¾Ý\r\n",U3_CB.TxOUT->e - U3_CB.TxOUT->s + 1);                              //´òÓ¡LoRa±¾´Î·¢ËÍµÄÊý¾Ý³¤¶È
			for(i=0;i<(U3_CB.TxOUT->e - U3_CB.TxOUT->s + 1);i++)                                                          //Ñ­»·Êä³ö·¢ËÍµÄÊý¾Ý
				u1_printf("%02x ",U3_CB.TxOUT->s[i]);                                                                     //16½øÖÆÊä³ö
			u1_printf("\r\n\r\n");                                                                                        //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?2ï¿½ï¿½ï¿½Ø³ï¿½ï¿½ï¿½ï¿½ï¿½
			if(HAL_UART_Transmit_DMA(&U3_CB.USART_Handler,U3_CB.TxOUT->s,U3_CB.TxOUT->e - U3_CB.TxOUT->s + 1)== HAL_OK){  //ï¿½ï¿½ï¿½ï¿½DMAï¿½ï¿½ï¿½Í£ï¿½ï¿½É¹ï¿½ï¿½ï¿½ï¿½ï¿½if
				U3_CB.TxCpltflag |= (TX_STA_DMA | TX_STA_ACK);                                                            //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½DMAï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½Òªï¿½È´ï¿½ï¿½Úµï¿½Ó¦ï¿½ï¿½
				HAL_TIM_Base_Start_IT(&tim3);                                                                             //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ê±ï¿½ï¿½3 3sï¿½ï¿½Ê±ï¿½ï¿½ï¿½ï¿½ï¿½Ú½Úµï¿½Ó¦ï¿½ï¿½Ê±ï¿½ï¿½Ê±				
				U3_CB.TxOUT ++;                                                                                           //ï¿½ï¿½ï¿½ï¿½3ï¿½ï¿½ï¿½Í»ï¿½ï¿½ï¿½ï¿½ï¿½ OUTÖ¸ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½Ô?
				if(U3_CB.TxOUT == &U3_CB.se_TxBuff[TX_NUM-1])                                                             //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Æµï¿½seÖ¸ï¿½ï¿½Ô½á¹¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½Ô±Î»ï¿½Ã£ï¿½ï¿½ï¿½ï¿½ï¿½ifï¿½ï¿½ï¿½ï¿½Òªï¿½Ø¾ï¿½
					U3_CB.TxOUT = &U3_CB.se_TxBuff[0];                                                                    //ï¿½ï¿½ï¿½ï¿½3ï¿½ï¿½ï¿½Í»ï¿½ï¿½ï¿½ï¿½ï¿½ OUTÖ¸ï¿½ï¿½Ö¸ï¿½ï¿½ seÖ¸ï¿½ï¿½Ô½á¹¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?0ï¿½Å³ï¿½Ô± 
			}			
		}
		/*-----------------------------------------------------------------------------------------*/  
		/*                             ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ 2 ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½                             */
		/*-----------------------------------------------------------------------------------------*/
		if(U2_CB.RxOUT != U2_CB.RxIN){                                                                                    //´®¿Ú2½ÓÊÕ»º³å OUTÖ¸Õë ºÍ INÖ¸Õë²»µÈ £¬ÊÇ¿ÕÏÐ×´Ì¬£¬½øif£¬ËµÃ÷´®¿Ú2½ÓÊÕ»º³åÇøÓÐÊý¾Ý£¬ÐèÒª²é¿´		   					   						
			u1_printf("4G Cat1Ä£¿é±¾´Î½ÓÊÕ%d×Ö½ÚÊý¾Ý\r\n",U2_CB.RxOUT->e - U2_CB.RxOUT->s);                               //´òÓ¡4G Cat1±¾´Î½ÓÊÕµÄÊý¾Ý³¤¶È
			for(i=0;i<(U2_CB.RxOUT->e - U2_CB.RxOUT->s);i++)                                                              //Ñ­»·Êä³ö½ÓÊÕµÄÊý¾Ý
				u1_printf("%02x ",U2_CB.RxOUT->s[i]);                                                                     //16½øÖÆÊä³ö
			u1_printf("\r\n\r\n");                                                                                        //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?2ï¿½ï¿½ï¿½Ø³ï¿½ï¿½ï¿½ï¿½ï¿½
			if(SysCB.SysEventFlag&CONNECT_CAT1){                                                                          //ï¿½ï¿½ï¿½ï¿½ï¿½Ï·ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ifï¿½ï¿½Ö§
				Server_ProcessData(U2_CB.RxOUT->s,U2_CB.RxOUT->e - U2_CB.RxOUT->s);                                       //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Õµï¿½ï¿½Ä·ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
			}else{                                                                                                        //ï¿½ï¿½Î´ï¿½ï¿½ï¿½ï¿½ï¿½Ï·ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½elseï¿½ï¿½Ö§							
				Cat1_ProcessData(U2_CB.RxOUT->s,U2_CB.RxOUT->e - U2_CB.RxOUT->s);                                         //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Õµï¿½ï¿½ï¿½4G Cat1Ä£ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½					
			}
			U2_CB.RxOUT ++;                                                                                               //ï¿½ï¿½ï¿½ï¿½2ï¿½ï¿½ï¿½Õ»ï¿½ï¿½ï¿½ï¿½ï¿½ OUTÖ¸ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½Ô?
			if(U2_CB.RxOUT == &U2_CB.se_RxBuff[RX_NUM-1])                                                                 //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Æµï¿½seÖ¸ï¿½ï¿½Ô½á¹¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½Ô±Î»ï¿½Ã£ï¿½ï¿½ï¿½ï¿½ï¿½ifï¿½ï¿½ï¿½ï¿½Òªï¿½Ø¾ï¿½
				U2_CB.RxOUT = &U2_CB.se_RxBuff[0];                                                                        //ï¿½ï¿½ï¿½ï¿½2ï¿½ï¿½ï¿½Õ»ï¿½ï¿½ï¿½ï¿½ï¿½ OUTÖ¸ï¿½ï¿½Ö¸ï¿½ï¿½ seÖ¸ï¿½ï¿½Ô½á¹¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?0ï¿½Å³ï¿½Ô± 
		}
		/*-----------------------------------------------------------------------------------------*/  
		/*                              ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ 2 ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½                               */
		/*-----------------------------------------------------------------------------------------*/
		if((U2_CB.TxOUT != U2_CB.TxIN)&&(U2_CB.TxCpltflag == 0)){                                                         //´®¿Ú2·¢ËÍ»º³å OUTÖ¸Õë ºÍ INÖ¸Õë²»µÈ ÇÒ ·¢ËÍ¿ÕÏÐ£¨DMA·¢ËÍ¿ÕÏÐ£©£¬ËµÃ÷·¢ËÍ»º³åÇøÓÐÊý¾Ý£¬ÇÒ¿ÉÒÔ·¢ËÍ         		             					   						
		    u1_printf("4G Cat1Ä£¿é±¾´Î·¢ËÍ%d×Ö½ÚÊý¾Ý\r\n",U2_CB.TxOUT->e - U2_CB.TxOUT->s + 1);                           //´òÓ¡4G Cat1±¾´Î·¢ËÍµÄÊý¾Ý³¤¶È
			for(i=0;i<(U2_CB.TxOUT->e - U2_CB.TxOUT->s + 1);i++)                                                          //Ñ­»·Êä³ö·¢ËÍµÄÊý¾Ý
				u1_printf("%02x ",U2_CB.TxOUT->s[i]);                                                                     //16½øÖÆÊä³ö
			u1_printf("\r\n\r\n");                                                                                        //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?2ï¿½ï¿½ï¿½Ø³ï¿½ï¿½ï¿½ï¿½ï¿½
			if(HAL_UART_Transmit_DMA(&U2_CB.USART_Handler,U2_CB.TxOUT->s,U2_CB.TxOUT->e - U2_CB.TxOUT->s + 1)== HAL_OK){  //ï¿½ï¿½ï¿½ï¿½DMAï¿½ï¿½ï¿½Í£ï¿½ï¿½É¹ï¿½ï¿½ï¿½ï¿½ï¿½if				                                                                                                   //ï¿½ï¿½ï¿½ï¿½×´Ì¬,ï¿½ï¿½ï¿½ï¿½else
				U2_CB.TxCpltflag |= TX_STA_DMA;                                                                           //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½DMAï¿½ï¿½ï¿½ï¿½								
				U2_CB.TxOUT ++;                                                                                           //ï¿½ï¿½ï¿½ï¿½2ï¿½ï¿½ï¿½Í»ï¿½ï¿½ï¿½ï¿½ï¿½ OUTÖ¸ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½Ô?
				if(U2_CB.TxOUT == &U2_CB.se_TxBuff[TX_NUM-1])                                                             //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Æµï¿½seÖ¸ï¿½ï¿½Ô½á¹¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½Ô±Î»ï¿½Ã£ï¿½ï¿½ï¿½ï¿½ï¿½ifï¿½ï¿½ï¿½ï¿½Òªï¿½Ø¾ï¿½
					U2_CB.TxOUT = &U2_CB.se_TxBuff[0];                                                                    //ï¿½ï¿½ï¿½ï¿½2ï¿½ï¿½ï¿½Í»ï¿½ï¿½ï¿½ï¿½ï¿½ OUTÖ¸ï¿½ï¿½Ö¸ï¿½ï¿½ seÖ¸ï¿½ï¿½Ô½á¹¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?0ï¿½Å³ï¿½Ô± 
			}			
		}		
		/*-----------------------------------------------------------------------------------------*/  
		/*                             ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ 4 ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½                             */
		/*-----------------------------------------------------------------------------------------*/
		if(U4_CB.RxOUT != U4_CB.RxIN){                                                                                    //´®¿Ú4½ÓÊÕ»º³å OUTÖ¸Õë ºÍ INÖ¸Õë²»µÈ£¬½øif£¬ËµÃ÷´®¿Ú4½ÓÊÕ»º³åÇøÓÐÊý¾Ý£¬ÐèÒª²é¿´		   					   						
			u1_printf("WiFiÄ£¿é±¾´Î½ÓÊÕ%d×Ö½ÚÊý¾Ý\r\n",U4_CB.RxOUT->e - U4_CB.RxOUT->s);                                  //´òÓ¡WiFi±¾´Î½ÓÊÕµÄÊý¾Ý³¤¶È
			for(i=0;i<(U4_CB.RxOUT->e - U4_CB.RxOUT->s);i++)                                                              //Ñ­»·Êä³ö½ÓÊÕµÄÊý¾Ý
				u1_printf("%02x ",U4_CB.RxOUT->s[i]);                                                                     //16½øÖÆÊä³ö
			u1_printf("\r\n\r\n");                                                                                        //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?2ï¿½ï¿½ï¿½Ø³ï¿½ï¿½ï¿½ï¿½ï¿½
			if(SysCB.SysEventFlag&CONNECT_WIFI){                                                                          //ï¿½ï¿½ï¿½ï¿½ï¿½Ï·ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ifï¿½ï¿½Ö§
				Server_ProcessData(U4_CB.RxOUT->s,U4_CB.RxOUT->e - U4_CB.RxOUT->s);                                       //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Õµï¿½ï¿½Ä·ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½	
			}else{                                                                                                        //ï¿½ï¿½Î´ï¿½ï¿½ï¿½ï¿½ï¿½Ï·ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½elseï¿½ï¿½Ö§							
				WiFi_ProcessData(U4_CB.RxOUT->s,U4_CB.RxOUT->e - U4_CB.RxOUT->s);                                         //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Õµï¿½ï¿½ï¿½WiFiÄ£ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½					
			}
			U4_CB.RxOUT ++;                                                                                               //ï¿½ï¿½ï¿½ï¿½4ï¿½ï¿½ï¿½Õ»ï¿½ï¿½ï¿½ï¿½ï¿½ OUTÖ¸ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½Ô?
			if(U4_CB.RxOUT == &U4_CB.se_RxBuff[RX_NUM-1])                                                                 //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Æµï¿½seÖ¸ï¿½ï¿½Ô½á¹¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½Ô±Î»ï¿½Ã£ï¿½ï¿½ï¿½ï¿½ï¿½ifï¿½ï¿½ï¿½ï¿½Òªï¿½Ø¾ï¿½
				U4_CB.RxOUT = &U4_CB.se_RxBuff[0];                                                                        //ï¿½ï¿½ï¿½ï¿½4ï¿½ï¿½ï¿½Õ»ï¿½ï¿½ï¿½ï¿½ï¿½ OUTÖ¸ï¿½ï¿½Ö¸ï¿½ï¿½ seÖ¸ï¿½ï¿½Ô½á¹¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?0ï¿½Å³ï¿½Ô± 
		}
		/*-----------------------------------------------------------------------------------------*/  
		/*                              ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ 4 ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½                               */
		/*-----------------------------------------------------------------------------------------*/
		if((U4_CB.TxOUT != U4_CB.TxIN)&&(U4_CB.TxCpltflag == 0)){                                                         //´®¿Ú4·¢ËÍ»º³å OUTÖ¸Õë ºÍ INÖ¸Õë²»µÈ ÇÒ ·¢ËÍ¿ÕÏÐ£¨DMA·¢ËÍ¿ÕÏÐ£©£¬ËµÃ÷·¢ËÍ»º³åÇøÓÐÊý¾Ý£¬ÇÒ¿ÉÒÔ·¢ËÍ         		             					   						
		    u1_printf("WiFiÄ£¿é±¾´Î·¢ËÍ%d×Ö½ÚÊý¾Ý\r\n",U4_CB.TxOUT->e - U4_CB.TxOUT->s + 1);                              //´òÓ¡WiFi±¾´Î·¢ËÍµÄÊý¾Ý³¤¶È
			for(i=0;i<(U4_CB.TxOUT->e - U4_CB.TxOUT->s + 1);i++)                                                          //Ñ­»·Êä³ö·¢ËÍµÄÊý¾Ý
				u1_printf("%02x ",U4_CB.TxOUT->s[i]);                                                                     //16½øÖÆÊä³ö
			u1_printf("\r\n\r\n");                                                                                        //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?2ï¿½ï¿½ï¿½Ø³ï¿½ï¿½ï¿½ï¿½ï¿½
			if(HAL_UART_Transmit_DMA(&U4_CB.USART_Handler,U4_CB.TxOUT->s,U4_CB.TxOUT->e - U4_CB.TxOUT->s + 1)== HAL_OK){  //ï¿½ï¿½ï¿½ï¿½DMAï¿½ï¿½ï¿½Í£ï¿½ï¿½É¹ï¿½ï¿½ï¿½ï¿½ï¿½if
				U4_CB.TxCpltflag |= TX_STA_DMA;                                                                           //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½DMAï¿½ï¿½ï¿½ï¿½								
				U4_CB.TxOUT ++;                                                                                           //ï¿½ï¿½ï¿½ï¿½4ï¿½ï¿½ï¿½Í»ï¿½ï¿½ï¿½ï¿½ï¿½ OUTÖ¸ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½Ô?
				if(U4_CB.TxOUT == &U4_CB.se_TxBuff[TX_NUM-1])                                                             //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Æµï¿½seÖ¸ï¿½ï¿½Ô½á¹¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½Ô±Î»ï¿½Ã£ï¿½ï¿½ï¿½ï¿½ï¿½ifï¿½ï¿½ï¿½ï¿½Òªï¿½Ø¾ï¿½
					U4_CB.TxOUT = &U4_CB.se_TxBuff[0];                                                                    //ï¿½ï¿½ï¿½ï¿½4ï¿½ï¿½ï¿½Í»ï¿½ï¿½ï¿½ï¿½ï¿½ OUTÖ¸ï¿½ï¿½Ö¸ï¿½ï¿½ seÖ¸ï¿½ï¿½Ô½á¹¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?0ï¿½Å³ï¿½Ô± 
			}			
		}
		/*----------------------------------------------------------------------------------------*/
		/*                                LoRa Ä£ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½                                  */
		/*----------------------------------------------------------------------------------------*/	
		LoRa_ActiveEvent();                                                                                                                                    				
		/*----------------------------------------------------------------------------------------*/
		/*                                   LoRa è½®è¯¢é€»è¾‘                                         */
		/*----------------------------------------------------------------------------------------*/
		if(HAL_GetTick() - Poll_Timer >= 2000)
		{
			Poll_Timer = HAL_GetTick();
			LoRa_SendPoll(Poll_ID);
			Poll_ID++;
			if(Poll_ID > SUN_NUMBER) Poll_ID = 1;
		}		/*----------------------------------------------------------------------------------------*/
		/*                            WiFiÄ£ï¿½ï¿½ or 4G Cat1Ä£ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½                         */
		/*----------------------------------------------------------------------------------------*/			
		WiFi_Cat1_ActiveEvent();                                                                                              
	}
}
