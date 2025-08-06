#ifndef __24CXX_H
#define __24CXX_H
#include "myiic.h"   
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//ALIENTEK STM32F407������
//24CXX ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/6
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) �������������ӿƼ����޹�˾ 2014-2024
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


//��������(managesoftware)���ò����������浽eeprom��
struct stEeprom
{
	u8 own_ip[4];
	u8 mask_ip[4];
	u8 gw_ip[4];
	u8 sv_ip[4];
	u8 port;
	u8 own_num[16];		//�����Ϊ12λ
	u8 spk_vol;
	u8 mic_vol;
	u8 music_name;
	u8 ans_time;		
	
	struct stEepromFlag flag;	//8������
};




extern u8 gEeprom_len;
extern struct stEeprom *gpsEeprom;




void AT24CXX_Init(void); //��ʼ��IIC
int AT24C02_SequentialRead(u8 Addr, u8 *Data, u8 len);
void AT24C02_PageWrite(u8 Addr, u8 *Data);


void EEPROM_Write(void);
void EEPROM_Read(void);
#endif
















