#include "delay.h"
#include "usart.h"
#include "malloc.h"
#include "net_rxtx.h"
#include "stdio.h"
#include "string.h" 
#include "timer.h"
#include "led.h"
#include "SipFunction.h"
#include "tel_func.h"
#include "rtp.h"
#include "Audio.h"
#include "HhdxMsg.h"
#include "24cxx.h" 
#include "cw2015.h" 

char *gTempPointer;
char gTempChar;
unsigned int gMswPort;

uint8_t *gpNetRxBuf;
uint8_t *gpNetTxBuf;
struct stNetRx *gpsNetRx;
struct stNetTx *gpsNetTx;
struct udp_pcb *gpsaUdppcb[3];  		//定义3个UDP服务器控制块

struct stNetFlag gsNetFlag;	

/*
写入发送区地址++
1. 先择udppcb[x]
2. 发送length
*/
void NET_TxPcbLenWr(u8 pcb,int j)
{
	gpsNetTx->udppcb[gpsNetTx->wr] =pcb;
	gpsNetTx->len[gpsNetTx->wr] =j;
	gpsNetTx->wr ++;	
	gpsNetTx->wr &= (~NET_TXBUFNB);
}

void NET_RxParse(void)
{
//	int i;	
	
	int name_index;
	char *addr;
	int len;
		
	addr = (char *)(gpNetRxBuf + gpsNetRx->rd*NET_BUF_SIZE);
	len = gpsNetRx->len[gpsNetRx->rd];
	
//	printf("len=%d\r\n",len);
//	for(i =0; i< gpsNetRx->len[gpsNetRx->rd]; i++){
//		printf("%c",*((gpNetRxBuf + gpsNetRx->rd*NET_BUF_SIZE)+i));
//	}	
//	printf("\r\n");	

//------------------ sip --------------------
	if(gpsNetRx->port[gpsNetRx->rd] == SIP_PORT){
		name_index = ANALYSIS_line(addr);
		if(name_index){
			ANALYSIS_line_to_msg(name_index,addr,len);		//??????又会addr从头开始比较
		}
	}
//------------------ manage/other --------------------	
	else{
		name_index = HHDX_Command(addr);
		if(name_index){
			HHDX_DivideJson(name_index,gTempPointer,len);
		}
	}
}

