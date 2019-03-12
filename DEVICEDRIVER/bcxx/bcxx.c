#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "delay.h"
#include "at_cmd.h"
#include "utils.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "bcxx.h"
#include "fifo.h"
#include "led.h"
#include "nbiot.h"

extern int8_t dl_buf_id;
char cmd_tx_buff[512];
char cmd_rx_buff[256];

unsigned char   bcxx_init_ok;


void bcxx_hard_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	BCXX_PWREN_LOW;
	BCXX_RST_LOW;
}

void bcxx_hard_reset(void)
{
	BCXX_PWREN_LOW;						//关闭电源
	delay_ms(300);
	BCXX_PWREN_HIGH;					//打开电源

	delay_ms(100);

	BCXX_RST_HIGH;						//硬件复位
	delay_ms(300);
	BCXX_RST_LOW;

	bcxx_init_ok = 1;
}


uint32_t ip_SendData(int8_t * buf, uint32_t len)
{
     SentData((char *)buf,"OK",100);
     return len;
}

void netif_rx(uint8_t*buf,uint16_t *read)
{
	uint8_t *msg_p = NULL;
	uint8_t ptr[1024] = {0};

	*read = fifo_get(dl_buf_id,ptr);

	if(*read != 0)
	{
		if((msg_p = (uint8_t *)strstr((const char *)ptr, "+MIPL")) != NULL)
		{
			memcpy(buf,ptr,*read);
		}
		else
		{
			*read = 0;
		}
	}

	msg_p = msg_p;
}

void netdev_init(void)
{
	RE_INIT:
	
	bcxx_hard_reset();

	nbiot_sleep(10000);

//	SendCmd("AT+NRB\r\n", "OK", 5000,0,10);
//	SendCmd("AT+CGMR\r\n", "OK", 5000,0,10);
	SendCmd("ATE0\r\n", "OK", 100,0,1);
//	SendCmd("AT+CIMI\r\n", "OK", 1000,3,5);
	SendCmd("AT+CMEE=1\r\n","OK", 100,0,10);
	SendCmd("AT+MIPLCONFIG=1,183.230.40.39,5683\r\n","OK",100,0,10);
	SendCmd("AT+QREGSWT=2\r\n","OK",100,0,10);
	SendCmd("AT+CSCON=1\r\n","OK", 100,0,10);
	SendCmd("AT+CEREG=2\r\n","OK", 100,0,10);
	SendCmd("AT+CGATT=1\r\n","OK", 100,0,10);
	
	SendCmd("AT+CEDRXS=0,5\r\n","OK", 100,0,5);
	SendCmd("AT+CPSMS=0\r\n","OK", 100,0,5);
//	SendCmd("AT+CSQ\r\n", "OK", 1000,0,5);
	
	nbiot_sleep(5000);
	if(!SendCmd("AT+CEREG?\r\n","CEREG:2,1", 1000,10,5))
	{
		goto RE_INIT;
	}

	printf("bc35_g init sucess\r\n");
}

void bcxx_miplconfig(void)
{
	 SendCmd("AT+MIPLCONFIG=1,183.230.40.39,5683\r\n","OK",300,0,10);
}

void bcxx_create(void)
{
	 SendCmd("AT+MIPLCREATE\r\n","+MIPLCREATE:0\r\n\r\nOK",300,0,10);
}

void bcxx_addobj(uint16_t objid,uint8_t insCount,char *insBitmap,uint16_t attrcount,uint16_t actcount)
{
	char tmp[10];

	memset(cmd_tx_buff,0,50);
	memcpy(cmd_tx_buff,"AT+MIPLADDOBJ=0,",strlen("AT+MIPLADDOBJ=0,"));

	nbiot_itoa(objid,tmp,10);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	nbiot_itoa(insCount,tmp,10);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	strcat(cmd_tx_buff,"\"");
	strcat(cmd_tx_buff,insBitmap);
	strcat(cmd_tx_buff,"\"");
	strcat(cmd_tx_buff,",");
	nbiot_itoa(attrcount,tmp,10);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	nbiot_itoa(actcount,tmp,10);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,"\r\n");

	SendCmd(cmd_tx_buff,"OK",300,0,5);
}

void bcxx_delobj(uint16_t objid)
{
	 char tmp[10];

	 memset(cmd_tx_buff,0,50);
	 memcpy(cmd_tx_buff,"AT+MIPLDELOBJ=0,",strlen("AT+MIPLDELOBJ=0,"));

	 nbiot_itoa(objid,tmp,10);
	 strcat(cmd_tx_buff,tmp);
	 strcat(cmd_tx_buff,"\r\n");

	 SendCmd(cmd_tx_buff,"OK",300,0,5);
}

