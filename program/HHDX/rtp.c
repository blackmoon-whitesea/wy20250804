#include "usart.h"
#include "malloc.h"
#include "net_rxtx.h"
#include "stdio.h"
#include "string.h" 
#include "timer.h"
#include "SipFunction.h"
//#include "Misc.h"
#include "tel_func.h"
#include "rtp.h"
#include "Audio.h"
#include "i2s.h"
#include "cw2015.h"

struct stRtp gsRtp;
u8 gRtpRxBuf[RTP_BUF_NUM][172];	//��Ӧ��ֻ��Ҫ160�ֽڣ���q->payload+12�������?
u8 gRtpTxBuf[RTP_BUF_NUM][160];
u8 gRtpRxNum;
u8 gRtpTxNum;

/*********************************************
1. �ջ�200 OK(INVITE)��ʼ��RTP
2. �ջ�200 OK(bye)ͣ��RTP
*********************************************/
void RTP_tx(void)
{	
//	if((gsSipRxFlag.ok_inv || gsSipTxFlag.ok_inv)				\
//		&&(!(gsSipRxFlag.ok_bye || gsSipTxFlag.ok_bye)))
	if(gsBatFlag.offhook||gsBatFlag.ringing)
	{
		if(gsSipTxFlag.rtp ==0){
			gsSipTxFlag.rtp =1;
			gsSipTxFlag.ringing =0;
			gsRtp.seq =0;
			gsRtp.ssrc =gSipRamdom1;			
			gpsT3->rtp=0;
//			I2S2_SampleRate_Set(8000);	//���ò���
			AUDIO_rec_mode();
		}
		if(gpsT3->rtp>=20){	//ÿ20ms
			gpsT3->rtp=0;
			RTP_build();
		}			
	}
}
/*********************************************
Real-Time Transport Protocol
    [Stream setup by SDP (frame 10)]
    10.. .... = Version: RFC 1889 Version (2)
    ..0. .... = Padding: False
    ...0 .... = Extension: False
    .... 0000 = Contributing source identifiers count: 0
0... .... = Marker: False
Payload type: ITU-T G.711 PCMA (8)
Sequence number: 7
[Extended sequence number: 65543]
Timestamp: 1123143454
Synchronization Source identifier: 0x2acb3b1e (717962014)
Payload: d5d55555555554��
-----------------------------------------------
Version: 80
Payload type: 08
Sequence number:  07
	16λ���� +1
Timestamp: 42 f1 cf 1e (1123143134 -1123143294 = 160)
	32λ���� +160
ssid : 2a cb 3b 1e
	
****************************************************************/
void RTP_build(void)
{
	int j;	
	char *SendMsg;
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	*SendMsg = 0x80;
	*(SendMsg+1) = 0x08;
	
	gsRtp.seq +=1;
	*(SendMsg+2) = ((gsRtp.seq >> 8 ) & (uint8_t)0xFF);
	*(SendMsg+3) = (gsRtp.seq  & (uint8_t)0xFF);
	
	gsRtp.timestamp +=160;
  *(SendMsg+4) = ((gsRtp.timestamp >> 24) & (uint8_t)0xFF);
  *(SendMsg+5) = ((gsRtp.timestamp >> 16) & (uint8_t)0xFF);
  *(SendMsg+6) = ((gsRtp.timestamp >> 8 ) & (uint8_t)0xFF);
  *(SendMsg+7) = ((gsRtp.timestamp) & (uint8_t)0xFF);
  
  *(SendMsg+8) = ((gsRtp.ssrc >> 24) & (uint8_t)0xFF);
  *(SendMsg+9) = ((gsRtp.ssrc >> 16) & (uint8_t)0xFF);
  *(SendMsg+10) = ((gsRtp.ssrc >> 8 ) & (uint8_t)0xFF);
  *(SendMsg+11) = ((gsRtp.ssrc) & (uint8_t)0xFF);
	
	if(gsAudio.rtp_tx_num != gRtpTxNum){
		for(j=0;j<160;j++)
		{
			*(SendMsg+12+j) =gRtpTxBuf[gRtpTxNum][j];			
		}
		gRtpTxNum ++;
		gRtpTxNum &= (~RTP_BUF_NUM);	//ע��������2,4,8,16...
	}
	else{
		for(j=0;j<160;j++)
		{
			*(SendMsg+12+j) =0xD5;		//������ܵ�д��ǰ��ȥ��?
		}
	}	
//	printf("%d=%d,%d\r\n",gsRtp.seq,gsAudio.rtp_tx_num,gRtpTxNum);
//	printf("%d=%d\r\n",gsRtp.seq,gRtpTxNum);

	NET_TxPcbLenWr(RTP_TX,160+12);
//	gpsNetTx->len[gpsNetTx->wr] =160+12;
//	gpsNetTx->wr ++;	
//	gpsNetTx->wr &= (~NET_TXBUFNB);
}



