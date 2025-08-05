#ifndef __24CXX_H
#define __24CXX_H
#include "myiic.h"   
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//24CXX 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/6
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	


struct stEepromFlag
{
	unsigned char ans_onoff:	1;		
	unsigned char alarm_led_onoff:	1;		
	unsigned char c:	1;
	unsigned char d:	1;
	
	unsigned char e:	1;
	unsigned char f:	1;
	unsigned char g:	1;
	unsigned char h:	1;
};	


//管理软件(managesoftware)设置参数，并保存到eeprom中
struct stEeprom
{
	u8 own_ip[4];
	u8 mask_ip[4];
	u8 gw_ip[4];
	u8 sv_ip[4];
	u8 own_num[16];		//号码最长为12位
	u8 spk_vol;
	u8 mic_vol;
	u8 music_name;
	u8 ans_time;		
	
	struct stEepromFlag flag;	//8个开关
};




extern u8 gEeprom_len;
extern struct stEeprom *gpsEeprom;




void AT24CXX_Init(void); //初始化IIC
int AT24C02_SequentialRead(u8 Addr, u8 *Data, u8 len);
void AT24C02_PageWrite(u8 Addr, u8 *Data);


void EEPROM_Write(void);
void EEPROM_Read(void);
#endif
















