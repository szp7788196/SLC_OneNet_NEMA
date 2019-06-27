#include "net_protocol.h"
#include "rtc.h"
#include "usart.h"
#include "24cxx.h"
#include "common.h"




////开关灯
//void ControlLightSwitch(void)
//{
//	if(light_control_switch.value.as_bool == true)
//	{
//		LightLevelPercent = 200;
//	}
//	else
//	{
//		LightLevelPercent = 0;
//	}

//	DeviceWorkMode = MODE_MANUAL;

//	memcpy(&HoldReg[LIGHT_LEVEL_ADD],&LightLevelPercent,LIGHT_LEVEL_LEN - 2);
//	WriteDataFromHoldBufToEeprom(&HoldReg[LIGHT_LEVEL_ADD],LIGHT_LEVEL_ADD, LIGHT_LEVEL_LEN - 2);
//}

//调光
void ControlLightLevel(void)
{
	LightLevelPercent = 2 * light_control_dimmer.value.as_int;

	DeviceWorkMode = MODE_MANUAL;

	memcpy(&HoldReg[LIGHT_LEVEL_ADD],&LightLevelPercent,LIGHT_LEVEL_LEN - 2);
	WriteDataFromHoldBufToEeprom(&HoldReg[LIGHT_LEVEL_ADD],LIGHT_LEVEL_ADD, LIGHT_LEVEL_LEN - 2);
}

//设置UUID
void SetDeviceUUID(void)
{
	if(device_uuid.value.as_buf.len == UU_ID_LEN - 2)
	{
		memcpy(&HoldReg[UU_ID_ADD],device_uuid.value.as_buf.val,device_uuid.value.as_buf.len);

		GetDeviceUUID();

		WriteDataFromHoldBufToEeprom(&HoldReg[UU_ID_ADD],UU_ID_ADD, UU_ID_LEN - 2);
	}
}

//OTA
void SetUpdateFirmWareInfo(void)
{
	u8 i = 0;
	u8 temp_buf[12];

	if(ota.value.as_buf.len == 11)
	{
		memset(temp_buf,0,12);
		
		memcpy(temp_buf,ota.value.as_buf.val,ota.value.as_buf.len);

		for(i = 0; i < 11; i ++)
		{
			temp_buf[i] -= 0x30;
		}

		NewFirmWareVer    = (((u16)temp_buf[0]) * 1000) + (((u16)temp_buf[1]) * 100) + (((u16)temp_buf[2]) * 10) + (u16)temp_buf[3];
		NewFirmWareBagNum = (((u16)temp_buf[4]) * 1000) + (((u16)temp_buf[5]) * 100) + (((u16)temp_buf[6]) * 10) + (u16)temp_buf[7];
		LastBagByteNum    = (((u16)temp_buf[8]) * 100) + (((u16)temp_buf[9]) * 10) + (u16)temp_buf[10];

		if(NewFirmWareBagNum == 0 || NewFirmWareBagNum > MAX_FW_BAG_NUM \
			|| NewFirmWareVer == 0 || NewFirmWareVer > MAX_FW_VER \
			|| LastBagByteNum == 0 || LastBagByteNum > MAX_FW_LAST_BAG_NUM)  //128 + 2 + 4 = 134
		{
			return;
		}
		else
		{
			HaveNewFirmWare = 0xAA;
			if(NewFirmWareAdd == 0xAA)
			{
				NewFirmWareAdd = 0x55;
			}
			else if(NewFirmWareAdd == 0x55)
			{
				NewFirmWareAdd = 0xAA;
			}
			else
			{
				NewFirmWareAdd = 0xAA;
			}

			WriteOTAInfo(HoldReg,0);

//			NeedToReset = 1;				//Éè±¸ÖØÆô£¬½øÈëBootLoader
		}
	}
}

//复位
void ControlDeviceReset(void)
{
	if(search_str((unsigned char *)reset.value.as_buf.val, "reset") != -1)
	{
		NeedToReset = 1;
	}
}

//切换模式
void SetDeviceWorkMode(void)
{
	if(search_str((unsigned char *)work_mode.value.as_buf.val, "auto") != -1)
	{
		DeviceWorkMode = MODE_AUTO;
	}
	else if(search_str((unsigned char *)work_mode.value.as_buf.val, "manual") != -1)
	{
		DeviceWorkMode = MODE_MANUAL;
	}
}

