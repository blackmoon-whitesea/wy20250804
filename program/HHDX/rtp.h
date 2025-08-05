#ifndef __RTP_H__
#define __RTP_H__

#define		RTP_BUF_NUM	8


struct stRtp
{ 
	unsigned short int seq;
	unsigned int timestamp;
	unsigned int ssrc;
//	unsigned int tx_len;
};

extern struct stRtp gsRtp;

extern unsigned char gRtpRxBuf[RTP_BUF_NUM][172];
extern unsigned char gRtpTxBuf[RTP_BUF_NUM][160];
extern unsigned char  gRtpRxNum;
extern unsigned char  gRtpTxNum;

void RTP_tx(void);
void RTP_build(void);


#endif

