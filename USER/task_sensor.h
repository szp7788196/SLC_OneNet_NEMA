#ifndef __TASK_SENSOR_H
#define __TASK_SENSOR_H

#include "sys.h"
#include "rtos_task.h"


typedef struct SensorMsg
{
	float temperature;
	float humidity;
	float illumination;
	float in_put_current;
	float in_put_voltage;
	float in_put_freq;
	float in_put_power_p;
	float in_put_power_q;
	float in_put_power_s;
	float in_put_energy_p;
	float in_put_energy_q;
	float in_put_energy_s;
	float out_put_current;
	float out_put_voltage;
	float signal_intensity;
	u8 hour;
	u8 minute;
	u8 second;
	char gps[32];
}SensorMsg_S;


extern TaskHandle_t xHandleTaskSENSOR;

void vTaskSENSOR(void *pvParameters);






































#endif
