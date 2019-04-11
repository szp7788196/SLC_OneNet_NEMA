#include "common.h"
#include "rtos_task.h"
#include "24cxx.h"
#include "led.h"
#include "rtc.h"
#include "usart.h"
#include "usart2.h"

u16 i = 0;
u8 eepbuf[256];
u16 cnt = 0;
u8 led_s = 0;
RCC_ClocksTypeDef RCC_Clocks;
pRegularTime group_time1 = NULL;
pRegularTime group_time2 = NULL;
pRegularTime group_time3 = NULL;
pRegularTime group_time4 = NULL;
pRegularTime group_time5 = NULL;

int main(void)
{
//	SCB->VTOR = FLASH_BASE | 0x24000; /* Vector Table Relocation in Internal FLASH. */
//	IWDG_Init(IWDG_Prescaler_128,625);	//128分频 312.5HZ 625为2秒

	RCC_GetClocksFreq(&RCC_Clocks);		//查看各个总线的时钟频率
	__set_PRIMASK(1);	//关闭全局中断

	NVIC_Configuration();
	delay_init(72);
	RTC_Init();
	AT24CXX_Init();
	LED_Init();
	TIM2_Init(99,7199);
	USART1_Init(115200);
	USART2_Init(9600);
	UART4_Init(9600);

	__set_PRIMASK(0);	//开启全局中断

//	for(i = 0; i < 256; i ++)
//	{
//		AT24CXX_WriteOneByte(i,i);
//	}
//	for(i = 0; i < 256; i ++)
//	{
//		eepbuf[i] = AT24CXX_ReadOneByte(i);
//	}
//	AT24CXX_WriteOneByte(UU_ID_ADD,255);

	mem_init();
	
//	RegularTimeWeekDay = (pRegularTime)mymalloc(sizeof(RegularTime_S));
//	RegularTimeWeekEnd = (pRegularTime)mymalloc(sizeof(RegularTime_S));
//	RegularTimeHoliday = (pRegularTime)mymalloc(sizeof(RegularTime_S));
//	
//	group_time1 = (pRegularTime)mymalloc(sizeof(RegularTime_S));
//	group_time2 = (pRegularTime)mymalloc(sizeof(RegularTime_S));
//	group_time3 = (pRegularTime)mymalloc(sizeof(RegularTime_S));
//	group_time4 = (pRegularTime)mymalloc(sizeof(RegularTime_S));
//	group_time5 = (pRegularTime)mymalloc(sizeof(RegularTime_S));
//	group_time1->next = NULL;
//	group_time2->next = NULL;
//	group_time3->next = NULL;
//	group_time4->next = NULL;
//	group_time5->next = NULL;
//	group_time1->prev = NULL;
//	group_time2->prev = NULL;
//	group_time3->prev = NULL;
//	group_time4->prev = NULL;
//	group_time5->prev = NULL;
//	
//	group_time1->number = 1;
//	group_time2->number = 2;
//	group_time3->number = 3;
//	group_time4->number = 4;
//	group_time5->number = 5;
//	
//	RegularTimeGroupAdd(TYPE_WEEKDAY,group_time1);
//	RegularTimeGroupAdd(TYPE_WEEKDAY,group_time2);
//	RegularTimeGroupAdd(TYPE_WEEKDAY,group_time3);
//	RegularTimeGroupAdd(TYPE_WEEKDAY,group_time4);
//	RegularTimeGroupAdd(TYPE_WEEKDAY,group_time5);
//	
//	RegularTimeGroupSub(TYPE_WEEKDAY,3);
//	RegularTimeGroupSub(TYPE_WEEKDAY,2);
//	RegularTimeGroupSub(TYPE_WEEKDAY,5);
	

	IWDG_Feed();				//喂看门狗

	ReadParametersFromEEPROM();	//读取所有的运行参数

	AppObjCreate();				//创建消息队列、互斥量
	AppTaskCreate();			//创建任务

	vTaskStartScheduler();		//启动调度，开始执行任务

	while(1)
	{
		delay_ms(100);
	}
}

























