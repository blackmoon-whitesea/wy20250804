#ifndef _KEY_H
#define _KEY_H
#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//按键输入驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

#define 	KEY_RST				PFin(15)	//PF15	0: be pressed
#define		KEY_NETLINK		PGin(0) //PG0


extern u8 gHandsetFlag;
extern u8 gHfFlag;

void KEY_Init(void);  //IO初始化
void KEY_reset_scan(void);
void KEY_netlink_scan(void);
#endif 
