#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//��ʱ�� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/6/16
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	


struct stTimer3{
	u8	timer100ms;	//100ms ��׼
	u8	sysled;			//500ms
	u16	sip_reg;		//sip register time
	u8	rtp;
	u8	eeprom;
	u8 	key_reset;
	u8  handset;		//�ֱ�ȥ��ʱ��100ms
	u8  netlink;		//100ms��������ȥ��
	u8  dialnum;		//3s���
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
