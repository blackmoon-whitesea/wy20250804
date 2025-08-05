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

#define SIP_LOCAL_PORT			5060	//定义SIP连接的端口 
#define SVR_RMT_PORT				5060	//定义服务器连接的端口 
#define RTP_LOCAL_PORT			6000	//定义服务器连接的端口 

#define NET_BUF_SIZE 			1524
#define NET_RXBUFNB        	8     /* 注：只能是2^n(2,4,8,16...),纯粹为了计算方便 */
#define NET_TXBUFNB        	8     /* 注：只能是2^n(2,4,8,16...),纯粹为了计算方便 */ 


//定义net flag
struct stNetFlag
{
	u8 udp_conn:	1;		//bit0 =1: UDP连接
	u8 netlink:		1;		//1: 插入网线
	u8 netok:			1;		//1: net 初始化成功
	u8 udppcb_new:1;		//1: 创建新udppcb
	u8 bind:			1;		//1: 绑定AOI FS网络
	unsigned char f:1;
	unsigned char g:1;
	unsigned char h:1;
};

extern struct stNetFlag gsNetFlag;

struct stNetRx{
	unsigned int	len[NET_RXBUFNB];		//10地址数组对应长度
	unsigned char	rd;						//读出数
	unsigned char	wr;						//写入数
	
	unsigned short int port[NET_RXBUFNB];
	unsigned char remoteip[NET_RXBUFNB][4];
};
struct stNetTx{
	unsigned int	len[NET_TXBUFNB];		//10地址数组对应长度
	unsigned char	rd;						//读出数
	unsigned char	wr;						//写入数
	unsigned int	udppcb[NET_TXBUFNB];//向哪个udppcb[x]发送
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
