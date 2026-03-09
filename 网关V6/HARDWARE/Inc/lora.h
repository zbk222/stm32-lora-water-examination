/*-------------------------------------------------*/
/*           ����˵������STM32ϵ�п�����           */
/*-------------------------------------------------*/
/*                                                 */
/*            ����LoRaģ�鹦�ܵ�ͷ�ļ�             */
/*                                                 */
/*-------------------------------------------------*/

#ifndef __LORA_H
#define __LORA_H

#include "stdint.h"    
#include "main.h"

typedef struct{    
	uint8_t LoRa_AddrH;         //ģ���ַ���ֽ�	      ��ַ0x00
	uint8_t LoRa_AddrL;         //ģ���ַ���ֽ�	      ��ַ0x01
	uint8_t LoRa_NetID;         //ģ�������ַ            ��ַ0x02
	
	uint8_t LoRa_Baudrate;      //ģ�鴮�ڲ�����          ��ַ0x03
	uint8_t LoRa_UartMode;      //ģ�鴮�ڹ���ģʽ        ��ַ0x03
	uint8_t LoRa_airvelocity;   //ģ���������            ��ַ0x03
	
	uint8_t LoRa_DataLen;       //���ݷְ���С            ��ַ0x04
	uint8_t LoRa_Rssi;          //��������ʹ��            ��ַ0x04
	uint8_t LoRa_Soft;          //ģ�鹤��ģʽ�����л�    ��ַ0x04
	uint8_t LoRa_TxPower;       //ģ�鷢�书��            ��ַ0x04
	
	uint8_t LoRa_CH;            //ģ���ŵ�                ��ַ0x05
	
	uint8_t LoRa_RssiByte;      //Rssi�ֽڹ���            ��ַ0x06
	uint8_t LoRa_DateMode;      //ģ�����ݴ���ģʽ        ��ַ0x06
	uint8_t LoRa_Relay;         //ģ���м̹���            ��ַ0x06
	uint8_t LoRa_LBT;           //ģ��LBT��������         ��ַ0x06
	uint8_t LoRa_WORmode;       //ģ��WORģʽ�¹�����ʽ   ��ַ0x06
	uint8_t LoRa_WORcycle;      //ģ��WORģʽ������ʱ��   ��ַ0x06

	uint8_t LoRa_KeyH;          //������Կ���ֽ�          ��ַ0x07
	uint8_t LoRa_KeyL;          //������Կ���ֽ�          ��ַ0x08
	
}LoRaParameter;

typedef struct{    
	uint8_t  sta;                        //LoRaģ��״̬����        0������״̬   1�����ݴ���״̬
	uint32_t timer;                      //��ȡ�ڵ㴫�������ݼ�ʱ����
	uint32_t counter;                    //��ȡ�ڵ�ļ�������
	uint32_t online[SUN_NUMBER+1];       //0������״̬  1������״̬
	uint32_t timeout[SUN_NUMBER+1];      //LoRa��ȡ���豸�����ۼƴ��������������������ظ����ݣ����϶����豸��������
	uint32_t SW_Sta[SUN_NUMBER+1];       //�����ڵ�忪��״̬

}LoRaCB;                                 //LoRaģ����ƽṹ��

#define LoRa_MODE0  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET) //LoRaģ�� ģʽ0
#define LoRa_MODE1  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET)   //LoRaģ�� ģʽ1
#define LoRa_MODE2  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET)   //LoRaģ�� ģʽ2
#define LoRa_MODE3  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET)     //LoRaģ�� ģʽ3

#define LoRa_AUX          HAL_GPIO_ReadPin(GPIOB,  GPIO_PIN_7)                  //��ȡ��ƽ״̬,�ж�ģ��״̬
#define LoRa_PowerON      HAL_GPIO_WritePin(GPIOF, GPIO_PIN_1, GPIO_PIN_SET)    //����loRaģ�鹩��,�ߵ�ƽON
#define LoRa_PowerOFF     HAL_GPIO_WritePin(GPIOF, GPIO_PIN_1, GPIO_PIN_RESET)  //����loRaģ�鹩��,�͵�ƽOFF
 
