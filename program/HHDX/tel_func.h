#ifndef __TEL_FUNC_H__
#define __TEL_FUNC_H__



//¶¨Òåtel function flag
struct stTelFlag
{
	unsigned char tel_on:	1;		//bit0 =1:Õª»ú
	unsigned char net_link:1;		//=1: 
	unsigned char c:1;
	unsigned char d:1;
	unsigned char e:1;
	unsigned char f:1;
	unsigned char g:1;
	unsigned char h:1;
};

extern struct stTelFlag gsTelFlag;

extern unsigned char gDialNumLen;

void TEL_PinInit(void);
void TEL_OffInit(void);
void TEL_OnOffFunc(void);
void TEL_HandfreeSPK_on(void);
void TEL_HandfreeSPK_off(void);
#endif
