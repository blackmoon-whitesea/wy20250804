#ifndef __USART3_H
#define __USART3_H

#include "sys.h" 

#define 	CID_LEN			128
#define 	CID_RINGING				PEin(7)	//PE7	0: Ringing

//定义cid flag
struct stCidFlag
{
	u8 parse:		1;		//bit0 =1: 没插电话线
	u8 b:		1;		//1: 挂机
	u8 c:		1;		//1: 摘机
	u8 d:		1;		//1: 振铃
	u8 e:			1;
	u8 f:			1;
	u8 g:			1;
	u8 h:			1;
};

extern struct stCidFlag gsCidFlag;


extern u8 gRingCount;
extern u8 gCallerIDBuffer[CID_LEN];
extern u16 gCidIndex;
extern u8 gCidReady;
extern u8 gFskCidBuf[17];
extern u8 gFskOwnBuf[17];


void USART3_Init(u32 bound);
u8 FSK_parse(void);
void CID_print(void);
void FSK_RTP_stop(void);
void FSK_ringing(void);

#endif