size_t bcxx_register_request(uint8_t *buffer,size_t buffer_len)
{
	size_t len = 0;
	char status = 0;

	len = strlen("AT+MIPLOPEN=0,1000,90\r\n") + 1;

	if(len < buffer_len)
	{
		memcpy(buffer,"AT+MIPLOPEN=0,1000,90\r\n",len);

		status=SendCmd("AT+MIPLOPEN=0,1000,90\r\n","OK",300,0,5);

		if(status == 2)
		{
//			SendCmd("AT+MIPLCLOSE=0\r\n","OK",300,0,5);
		}

		return len;
	}

	return 0;
}

size_t bcxx_register_update(uint16_t lifttime, uint8_t *buffer,size_t buffer_len)
{
	size_t len = 0;
	char ative[6] = {0};

	nbiot_itoa(lifttime,ative,6);

	memcpy(buffer,"AT+MIPLUPDATE=0,",sizeof("AT+MIPLUPDATE=0,"));

	strcat((char *)buffer,ative);
	strcat((char *)buffer,",0\r\n");

	len = strlen((char *)buffer) + 1;

	if(len < buffer_len)
	{
		SendCmd((char *)buffer,"OK",300,0,5);

		return len;
	}

	return 0;
}

size_t bcxx_close_request(uint8_t *buffer,size_t buffer_len)
{
	size_t len = 0;

	len = strlen("AT+MIPLCLOSE=0\r\n") + 1;

	if(len < buffer_len)
	{
		memcpy(buffer,"AT+MIPLCLOSE=0\r\n",len);
		SendCmd("AT+MIPLCLOSE=0\r\n","OK",300,0,5);

		return len;
	}

	return 0;
}

void bcxx_delinstance(void)
{
	 SendCmd("AT+MIPLDELETE=0\r\n","OK",300,0,10);
}

void bcxx_notify_upload(const nbiot_uri_t uri,uint8_t type,char *data)
{
	char tmp[10];
	uint8_t len = 0;

	memset(cmd_tx_buff,0,sizeof(cmd_tx_buff));

	memcpy(cmd_tx_buff,"AT+MIPLNOTIFY=0",strlen("AT+MIPLNOTIFY=0"));

	strcat(cmd_tx_buff,",");
	nbiot_itoa(uri.msgid,tmp,10);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	nbiot_itoa(uri.objid,tmp,10);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	nbiot_itoa(uri.instid,tmp,10);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	nbiot_itoa(uri.resid,tmp,10);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	nbiot_itoa(type,tmp,1);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");

	if(type == NBIOT_STRING)
	{
		len = strlen(data);
		nbiot_itoa(len,tmp,10);
	}
	else if(type == NBIOT_BINARY)
	{
		len = strlen(data);
		nbiot_itoa(len,tmp,10);
	}
	else if(type == NBIOT_INTEGER)
	{
		len = 2;
		nbiot_itoa(len,tmp,1);
	}
	else if(type == NBIOT_FLOAT)
	{
		len = 4;
		nbiot_itoa(len,tmp,1);
	}
	else
	{
		len = 1;
		nbiot_itoa(len,tmp,1);
	}

	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");

	if(type == NBIOT_STRING || type == NBIOT_BINARY)
	{
		strcat(cmd_tx_buff,"\"");
		strcat(cmd_tx_buff,data);
		strcat(cmd_tx_buff,"\"");
	}
	else
	{
		strcat(cmd_tx_buff,data);
	}

	strcat(cmd_tx_buff,",");
	nbiot_itoa(0,tmp,1);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	nbiot_itoa(0,tmp,1);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,"\r\n");
#ifdef DEBUG_LOG
	printf("send data:");
	printf("%s\r\n",cmd_tx_buff);
#endif
	SentData(cmd_tx_buff,"OK",100);
}

void bcxx_read_upload(const nbiot_uri_t uri,uint8_t type,char *data)
{
	char tmp[10];
	uint8_t len = 0;

	memset(cmd_tx_buff,0,sizeof(cmd_tx_buff));

	memcpy(cmd_tx_buff,"AT+MIPLREADRSP=0,",strlen("AT+MIPLREADRSP=0,"));

	nbiot_itoa(uri.msgid,tmp,10);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	nbiot_itoa(1,tmp,1);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	nbiot_itoa(uri.objid,tmp,10);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	nbiot_itoa(uri.instid,tmp,10);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	nbiot_itoa(uri.resid,tmp,10);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	nbiot_itoa(type,tmp,1);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");

	if(type == NBIOT_STRING)
	{
		len = strlen(data);
		nbiot_itoa(len,tmp,10);
	}
	else if(type == NBIOT_BINARY)
	{
		len = strlen(data);
		nbiot_itoa(len,tmp,10);
	}
	else if(type == NBIOT_INTEGER)
	{
		len = 2;
		nbiot_itoa(len,tmp,1);
	}
	else if(type == NBIOT_FLOAT)
	{
		len = 4;
		nbiot_itoa(len,tmp,1);
	}
	else
	{
		len = 1;
		nbiot_itoa(len,tmp,1);
	}

	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");

	if(type == NBIOT_STRING || type == NBIOT_BINARY)
	{
		strcat(cmd_tx_buff,"\"");
		strcat(cmd_tx_buff,data);
		strcat(cmd_tx_buff,"\"");
	}
	else
	{
		strcat(cmd_tx_buff,data);
	}

	strcat(cmd_tx_buff,",");
	nbiot_itoa(0,tmp,1);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	nbiot_itoa(0,tmp,1);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,"\r\n");
