#include "stm32g0xx_hal.h"   
#include "main.h"
#include "mqtt.h"
#include "string.h"
#include "stdio.h"
#include "utils_hmac.h"

MQTT_CB mqtt;                //mqtt结构体
Token_CB token;              //签名加密时需要用的数组缓冲区
char TopicBuff[5][128];      //二维数组，存放需要订阅的topic字符串
char TopicNum;               //需要订阅的数据topic数量
char tempdata[8]={'+',' ','/','?','%','#','&','='};
char URLdata[8][3]={"%2B","%20","%2F","%3F","%25","%23","%26","%3D"};

/*-------------------------------------------------*/
/*函数名：URL编码转换                              */
/*参  数：data：需要转换的数据                     */
/*参  数：ddata_len：需要转换的数据长度            */
/*参  数：outdata：转换后的数据                    */
/*返回值：无                                       */
/*-------------------------------------------------*/
void URL_encode(char *data, int data_len, char *outdata)
{
	int i,j,k;
	
	i=j=k=0;
	
	for(i=0;i<data_len;i++){                  //轮询每个字节
		for(j=0;j<8;j++){                     //每个字节对比8次
			if(data[i] == tempdata[j]){       //如果是特殊字符，进入if
				break;                        //主动跳出for
			}
		}
		if(j<8){                              //是特殊字符
			memcpy(&outdata[k],URLdata[j],3);
			k+=3;
		}else{                                //不是特殊字符
			outdata[k] = data[i];
			k++;
		}
	}
}
/*-------------------------------------------------*/
/*函数名：初始化MQTT登录参数                       */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void MQTT_Init(void)
{
	memset(&mqtt,0,sizeof(mqtt));
	sprintf(mqtt.ClientID,GW_DEVICENAME);
	sprintf(mqtt.UserName,GW_PRODUCTID);

	base64_decode(GW_DEVICESECRET,(unsigned char *)token.decodekey);
	sprintf(token.StringForSignature,"%s\nsha1\nproducts/%s/devices/%s\n2018-10-31",UNIX,GW_PRODUCTID,GW_DEVICENAME);
	utils_hmac_sha1_hex(token.StringForSignature,strlen(token.StringForSignature),token.signtemp,token.decodekey,strlen(token.decodekey));
	base64_encode((unsigned char *)token.signtemp,token.sign,strlen(token.signtemp));
	sprintf(token.res,"products/%s/devices/%s",GW_PRODUCTID,GW_DEVICENAME);
	URL_encode(token.sign,strlen(token.sign),token.signURL);
	URL_encode(token.res,strlen(token.res),token.resURL);
	sprintf(mqtt.PassWard,"version=2018-10-31&res=%s&et=%s&method=sha1&sign=%s",token.resURL,UNIX,token.signURL);
	mqtt.MessageID = 1;
	
	sprintf(TopicBuff[0],"$sys/%s/%s/thing/property/set",GW_PRODUCTID,GW_DEVICENAME);            
	sprintf(TopicBuff[1],"$sys/%s/%s/thing/sub/login/reply",GW_PRODUCTID,GW_DEVICENAME);      
	sprintf(TopicBuff[2],"$sys/%s/%s/thing/sub/logout/reply",GW_PRODUCTID,GW_DEVICENAME);
	sprintf(TopicBuff[3],"$sys/%s/%s/thing/pack/post/reply",GW_PRODUCTID,GW_DEVICENAME);         
	sprintf(TopicBuff[4],"$sys/%s/%s/thing/sub/property/set",GW_PRODUCTID,GW_DEVICENAME);         
	TopicNum = 5;                                   
} 
/*-------------------------------------------------*/
/*函数名：1号Connect报文                           */
/*参  数：keepalive：保活时长                      */
/*返回值：无                                       */
/*-------------------------------------------------*/
void MQTT_Connect(unsigned int keepalive)
{
	mqtt.Fixed_len = 1;                                                                                                    //固定报头中第一个字节0x10可以确定，长度当前是1
	mqtt.Variable_len = 10;                                                                                                //可变报头长度固定是10字节
	mqtt.Payload_len = 2 + strlen(mqtt.ClientID) + 2 + strlen(mqtt.UserName) + 2 + strlen(mqtt.PassWard);                  //计算负载长度
	mqtt.Reamining_len = mqtt.Variable_len + mqtt.Payload_len;                                                             //计算剩余长度
	
	mqtt.buff[0] = 0x10;                                                                                                   //固定报头中第一个字节0x10
	do{                                                                                                                    //根据计算好的剩余长度，循环构建各个字节
		if(mqtt.Reamining_len/128 == 0){                                                                                   //不需要再向前进位 
			mqtt.buff[mqtt.Fixed_len] = mqtt.Reamining_len;                                                                //设置相应值
		}else{                                                                                                             //需要再向前进位
			mqtt.buff[mqtt.Fixed_len] = (mqtt.Reamining_len%128)|0x80;                                                     //设设置相应值 且 BIT7置位为1表示需要进位
		} 
		mqtt.Fixed_len++;                                                                                                  //固定报头长度+1
		mqtt.Reamining_len = mqtt.Reamining_len/128;                                                                       //计算剩余长度剩余值
	}while(mqtt.Reamining_len);	                                                                                           //剩余长度计算到0后，退出while           

	mqtt.buff[mqtt.Fixed_len] = 0x00;                                                                                       //可变报头第1个字节
	mqtt.buff[mqtt.Fixed_len+1] = 0x04;                                                                                     //可变报头第2个字节
	mqtt.buff[mqtt.Fixed_len+2] = 0x4D;                                                                                     //可变报头第3个字节
	mqtt.buff[mqtt.Fixed_len+3] = 0x51;                                                                                     //可变报头第4个字节
	mqtt.buff[mqtt.Fixed_len+4] = 0x54;                                                                                     //可变报头第5个字节
	mqtt.buff[mqtt.Fixed_len+5] = 0x54;                                                                                     //可变报头第6个字节
	mqtt.buff[mqtt.Fixed_len+6] = 0x04;                                                                                     //可变报头第7个字节
	mqtt.buff[mqtt.Fixed_len+7] = 0xC2;                                                                                     //可变报头第8个字节
	mqtt.buff[mqtt.Fixed_len+8] = keepalive/256;                                                                            //可变报头第9个字节
	mqtt.buff[mqtt.Fixed_len+9] = keepalive%256;                                                                            //可变报头第10个字节
	
	mqtt.buff[mqtt.Fixed_len+10] = strlen(mqtt.ClientID)/256;                                                               //负载中ClientID字符串长度 高字节
	mqtt.buff[mqtt.Fixed_len+11] = strlen(mqtt.ClientID)%256;                                                               //负载中ClientID字符串长度 低字节
	memcpy(&mqtt.buff[mqtt.Fixed_len+12],mqtt.ClientID,strlen(mqtt.ClientID));                                              //添加ClientID字符串
	
	mqtt.buff[mqtt.Fixed_len+12+strlen(mqtt.ClientID)] = strlen(mqtt.UserName)/256;                                         //负载中UserName字符串长度 高字节
	mqtt.buff[mqtt.Fixed_len+13+strlen(mqtt.ClientID)] = strlen(mqtt.UserName)%256;                                         //负载中UserName字符串长度 低字节
	memcpy(&mqtt.buff[mqtt.Fixed_len+14+strlen(mqtt.ClientID)],mqtt.UserName,strlen(mqtt.UserName));	                    //添加UserName字符串
	
	mqtt.buff[mqtt.Fixed_len+14+strlen(mqtt.ClientID)+strlen(mqtt.UserName)] = strlen(mqtt.PassWard)/256;                   //负载中PassWard字符串长度 高字节  
	mqtt.buff[mqtt.Fixed_len+15+strlen(mqtt.ClientID)+strlen(mqtt.UserName)] = strlen(mqtt.PassWard)%256;                   //负载中PassWard字符串长度 低字节
	memcpy(&mqtt.buff[mqtt.Fixed_len+16+strlen(mqtt.ClientID)+strlen(mqtt.UserName)],mqtt.PassWard,strlen(mqtt.PassWard));	//添加PassWard字符串
	
	mqtt.lenth = mqtt.Fixed_len + mqtt.Variable_len + mqtt.Payload_len;                                                     //计算整个报文长度 
}  
/*-------------------------------------------------*/
/*函数名：1号Connect报文带遗嘱功能                 */
/*参  数：keepalive：保活时长                      */
/*参  数：willretain：遗嘱保留功能                 */
/*参  数：willqs：遗嘱服务质量等级                 */
/*参  数：clean：清理会话功能                      */
/*返回值：无                                       */
/*-------------------------------------------------*/
void MQTT_ConnectWill(unsigned int keepalive, unsigned char willretain, unsigned char willqs, unsigned char clean)
{
	mqtt.Fixed_len = 1;                                                                                                                                           //固定报头中第一个字节0x10可以确定，长度当前是1
	mqtt.Variable_len = 10;                                                                                                                                       //可变报头长度固定是10字节
	mqtt.Payload_len = 2 + strlen(mqtt.ClientID) + 2 + strlen(mqtt.WillTopic) + 2 + strlen(mqtt.WillData) + 2 + strlen(mqtt.UserName) + 2 + strlen(mqtt.PassWard);//计算负载长度
	mqtt.Reamining_len = mqtt.Variable_len + mqtt.Payload_len;                                                                                                    //计算剩余长度
	
	mqtt.buff[0] = 0x10;                                                                                                                                          //固定报头中第一个字节0x10
	do{                                                                                                                                                           //根据计算好的剩余长度，循环构建各个字节
		if(mqtt.Reamining_len/128 == 0){                                                                                                                          //不需要再向前进位 
			mqtt.buff[mqtt.Fixed_len] = mqtt.Reamining_len;                                                                                                       //设置相应值
		}else{                                                                                                                                                    //需要再向前进位
			mqtt.buff[mqtt.Fixed_len] = (mqtt.Reamining_len%128)|0x80;                                                                                            //设设置相应值 且 BIT7置位为1表示需要进位
		} 
		mqtt.Fixed_len++;                                                                                                                                         //固定报头长度+1
		mqtt.Reamining_len = mqtt.Reamining_len/128;                                                                                                              //计算剩余长度剩余值
	}while(mqtt.Reamining_len);	                                                                                                                                  //剩余长度计算到0后，退出while           

	mqtt.buff[mqtt.Fixed_len] = 0x00;                                                                                                                             //可变报头第1个字节
	mqtt.buff[mqtt.Fixed_len+1] = 0x04;                                                                                                                           //可变报头第2个字节
	mqtt.buff[mqtt.Fixed_len+2] = 0x4D;                                                                                                                           //可变报头第3个字节
	mqtt.buff[mqtt.Fixed_len+3] = 0x51;                                                                                                                           //可变报头第4个字节
	mqtt.buff[mqtt.Fixed_len+4] = 0x54;                                                                                                                           //可变报头第5个字节
	mqtt.buff[mqtt.Fixed_len+5] = 0x54;                                                                                                                           //可变报头第6个字节
	mqtt.buff[mqtt.Fixed_len+6] = 0x04;                                                                                                                           //可变报头第7个字节
	mqtt.buff[mqtt.Fixed_len+7] = 0xC4|(willretain<<5)|(willqs<<3)|(clean<<1);                                                                                    //可变报头第8个字节
	mqtt.buff[mqtt.Fixed_len+8] = keepalive/256;                                                                                                                  //可变报头第9个字节
	mqtt.buff[mqtt.Fixed_len+9] = keepalive%256;                                                                                                                  //可变报头第10个字节
	
	mqtt.buff[mqtt.Fixed_len+10] = strlen(mqtt.ClientID)/256;                                                                                                     //负载中ClientID字符串长度 高字节
	mqtt.buff[mqtt.Fixed_len+11] = strlen(mqtt.ClientID)%256;                                                                                                     //负载中ClientID字符串长度 低字节
	memcpy(&mqtt.buff[mqtt.Fixed_len+12],mqtt.ClientID,strlen(mqtt.ClientID));                                                                                    //添加ClientID字符串
	
	mqtt.buff[mqtt.Fixed_len+12+strlen(mqtt.ClientID)] = strlen(mqtt.WillTopic)/256;                                                                              //负载中WillTopic字符串长度 高字节
	mqtt.buff[mqtt.Fixed_len+13+strlen(mqtt.ClientID)] = strlen(mqtt.WillTopic)%256;                                                                              //负载中WillTopic字符串长度 低字节
	memcpy(&mqtt.buff[mqtt.Fixed_len+14+strlen(mqtt.ClientID)],mqtt.WillTopic,strlen(mqtt.WillTopic));                                                            //添加WillTopic字符串
	
	
	mqtt.buff[mqtt.Fixed_len+14+strlen(mqtt.ClientID)+strlen(mqtt.WillTopic)] = strlen(mqtt.WillData)/256;                                                        //负载中WillData字符串长度 高字节
	mqtt.buff[mqtt.Fixed_len+15+strlen(mqtt.ClientID)+strlen(mqtt.WillTopic)] = strlen(mqtt.WillData)%256;                                                        //负载中WillData字符串长度 低字节
	memcpy(&mqtt.buff[mqtt.Fixed_len+16+strlen(mqtt.ClientID)+strlen(mqtt.WillTopic)],mqtt.WillData,strlen(mqtt.WillData));                                       //添加WillData字符串
		
	mqtt.buff[mqtt.Fixed_len+16+strlen(mqtt.ClientID)+strlen(mqtt.WillTopic)+strlen(mqtt.WillData)] = strlen(mqtt.UserName)/256;                                  //负载中UserName字符串长度 高字节
	mqtt.buff[mqtt.Fixed_len+17+strlen(mqtt.ClientID)+strlen(mqtt.WillTopic)+strlen(mqtt.WillData)] = strlen(mqtt.UserName)%256;                                  //负载中UserName字符串长度 低字节
	memcpy(&mqtt.buff[mqtt.Fixed_len+18+strlen(mqtt.ClientID)+strlen(mqtt.WillTopic)+strlen(mqtt.WillData)],mqtt.UserName,strlen(mqtt.UserName));                 //添加UserName字符串	
	
	mqtt.buff[mqtt.Fixed_len+18+strlen(mqtt.ClientID)+strlen(mqtt.WillTopic)+strlen(mqtt.WillData)+strlen(mqtt.UserName)] = strlen(mqtt.PassWard)/256;            //负载中PassWard字符串长度 高字节
	mqtt.buff[mqtt.Fixed_len+19+strlen(mqtt.ClientID)+strlen(mqtt.WillTopic)+strlen(mqtt.WillData)+strlen(mqtt.UserName)] = strlen(mqtt.PassWard)%256;            //负载中PassWard字符串长度 低字节
	memcpy(&mqtt.buff[mqtt.Fixed_len+20+strlen(mqtt.ClientID)+strlen(mqtt.WillTopic)+strlen(mqtt.WillData)+strlen(mqtt.UserName)],mqtt.PassWard,strlen(mqtt.PassWard));//负载中PassWard字符串长度 高字节	
	
	mqtt.lenth = mqtt.Fixed_len + mqtt.Variable_len + mqtt.Payload_len;                                                                                           //计算整个报文长度 
} 
/*---------------------------------------------------------*/
/*函数名：处理2号CONNACK报文                               */
/*参  数：rxdata：接收的数据                               */
/*参  数：rxdata_len：接收的数据长度                       */
/*返回值：-1：不是2号CONNACK报文  rxdata[3]：返回码        */
/*---------------------------------------------------------*/
int8_t MQTT_CONNACK(unsigned char *rxdata, unsigned int rxdata_len)
{
	if((rxdata_len == 4)&&(rxdata[0] == 0x20)){      //如果接收4字节数据 且 第一个字节是0x20，是2号CONNACK报文，进入if分支
		
	}else{                                           //反之，进入else分支
		return -1;                                   //返回-1，不是2号CONNACK报文
	}
	
	return rxdata[3];                                //返回rxdata[3]，2号CONNACK报文的返回码 
}
/*---------------------------------------------------------*/
/*函数名：14号DISCONNECT报文                               */
/*参  数：无                                               */
/*返回值：无                                               */
/*---------------------------------------------------------*/
void MQTT_DISCONNECT(void)
{
	mqtt.buff[0] = 0xE0;    //14号报文固定报头第1个字节
	mqtt.buff[1] = 0x00;    //14号报文固定报头第2个字节
	mqtt.lenth = 2;         //14号报文长度2
}
/*---------------------------------------------------------*/
/*函数名：8号SUBSCRIBE报文                                 */
/*参  数：topic：需要订阅的 topic                          */
/*参  数：topic：需要订阅的 topic等级                      */
/*返回值：无                                               */
/*---------------------------------------------------------*/
void MQTT_SUBSCRIBE(char *topic, char Qs)
{
	mqtt.Fixed_len = 1;                                                    //固定报头中第一个字节0x82可以确定，长度当前是1
	mqtt.Variable_len = 2;                                                 //可变报头长度固定2字节长
	mqtt.Payload_len = 2 + strlen(topic) + 1;                              //计算负载长度
	mqtt.Reamining_len = mqtt.Variable_len + mqtt.Payload_len;             //计算剩余长度
	
	mqtt.buff[0] = 0x82;                                                   //固定报头中第一个字节0x82
	do{                                                                    //根据计算好的剩余长度，循环构建各个字节
		if(mqtt.Reamining_len/128 == 0){                                   //不需要再向前进位 
			mqtt.buff[mqtt.Fixed_len] = mqtt.Reamining_len;                //设置相应值
		}else{                                                             //需要再向前进位
			mqtt.buff[mqtt.Fixed_len] = (mqtt.Reamining_len%128)|0x80;     //设设置相应值 且 BIT7置位为1表示需要进位
		} 
		mqtt.Fixed_len++;                                                  //固定报头长度+1
		mqtt.Reamining_len = mqtt.Reamining_len/128;                       //计算剩余长度剩余值
	}while(mqtt.Reamining_len);	                                           //剩余长度计算到0后，退出while           

	mqtt.buff[mqtt.Fixed_len] = mqtt.MessageID/256;                        //可变报头第1个字节，报文标识符高字节
	mqtt.buff[mqtt.Fixed_len+1] = mqtt.MessageID%256;                      //可变报头第2个字节，报文标识符低字节
	mqtt.MessageID++;                                                      //报文标识符用后+1
	if(mqtt.MessageID == 0)                                                //如果等于0了，进入if
		mqtt.MessageID =1;                                                 //设置为1，从1开始使用
	
	mqtt.buff[mqtt.Fixed_len+2] = strlen(topic)/256;                       //负载中需要订阅的 topic字符串长度，高字节
	mqtt.buff[mqtt.Fixed_len+3] = strlen(topic)%256;                       //负载中需要订阅的 topic字符串长度，低字节
	memcpy(&mqtt.buff[mqtt.Fixed_len+4],topic,strlen(topic));              //添加需要订阅的 topic字符串
	
	mqtt.buff[mqtt.Fixed_len+4+strlen(topic)] = Qs;                        //添加需要订阅的 topic等级 
	
	mqtt.lenth = mqtt.Fixed_len + mqtt.Variable_len + mqtt.Payload_len;    //计算整个报文长度
}
/*---------------------------------------------------------*/
/*函数名：处理9号SUBACK报文                                */
/*参  数：rxdata：接收的数据                               */
/*参  数：rxdata_len：接收的数据长度                       */
/*返回值：-1：不是9号SUBACK报文  rxdata[4]：返回码         */
/*---------------------------------------------------------*/
int8_t MQTT_SUBACK(unsigned char *rxdata, unsigned int rxdata_len)
{
	if((rxdata_len == 5)&&(rxdata[0] == 0x90)){   //如果接收5字节数据 且 第一个字节是0x90，是9号SUBACK报文，进入if分支
		
	}else{                                        //反之，进入else分支 
		return -1;                                //返回-1，不是9号SUBACK报文
	}
	
	return rxdata[4];                             //返回rxdata[4]，9号SUBACK报文的返回码 
}
/*---------------------------------------------------------*/
/*函数名：10号UNSUBSCRIBE报文                              */
/*参  数：topic：需要取消订阅的 topic                      */
/*返回值：无                                               */
/*---------------------------------------------------------*/
void MQTT_UNSUBSCRIBE(char *topic)
{
	mqtt.Fixed_len = 1;                                                    //固定报头中第一个字节0xA2可以确定，长度当前是1
	mqtt.Variable_len = 2;                                                 //可变报头长度固定2字节长
	mqtt.Payload_len = 2 + strlen(topic) ;                                 //计算负载长度
	mqtt.Reamining_len = mqtt.Variable_len + mqtt.Payload_len;             //计算剩余长度	

	mqtt.buff[0] = 0xA2;                                                   //固定报头中第一个字节0xA2
	do{                                                                    //根据计算好的剩余长度，循环构建各个字节
		if(mqtt.Reamining_len/128 == 0){                                   //不需要再向前进位 
			mqtt.buff[mqtt.Fixed_len] = mqtt.Reamining_len;                //设置相应值
		}else{                                                             //需要再向前进位
			mqtt.buff[mqtt.Fixed_len] = (mqtt.Reamining_len%128)|0x80;     //设设置相应值 且 BIT7置位为1表示需要进位
		} 
		mqtt.Fixed_len++;                                                  //固定报头长度+1
		mqtt.Reamining_len = mqtt.Reamining_len/128;                       //计算剩余长度剩余值
	}while(mqtt.Reamining_len);	                                           //剩余长度计算到0后，退出while           

	mqtt.buff[mqtt.Fixed_len] = mqtt.MessageID/256;                        //可变报头第1个字节，报文标识符高字节
	mqtt.buff[mqtt.Fixed_len+1] = mqtt.MessageID%256;                      //可变报头第2个字节，报文标识符低字节
	mqtt.MessageID++;                                                      //报文标识符用后+1
	if(mqtt.MessageID == 0)                                                //如果等于0了，进入if
		mqtt.MessageID =1;                                                 //设置为1，从1开始使用

	mqtt.buff[mqtt.Fixed_len+2] = strlen(topic)/256;                       //负载中需要取消订阅的 topic字符串长度，高字节
	mqtt.buff[mqtt.Fixed_len+3] = strlen(topic)%256;                       //负载中需要取消订阅的 topic字符串长度，低字节
	memcpy(&mqtt.buff[mqtt.Fixed_len+4],topic,strlen(topic));              //添加需要取消订阅的 topic字符串
		
	mqtt.lenth = mqtt.Fixed_len + mqtt.Variable_len + mqtt.Payload_len;    //计算整个报文长度
}
/*---------------------------------------------------------*/
/*函数名：处理11号UNSUBACK报文                             */
/*参  数：rxdata：接收的数据                               */
/*参  数：rxdata_len：接收的数据长度                       */
/*返回值：-1：不是11号UNSUBACK报文  0：是11号UNSUBACK报文  */
/*---------------------------------------------------------*/
int8_t MQTT_UNSUBACK(unsigned char *rxdata, unsigned int rxdata_len)
{
	if((rxdata_len == 4)&&(rxdata[0] == 0xB0)){   //如果接收4字节数据 且 第一个字节是0xB0，11号UNSUBACK报文，进入if分支
		
	}else{                                        //反之，进入else分支 
		return -1;                                //返回-1，不是11号UNSUBACK报文
	}
	
	return 0;                                     //返回0，是11号UNSUBACK报文 
}
/*---------------------------------------------------------*/
/*函数名：12号PINGREQ报文                                  */
/*参  数：无                                               */
/*返回值：无                                               */
/*---------------------------------------------------------*/
void MQTT_PINGREQ(void)
{
	mqtt.buff[0] = 0xC0;    //12号报文固定报头第1个字节
	mqtt.buff[1] = 0x00;    //12号报文固定报头第2个字节
	mqtt.lenth = 2;         //12号报文长度2
}
/*---------------------------------------------------------*/
/*函数名：处理13号PINGRESP报文                             */
/*参  数：rxdata：接收的数据                               */
/*参  数：rxdata_len：接收的数据长度                       */
/*返回值：-1：不是13号PINGRESP报文  0：是13号PINGRESP报文  */
/*---------------------------------------------------------*/
int8_t MQTT_PINGRESP(unsigned char *rxdata, unsigned int rxdata_len)
{
	if((rxdata_len == 2)&&(rxdata[0] == 0xD0)){   //如果接收2字节数据 且 第一个字节是0xD0，13号PINGRESP报文，进入if分支
		
	}else{                                        //反之，进入else分支 
		return -1;                                //返回-1，不是13号PINGRESP报文
	}
	
	return 0;                                     //返回0，是13号PINGRESP报文
}
/*---------------------------------------------------------*/
/*函数名：3号等级0的PUBLISH报文                            */
/*参  数：retain：保留功能                                 */
/*参  数：topic：需要发送数据的 topic                      */
/*参  数：data：数据                                       */
/*参  数：data_len：数据长度                               */
/*返回值：无                                               */
/*---------------------------------------------------------*/
void MQTT_PUBLISH0(char retain,char *topic, unsigned char *data, unsigned int data_len)
{
	mqtt.Fixed_len = 1;                                                    //固定报头中第一个字节 0x30|(retain<<0)可以确定，长度当前是1
	mqtt.Variable_len = 2 + strlen(topic);                                 //计算可变报头长度
	mqtt.Payload_len = data_len;                                           //负载长度就等于data_len
	mqtt.Reamining_len = mqtt.Variable_len + mqtt.Payload_len;             //计算剩余长度
	
	mqtt.buff[0] = 0x30|(retain<<0);                                       //固定报头中第一个字节0x30|(retain<<0)
	do{                                                                    //根据计算好的剩余长度，循环构建各个字节
		if(mqtt.Reamining_len/128 == 0){                                   //不需要再向前进位 
			mqtt.buff[mqtt.Fixed_len] = mqtt.Reamining_len;                //设置相应值
		}else{                                                             //需要再向前进位
			mqtt.buff[mqtt.Fixed_len] = (mqtt.Reamining_len%128)|0x80;     //设设置相应值 且 BIT7置位为1表示需要进位
		} 
		mqtt.Fixed_len++;                                                  //固定报头长度+1
		mqtt.Reamining_len = mqtt.Reamining_len/128;                       //计算剩余长度剩余值
	}while(mqtt.Reamining_len);	                                           //剩余长度计算到0后，退出while           

	mqtt.buff[mqtt.Fixed_len+0] = strlen(topic)/256;                       //可变报头第1个字节，需要发送数据的 topic字符串长度，高字节
	mqtt.buff[mqtt.Fixed_len+1] = strlen(topic)%256;                       //可变报头第2个字节，需要发送数据的 topic字符串长度，低字节
	memcpy(&mqtt.buff[mqtt.Fixed_len+2],topic,strlen(topic));              //添加需要发送数据的 topic字符串
	memcpy(&mqtt.buff[mqtt.Fixed_len+2+strlen(topic)],data,data_len);      //添加负载数据
  
	mqtt.lenth = mqtt.Fixed_len + mqtt.Variable_len + mqtt.Payload_len;    //计算整个报文长度
} 
/*---------------------------------------------------------*/
/*函数名：3号等级1的PUBLISH报文                            */
/*参  数：dup：重发标志                                    */
/*参  数：retain：保留功能                                 */
/*参  数：topic：需要发送数据的 topic                      */
/*参  数：data：数据                                       */
/*参  数：data_len：数据长度                               */
/*返回值：无                                               */
/*---------------------------------------------------------*/
void MQTT_PUBLISH1(char dup, char retain,char *topic, unsigned char *data, unsigned int data_len)
{
	mqtt.Fixed_len = 1;                                                    //固定报头中第一个字节 0x32|(dup<<3)|(retain<<0)可以确定，长度当前是1
	mqtt.Variable_len = 2 + strlen(topic) +2;                              //计算可变报头长度
	mqtt.Payload_len = data_len;                                           //负载长度就等于data_len
	mqtt.Reamining_len = mqtt.Variable_len + mqtt.Payload_len;             //计算剩余长度
	
	mqtt.buff[0] = 0x32|(dup<<3)|(retain<<0);                              //固定报头中第一个字节0x32|(dup<<3)|(retain<<0)
	do{                                                                    //根据计算好的剩余长度，循环构建各个字节
		if(mqtt.Reamining_len/128 == 0){                                   //不需要再向前进位 
			mqtt.buff[mqtt.Fixed_len] = mqtt.Reamining_len;                //设置相应值
		}else{                                                             //需要再向前进位
			mqtt.buff[mqtt.Fixed_len] = (mqtt.Reamining_len%128)|0x80;     //设设置相应值 且 BIT7置位为1表示需要进位
		} 
		mqtt.Fixed_len++;                                                  //固定报头长度+1
		mqtt.Reamining_len = mqtt.Reamining_len/128;                       //计算剩余长度剩余值
	}while(mqtt.Reamining_len);	                                           //剩余长度计算到0后，退出while           

	mqtt.buff[mqtt.Fixed_len+0] = strlen(topic)/256;                       //可变报头第1个字节，需要发送数据的 topic字符串长度，高字节
	mqtt.buff[mqtt.Fixed_len+1] = strlen(topic)%256;                       //可变报头第2个字节，需要发送数据的 topic字符串长度，低字节
	memcpy(&mqtt.buff[mqtt.Fixed_len+2],topic,strlen(topic));              //添加需要发送数据的 topic字符串
	
	mqtt.buff[mqtt.Fixed_len+2+strlen(topic)] = mqtt.MessageID/256;        //添加报文标识符高字节
	mqtt.buff[mqtt.Fixed_len+3+strlen(topic)] = mqtt.MessageID%256;        //添加报文标识符低字节
	mqtt.MessageID++;                                                      //报文标识符用后+1
	if(mqtt.MessageID == 0)                                                //如果等于0了，进入if
		mqtt.MessageID = 1;                                                //设置为1，从1开始使用
	
	memcpy(&mqtt.buff[mqtt.Fixed_len+4+strlen(topic)],data,data_len);      //添加负载数据

	mqtt.lenth = mqtt.Fixed_len + mqtt.Variable_len + mqtt.Payload_len;    //计算整个报文长度
} 
/*---------------------------------------------------------*/
/*函数名：3号等级2的PUBLISH报文                            */
/*参  数：dup：重发标志                                    */
/*参  数：retain：保留功能                                 */
/*参  数：topic：需要发送数据的 topic                      */
/*参  数：data：数据                                       */
/*参  数：data_len：数据长度                               */
/*返回值：无                                               */
/*---------------------------------------------------------*/
void MQTT_PUBLISH2(char dup, char retain,char *topic, unsigned char *data, unsigned int data_len)
{
	mqtt.Fixed_len = 1;                                                    //固定报头中第一个字节 0x34|(dup<<3)|(retain<<0)可以确定，长度当前是1
	mqtt.Variable_len = 2 + strlen(topic) +2;                              //计算可变报头长度
	mqtt.Payload_len = data_len;                                           //负载长度就等于data_len
	mqtt.Reamining_len = mqtt.Variable_len + mqtt.Payload_len;             //计算剩余长度
	
	mqtt.buff[0] = 0x34|(dup<<3)|(retain<<0);                              //固定报头中第一个字节0x34|(dup<<3)|(retain<<0)
	do{                                                                    //根据计算好的剩余长度，循环构建各个字节
		if(mqtt.Reamining_len/128 == 0){                                   //不需要再向前进位 
			mqtt.buff[mqtt.Fixed_len] = mqtt.Reamining_len;                //设置相应值
		}else{                                                             //需要再向前进位
			mqtt.buff[mqtt.Fixed_len] = (mqtt.Reamining_len%128)|0x80;     //设设置相应值 且 BIT7置位为1表示需要进位
		} 
		mqtt.Fixed_len++;                                                  //固定报头长度+1
		mqtt.Reamining_len = mqtt.Reamining_len/128;                       //计算剩余长度剩余值
	}while(mqtt.Reamining_len);	                                           //剩余长度计算到0后，退出while           

	mqtt.buff[mqtt.Fixed_len+0] = strlen(topic)/256;                       //可变报头第1个字节，需要发送数据的 topic字符串长度，高字节
	mqtt.buff[mqtt.Fixed_len+1] = strlen(topic)%256;                       //可变报头第2个字节，需要发送数据的 topic字符串长度，低字节
	memcpy(&mqtt.buff[mqtt.Fixed_len+2],topic,strlen(topic));              //添加需要发送数据的 topic字符串
	
	mqtt.buff[mqtt.Fixed_len+2+strlen(topic)] = mqtt.MessageID/256;        //添加报文标识符高字节
	mqtt.buff[mqtt.Fixed_len+3+strlen(topic)] = mqtt.MessageID%256;        //添加报文标识符低字节
	mqtt.MessageID++;                                                      //报文标识符用后+1
	if(mqtt.MessageID == 0)                                                //如果等于0了，进入if
		mqtt.MessageID = 1;                                                //设置为1，从1开始使用
	
	memcpy(&mqtt.buff[mqtt.Fixed_len+4+strlen(topic)],data,data_len);      //添加负载数据

	mqtt.lenth = mqtt.Fixed_len + mqtt.Variable_len + mqtt.Payload_len;    //计算整个报文长度
} 
/*---------------------------------------------------------*/
/*函数名：处理接收到的3号PUBLISH报文                       */
/*参  数：rxdata：接收的数据                               */
/*参  数：rxdata_len：接收的数据长度                       */
/*参  数：qs：接收到的3号PUBLISH报文 等级                  */
/*参  数：messageid：接收到的3号PUBLISH报文 报文标识符     */
/*返回值：-1：不是3号PUBLISH报文  0：是3号PUBLISH报文      */
/*---------------------------------------------------------*/
int8_t MQTT_ProcessPUBLISH(unsigned char *rxdata, unsigned int rxdata_len, unsigned char *qs, unsigned int *messageid)
{
	char i;
	int topic_len;
	int data_len;
	
	if((rxdata[0]&0xF0) == 0x30){                                                                  //判断第一个字节高4位是3，进入if
		for(i=1;i<5;i++){                                                                          //最多循环4次，因为剩余长度最多占用4个字节
			if((rxdata[i]&0x80) == 0x00)                                                           //判断当前字节BIT7是不是0，是的话进入if
				break;                                                                             //跳出for循环，此时i的值就是剩余长度占用的字节数
		}
		switch(rxdata[0]&0x06){                                                                    //判断接收到的3号PUBLISH报文的等级
			case 0x00:			                                                                   //等级0分支
						*qs = 0;                                                                   //qs记录等级0
						*messageid = 0;                                                            //等级0的报文没有报文标识符，记为0
						topic_len = rxdata[1+i]*256 + rxdata[1+i+1];                               //计算报文中包含的topic字符串的长度
						data_len = rxdata_len - 1 - i - 2 - topic_len;                             //计算报文中包含的数据长度
						memset(mqtt.topic,0,TOPIC_SIZE);                                           //清空存放topic字符串的缓冲区
						memset(mqtt.data,0,DATR_SIZE);                                             //清空存放数据的缓冲区
						memcpy(mqtt.topic,&rxdata[1+i],topic_len+2);                               //拷贝topic字符串到缓冲区
						mqtt.data[0] = data_len/256;                                               //记录数据长度高字节
						mqtt.data[1] = data_len%256;                                               //记录数据长度低字节
						memcpy(&mqtt.data[2],&rxdata[1+i+2+topic_len],data_len);                   //拷贝数据						
						break; 
			case 0x02:			                                                                   //等级1分支			
						*qs = 1;                                                                   //qs记录等级1
						topic_len = rxdata[1+i]*256 + rxdata[1+i+1];                               //计算报文中包含的topic字符串的长度
						*messageid = rxdata[1+i+2+topic_len]*256 + rxdata[1+i+2+topic_len+1];      //记录报文中的报文标识符
						data_len = rxdata_len - 1 - i - 2 - topic_len - 2;                         //计算报文中包含的数据长度
						memset(mqtt.topic,0,TOPIC_SIZE);                                           //清空存放topic字符串的缓冲区
						memset(mqtt.data,0,DATR_SIZE);                                             //清空存放数据的缓冲区
						memcpy(mqtt.topic,&rxdata[1+i],topic_len+2);                               //拷贝topic字符串到缓冲区
						mqtt.data[0] = data_len/256;                                               //记录数据长度高字节
						mqtt.data[1] = data_len%256;                                               //记录数据长度低字节
						memcpy(&mqtt.data[2],&rxdata[1+i+2+topic_len+2],data_len);                 //拷贝数据					
						break; 
			case 0x04:				                                                               //等级2分支				
						*qs = 2;                                                                   //qs记录等级2
						topic_len = rxdata[1+i]*256 + rxdata[1+i+1];                               //计算报文中包含的topic字符串的长度
						*messageid = rxdata[1+i+2+topic_len]*256 + rxdata[1+i+2+topic_len+1];      //记录报文中的报文标识符
						data_len = rxdata_len - 1 - i - 2 - topic_len - 2;                         //计算报文中包含的数据长度
						memset(mqtt.topic,0,TOPIC_SIZE);                                           //清空存放topic字符串的缓冲区
						memset(mqtt.data,0,DATR_SIZE);                                             //清空存放数据的缓冲区
						memcpy(mqtt.topic,&rxdata[1+i],topic_len+2);                               //拷贝topic字符串到缓冲区
						mqtt.data[0] = data_len/256;                                               //记录数据长度高字节
						mqtt.data[1] = data_len%256;                                               //记录数据长度低字节
						memcpy(&mqtt.data[2],&rxdata[1+i+2+topic_len+2],data_len);                 //拷贝数据						
						break; 
		} 
	}else{                                                                                         //反之不是3号PUBLISH报文，进入else分支
		return -1;                                                                                 //返回-1不是3号PUBLISH报文
	}
	
	return 0;                                                                                      //返回0是3号PUBLISH报文
}
/*---------------------------------------------------------*/
/*函数名：4号PUBACK报文                                    */
/*参  数：messageid：报文标识符                            */
/*返回值：无                                               */
/*---------------------------------------------------------*/
void MQTT_PUBACK(unsigned int messageid)
{
	mqtt.buff[0] = 0x40;           //4号PUBACK报文固定报头第1个字节
	mqtt.buff[1] = 0x02;           //4号PUBACK报文固定报头第2个字节
	mqtt.buff[2] = messageid/256;  //4号PUBACK报文固定报头第3个字节
	mqtt.buff[3] = messageid%256;  //4号PUBACK报文固定报头第4个字节
	
	mqtt.lenth = 4;                //4号PUBACK报文长度为4
}
/*---------------------------------------------------------*/
/*函数名：处理收到的4号PUBACK报文                          */
/*参  数：rxdata：接收的数据                               */
/*参  数：rxdata_len：接收的数据长度                       */
/*参  数：messageid：接收的4号PUBACK报文标识符             */
/*返回值：-1：不是4号PUBACK报文  0：是4号PUBACK报文        */
/*---------------------------------------------------------*/
int8_t MQTT_ProcessPUBACK(unsigned char *rxdata, unsigned int rxdata_len, unsigned int *messageid)
{
	if((rxdata_len == 4)&&(rxdata[0] == 0x40)){     //如果接收的数据长度是4 且 第一个字节是0x40，进入if
		*messageid = rxdata[2]*256 + rxdata[3];     //messageid记录4号PUBACK报文标识符
	}else{                                          //反之不是4号PUBACK报文 ，进入else
		return -1;                                  //返回-1不是4号PUBACK报文
	}
	
	return 0;                                       //返回0是4号PUBACK报文
}
/*---------------------------------------------------------*/
/*函数名：5号PUBREC报文                                    */
/*参  数：messageid：报文标识符                            */
/*返回值：无                                               */
/*---------------------------------------------------------*/
void MQTT_PUBREC(unsigned int messageid)
{	
	mqtt.buff[0] = 0x50;           //5号PUBREC报文固定报头第1个字节
	mqtt.buff[1] = 0x02;           //5号PUBREC报文固定报头第2个字节
	mqtt.buff[2] = messageid/256;  //5号PUBREC报文固定报头第3个字节
	mqtt.buff[3] = messageid%256;  //5号PUBREC报文固定报头第4个字节
	
	mqtt.lenth = 4;                //5号PUBREC报文长度为4
}
/*---------------------------------------------------------*/
/*函数名：处理收到的5号PUBREC报文                          */
/*参  数：rxdata：接收的数据                               */
/*参  数：rxdata_len：接收的数据长度                       */
/*参  数：messageid：接收的4号PUBACK报文标识符             */
/*返回值：-1：不是5号PUBREC报文  0：是5号PUBREC报文        */
/*---------------------------------------------------------*/
int8_t MQTT_ProcessPUBREC(unsigned char *rxdata, unsigned int rxdata_len, unsigned int *messageid)
{
	if((rxdata_len == 4)&&(rxdata[0] == 0x50)){     //如果接收的数据长度是4 且 第一个字节是0x50，进入if
		*messageid = rxdata[2]*256 + rxdata[3];     //messageid记录5号PUBREC报文标识符
	}else{                                          //反之不是5号PUBREC报文 ，进入else
		return -1;                                  //返回-1不是5号PUBREC报文
	}
	
	return 0;                                       //返回0是5号PUBREC报文
}
/*---------------------------------------------------------*/
/*函数名：6号PUBREC报文                                    */
/*参  数：messageid：报文标识符                            */
/*返回值：无                                               */
/*---------------------------------------------------------*/
void MQTT_PUBREL(unsigned int messageid)
{
	mqtt.buff[0] = 0x62;           //6号PUBREC报文固定报头第1个字节
	mqtt.buff[1] = 0x02;           //6号PUBREC报文固定报头第2个字节
	mqtt.buff[2] = messageid/256;  //6号PUBREC报文固定报头第3个字节
	mqtt.buff[3] = messageid%256;  //6号PUBREC报文固定报头第4个字节
	
	mqtt.lenth = 4;                //6号PUBREC报文长度为4
}
/*---------------------------------------------------------*/
/*函数名：处理收到的6号PUBREL报文                          */
/*参  数：rxdata：接收的数据                               */
/*参  数：rxdata_len：接收的数据长度                       */
/*参  数：messageid：接收的4号PUBACK报文标识符             */
/*返回值：-1：不是6号PUBREL报文  0：是6号PUBREL报文        */
/*---------------------------------------------------------*/
int8_t MQTT_ProcessPUBREL(unsigned char *rxdata, unsigned int rxdata_len, unsigned int *messageid)
{
	if((rxdata_len == 4)&&(rxdata[0] == 0x62)){     //如果接收的数据长度是4 且 第一个字节是0x62，进入if
		*messageid = rxdata[2]*256 + rxdata[3];     //messageid记录6号PUBREL报文标识符
	}else{                                          //反之不是6号PUBREL报文 ，进入else
		return -1;                                  //返回-1不是6号PUBREL报文
	}
	
	return 0;                                       //返回0是6号PUBREL报文
}
/*---------------------------------------------------------*/
/*函数名：7号PUBCOMP报文                                   */
/*参  数：messageid：报文标识符                            */
/*返回值：无                                               */
/*---------------------------------------------------------*/
void MQTT_PUBCOMP(unsigned int messageid)
{
	mqtt.buff[0] = 0x70;           //7号PUBCOMP报文固定报头第1个字节
	mqtt.buff[1] = 0x02;           //7号PUBCOMP报文固定报头第2个字节
	mqtt.buff[2] = messageid/256;  //7号PUBCOMP报文固定报头第3个字节
	mqtt.buff[3] = messageid%256;  //7号PUBCOMP报文固定报头第4个字节
	
	mqtt.lenth = 4;                //7号PUBCOMP报文长度为4
}
/*---------------------------------------------------------*/
/*函数名：处理收到的7号PUBCOMP报文                         */
/*参  数：rxdata：接收的数据                               */
/*参  数：rxdata_len：接收的数据长度                       */
/*参  数：messageid：接收的4号PUBACK报文标识符             */
/*返回值：-1：不是7号PUBCOMP报文  0：是7号PUBCOMP报文      */
/*---------------------------------------------------------*/
int8_t MQTT_ProcessPUBCOMP(unsigned char *rxdata, unsigned int rxdata_len, unsigned int *messageid)
{
	if((rxdata_len == 4)&&(rxdata[0] == 0x70)){     //如果接收的数据长度是4 且 第一个字节是0x70，进入if
		*messageid = rxdata[2]*256 + rxdata[3];     //messageid记录7号PUBCOMP报文标识符
	}else{                                          //反之不是7号PUBCOMP报文 ，进入else
		return -1;                                  //返回-1不是7号PUBCOMP报文
	}
	
	return 0;                                       //返回0是7号PUBCOMP报文
}
