#include "task_sensor.h"
#include "delay.h"
#include "sht2x.h"
#include "bh1750.h"
#include "task_net.h"
#include "common.h"
#include "inventr.h"
#include "rtc.h"
#include "usart.h"


TaskHandle_t xHandleTaskSENSOR = NULL;

SensorMsg_S *p_tSensorMsg = NULL;	//����װ�ڴ��������ݵĽṹ�����
unsigned portBASE_TYPE SENSOR_Satck;
void vTaskSENSOR(void *pvParameters)
{
	time_t times_sec = 0;

#ifndef SMALLER_BOARD
	SHT2x_Init();
	Bh1750_Init();
#endif

	p_tSensorMsg = (SensorMsg_S *)mymalloc(sizeof(SensorMsg_S));

	while(1)
	{
		if(GetSysTick1s() - times_sec >= 10)		//ÿ��10�����һ�δ���������
		{
			times_sec = GetSysTick1s();

#ifndef SMALLER_BOARD
			Temperature = Sht2xReadTemperature();				//��ȡ�¶�
			Humidity = Sht2xReadHumidity();						//��ȡʪ��
			Illumination = Bh1750ReadIllumination();			//��ȡ����
#endif

			InventrOutPutCurrent = InventrGetOutPutCurrent();	//��ȡ��Դ�������
			delay_ms(500);
			InventrOutPutVoltage = InventrGetOutPutVoltage();	//��ȡ��Դ�����ѹ

			if( dev->state == STATE_REGISTERED ||
				dev->state == STATE_REG_UPDATE_PENDING ||
				dev->state == STATE_REG_UPDATE_NEEDED)			//�豸��ʱ������״̬
			{
#ifndef	SMALLER_BOARD
				p_tSensorMsg->temperature = Temperature;
				p_tSensorMsg->humidity = Humidity;
				p_tSensorMsg->illumination = Illumination;
#endif
				p_tSensorMsg->out_put_current = InventrOutPutCurrent;
				p_tSensorMsg->out_put_voltage = InventrOutPutVoltage;
				p_tSensorMsg->signal_intensity = (float)((-113) + (SignalIntensity * 2));
				p_tSensorMsg->hour = calendar.hour;
				p_tSensorMsg->minute = calendar.min;
				p_tSensorMsg->second = calendar.sec;

				memset(p_tSensorMsg->gps,0,32);

				if(GpsInfo != NULL && strlen((char *)GpsInfo) <= 32)
				{
					memcpy(p_tSensorMsg->gps,GpsInfo,strlen((char *)GpsInfo));
				}
				else
				{
					memcpy(p_tSensorMsg->gps,"3948.0975N11632.7539E",21);
				}

				if(xQueueSend(xQueue_sensor,(void *)p_tSensorMsg,(TickType_t)10) != pdPASS)
				{
#ifdef DEBUG_LOG
					printf("send p_tSensorMsg fail 1.\r\n");
#endif
				}
			}
		}

		delay_ms(100);

		SENSOR_Satck = uxTaskGetStackHighWaterMark(NULL);
	}
}






































