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
#include "common.h"
#include "usart2.h"


char cmd_tx_buff[512];
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

	nbiot_sleep(8000);
	
	if(bcxx_set_NATSPEED(115200) != 1)
		goto RE_INIT;

	if(bcxx_set_AT_ATE(0) != 1)
		goto RE_INIT;
	
	if(bcxx_get_AT_CGSN() != 1)
		goto RE_INIT;
	
	if(bcxx_get_AT_NCCID() != 1)
		goto RE_INIT;
	
	if(bcxx_get_AT_CIMI() != 1)
		goto RE_INIT;
	
	if(bcxx_set_AT_CFUN(0) != 1)
		goto RE_INIT;
	
	if(bcxx_set_AT_NBAND(DeviceIMSI) != 1)
		goto RE_INIT;
	
	if(bcxx_set_AT_CELL_RESELECTION() != 1)
		goto RE_INIT;
	
	if(bcxx_set_AT_NRB() != 1)
		goto RE_INIT;
	
	if(bcxx_set_AT_CFUN(1) != 1)
		goto RE_INIT;
	
	if(bcxx_set_AT_MIPLCONFIG("183.230.40.39","5683") != 1)
		goto RE_INIT;
	
	if(bcxx_set_AT_CEDRXS(0) != 1)
		goto RE_INIT;
	
	if(bcxx_set_AT_CPSMS(0) != 1)
		goto RE_INIT;
	
	if(bcxx_set_AT_CSCON(1) != 1)
		goto RE_INIT;
	
	if(bcxx_set_AT_CEREG(2) != 1)
		goto RE_INIT;
	
	if(bcxx_set_AT_QREGSWT(2) != 1)
		goto RE_INIT;
	
	if(bcxx_set_AT_NSONMI(3) != 1)
		goto RE_INIT;
	
	if(bcxx_set_AT_CGATT(1) != 1)
		goto RE_INIT;
	
	nbiot_sleep(10000);

	if(!SendCmd("AT+CGATT?\r\n","+CGATT:1", 1000,5,TIMEOUT_5S))
	{
		goto RE_INIT;
	}

	printf("bc35_g init sucess\r\n");
}

//设置回显功能
unsigned char bcxx_set_AT_ATE(char cmd)
{
	unsigned char ret = 0;
    char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);
	
	sprintf(cmd_tx_buf,"ATE%d\r\n", cmd);
	
    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//设定模块波特率
unsigned char bcxx_set_NATSPEED(u32 baud_rate)
{
	unsigned char ret = 0;
    char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);
	
	sprintf(cmd_tx_buf,"AT+NATSPEED?\r\n");

    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);
	
	if(ret == 1)				//波特率默认9600
	{
		memset(cmd_tx_buf,0,64);
		
		sprintf(cmd_tx_buf,"AT+NATSPEED=%d,30,1,2,1,0,0\r\n",baud_rate);
		
		ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);
		
		if(ret == 1)
		{
			USART2_Init(baud_rate);
			
			memset(cmd_tx_buf,0,64);
	
			sprintf(cmd_tx_buf,"AT+NATSPEED?\r\n");

			ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);
		}
	}
	else if(ret == 0)
	{
		USART2_Init(baud_rate);
			
		memset(cmd_tx_buf,0,64);

		sprintf(cmd_tx_buf,"AT+NATSPEED?\r\n");

		ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);
	}

    return ret;
}

unsigned char bcxx_set_AT_CFUN(char cmd)
{
	unsigned char ret = 0;
    char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);
	
	sprintf(cmd_tx_buf,"AT+CFUN=%d\r\n", cmd);
	
    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_90S);

    return ret;
}

//设置频带
unsigned char bcxx_set_AT_NBAND(unsigned char *imsi)
{
	unsigned char ret = 0;
	unsigned char band = 8;
	unsigned char operators_code = 0;
	char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);
	
	if(imsi == NULL)
	{
		return ret;
	}
	
	operators_code = (*(imsi + 3) - 0x30) * 10 + *(imsi + 4) - 0x30;
	
	if(operators_code == 0 ||
	   operators_code == 2 ||
	   operators_code == 4 ||
	   operators_code == 7 ||
	   operators_code == 1 ||
	   operators_code == 6 ||
	   operators_code == 9)
	{
		band = 8;
	}
	else if(operators_code == 3 ||
	        operators_code == 5 ||
	        operators_code == 11)
	{
		band = 5;
	}
	else
	{
		band = 8;
	}
	
	sprintf(cmd_tx_buf,"AT+NBAND=%d\r\n",band);
    
	ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_90S);

    return ret;
}

