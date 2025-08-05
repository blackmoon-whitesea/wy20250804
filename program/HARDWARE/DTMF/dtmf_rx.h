
#ifndef __DTMF_RX_H
#define __DTMF_RX_H 

#include "sys.h" 


#define 	DTMF_DV				PEin(15)	//PE15	0: DV
#define 	DTMF_OE				PEout(10)	//PE10	0: OE


extern uint8_t gDtmfRxStep;
extern uint8_t gDtmfRxFlag0;
extern uint8_t gDtmfRxFlag1;
extern uint8_t gCidFlag;


//PE10~PE13: DTMF D0~D3ÊäÈë: 0111 1000 0000 0000
__STATIC_INLINE uint16_t DTMF_D0_3()
{
	return (uint16_t)(((GPIOE->IDR)>>11) & 0x000f);	
}

void DTMF_Init(void);
void DTMF_rx(void);
void DTMF_rx_parse(u8 data);
void DTMF_CID(void);


#endif
