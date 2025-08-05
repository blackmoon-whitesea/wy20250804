
#ifndef __24C02_H
#define __24C02_H

#include "iic.h"   

//定义bat flag
struct stBatFlag
{
	u8 discon:		1;		//bit0 =1: 没插电话线
	u8 onhook:		1;		//1: 挂机
	u8 offhook:		1;		//1: 摘机
	u8 ringing:		1;		//1: 振铃
	u8 flag1:			1;
	u8 flag2:			1;
	u8 flag3:			1;
	u8 flag4:			1;
};

extern struct stBatFlag gsBatFlag;

extern unsigned int gBatVal;
extern u8 onhook_delay;

#define  QSTART    PFout(3)   
					  
void CW2015_WriteOneByte(u16 RegisterAddr,u8 DataToWrite);
u8 CW2015_ReadOneByte(u8 ReadAddr); 
void CW2015_Init(void);                               
void CW2015_Data(void);
void CW2015_Display(u8 energy);
void CW2015_BatSample(void);
void CW2015_BatStatus(void);
#endif
