//获取IMEI号
unsigned char bcxx_get_AT_CGSN(void)
{
	unsigned char ret = 0;
	char buf[32];

    if(SendCmd("AT+CGSN=1\r\n", "+CGSN", 100,0,TIMEOUT_1S) == 1)
    {
		memset(buf,0,32);

		get_str1((unsigned char *)result_ptr->data, "CGSN:", 1, "\r\n", 2, (unsigned char *)buf);
		
		if(strlen(buf) == UU_ID_LEN - 4)
		{
			memcpy(&HoldReg[UU_ID_ADD],"00",2);
			
			memcpy(&HoldReg[UU_ID_ADD + 2],buf,strlen(buf));

			GetDeviceUUID();

			WriteDataFromHoldBufToEeprom(&HoldReg[UU_ID_ADD],UU_ID_ADD, UU_ID_LEN - 2);
			
			ret = 1;
		}
    }

    return ret;
}

//获取ICCID
unsigned char bcxx_get_AT_NCCID(void)
{
	unsigned char ret = 0;
	char buf[32];

    if(SendCmd("AT+NCCID\r\n", "OK", 100,0,TIMEOUT_5S) == 1)
    {
		memset(buf,0,32);

		get_str1((unsigned char *)result_ptr->data, "NCCID:", 1, "\r\n", 2, (unsigned char *)buf);

		if(strlen(buf) == ICC_ID_LEN - 2)
		{
			memcpy(&HoldReg[ICC_ID_ADD],buf,strlen(buf));

			GetDeviceICCID();

			WriteDataFromHoldBufToEeprom(&HoldReg[ICC_ID_ADD],ICC_ID_ADD, ICC_ID_LEN - 2);
			
			ret = 1;
		}
    }

    return ret;
}

//获取IMSI
unsigned char bcxx_get_AT_CIMI(void)
{
	unsigned char ret = 0;
	char buf[32];

    if(SendCmd("AT+CIMI\r\n", "OK", 100,0,TIMEOUT_1S) == 1)
    {
		memset(buf,0,32);

		get_str1((unsigned char *)result_ptr->data, "\r\n", 1, "\r\n", 2, (unsigned char *)buf);

		if(strlen(buf) == IMSI_ID_LEN - 2)
		{
			memcpy(&HoldReg[IMSI_ID_ADD],buf,strlen(buf));

			GetDeviceIMSI();

			WriteDataFromHoldBufToEeprom(&HoldReg[IMSI_ID_ADD],IMSI_ID_ADD, IMSI_ID_LEN - 2);
			
			ret = 1;
		}
    }

    return ret;
}

//打开小区重选功能
unsigned char bcxx_set_AT_CELL_RESELECTION(void)
{
	unsigned char ret = 0;
	
    ret = SendCmd("AT+NCONFIG=CELL_RESELECTION,TRUE\r\n", "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//重启模块
unsigned char bcxx_set_AT_NRB(void)
{
	unsigned char ret = 0;
    
	SendCmd("AT+NRB\r\n", "OK", 1000,0,TIMEOUT_10S);
	
	ret = SendCmd("AT\r\n", "OK", 100,0,TIMEOUT_1S);
	
    return ret;
}

//设置IOT平台IP和端口
unsigned char bcxx_set_AT_NCDP(char *addr, char *port)
{
	unsigned char ret = 0;
    char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);
	
	sprintf(cmd_tx_buf,"AT+NCDP=%s,%s\r\n",addr,port);
	
    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//设置连接状态自动回显
unsigned char bcxx_set_AT_CSCON(unsigned char cmd)
{
	unsigned char ret = 0;
    char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);
	
	sprintf(cmd_tx_buf,"AT+CSCON=%d\r\n",cmd);
	
    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);
	
    return ret;
}

//EPS Network Registration Status
unsigned char bcxx_set_AT_CEREG(unsigned char cmd)
{
	unsigned char ret = 0;
    char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);
	
	sprintf(cmd_tx_buf,"AT+CEREG=%d\r\n",cmd);
	
    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);
	
    return ret;
}

//设置网络数据接收模式HUAWEI IOT
unsigned char bcxx_set_AT_NNMI(unsigned char cmd)
{
	unsigned char ret = 0;
    char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);
	
	sprintf(cmd_tx_buf,"AT+NNMI=%d\r\n",cmd);
	
    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);
	
    return ret;
}