#define LoRa_1200         0x00   //ģ�鴮�� ������ 1200
#define LoRa_2400         0x20   //ģ�鴮�� ������ 2400
#define LoRa_4800         0x40   //ģ�鴮�� ������ 4800
#define LoRa_9600         0x60   //ģ�鴮�� ������ 9600
#define LoRa_19200        0x80   //ģ�鴮�� ������ 19200
#define LoRa_38400        0xA0   //ģ�鴮�� ������ 38400
#define LoRa_57600        0xC0   //ģ�鴮�� ������ 57600
#define LoRa_115200       0xE0   //ģ�鴮�� ������ 115200

#define LoRa_8N1          0x00   //ģ�鴮�ڲ��� 8����λ ��У�� 1ֹͣλ
#define LoRa_8O1          0x08   //ģ�鴮�ڲ��� 8����λ ��У�� 1ֹͣλ
#define LoRa_8E1          0x10   //ģ�鴮�ڲ��� 8����λ żУ�� 1ֹͣλ

#define LoRa_2_4          0x02   //ģ��������� 2.4K
#define LoRa_4_8          0x03   //ģ��������� 4.8K
#define LoRa_9_6          0x04   //ģ��������� 9.6K
#define LoRa_19_2         0x05   //ģ��������� 19.2K
#define LoRa_38_4         0x06   //ģ��������� 38.4K
#define LoRa_62_5         0x07   //ģ��������� 62.5K

#define LoRa_Data240      0x00   //���ݷְ���С 240�ֽ�
#define LoRa_Data128      0x40   //���ݷְ���С 128�ֽ�
#define LoRa_Data64       0x80   //���ݷְ���С 64�ֽ�
#define LoRa_Data32       0xC0   //���ݷְ���С 32�ֽ�

#define LoRa_RssiEN       0x20   //����RSSI����
#define LoRa_RssiDIS      0x00   //����RSSI����

#define LoRa_SoftEN       0x10   //���������л�ģ�鹤��ģʽ����
#define LoRa_SoftDIS      0x00   //���������л�ģ�鹤��ģʽ����

#define LoRa_FEC_22DBM    0x00   //���书��22dbm
#define LoRa_FEC_17DBM    0x01   //���书��17dbm
#define LoRa_FEC_13DBM    0x02   //���书��13dbm
#define LoRa_FEC_10DBM    0x03   //���书��10dbm

