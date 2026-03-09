#ifndef  __MQTT_
#define  __MQTT_

#include "stdint.h"

#define PARA_SIZE  64
#define BUFF_SIZE  512
#define TOPIC_SIZE 256
#define DATR_SIZE  256

typedef struct{
	unsigned int Fixed_len;       //固定报头长度 
	unsigned int Variable_len;    //可变报头长度 
	unsigned int Payload_len;     //负载长度 
	unsigned int Reamining_len;   //剩余长度   
	unsigned int MessageID;       //报文标识符 
    unsigned int lenth;           //报文整体长度 
	unsigned char buff[BUFF_SIZE];//数据缓冲区
	char ClientID[PARA_SIZE];     //参数缓冲区 
	char UserName[PARA_SIZE];     //参数缓冲区 
	char PassWard[PARA_SIZE];     //参数缓冲区 
	char WillTopic[PARA_SIZE];    //遗嘱主题缓冲区 
	char WillData[PARA_SIZE];     //遗嘱数据缓冲区
	char topic[PARA_SIZE];        //当我们接收到服务器推送的3号报文，提取topic的缓冲区 
	unsigned char data[DATR_SIZE];//当我们接收到服务器推送的3号报文，提取data的缓冲区 	 
}MQTT_CB;

typedef struct{
	char decodekey[128];               //对设备密钥进行base64解码后的结果
	char StringForSignature[256];      //进行StringForSignature字符串的建立，结果作为明文
	char signtemp[128];                //sign计算的临时缓冲区
	char sign[128];                    //sign的最终结果
	char res[128];                     //存放res的字符串
	char signURL[128];                 //sign进行URL编码后的结果
	char resURL[128];                  //res进行URL编码后的结果
}Token_CB;

extern MQTT_CB mqtt;                 
extern char TopicBuff[5][128];         //外部变量声明，二维数组，存放需要订阅的topic字符串
extern char TopicNum;                  //外部变量声明，需要订阅的数据topic数量

void URL_encode(char *data, int data_len, char *outdata);
void MQTT_Init(void);
void MQTT_Connect(unsigned int keepalive);
void MQTT_ConnectWill(unsigned int keepalive, unsigned char willretain, unsigned char willqs, unsigned char clean);
int8_t MQTT_CONNACK(unsigned char *rxdata, unsigned int rxdata_len);
void MQTT_DISCONNECT(void);
void MQTT_SUBSCRIBE(char *topic, char Qs);
int8_t MQTT_SUBACK(unsigned char *rxdata, unsigned int rxdata_len);
void MQTT_UNSUBSCRIBE(char *topic);
int8_t MQTT_UNSUBACK(unsigned char *rxdata, unsigned int rxdata_len);
void MQTT_PINGREQ(void);
int8_t MQTT_PINGRESP(unsigned char *rxdata, unsigned int rxdata_len);
void MQTT_PUBLISH0(char retain,char *topic, unsigned char *data, unsigned int data_len);
void MQTT_PUBLISH1(char dup, char retain,char *topic, unsigned char *data, unsigned int data_len);
void MQTT_PUBLISH2(char dup, char retain,char *topic, unsigned char *data, unsigned int data_len);
int8_t MQTT_ProcessPUBLISH(unsigned char *rxdata, unsigned int rxdata_len, unsigned char *qs, unsigned int *messageid);
void MQTT_PUBACK(unsigned int messageid);
int8_t MQTT_ProcessPUBACK(unsigned char *rxdata, unsigned int rxdata_len, unsigned int *messageid);
void MQTT_PUBREC(unsigned int messageid);
int8_t MQTT_ProcessPUBREC(unsigned char *rxdata, unsigned int rxdata_len, unsigned int *messageid);
void MQTT_PUBREL(unsigned int messageid);
int8_t MQTT_ProcessPUBREL(unsigned char *rxdata, unsigned int rxdata_len, unsigned int *messageid);
void MQTT_PUBCOMP(unsigned int messageid);
int8_t MQTT_ProcessPUBCOMP(unsigned char *rxdata, unsigned int rxdata_len, unsigned int *messageid);

#endif
