#ifndef __SIP_FUNC_H__
#define __SIP_FUNC_H__

#include "SipStructure.h"
#include "SipConstant.h"



extern unsigned int gSipRamdom1;
extern unsigned int gSipRamdom2;

extern unsigned char gSipMechanism;
extern unsigned char gSipTxFlow;

//------------------ SipAnalysis.c ------------------
int ANALYSIS_line(char *p_line);
int ANALYSIS_get_lname(char *p_line_name);
void ANALYSIS_line_to_msg(int line_name,char *p_msg,int msg_len);
int ANALYSIS_message(char *p_msg,int msg_len);
void ANALYSIS_msghdr(char *p_msg, int len);
int ANALYSIS_get_msgname(char *p_msg_name);
void ANALYSIS_RxParamInit(void);
void ANALYSIS_get_cseq(char *p_msg,int msg_len);
void ANALYSIS_cseq(char *p_cseq,char num_name);
void ANALYSIS_cseq_type(char *p_msg_name);
void ANALYSIS_sdp(char *p_sdp, int sdp_len);


//------------------ SdpMsgBuilding.c ------------------
int SDP_building(char *SendMsg,struct stUaInfo *UAinfo,int j);
//int SdpOKBuilding(char *SendMsg,struct stUaInfo *UAinfo,struct sip_packet *ipacket,struct sdp_packet *dpacket,int j);

//------------------ SippMsgBuilding.c ------------------
void BUILD_CID_start(void);
void BUILD_CID_stop(void);
void BUILD_CID_offhook(void);
void BUILD_DIAL_DTMF(unsigned char dtmf_val);

void BUILD_register(void);

void BUILD_invite(void);
void BUILD_ack(void);
void BUILD_bye(void);
void BUILD_cancel(void);

void BUILD_trying(void);
void BUILD_ringing(void);
void BUILD_invite_ok(void);
void BUILD_bye_cancel_ok(void);
void BUILD_487(void);


//void SipRingingBuilding(char *SendMsg,struct stUaInfo *UAinfo,struct sip_packet *ipacket);
//void SipAckBuilding(char *SendMsg,struct stUaInfo *UAinfo,struct sip_packet *ipacket);
//void SipByeBuilding(char *SendMsg,struct stUaInfo *UAinfo,struct sip_packet *ipacket);
//void SipOKBuilding(char *SendMsg,struct stUaInfo *UAinfo,struct sip_packet *ipacket,struct sdp_packet *dpacket);

//------------------ StrInt.c ------------------
void Int2String(char *string,unsigned short value);
void String2Int(unsigned int *value,char *string);
void IpInt2Str(char *string,unsigned char *ip_addr);
void Hex2Asc(void *hex,char *asc,unsigned char len);
char *MymallocCopy(char *p_dst,int len,char *p_src);
void Str2IpArray(unsigned char *ip_array,char *string);
char *MymallocCopy(char *p_dst,int len,char *p_src);
unsigned char MISC_uint16_bit_0(unsigned int val, unsigned char bit_num);


//------------------ SipMain.c ------------------
void SIP_UainfoInit(void);
void SIP_date(void);
void SIP_RegisterTx(void);
void SIP_TxFlow(void);
void SIP_CountCallid(char *call_id);
void SIP_CountTag(char *tag);
void SIP_CountBranch(char *branch);

#endif
