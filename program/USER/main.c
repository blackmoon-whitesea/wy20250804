#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lwip_comm.h"
#include "LAN8720.h"
#include "timer.h"
#include "malloc.h"
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "net_rxtx.h"
#include "sipfunction.h"
#include "tel_func.h"
#include "wm8978.h"	 
#include "Audio.h"
#include "HhdxMsg.h"
#include "24cxx.h"
#include "rtp.h"
#include "iic.h"
#include "cw2015.h"
#include "usart3_FSK.h"
#include "dtmf_rx.h"

#include "stm32f4xx_iwdg.h"

// �������Ź���ʼ������
//void IWDG_Init(void)
//{
//	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); // ʹ��д����
//	IWDG_SetPrescaler(IWDG_Prescaler_64);         // ���÷�Ƶϵ��64
//	IWDG_SetReload(1250);                         // ��������ֵ1250 (Լ2.5�볬ʱ)
//	IWDG_ReloadCounter();                         // ���ؼ�����
//	IWDG_Enable();                                // ʹ��IWDG
//}

int main(void)
{
//	u8 key;
	//ϵͳ��ʼ��
	delay_init(168);       	//��ʱ��ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//�жϷ�������	
	mymem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
	mymem_init(SRAMCCM);	//��ʼ��CCM�ڴ��		
	ETH_MemMalloc();;		//�ڴ�����ʧ��
	lwip_comm_mem_malloc();	//�ڴ�����ʧ��
	NET_MemMalloc();			//�����շ��ڴ�	
	uart_init(115200);   	//���ڲ���������
	USART3_Init(1200);
	//GPIO��ʼ��
	//GPIO_Init();
	LED_Init();  					//LED��ʼ��
	KEY_Init();  					//������ʼ��
	AT24CXX_Init();				//��ʼ��IIC�ӿ� 
	//TIM3_Int_Init(999,839); //100khz��Ƶ��,����1000Ϊ10ms
	TIM3_Int_Init(999,83); //100khz��Ƶ��,����100Ϊ1ms	
	EEPROM_Read();			
	WM8978_Init();				//��ʼ��WM8978	
	AUDIO_Init();
	TEL_PinInit();
	NET_VariateInit();
	HHDX_VariateInit();
	ANALYSIS_RxParamInit();
	TEL_OffInit();
	IIC1_Init();				//IIC��ʼ��
	CW2015_Init();	   	//CW2015��ʼ��
	DTMF_OE =1;
	DTMF_Init();
	
	// ��ʼ����̬�˿�Э��״̬
	gpsUaInfo->dynamic_port = 0;
	gpsUaInfo->port_negotiated = 0;
	gpsUaInfo->port_verified = 0;
	gpsUaInfo->port_request_timer = 0;
	gpsUaInfo->port_request_retry = 0;
	
	if(lwip_comm_init())
	{ //lwip��ʼ��
		printf("LWIP��ʼ��ʧ�ܣ�\r\n");
		gsNetFlag.netok =0;
	}
	else
	{
		gsNetFlag.netok =1;
		printf("LWIP��ʼ���ɹ���\r\n");
	}	

	// ��ʼ���������Ź�
	//IWDG_Init();
		
#if LWIP_DHCP
	if(gsNetFlag.netok ==1){
		while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF))//�ȴ�DHCP��ȡ�ɹ�/��ʱ���
		{
			lwip_periodic_handle();
		}
	}
#endif	
	SIP_UainfoInit();		
	gsNetFlag.netlink =KEY_NETLINK;	//��ȡnet link pin
	
	while(1)
	{
		
//1. ֻ�������ʼ���ɹ�����еĹ���
		if(gsNetFlag.netok ==1){	

	//1.1. �ڴ涯̬����ֻ��һ��
			if(gsNetFlag.udppcb_new ==0){	
				gsNetFlag.udppcb_new =1;
				NET_UdppcbNew();		//ע��ֻ�������ʼ���ɹ��Ž���new udppcb
			}			
			
	//1.2. lwip�������ڴ���
			lwip_periodic_handle();
	
	//1.3. ���Ӳ�ע��sip sever ip(��һ��)		
			if(gsNetFlag.bind ==0){
				gsNetFlag.bind =1;	
				NET_UdppcbConnect(gpsaUdppcb[0],gpsEeprom->sv_ip,SIP_PORT,SIP_PORT);				
				// ���ݶ�̬�˿�����RTP����
				if(gpsUaInfo->dynamic_port > 0)
				{
					NET_UdppcbConnect(gpsaUdppcb[1],gpsEeprom->sv_ip,gpsUaInfo->dynamic_port,gpsUaInfo->dynamic_port);
				}
				else
				{
					NET_UdppcbConnect(gpsaUdppcb[1],gpsEeprom->sv_ip,RTP_LOCAL_PORT,RTP_LOCAL_PORT);
				}
			}
		}
			
//2. NET�����շ�		
	//----------------- TX -----------------
		if(gpsNetTx->rd != gpsNetTx->wr){	
			//0: SIP		1: RTP		2: Manage software
			NET_UdpTx(gpsaUdppcb[gpsNetTx->udppcb[gpsNetTx->rd]]);
			gpsNetTx->rd ++;
			gpsNetTx->rd &= (~NET_TXBUFNB);
		}
		
	//----------------- RX -----------------
		if(gpsNetRx->rd != gpsNetRx->wr){
			NET_RxParse();
			gpsNetRx->rd ++;
			gpsNetRx->rd &= (~NET_RXBUFNB);
		}
	
//3. ɨ�谴��		
		KEY_reset_scan();
		KEY_netlink_scan();
		
//4. Sip register		
		gSipRamdom1 ++;
		gSipRamdom2 ++;
		//SIP_RegisterTx();

//5. ֻ��sipע��ɹ����ִ��		
		if(gsSipRxFlag.ok_reg){
			SIP_TxFlow();
		}
		
//6. �յ�SDPЭ��
		if(gsSipRxFlag.sdp ==1){
			gsSipRxFlag.sdp =0;			
			// ���ݶ�̬�˿���������RTP
			if(gpsUaInfo->dynamic_port > 0)
			{
				NET_UdppcbConnect(gpsaUdppcb[1],gpsEeprom->sv_ip,gpsUaInfo->rtp_port,gpsUaInfo->dynamic_port);
			}
			else
			{
				NET_UdppcbConnect(gpsaUdppcb[1],gpsEeprom->sv_ip,gpsUaInfo->rtp_port,RTP_LOCAL_PORT);
			}
		}

//7. ����rtp
		RTP_tx();		

//8. дAT24C02 EEPROM   
		EEPROM_Write();	//ע��ֻ���ڿ���ʱ��дEEPROM����������

//9. ժ�һ��¹���
		TEL_OnOffFunc();		

//10. ��ز���
		CW2015_BatSample();
		FSK_ringing();		
		//CID_print();
		FSK_parse();
		FSK_RTP_stop();
		DTMF_rx();
//x. sysled 5*100ms =500ms
		if(gpsT3->sysled >5){
			gpsT3->sysled =0;
			//LED0=!LED0;		

			SIP_date();	

			//ι��
		//	IWDG_ReloadCounter();  // ι������ֹ��λ
		}
	}
}









