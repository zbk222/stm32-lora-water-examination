#include "sensor.h"
#include "adc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ds18b20.h"
#include <stdio.h>
extern ADC_HandleTypeDef hadc1;
float tds_kValue = 1.0f; //tds校准系数
float K_Value=1600.0f;//浊度校准系数
uint16_t adc_value[3]={0};
SensorData sensorData;
volatile SensorThresholds sensorThresholds;
void sensor_init()
{
    DS18B20_Init();
HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_value, 3);

}

void temp_read()
{
    short temp;
    temp = DS18B20_Get_Temp();
    sensorData.temp = (float)temp/10.0f;
    // printf("temperature=%f\n",sensorData.temp);
}

void tds_value(void)
{
    // 1. 读取ADC值并转换为电压
    float analogVoltage = (float)adc_value[2] / 4096.0f * 3.3f;
    
    // 打印原始电压（调试用）
    // printf("TDS原始电压: %.3f V, ", analogVoltage);
    
    // 2. 温度补偿（手册第5页公式）
    // 温度修正系数: T_correction = 1 + 0.02*(T - 25)
    float T_correction = 1.0f + 0.02f * (sensorData.temp - 25.0f);
    float correctedVoltage = analogVoltage * T_correction;
    
    // 3. 使用标准曲线公式计算TDS（手册第4页公式）
    // TDS = 66.71*V^3 - 127.93*V^2 + 428.7*V
    float V = correctedVoltage;
    float tdsTemp = 66.71f * V * V * V 
                  - 127.93f * V * V 
                  + 428.7f * V;
    
    // 4. 应用校准系数（手册第5页公式）
    // K = TDS标准值 / TDS测量值
    sensorData.tds_Value = tdsTemp * tds_kValue;
    
    // 5. 限制范围 (0~1000ppm)
    if (sensorData.tds_Value < 0) {
        sensorData.tds_Value = 0;
    }
    if (sensorData.tds_Value > 1000) {
        sensorData.tds_Value = 1000;
    }
    
        // 6. 输出结果（ASCII，避免多字节编码告警）
        // printf("Vcorr: %.3f V, TDS: %.1f ppm\r\n",
        //    correctedVoltage, sensorData.tds_Value);
    
}


void ph_value()
{
 float ADC_ConvertedValueLocal =(float) adc_value[1]/4096*3.3f; // 读取转换的AD值
	
		sensorData.pH_Value=-5.7541*ADC_ConvertedValueLocal+16.654;
	
    
	if(sensorData.pH_Value<=0.0){sensorData.pH_Value=0.0;}
	if(sensorData.pH_Value>=14.0){sensorData.pH_Value=14.0;}
    // printf("PH Value: %.2f\r\n",sensorData.pH_Value);



}

void TU_Value_Conversion()
{
    float TU =(float) adc_value[0] / 4095.0f * 5.0f; // 读取转换的AD值（12位→4095）
    float TU_calibration = -0.0192f * (sensorData.temp - 25.0f) + TU;  
    float tu_raw = -865.68f * TU_calibration + K_Value;
    sensorData.TU_Value = tu_raw;

    if(sensorData.TU_Value <= 0){sensorData.TU_Value = 0;}
    if(sensorData.TU_Value >= 1000){sensorData.TU_Value = 1000;}
    // printf("TU raw: %.2f, V: %.3f, Vcorr: %.3f, T: %.2f, TU: %.2f\r\n",
    //        tu_raw, TU, TU_calibration, sensorData.temp, sensorData.TU_Value);
}


