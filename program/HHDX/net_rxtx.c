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
#include "RecMsgBuilding.h"

char *gTempPointer;
char gTempChar;
unsigned int gMswPort;

uint8_t *gpNetRxBuf;
uint8_t *gpNetTxBuf;
struct stNetRx *gpsNetRx;
struct stNetTx *gpsNetTx;
struct udp_pcb *gpsaUdppcb[3];  		//����3��UDP���������ƿ�

struct stNetFlag gsNetFlag;	

/*
д�뷢������ַ++
1. ����udppcb[x]
2. ����length
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
	// if(gpsNetRx->port[gpsNetRx->rd] == SIP_PORT){
	// 	name_index = ANALYSIS_line(addr);
	// 	if(name_index){
	// 		ANALYSIS_line_to_msg(name_index,addr,len);		//??????�ֻ�addr��ͷ��ʼ�Ƚ�
	// 	}
	// }

	if(gpsNetRx->port[gpsNetRx->rd] == SIP_PORT){
		name_index = HHDX_RecCommand(addr);
		if(name_index){
			HHDX_RecDivideJson(name_index,gTempPointer,len);
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

//UDP���ջص�����
void NET_UdpRx(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
{
	struct pbuf *q;
	uint8_t *dst_addr;
	struct ip_addr ip_addr_t;
	if(p!=NULL)	//���յ���Ϊ�յ�����ʱ
	{
	// ----------- RTP ---------------
		if(port == gpsUaInfo->rtp_port)			//ע��manage port�п���=rtp port!!!!!�������жϵ�һ���ֽ�=��0x80��
		{
			q=p;
			if(q->len ==172){	
				memcpy(gRtpRxBuf[gRtpRxNum],q->payload,172);	//�������ݣ���Ӧ�ôӵ�12�ֽڴӿ�ʼ����q->payload+12�������
//				memcpy(gRtpRxBuf[gRtpRxNum],(char *)(q->payload+12),160);	//�ȴ�绰ʱ�ٲ���
//				printf("l=%d,0x%x,0x%x\r\n",q->len,gRtpRxBuf[gRtpRxNum][3],gRtpRxBuf[gRtpRxNum][12]);
				gRtpRxNum ++;
				gRtpRxNum &= (~RTP_BUF_NUM);	//ע��������2,4,8,16...
				
			}
		}
	// ----------- Other(SIP/manage) ---------------
		else{
			dst_addr =(gpNetRxBuf + gpsNetRx->wr*NET_BUF_SIZE);
			gpsNetRx->len[gpsNetRx->wr] =0;
			for(q=p;q!=NULL;q=q->next)  //����������pbuf����
			{
				//q->lenһ��С��NET_MAX_BUF_SIZE
				memcpy(dst_addr,q->payload,q->len);//��������	
				dst_addr=dst_addr+q->len;	
				gpsNetRx->len[gpsNetRx->wr] = gpsNetRx->len[gpsNetRx->wr]+q->len;
	
//				printf("rx=%s",(char *)q->payload);
				printf("rx=%d\r\n",q->len);
			}
		
//		upcb->remote_ip=*addr; 				//��¼Զ��������IP��ַ
//		upcb->remote_port=port;  			//��¼Զ�������Ķ˿ں�
//		lwipdev.remoteip[0]=upcb->remote_ip.addr&0xff; 		//IADDR4
//		lwipdev.remoteip[1]=(upcb->remote_ip.addr>>8)&0xff; //IADDR3
//		lwipdev.remoteip[2]=(upcb->remote_ip.addr>>16)&0xff;//IADDR2
//		lwipdev.remoteip[3]=(upcb->remote_ip.addr>>24)&0xff;//IADDR1
		
			ip_addr_t =*addr;
			gpsNetRx->port[gpsNetRx->wr]=port;  			//��¼Զ�������Ķ˿ں�
//			printf("%d\r\n",gpsNetRx->port[gpsNetRx->wr]);
			gpsNetRx->remoteip[gpsNetRx->wr][0]=(ip_addr_t.addr>>0)&0xff; 		//IADDR4
			gpsNetRx->remoteip[gpsNetRx->wr][1]=(ip_addr_t.addr>>8)&0xff; 		//IADDR3
			gpsNetRx->remoteip[gpsNetRx->wr][2]=(ip_addr_t.addr>>16)&0xff;		//IADDR2
			gpsNetRx->remoteip[gpsNetRx->wr][3]=(ip_addr_t.addr>>24)&0xff;		//IADDR1
			
//		gpsNetRx->len[gpsNetRx->wr]= q->tot_len;		//???????Ϊʲô�ǹ̶���805
//		printf("wr=%d\r\n",gpsNetRx->len[gpsNetRx->wr]);
			gpsNetRx->wr ++;
			gpsNetRx->wr &= (~NET_RXBUFNB);
		
		}
		
		pbuf_free(p);//�ͷ��ڴ�
		
	}else
	{
		udp_disconnect(upcb); 
//		udp_demo_flag &= ~(1<<0);	//������ӶϿ�
	}
} 

//UDP��������������
void NET_UdpTx(struct udp_pcb *upcb)
{
	struct pbuf *ptr;
	u16_t data_len;
	data_len =gpsNetTx->len[gpsNetTx->rd];
	ptr=pbuf_alloc(PBUF_TRANSPORT,data_len,PBUF_POOL); //�����ڴ�
	if(ptr)
	{
//		printf("tol=%d,len=%d\r\n",ptr->tot_len ,data_len);
		pbuf_take(ptr,(char*)(gpNetTxBuf + gpsNetTx->rd*NET_BUF_SIZE),data_len); //��gpNetTxBuf�е����ݴ����pbuf�ṹ��
		udp_send(upcb,ptr);	//udp�������� 
		pbuf_free(ptr);//�ͷ��ڴ�
	} 
} 

/*
 Ϊnet�շ������ڴ�
*/
u8 NET_MemMalloc(void)
{ 
	char i;
	gpNetRxBuf=mymalloc(SRAMIN,NET_BUF_SIZE*NET_RXBUFNB);	//�����ڴ�
	gpNetTxBuf=mymalloc(SRAMIN,NET_BUF_SIZE*NET_TXBUFNB);	//�����ڴ�
	gpsNetRx=mymalloc(SRAMIN,sizeof(struct stNetRx));	//�����ڴ�
	gpsNetTx=mymalloc(SRAMIN,sizeof(struct stNetTx));	//�����ڴ�
	gpsT3=mymalloc(SRAMIN,sizeof(struct stTimer3));	//�����ڴ�	
//Initial struct UAinfo = 0;	
	gpsUaInfo=mymalloc(SRAMIN,sizeof(struct stUaInfo));	//�����ڴ�	
	gpsRecInfo=mymalloc(SRAMIN,sizeof(struct stRecInfo));	//�����ڴ�
	memset(gpsUaInfo,'\0',sizeof(struct stUaInfo));
//eeprom	
	gEeprom_len = sizeof(struct stEeprom);
	
	gpsEeprom=mymalloc(SRAMIN,gEeprom_len);	//�����ڴ�
	
	for(i=0; i<MAX_SERVER_NUMBER; i++){
		gsMsg.via[i] = NULL;
		gsMsg.re_route[i]= NULL;
	}
	gsMsg.from= NULL;
	gsMsg.to= NULL;
	gsMsg.callid= NULL;
	gsMsg.cseq= NULL;		
	gsNetFlag.bind =0;		//����bind
	
	if(!gpNetRxBuf||!gpNetTxBuf||!gpsNetRx||!gpsNetTx||!gpsT3)
	{
		NET_MemFree();
		return 1;	//����ʧ��
	}	
	return 0;		//����ɹ�
}