#define LoRa_CH0          0x00   //ģ���ŵ� ��ӦƵ��410MHz
#define LoRa_CH1          0x01   //ģ���ŵ� ��ӦƵ��411MHz
#define LoRa_CH2          0x02   //ģ���ŵ� ��ӦƵ��412MHz
#define LoRa_CH3          0x03   //ģ���ŵ� ��ӦƵ��413MHz
#define LoRa_CH4          0x04   //ģ���ŵ� ��ӦƵ��414MHz
#define LoRa_CH5          0x05   //ģ���ŵ� ��ӦƵ��415MHz
#define LoRa_CH6          0x06   //ģ���ŵ� ��ӦƵ��416MHz
#define LoRa_CH7          0x07   //ģ���ŵ� ��ӦƵ��417MHz
#define LoRa_CH8          0x08   //ģ���ŵ� ��ӦƵ��418MHz
#define LoRa_CH9          0x09   //ģ���ŵ� ��ӦƵ��419MHz
#define LoRa_CH10         0x0A   //ģ���ŵ� ��ӦƵ��420MHz
#define LoRa_CH11         0x0B   //ģ���ŵ� ��ӦƵ��421MHz
#define LoRa_CH12         0x0C   //ģ���ŵ� ��ӦƵ��422MHz
#define LoRa_CH13         0x0D   //ģ���ŵ� ��ӦƵ��423MHz
#define LoRa_CH14         0x0E   //ģ���ŵ� ��ӦƵ��424MHz
#define LoRa_CH15         0x0F   //ģ���ŵ� ��ӦƵ��425MHz
#define LoRa_CH16         0x10   //ģ���ŵ� ��ӦƵ��426MHz
#define LoRa_CH17         0x11   //ģ���ŵ� ��ӦƵ��427MHz
#define LoRa_CH18         0x12   //ģ���ŵ� ��ӦƵ��428MHz
#define LoRa_CH19         0x13   //ģ���ŵ� ��ӦƵ��429MHz
#define LoRa_CH20         0x14   //ģ���ŵ� ��ӦƵ��430MHz
#define LoRa_CH21         0x15   //ģ���ŵ� ��ӦƵ��431MHz
#define LoRa_CH22         0x16   //ģ���ŵ� ��ӦƵ��432MHz
#define LoRa_CH23         0x17   //ģ���ŵ� ��ӦƵ��433MHz
#define LoRa_CH24         0x18   //ģ���ŵ� ��ӦƵ��434MHz
#define LoRa_CH25         0x19   //ģ���ŵ� ��ӦƵ��435MHz
#define LoRa_CH26         0x1A   //ģ���ŵ� ��ӦƵ��436MHz
#define LoRa_CH27         0x1B   //ģ���ŵ� ��ӦƵ��437MHz
#define LoRa_CH28         0x1C   //ģ���ŵ� ��ӦƵ��438MHz
#define LoRa_CH29         0x1D   //ģ���ŵ� ��ӦƵ��439MHz
#define LoRa_CH30         0x1E   //ģ���ŵ� ��ӦƵ��440MHz
#define LoRa_CH31         0x1F   //ģ���ŵ� ��ӦƵ��441MHz
#define LoRa_CH32         0x20   //ģ���ŵ� ��ӦƵ��442MHz
#define LoRa_CH33         0x21   //ģ���ŵ� ��ӦƵ��442MHz
#define LoRa_CH34         0x22   //ģ���ŵ� ��ӦƵ��444MHz
#define LoRa_CH35         0x23   //ģ���ŵ� ��ӦƵ��445MHz
#define LoRa_CH36         0x24   //ģ���ŵ� ��ӦƵ��446MHz
#define LoRa_CH37         0x25   //ģ���ŵ� ��ӦƵ��447MHz
#define LoRa_CH38         0x26   //ģ���ŵ� ��ӦƵ��448MHz
#define LoRa_CH39         0x27   //ģ���ŵ� ��ӦƵ��449MHz
#define LoRa_CH40         0x28   //ģ���ŵ� ��ӦƵ��450MHz
#define LoRa_CH41         0x29   //ģ���ŵ� ��ӦƵ��451MHz
#define LoRa_CH42         0x2A   //ģ���ŵ� ��ӦƵ��452MHz
#define LoRa_CH43         0x2B   //ģ���ŵ� ��ӦƵ��453MHz
#define LoRa_CH44         0x2C   //ģ���ŵ� ��ӦƵ��454MHz
#define LoRa_CH45         0x2D   //ģ���ŵ� ��ӦƵ��455MHz
#define LoRa_CH46         0x2E   //ģ���ŵ� ��ӦƵ��456MHz
#define LoRa_CH47         0x2F   //ģ���ŵ� ��ӦƵ��457MHz
#define LoRa_CH48         0x30   //ģ���ŵ� ��ӦƵ��458MHz
#define LoRa_CH49         0x31   //ģ���ŵ� ��ӦƵ��459MHz
#define LoRa_CH50         0x32   //ģ���ŵ� ��ӦƵ��460MHz
#define LoRa_CH51         0x33   //ģ���ŵ� ��ӦƵ��461MHz
#define LoRa_CH52         0x34   //ģ���ŵ� ��ӦƵ��462MHz
#define LoRa_CH53         0x35   //ģ���ŵ� ��ӦƵ��463MHz
#define LoRa_CH54         0x36   //ģ���ŵ� ��ӦƵ��464MHz
#define LoRa_CH55         0x37   //ģ���ŵ� ��ӦƵ��465MHz
#define LoRa_CH56         0x38   //ģ���ŵ� ��ӦƵ��466MHz
#define LoRa_CH57         0x39   //ģ���ŵ� ��ӦƵ��467MHz
#define LoRa_CH58         0x3A   //ģ���ŵ� ��ӦƵ��468MHz
#define LoRa_CH59         0x3B   //ģ���ŵ� ��ӦƵ��469MHz
#define LoRa_CH60         0x3C   //ģ���ŵ� ��ӦƵ��470MHz
#define LoRa_CH61         0x3D   //ģ���ŵ� ��ӦƵ��471MHz
#define LoRa_CH62         0x3E   //ģ���ŵ� ��ӦƵ��472MHz
#define LoRa_CH63         0x3F   //ģ���ŵ� ��ӦƵ��473MHz
#define LoRa_CH64         0x40   //ģ���ŵ� ��ӦƵ��474MHz
#define LoRa_CH65         0x41   //ģ���ŵ� ��ӦƵ��475MHz
#define LoRa_CH66         0x42   //ģ���ŵ� ��ӦƵ��476MHz
#define LoRa_CH67         0x43   //ģ���ŵ� ��ӦƵ��477MHz
#define LoRa_CH68         0x44   //ģ���ŵ� ��ӦƵ��478MHz
#define LoRa_CH69         0x45   //ģ���ŵ� ��ӦƵ��479MHz
#define LoRa_CH70         0x46   //ģ���ŵ� ��ӦƵ��480MHz
#define LoRa_CH71         0x47   //ģ���ŵ� ��ӦƵ��481MHz
#define LoRa_CH72         0x48   //ģ���ŵ� ��ӦƵ��482MHz
#define LoRa_CH73         0x49   //ģ���ŵ� ��ӦƵ��483MHz
#define LoRa_CH74         0x4A   //ģ���ŵ� ��ӦƵ��484MHz
#define LoRa_CH75         0x4B   //ģ���ŵ� ��ӦƵ��485MHz
#define LoRa_CH76         0x4C   //ģ���ŵ� ��ӦƵ��486MHz
#define LoRa_CH77         0x4D   //ģ���ŵ� ��ӦƵ��487MHz
#define LoRa_CH78         0x4E   //ģ���ŵ� ��ӦƵ��488MHz
#define LoRa_CH79         0x4F   //ģ���ŵ� ��ӦƵ��489MHz
#define LoRa_CH80         0x50   //ģ���ŵ� ��ӦƵ��490MHz
#define LoRa_CH81         0x51   //ģ���ŵ� ��ӦƵ��491MHz
#define LoRa_CH82         0x52   //ģ���ŵ� ��ӦƵ��492MHz
#define LoRa_CH83         0x53   //ģ���ŵ� ��ӦƵ��493MHz

