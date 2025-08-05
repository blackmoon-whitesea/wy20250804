#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//定时器 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/6/16
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	


struct stTimer3{
	u8	timer100ms;	//100ms 基准
	u8	sysled;			//500ms
	u16	sip_reg;		//sip register time
	u8	rtp;
	u8	eeprom;
	u8 	key_reset;
	u8  handset;		//手柄去抖时间100ms
	u8  netlink;		//100ms插入网线去抖
	u8  dialnum;		//3s间隔
	u16 bat_sample;
	u8	bat_discon;
	u8	bat_onhook;
	u8	bat_offhook;
	u8	bat_ringing;
	u8  dtmf_low;
	u8	dtmf_hi;
};

extern struct stTimer3 *gpsT3;



void TIM3_Int_Init(u16 arr,u16 psc);
extern u8 gLedTimer;
#endif