void NET_MemFree(void)
{ 
	myfree(SRAMIN,gpNetRxBuf);		//�ͷ��ڴ�
	myfree(SRAMIN,gpNetTxBuf);		//�ͷ��ڴ�  
	myfree(SRAMIN,gpsNetTx);		//�ͷ��ڴ�
	myfree(SRAMIN,gpsNetRx);		//�ͷ��ڴ�  
	myfree(SRAMIN,gpsT3);		//�ͷ��ڴ� 
}

/*
 ��ʼ��������
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
	struct ip_addr rmtipaddr;  	//Զ��ip��ַ
	lwipdev.remoteip[0]=dst_ip[0];	
	lwipdev.remoteip[1]=dst_ip[1];
	lwipdev.remoteip[2]=dst_ip[2];
	lwipdev.remoteip[3]=dst_ip[3];
	IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);	//��4λ8bytes => 1��32word
	udp_connect(upcb,&rmtipaddr,dst_port);	//�ͻ������ӵ�ָ��IP��ַ�Ͷ˿ںŵķ�����
	udp_bind(upcb,IP_ADDR_ANY,src_port);//�󶨱���IP��ַ��˿ں�
	udp_recv(upcb,NET_UdpRx,NULL);//ע����ջص�����
}


/**********************************
1. �������udppcb
2. ���Ӳ�ע��sip server
**********************************/
void NET_UdppcbNew(void)
{		
	gpsaUdppcb[0]=udp_new();	//����sip struct udp_pcb�ڴ�
	gpsaUdppcb[1]=udp_new();	//����rtp struct udp_pcb�ڴ�
	gpsaUdppcb[2]=udp_new();	//����management software struct udp_pcb�ڴ�
}



//�ر�UDP����
void NET_udp_connection_close(struct udp_pcb *upcb)
{
	udp_disconnect(upcb); 
	udp_remove(upcb);			//�Ͽ�UDP���� 
	gsNetFlag.udp_conn =0;	//������ӶϿ�
}