//UDP接收回调函数
void NET_UdpRx(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
{
	struct pbuf *q;
	uint8_t *dst_addr;
	struct ip_addr ip_addr_t;
	if(p!=NULL)	//接收到不为空的数据时
	{
	// ----------- RTP ---------------
		if(port == gpsUaInfo->rtp_port)			//注：manage port有可能=rtp port!!!!!需增加判断第一个字节=‘0x80’
		{
			q=p;
			if(q->len ==172){	
				memcpy(gRtpRxBuf[gRtpRxNum],q->payload,172);	//拷贝数据，本应该从第12字节从开始，但q->payload+12编译出错
//				memcpy(gRtpRxBuf[gRtpRxNum],(char *)(q->payload+12),160);	//等打电话时再测试
//				printf("l=%d,0x%x,0x%x\r\n",q->len,gRtpRxBuf[gRtpRxNum][3],gRtpRxBuf[gRtpRxNum][12]);
				gRtpRxNum ++;
				gRtpRxNum &= (~RTP_BUF_NUM);	//注：必须是2,4,8,16...
				
			}
		}
	// ----------- Other(SIP/manage) ---------------
		else{
			dst_addr =(gpNetRxBuf + gpsNetRx->wr*NET_BUF_SIZE);
			gpsNetRx->len[gpsNetRx->wr] =0;
			for(q=p;q!=NULL;q=q->next)  //遍历完整个pbuf链表
			{
				//q->len一定小于NET_MAX_BUF_SIZE
				memcpy(dst_addr,q->payload,q->len);//拷贝数据	
				dst_addr=dst_addr+q->len;	
				gpsNetRx->len[gpsNetRx->wr] = gpsNetRx->len[gpsNetRx->wr]+q->len;
	
//				printf("rx=%s",(char *)q->payload);
				printf("rx=%d\r\n",q->len);
			}
		
//		upcb->remote_ip=*addr; 				//记录远程主机的IP地址
//		upcb->remote_port=port;  			//记录远程主机的端口号
//		lwipdev.remoteip[0]=upcb->remote_ip.addr&0xff; 		//IADDR4
//		lwipdev.remoteip[1]=(upcb->remote_ip.addr>>8)&0xff; //IADDR3
//		lwipdev.remoteip[2]=(upcb->remote_ip.addr>>16)&0xff;//IADDR2
//		lwipdev.remoteip[3]=(upcb->remote_ip.addr>>24)&0xff;//IADDR1
		
			ip_addr_t =*addr;
			gpsNetRx->port[gpsNetRx->wr]=port;  			//记录远程主机的端口号
//			printf("%d\r\n",gpsNetRx->port[gpsNetRx->wr]);
			gpsNetRx->remoteip[gpsNetRx->wr][0]=(ip_addr_t.addr>>0)&0xff; 		//IADDR4
			gpsNetRx->remoteip[gpsNetRx->wr][1]=(ip_addr_t.addr>>8)&0xff; 		//IADDR3
			gpsNetRx->remoteip[gpsNetRx->wr][2]=(ip_addr_t.addr>>16)&0xff;		//IADDR2
			gpsNetRx->remoteip[gpsNetRx->wr][3]=(ip_addr_t.addr>>24)&0xff;		//IADDR1
			
//		gpsNetRx->len[gpsNetRx->wr]= q->tot_len;		//???????为什么是固定的805
//		printf("wr=%d\r\n",gpsNetRx->len[gpsNetRx->wr]);
			gpsNetRx->wr ++;
			gpsNetRx->wr &= (~NET_RXBUFNB);
		
		}
		
		pbuf_free(p);//释放内存
		
	}else
	{
		udp_disconnect(upcb); 
//		udp_demo_flag &= ~(1<<0);	//标记连接断开
	}
} 

//UDP服务器发送数据
void NET_UdpTx(struct udp_pcb *upcb)
{
	struct pbuf *ptr;
	u16_t data_len;
	data_len =gpsNetTx->len[gpsNetTx->rd];
	ptr=pbuf_alloc(PBUF_TRANSPORT,data_len,PBUF_POOL); //申请内存
	if(ptr)
	{
//		printf("tol=%d,len=%d\r\n",ptr->tot_len ,data_len);
		pbuf_take(ptr,(char*)(gpNetTxBuf + gpsNetTx->rd*NET_BUF_SIZE),data_len); //将gpNetTxBuf中的数据打包进pbuf结构中
		udp_send(upcb,ptr);	//udp发送数据 
		pbuf_free(ptr);//释放内存
	} 
} 

/*
 为net收发分配内存
*/
u8 NET_MemMalloc(void)
{ 
	char i;
	gpNetRxBuf=mymalloc(SRAMIN,NET_BUF_SIZE*NET_RXBUFNB);	//申请内存
	gpNetTxBuf=mymalloc(SRAMIN,NET_BUF_SIZE*NET_TXBUFNB);	//申请内存
	gpsNetRx=mymalloc(SRAMIN,sizeof(struct stNetRx));	//申请内存
	gpsNetTx=mymalloc(SRAMIN,sizeof(struct stNetTx));	//申请内存
	gpsT3=mymalloc(SRAMIN,sizeof(struct stTimer3));	//申请内存	
//Initial struct UAinfo = 0;	
	gpsUaInfo=mymalloc(SRAMIN,sizeof(struct stUaInfo));	//申请内存	
	memset(gpsUaInfo,'\0',sizeof(struct stUaInfo));
//eeprom	
	gEeprom_len = sizeof(struct stEeprom);
	gpsEeprom=mymalloc(SRAMIN,gEeprom_len);	//申请内存
	
	for(i=0; i<MAX_SERVER_NUMBER; i++){
		gsMsg.via[i] = NULL;
		gsMsg.re_route[i]= NULL;
	}
	gsMsg.from= NULL;
	gsMsg.to= NULL;
	gsMsg.callid= NULL;
	gsMsg.cseq= NULL;		
	gsNetFlag.bind =0;		//允许bind
	
	if(!gpNetRxBuf||!gpNetTxBuf||!gpsNetRx||!gpsNetTx||!gpsT3)
	{
		NET_MemFree();
		return 1;	//申请失败
	}	
	return 0;		//申请成功
}

void NET_MemFree(void)
{ 
	myfree(SRAMIN,gpNetRxBuf);		//释放内存
	myfree(SRAMIN,gpNetTxBuf);		//释放内存  
	myfree(SRAMIN,gpsNetTx);		//释放内存
	myfree(SRAMIN,gpsNetRx);		//释放内存  
	myfree(SRAMIN,gpsT3);		//释放内存 
}

/*
 初始化各变量
*/
void NET_VariateInit(void)
{
	gpsNetRx->rd=0;
	gpsNetRx->wr=0;
	gpsNetRx->len[0] =0;
	
	gpsNetTx->rd=0;
	gpsNetTx->wr=0;
	gpsNetTx->len[0] =0;
	
	gpsT3->sip_reg =0;
	gsNetFlag.udp_conn =0;
	gsNetFlag.udppcb_new =0;
	gsSipRxFlag.ok_reg =0;
}

void NET_UdppcbConnect(struct udp_pcb *upcb,u8 *dst_ip, u16 dst_port, u16 src_port)
{	
	struct ip_addr rmtipaddr;  	//远端ip地址
	lwipdev.remoteip[0]=dst_ip[0];	
	lwipdev.remoteip[1]=dst_ip[1];
	lwipdev.remoteip[2]=dst_ip[2];
	lwipdev.remoteip[3]=dst_ip[3];
	IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);	//将4位8bytes => 1个32word
	udp_connect(upcb,&rmtipaddr,dst_port);	//客户端连接到指定IP地址和端口号的服务器
	udp_bind(upcb,IP_ADDR_ANY,src_port);//绑定本地IP地址与端口号
	udp_recv(upcb,NET_UdpRx,NULL);//注册接收回调函数
}


/**********************************
1. 创建多个udppcb
2. 连接并注册sip server
**********************************/
void NET_UdppcbNew(void)
{		
	gpsaUdppcb[0]=udp_new();	//分配sip struct udp_pcb内存
	gpsaUdppcb[1]=udp_new();	//分配rtp struct udp_pcb内存
	gpsaUdppcb[2]=udp_new();	//分配management software struct udp_pcb内存
}



//关闭UDP连接
void NET_udp_connection_close(struct udp_pcb *upcb)
{
	udp_disconnect(upcb); 
	udp_remove(upcb);			//断开UDP连接 
	gsNetFlag.udp_conn =0;	//标记连接断开
}

