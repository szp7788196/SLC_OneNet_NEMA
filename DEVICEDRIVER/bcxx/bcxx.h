#ifndef __BCXX_H
#define __BCXX_H

#include <stdint.h>
#include <string.h>
#include "internal.h"


#define BCXX_RST_HIGH		GPIO_SetBits(GPIOC,GPIO_Pin_2)
#define BCXX_RST_LOW		GPIO_ResetBits(GPIOC,GPIO_Pin_2)

#define BCXX_PWREN_HIGH		GPIO_SetBits(GPIOC,GPIO_Pin_3)
#define BCXX_PWREN_LOW		GPIO_ResetBits(GPIOC,GPIO_Pin_3)


#define BCXX_PRINTF_RX_BUF


typedef enum
{
	MIPL_DEBUG_LEVEL_NONE = 0,
	MIPL_DEBUG_LEVEL_RXL,
	MIPL_DEBUG_LEVEL_RXL_RXD,
	MIPL_DEBUG_LEVEL_TXL_TXD,
} MIPL_DEBUG_LEVEL_E;

//接收数据的状态
typedef enum
{
    WAITING = 0,
    RECEIVING = 1,
    RECEIVED  = 2,
    TIMEOUT = 3
} CMD_STATE_E;


#define MIPL_BLOCK1  		5
#define MIPL_BLOCK2  		5
#define MIPL_BLOCK2TH  		2
#define MIPL_PORT  			0
#define MIPL_KEEPALIVE  	300
#define MIPL_DEBUG  		MIPL_DEBUG_LEVEL_NONE
#define MIPL_BOOT  			0
#define MIPL_ENCRYPT  		0

#define NBIOT_SOCK_BUF_SIZE 1024

typedef struct
{
	uint8_t boot;
	uint8_t encrypt;
	MIPL_DEBUG_LEVEL_E debug;
	uint16_t port;
	uint32_t keep_alive;
	size_t uri_len;
	const char* uri;
	size_t ep_len;
	const char* ep;
	uint8_t block1;		//COAP option BLOCK1(PUT or POST),0-6. 2^(4+n)  bytes
	uint8_t block2;		//COAP option BLOCK2(GET),0-6. 2^(4+n)  bytes
	uint8_t block2th;	//max size to trigger block-wise operation,0-2. 2^(8+n) bytes
} MIPL_T;


extern char cmd_rx_buff[256];

void bcxx_hard_init(void);
void netdev_init(void);

uint32_t ip_SendData(int8_t * buf, uint32_t len);

void netif_rx(uint8_t*buf,uint16_t *read);

void bcxx_create(void);
void bcxx_addobj(uint16_t objid,uint8_t insCount,char *insBitmap,uint16_t attrcount,uint16_t actcount);
void bcxx_delobj(uint16_t objid);
size_t bcxx_register_request(uint8_t *buffer,size_t buffer_len);
size_t bcxx_register_update(uint16_t lifttime,uint8_t *buffer,size_t buffer_len);
size_t bcxx_close_request(uint8_t *buffer,size_t buffer_len);
void bcxx_delinstance(void);
void bcxx_notify_upload(const nbiot_uri_t uri,uint8_t type,char *data);
void bcxx_read_upload(const nbiot_uri_t uri,uint8_t type,char *data);
void bcxx_observe_rsp(int suc,const nbiot_uri_t uri);
void bcxx_discover_rsp(const nbiot_uri_t *uri,size_t lenth,char *value);
void bcxx_write_rsp(int suc,uint16_t ackid);
void bcxx_execute_rsp(int suc,uint16_t ackid);
uint8_t bcxx_get_csq(void);
uint8_t bcxx_get_AT_CCLK(char *buf);

#endif
