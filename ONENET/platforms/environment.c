/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/
#include "platform.h"
#include "platform_config.h"
#include "bcxx.h"
#include "usart.h"
#include "usart2.h"
#include "delay.h"

#include "led.h"

static bool _nbiot_init_state = false;

void nbiot_init_environment(void)
{
	if (!_nbiot_init_state)
	{
		bcxx_hard_init();

		delay_ms(500);

		netdev_init();

		_nbiot_init_state = true;
	}
}

void nbiot_clear_environment(void)
{
	if(_nbiot_init_state)
	{
		bcxx_delinstance();
		
		Registered_Flag = 0;
		_nbiot_init_state = false;
	}
}

void nbiot_reset(void)
{
//	NVIC_GenerateSystemReset();
}