//设置网络数据接收模式 TCP/IP
unsigned char bcxx_set_AT_NSONMI(unsigned char cmd)
{
	unsigned char ret = 0;
    char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);
	
	sprintf(cmd_tx_buf,"AT+NSONMI=%d\r\n",cmd);
	
    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);
	
    return ret;
}

unsigned char bcxx_set_AT_CGATT(unsigned char cmd)
{
	unsigned char ret = 0;
    char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);
	
	sprintf(cmd_tx_buf,"AT+CGATT=%d\r\n",cmd);
	
    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_2S);
	
    return ret;
}

unsigned char bcxx_set_AT_QREGSWT(unsigned char cmd)
{
	unsigned char ret = 0;
    char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);
	
	sprintf(cmd_tx_buf,"AT+QREGSWT=%d\r\n",cmd);
	
    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_2S);
	
    return ret;
}

//设置eDRX开关
unsigned char bcxx_set_AT_CEDRXS(unsigned char cmd)
{
	unsigned char ret = 0;
    char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);
	
	sprintf(cmd_tx_buf,"AT+CEDRXS=%d,5\r\n",cmd);
	
    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);
	
    return ret;
}

//设置PMS开关
unsigned char bcxx_set_AT_CPSMS(unsigned char cmd)
{
	unsigned char ret = 0;
    char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);
	
	sprintf(cmd_tx_buf,"AT+CPSMS=%d\r\n",cmd);
	
    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);
	
    return ret;
}

//向IOT平台发送数据
unsigned char bcxx_set_AT_NMGS(unsigned int len,char *buf)
{
	unsigned char ret = 0;
    char cmd_tx_buf[256];
   
	memset(cmd_tx_buf,0,256);
	
	sprintf(cmd_tx_buf,"AT+NMGS=%d,%s\r\n",len,buf);
	
    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//获取本地IP地址
unsigned char bcxx_get_AT_CGPADDR(char **ip)
{
	unsigned char ret = 0;
	unsigned char len = 0;
	unsigned char new_len = 0;
	unsigned char msg[20];

    if(SendCmd("AT+CGPADDR\r\n", ",", 100,20,TIMEOUT_1S) == 1)
    {
		memset(msg,0,20);

		get_str1((unsigned char *)result_ptr->data, "+CGPADDR:0,", 1, "\r\nOK", 1, (unsigned char *)msg);

		new_len = strlen((char *)msg);

		if(new_len != 0)
		{
			if(*ip == NULL)
			{
				*ip = (char *)mymalloc(sizeof(u8) * len + 1);
			}

			if(*ip != NULL)
			{
				len = strlen((char *)*ip);

				if(len == new_len)
				{
					memset(*ip,0,new_len + 1);
					memcpy(*ip,msg,new_len);
					ret = 1;
				}
				else
				{
					myfree(*ip);
					*ip = (char *)mymalloc(sizeof(u8) * new_len + 1);
					if(ip != NULL)
					{
						memset(*ip,0,new_len + 1);
						memcpy(*ip,msg,new_len);
						len = new_len;
						new_len = 0;
						ret = 1;
					}
				}
			}
		}
    }

    return ret;
}

//新建一个SOCKET
unsigned char bcxx_set_AT_NSOCR(char *type, char *protocol,char *port)
{
	unsigned char ret = 255;
	char cmd_tx_buf[64];
	unsigned char buf[3] = {0,0,0};
   
	memset(cmd_tx_buf,0,64);
	
	sprintf(cmd_tx_buf,"AT+NSOCR=%s,%s,%s,1\r\n",type,protocol,port);

    if(SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S) == 1)
    {
		get_str1((unsigned char *)result_ptr->data, "\r\n", 1, "\r\n", 2, (unsigned char *)buf);
		if(strlen((char *)buf) == 1)
		{
			ret = buf[0] - 0x30;
		}
    }

    return ret;
}

//关闭一个SOCKET
unsigned char bcxx_set_AT_NSOCL(unsigned char socket)
{
	unsigned char ret = 0;
	char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);
	
	sprintf(cmd_tx_buf,"AT+NSOCL=%d\r\n",socket);
	
    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//向UDP服务器发送数据并等待响应数据
unsigned char bcxx_set_AT_NSOFT(unsigned char socket, char *ip,char *port,unsigned int len,char *inbuf)
{
	unsigned char ret = 0;
    char cmd_tx_buf[256];

	memset(cmd_tx_buf,0,256);
	
	sprintf(cmd_tx_buf,"AT+NSOST=%d,%s,%s,%d,%s\r\n",socket,ip,port,len,inbuf);
	
    ret = SendCmd(cmd_tx_buf, "+NSOSTR:", 100,0,TIMEOUT_60S);
	
    return ret;
}