//更改上传间隔
void SetDeviceUpLoadINCL(void)
{
	u16 up_load_incl = 0;
	u8 incl[2];

	up_load_incl = (u16)(data_upload_interval.value.as_float);

	if(up_load_incl <= MAX_UPLOAD_INVL)
	{
		incl[0] = (u8)((up_load_incl >> 8) & 0x00FF);
		incl[1] = (u8)(up_load_incl & 0x00FF);
		
		UpLoadINCL = up_load_incl;

		memcpy(&HoldReg[UPLOAD_INVL_ADD],incl,2);
		WriteDataFromHoldBufToEeprom(&HoldReg[UPLOAD_INVL_ADD],UPLOAD_INVL_ADD, UPLOAD_INVL_LEN - 2);
	}
}

//切换电源接口
void SetDevicePowerIntfc(void)
{
	u8 intfc = 0;

	intfc = (u16)(power_interface.value.as_float);

	if(intfc <= 3)
	{
		PowerINTFC = intfc;	

		memcpy(&HoldReg[POWER_INTFC_ADD],&PowerINTFC,POWER_INTFC_ADD - 2);

		WriteDataFromHoldBufToEeprom(&HoldReg[POWER_INTFC_ADD],POWER_INTFC_ADD, POWER_INTFC_LEN - 2);
	}
}

//设置时间策略
void SetRegularTimeGroups(void)
{
	u8 group_num = 0;
	u16 i = 0;
	u8 temp_buf[32];
	u8 time_group[32];
	u16 crc16 = 0;

	if(time_strategy.value.as_buf.len == 16)
	{
		memset(temp_buf,0,32);
		memset(time_group,0,32);

		memcpy(temp_buf,time_strategy.value.as_buf.val,time_strategy.value.as_buf.len);

		for(i = 0; i < 16; i ++)
		{
			temp_buf[i] -= 0x30;
		}

		group_num = temp_buf[0] * 10 + temp_buf[1];

		if(group_num <= MAX_GROUP_NUM)
		{
			time_group[0] = temp_buf[2];												//type
			time_group[1] = temp_buf[3] * 10 + temp_buf[4];								//year
			time_group[2] = temp_buf[5] * 10 + temp_buf[6];								//month
			time_group[3] = temp_buf[7] * 10 + temp_buf[8];								//date
			time_group[4] = temp_buf[9] * 10 + temp_buf[10];							//hour
			time_group[5] = temp_buf[11] * 10 + temp_buf[12];							//minute
			time_group[6] = temp_buf[13] * 100 + temp_buf[14] * 10 + temp_buf[15];		//percent

			if(time_group[0] != 0)
			{
				pRegularTime tmp_time = NULL;
				
				tmp_time = (pRegularTime)mymalloc(sizeof(RegularTime_S));

				tmp_time->prev = NULL;
				tmp_time->next = NULL;
				
				crc16 = CRC16(&time_group[0],7);
				time_group[7] = (u8)(crc16 >> 8);
				time_group[8] = (u8)(crc16 & 0x00FF);

				tmp_time->number	= group_num;
				tmp_time->type 		= time_group[0];
				tmp_time->year 		= time_group[1];
				tmp_time->month 	= time_group[2];
				tmp_time->date 		= time_group[3];
				tmp_time->hour 		= time_group[4];
				tmp_time->minute 	= time_group[5];
				tmp_time->percent 	= time_group[6];
				
				switch(tmp_time->type)
				{
					case TYPE_WEEKDAY:
						RegularTimeGroupAdd(TYPE_WEEKDAY,tmp_time);
					break;
					
					case TYPE_HOLIDAY_START:
						RegularTimeGroupAdd(TYPE_HOLIDAY_START,tmp_time);
					break;
					
					case TYPE_HOLIDAY_END:
						RegularTimeGroupAdd(TYPE_HOLIDAY_END,tmp_time);
					break;
					
					default:
						
					break;
				}
			}
			else
			{
				RemoveAllStrategy();
			}
			
			for(i = 0; i < TIME_RULE_LEN; i ++)
			{
				AT24CXX_WriteOneByte(TIME_RULE_ADD + group_num * TIME_RULE_LEN + i,time_group[i]);
			}
		}
	}
}




