#define LoRa_RssiByteEN   0x80   //����RSSI�ֽڹ���
#define LoRa_RssiByteDIS  0x00   //����RSSI�ֽڹ���

#define LoRa_ModeTRANS    0x00   //ģ��͸������
#define LoRa_ModePOINT    0x40   //ģ�鶨�㴫��

#define LoRa_RelayEN      0x20   //�����м�
#define LoRa_RelayDIS     0x00   //�����м�

#define LoRa_LBTEN        0x10   //����LBT
#define LoRa_LBTDIS       0x00   //����LBT

#define LoRa_WorTX        0x08   //Worģʽ����
#define LoRa_WorRX        0x00   //Worģʽ����

#define LoRa_Wor500ms     0x00   //Wor����500����
#define LoRa_Wor1000ms    0x01   //Wor����1000����
#define LoRa_Wor1500ms    0x02   //Wor����1500����
#define LoRa_Wor2000ms    0x03   //Wor����2000����
#define LoRa_Wor2500ms    0x04   //Wor����2500����
#define LoRa_Wor3000ms    0x05   //Wor����3000����
#define LoRa_Wor3500ms    0x06   //Wor����3500����
#define LoRa_Wor4000ms    0x07   //Wor����4000����

#define TEMP_REGISTER     0x0300  //�¶����ݼĴ���
#define HUMI_REGISTER     0x0301  //ʪ�����ݼĴ���
#define LIHGHT_REGISTER   0x0302  //��ǿ�����ݼĴ���
#define LED_REGISTER      0x0303  //LED���ݼĴ���
#define ADC1_REGISTER     0x0304  //ADC1���ݼĴ���
#define ADC2_REGISTER     0x0305  //ADC2���ݼĴ���
#define ADC3_REGISTER     0x0306  //ADC3���ݼĴ���

extern LoRaCB lora;                               //�ⲿ����������LoRaģ����ƽṹ�� 

void LoRa_Init(void);         	                  //������������ʼ��LoRaģ��
void LoRa_ConfigData(uint8_t *, uint16_t);        //��������������LoRa����״̬������  
void LoRa_TransData(uint8_t *, uint16_t);         //��������������LoRa����״̬������
void LoRa_ActiveEvent(void);                      //����������LoRaģ�������¼� 

void LoRa_SendPoll(uint8_t id);

#endif