#ifdef DEBUG_LOG
	printf("read rsp:");
	printf("%s\r\n",cmd_tx_buff);
#endif
	SentData(cmd_tx_buff,"OK",100);
}

void bcxx_observe_rsp(int suc,const nbiot_uri_t uri)
{
	char tmp[10];

	memset(cmd_tx_buff,0,50);

	memcpy(cmd_tx_buff,"AT+MIPLOBSERVERSP=0,",strlen("AT+MIPLOBSERVERSP=0,"));

	nbiot_itoa(uri.msgid,tmp,10);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	nbiot_itoa(suc,tmp,1);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,"\r\n");
#ifdef DEBUG_LOG
	printf("observe rsp:");
	printf("%s\r\n",cmd_tx_buff);
#endif
	SentData(cmd_tx_buff,"OK",100);
}

void bcxx_discover_rsp(const nbiot_uri_t *uri,size_t lenth,char *value)
{
	char tmp[10];

	memset(cmd_tx_buff,0,50);

	memcpy(cmd_tx_buff,"AT+MIPLDISCOVERRSP=0,",strlen("AT+MIPLDISCOVERRSP=0,"));

	nbiot_itoa(uri->msgid,tmp,10);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	strcat(cmd_tx_buff,"1");
	strcat(cmd_tx_buff,",");
	nbiot_itoa(lenth,tmp,10);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	strcat(cmd_tx_buff,"\"");
	strcat(cmd_tx_buff,value);
	strcat(cmd_tx_buff,"\"");
	strcat(cmd_tx_buff,"\r\n");
#ifdef DEBUG_LOG
	printf("discover rsp:");
	printf("%s\r\n",cmd_tx_buff);
#endif
	SentData(cmd_tx_buff,"OK",100);
}

void bcxx_write_rsp(int suc,uint16_t ackid)
{
	char tmp[10];

	memset(cmd_tx_buff,0,50);

	memcpy(cmd_tx_buff,"AT+MIPLWRITERSP=0,",strlen("AT+MIPLWRITERSP=0,"));

	nbiot_itoa(ackid,tmp,10);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	nbiot_itoa(suc,tmp,1);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,"\r\n");
#ifdef DEBUG_LOG
	printf("write rsp:");
	printf("%s\r\n",cmd_tx_buff);
#endif
	SentData(cmd_tx_buff,"OK",100);
}

void bcxx_execute_rsp(int suc,uint16_t ackid)
{
	char tmp[10];

	memset(cmd_tx_buff,0,50);

	memcpy(cmd_tx_buff,"AT+MIPLEXECUTERSP=0,",strlen("AT+MIPLEXECUTERSP=0,"));

	nbiot_itoa(ackid,tmp,10);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,",");
	nbiot_itoa(suc,tmp,1);
	strcat(cmd_tx_buff,tmp);
	strcat(cmd_tx_buff,"\r\n");
#ifdef DEBUG_LOG
	printf("execute rsp:");
	printf("%s\r\n",cmd_tx_buff);
#endif
	SentData(cmd_tx_buff,"OK",100);
}

uint8_t bcxx_get_csq(void)
{
	uint8_t ret = 0;
	unsigned short pos = 0;

	memset(cmd_tx_buff,0,50);

	SendCmd("AT+CSQ\r\n", "+CSQ:", 100,0,1);

	if(search_str((unsigned char *)cmd_rx_buff, "+CSQ:") != -1)
	{
		pos = MyStrstr((u8 *)cmd_rx_buff, "+CSQ:", 128, 5);
		if(pos != 0xFFFF)
		{
			if(cmd_rx_buff[pos + 6] != ',')
			{
				ret = (cmd_rx_buff[pos + 5] - 0x30) * 10 +\
					cmd_rx_buff[pos + 6] - 0x30;
			}
			else
			{
				ret = cmd_rx_buff[pos + 5] - 0x30;
			}

			if(ret == 0 && ret == 99)
			{
				ret = 0;
			}
		}
	}

    return ret;
}

//从模块获取时间
uint8_t bcxx_get_AT_CCLK(char *buf)
{
	uint8_t ret = 0;

	memset(cmd_tx_buff,0,128);

	SendCmd("AT+CCLK?\r\n", "+CCLK:", 100,0,1);

	if(search_str((unsigned char *)cmd_rx_buff, "+CCLK:") != -1)
	{
		get_str1((unsigned char *)cmd_rx_buff, "+CCLK:", 1, "\r\n\r\nOK", 1, (unsigned char *)buf);

		ret = 1;
	}

    return ret;
}