//建立一个TCP连接
unsigned char bcxx_set_AT_NSOCO(unsigned char socket, char *ip,char *port)
{
	unsigned char ret = 0;
    char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);
	
	sprintf(cmd_tx_buf,"AT+NSOCO=%d,%s,%s\r\n",socket,ip,port);
	
    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//通过TCP连接发送数据
unsigned char bcxx_set_AT_NSOSD(unsigned char socket, unsigned int len,char *inbuf)
{
	unsigned char ret = 0;
	char cmd_tx_buf[512];

	memset(cmd_tx_buf,0,512);
	
	sprintf(cmd_tx_buf,"AT+NSOSD=%d,%d,%s,0x100,100\r\n",socket,len,inbuf);
	
    ret = SendCmd(cmd_tx_buf, "+NSOSTR:", 100,0,TIMEOUT_60S);
	
    return ret;
}

//获取信号强度
unsigned char bcxx_get_AT_CSQ(void)
{
	u8 ret = 0;
	u8 i = 0;
	char *msg = NULL;
	char tmp[10];

	if(SendCmd("AT+CSQ\r\n", "OK", 100,0,TIMEOUT_1S) == 1)
	{
		msg = strstr((char *)result_ptr->data,"+CSQ:");

		if(msg == NULL)
			return 0;

		memset(tmp,0,10);

		msg = msg + 5;

		while(*msg != ',')
		tmp[i ++] = *(msg ++);
		tmp[i] = '\0';

		ret = nbiot_atoi(tmp,strlen(tmp));

		if(ret == 0 || ret >= 99)
		{
			ret = 0;
		}
	}

	return ret;
}

//从模块获取时间
unsigned char bcxx_get_AT_CCLK(char *buf)
{
	unsigned char ret = 0;
    
    if(SendCmd("AT+CCLK?\r\n", "OK", 100,0,TIMEOUT_1S) == 1)
    {
        if(search_str((unsigned char *)result_ptr->data, "+CCLK:") != -1)
		{
			get_str1((unsigned char *)result_ptr->data, "+CCLK:", 1, "\r\n\r\nOK", 1, (unsigned char *)buf);

			ret = 1;
		}
    }

    return ret;
}

//设置OneNET平台IP和PORT
unsigned char bcxx_set_AT_MIPLCONFIG(char *ip,char *port)
{
	unsigned char ret = 0;
	char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);
	
	sprintf(cmd_tx_buf,"AT+MIPLCONFIG=1,%s,%s\r\n",ip,port);
	
    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);
	
    return ret;
}

void bcxx_create(void)
{
	 SendCmd("AT+MIPLCREATE\r\n","+MIPLCREATE:0\r\n\r\nOK",300,0,TIMEOUT_10S);
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

	SendCmd(cmd_tx_buff,"OK",300,0,TIMEOUT_5S);
}

void bcxx_delobj(uint16_t objid)
{
	 char tmp[10];

	 memset(cmd_tx_buff,0,50);
	 memcpy(cmd_tx_buff,"AT+MIPLDELOBJ=0,",strlen("AT+MIPLDELOBJ=0,"));

	 nbiot_itoa(objid,tmp,10);
	 strcat(cmd_tx_buff,tmp);
	 strcat(cmd_tx_buff,"\r\n");

	 SendCmd(cmd_tx_buff,"OK",300,0,TIMEOUT_5S);
}

size_t bcxx_register_request(uint8_t *buffer,size_t buffer_len)
{
	size_t len = 0;
	char status = 0;

	len = strlen("AT+MIPLOPEN=0,200,90\r\n") + 1;

	if(len < buffer_len)
	{
		memcpy(buffer,"AT+MIPLOPEN=0,200,90\r\n",len);

		status=SendCmd("AT+MIPLOPEN=0,200,90\r\n","OK",300,0,TIMEOUT_5S);

		if(status == 2)
		{
//			SendCmd("AT+MIPLCLOSE=0\r\n","OK",300,0,TIMEOUT_5S);
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
		SendCmd((char *)buffer,"OK",300,0,TIMEOUT_5S);

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
		SendCmd("AT+MIPLCLOSE=0\r\n","OK",300,0,TIMEOUT_5S);

		return len;
	}

	return 0;
}

void bcxx_delinstance(void)
{
	 SendCmd("AT+MIPLDELETE=0\r\n","OK",300,0,TIMEOUT_10S);
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

