#ifndef __TASK_NET_H
#define __TASK_NET_H

#include "sys.h"
#include "rtos_task.h"
#include "task_sensor.h"
#include "internal.h"




extern TaskHandle_t xHandleTaskNET;
extern SensorMsg_S *p_tSensorMsgNet;

extern u8 SignalIntensity;				//bg96的信号强度

extern nbiot_device_t *dev;

extern nbiot_value_t in_put_current;			//输入电流 float		3317 0 5700
extern nbiot_value_t out_put_current;			//输出电流 float		3317 1 5700
extern nbiot_value_t in_put_voltage;			//输入电压 float		3316 0 5700
extern nbiot_value_t out_put_voltage;			//输出电压 float		3316 1 5700
extern nbiot_value_t in_put_freq;				//电网频率 float		3318 0 5700
extern nbiot_value_t in_put_power_p;			//有功功率 float		3328 0 5700
extern nbiot_value_t in_put_power_q;			//无功功率 float		3328 1 5700
extern nbiot_value_t in_put_power_s;			//视在功率 float		3328 2 5700
extern nbiot_value_t in_put_energy_p;			//有功能量 float		3331 0 5805
extern nbiot_value_t in_put_energy_q;			//无功能量 float		3331 0 5805
extern nbiot_value_t in_put_energy_s;			//视在能量 float		3331 0 5805
extern nbiot_value_t signal_intensity;			//信号强度 float		3331 0 5805

//extern nbiot_value_t light_control_switch;		//灯具控制 Boolean		3311 0 5850
extern nbiot_value_t light_control_dimmer;		//灯具控制 Integer		3311 0 5851
extern nbiot_value_t device_uuid;				//设备UUID string		3308 0 5750
extern nbiot_value_t ota;						//在线升级 string		3308 0 5750
extern nbiot_value_t reset;						//远程复位 string		3308 1 5750
extern nbiot_value_t work_mode;					//工作模式 string		3308 2 5750
extern nbiot_value_t time_strategy;				//时间策略 string		3308 3 5750
extern nbiot_value_t data_upload_interval;		//上传间隔 float		3308 0 5900
extern nbiot_value_t power_interface;			//电源接口 float		3308 1 5900


void vTaskNET(void *pvParameters);
































#endif
