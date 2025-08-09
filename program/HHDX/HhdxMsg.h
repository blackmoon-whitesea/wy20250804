#ifndef __HHDX_MANAGE_SOFTWARE_H__
#define __HHDX_MANAGE_SOFTWARE_H__
#include "sys.h"

#define HHDX_PARAM	1
#define HHDX_FUNC		2
#define HHDX_REC_START	3
#define HHDX_REC_STOP	4
#define HHDX_REC_OFFHOOK	5

#define	JS_RESET					10
#define	JS_PARAM_READ				11

#define JS_LOCAL_IP					12
#define JS_MASK							13
#define JS_GW_IP						14
#define JS_SV_IP						15
#define JS_LOCAL_NUM				16
#define JS_HF_SKP_VOL				17
#define JS_HF_MIC_VOL				18
#define JS_MUSIC_NAME				19
#define JS_ANS_TIME					20
#define JS_ANS_ONOFF				21
#define JS_ALARM_LED_ONOFF	22


#define JS_TEL_ONOFF				50
#define JS_DIAL_NUMS				51
#define JS_DIAL_NUM					52
#define JS_REC_START			    53
#define JS_REC_STOP				    54
#define JS_REC_OFFHOOK			    55

struct stHhdxFlag
{
	unsigned char sv_ip:		1;		//bit0 =1:����sip ip
	unsigned char eepromwr:	1;		//=1: ��ʼдeeprom		0:����
	unsigned char c:1;
	unsigned char d:1;
	unsigned char e:1;
	unsigned char f:1;
	unsigned char g:1;
	unsigned char h:1;
};

extern struct stHhdxFlag gsHhdxFlag;



void HHDX_FactoryReset(void);
void HHDX_VariateInit(void);


int HHDX_Command(char *p_line);
int HHDX_DivideJson(int cmd_type,char *p_msg,int msg_len);
char *HHDX_GetFieldName(int cmd_type,char *p_msg);
char *HHDX_GetFieldValue(char name,char *p_msg);
char HHDX_AnalyseJsonName(int cmd_type,char *p_msg);
void HHDX_AnalyseJsonValue(char name,char *p_msg);
// int HHDX_FuncRx(char *p_msg_name);
int HHDX_FuncRx(int cmd_type, char *p_msg_name);
int HHDX_ParamRx(char *p_msg_name);

void HHDX_ParamTxBuild(void);

#endif
	
