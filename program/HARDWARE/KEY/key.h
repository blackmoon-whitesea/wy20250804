#ifndef _KEY_H
#define _KEY_H
#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//����������������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

#define 	KEY_RST				PFin(15)	//PF15	0: be pressed
#define		KEY_NETLINK		PGin(0) //PG0


extern u8 gHandsetFlag;
extern u8 gHfFlag;

void KEY_Init(void);  //IO��ʼ��
void KEY_reset_scan(void);
void KEY_netlink_scan(void);
#endif 
