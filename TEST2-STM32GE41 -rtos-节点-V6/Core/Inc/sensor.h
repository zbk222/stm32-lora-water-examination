#ifndef __SENSOR_H__
#define __SENSOR_H__
#include "stm32g4xx_hal.h"
#include "FreeRTOS.h"
typedef struct {
    float tds_Value; // TDS值
    float temp;       // 温度值
    float TU_Value;  // 浊度值
    float pH_Value;  // pH值
} SensorData;
typedef struct {
    float pH_threshold; // pH阈值
    float TU_threshold; // 浊度阈值
} SensorThresholds;
extern SensorData sensorData;
extern volatile SensorThresholds sensorThresholds;
extern uint16_t adc_value[3];
void sensor_init(void);
void TU_Value_Conversion(void);
void ph_value(void);
void tds_value(void);
void temp_read(void);
#endif
