#ifndef __NET_RXTX_H__
#define __NET_RXTX_H__

#include "sys.h"
#include "lwip_comm.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"


#define	SIP_TX						0
#define	RTP_TX						1
#define	MSW_TX						2

#define SIP_LOCAL_PORT			5060	//����SIP���ӵĶ˿� 
#define SVR_RMT_PORT			5060	//������������ӵĶ˿� 
#define RTP_LOCAL_PORT			6000	//������������ӵĶ˿� 

#define NET_BUF_SIZE 			1524
#define NET_RXBUFNB        	8     /* ע��ֻ����2^n(2,4,8,16...),����Ϊ�˼��㷽�� */
#define NET_TXBUFNB        	8     /* ע��ֻ����2^n(2,4,8,16...),����Ϊ�˼��㷽�� */ 


//����net flag
struct stNetFlag
{
	u8 udp_conn:	1;		//bit0 =1: UDP����
	u8 netlink:		1;		//1: ��������
	u8 netok:			1;		//1: net ��ʼ���ɹ�
	u8 udppcb_new:1;		//1: ������udppcb
	u8 bind:			1;		//1: ��AOI FS����
	unsigned char f:1;
	unsigned char g:1;
	unsigned char h:1;
};

extern struct stNetFlag gsNetFlag;

struct stNetRx{
	unsigned int	len[NET_RXBUFNB];		//10��ַ�����Ӧ����
	unsigned char	rd;						//������
	unsigned char	wr;						//д����
	
	unsigned short int port[NET_RXBUFNB];
	unsigned char remoteip[NET_RXBUFNB][4];
};
struct stNetTx{
	unsigned int	len[NET_TXBUFNB];		//10��ַ�����Ӧ����
	unsigned char	rd;						//������
	unsigned char	wr;						//д����
	unsigned int	udppcb[NET_TXBUFNB];//���ĸ�udppcb[x]����
};



extern struct stNetRx *gpsNetRx;
extern struct stNetTx *gpsNetTx;
 
extern unsigned char *gpNetRxBuf;
extern unsigned char *gpNetTxBuf;

extern char *gTempPointer;
extern char gTempChar;
extern unsigned int gMswPort;

unsigned char NET_MemMalloc(void);
void NET_MemFree(void);
void NET_RxParse(void);
void NET_UdpRx(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port);
void NET_UdpTx(struct udp_pcb *upcb);
void NET_TxPcbLenWr(u8 pcb,int j);
void NET_udp_connection_close(struct udp_pcb *upcb);
void NET_VariateInit(void);
void NET_UdppcbConnect(struct udp_pcb *upcb,u8 *dst_ip, u16 dst_port, u16 src_port);
void NET_UdppcbNew(void);

#endif
