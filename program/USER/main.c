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
#include "usmart.h"       // 添加USMART头文件
#include "ip_config.h"    // 添加IP配置头文件

#include "stm32f4xx_iwdg.h"
// struct stTimer3 已在 timer.h 中声明

// 初始化相关变量
//void IWDG_Init(void)
//{
//	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); // 使能写访问
//	IWDG_SetPrescaler(IWDG_Prescaler_64);         // 设置分频值64
//	IWDG_SetReload(1250);                         // 设置重装载值1250 (约2.5秒)
//	IWDG_ReloadCounter();                         // ���ؼ�����
//	IWDG_Enable();                                // ʹ��IWDG
//}

int main(void)
{
//	u8 key;
	//系统初始化
	delay_init(168);       	//延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断分组配置	
	mymem_init(SRAMIN);		//初始化内部内存池
	mymem_init(SRAMCCM);	//初始化CCM内存池		
	ETH_MemMalloc();;		//内存申请失败
	lwip_comm_mem_malloc();	//内存申请失败
	NET_MemMalloc();			//分配收发内存	
	uart_init(115200);   	//串口波特率设置

	// LED0 = 0;  // 点亮LED0（假设LED0低电平有效）
	// delay_ms(500);
	// LED0 = 1;  // 熄灭LED0

	USART3_Init(1200);
	//GPIO初始化
	//GPIO_Init();
	LED_Init();  					//LED初始化
	KEY_Init();  					//按键初始化
	AT24CXX_Init();				//初始化IIC接口 
	//TIM3_Int_Init(999,839); //100khz的频率,计数1000为10ms
	TIM3_Int_Init(999,83); //100khz的频率,计数100为1ms	
	EEPROM_Read();			
	WM8978_Init();				//初始化WM8978	
	AUDIO_Init();
	TEL_PinInit();
	NET_VariateInit();
	HHDX_VariateInit();
	ANALYSIS_RxParamInit();
	TEL_OffInit();
	IIC1_Init();				//IIC初始化
	CW2015_Init();	   	//CW2015初始化
	DTMF_OE =1;
	DTMF_Init();
	
	// 初始化动态端口协商状态
	gpsUaInfo->dynamic_port = 0;
	gpsUaInfo->port_negotiated = 0;
	gpsUaInfo->port_verified = 0;
	gpsUaInfo->port_request_timer = 0;
	gpsUaInfo->port_request_retry = 0;
	
	// 初始化USMART调试系统
	usmart_init(168);  // 168MHz系统时钟
	printf("USMART Debug System Initialized\r\n");
	
	// 初始化IP配置系统
	//ip_config_init();
	
	if(lwip_comm_init())
	{ //lwip初始化
		printf("LWIP初始化失败！\r\n");
		gsNetFlag.netok =0;
	}
	else
	{
		gsNetFlag.netok =1;
		printf("LWIP初始化成功！\r\n");
	}	

	// 初始化独立看门狗
	//IWDG_Init();
		
#if LWIP_DHCP
	if(gsNetFlag.netok ==1){
		while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF))//等待DHCP获取成功/超时溢出
		{
			lwip_periodic_handle();
		}
	}
#endif	
	SIP_UainfoInit();		
	gsNetFlag.netlink =KEY_NETLINK;	//读取net link pin
	

	printf("初始化已完成！\r\n");

	while(1)
	{
		
//1. 只有网络初始化成功后才有的功能
		if(gsNetFlag.netok ==1){	

	//1.1. 内存动态分配只能一次
			if(gsNetFlag.udppcb_new ==0){	
				gsNetFlag.udppcb_new =1;
				NET_UdppcbNew();		//注：只有网络初始化成功才建立new udppcb
			}			
			
	//1.2. lwip必须周期处理
			lwip_periodic_handle();
	
	//1.3. 连接并注册sip sever ip(仅一次)		
			if(gsNetFlag.bind ==0){
				gsNetFlag.bind =1;	
				NET_UdppcbConnect(gpsaUdppcb[0],gpsEeprom->sv_ip,SIP_PORT,SIP_PORT);				
				// 根据动态端口设置RTP连接
				//if(gpsUaInfo->dynamic_port > 0)
				//{
				//	NET_UdppcbConnect(gpsaUdppcb[1],gpsEeprom->sv_ip,gpsUaInfo->dynamic_port,gpsUaInfo->dynamic_port);
				//}
				//else
				//{
					//NET_UdppcbConnect(gpsaUdppcb[1],gpsEeprom->sv_ip,RTP_LOCAL_PORT,RTP_LOCAL_PORT);
					NET_UdppcbConnect(gpsaUdppcb[1],gpsEeprom->sv_ip,gpsEeprom->port,RTP_LOCAL_PORT);
				//}
			}
		}
			
//2. NET数据收发		
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
	
//3. 扫描按键		
		KEY_reset_scan();
		KEY_netlink_scan();
		
//4. USMART调试系统处理 (集成IP配置功能)
		// 检查是否只有回车符，没有换行符的情况
		if((USART_RX_STA & 0x4000) && !(USART_RX_STA & 0x8000))
		{
			static u16 wait_counter = 0;
			wait_counter++;
			if(wait_counter > 10) // 等待100ms
			{
				USART_RX_STA |= 0x8000; // 手动设置完成标志
				wait_counter = 0;
			}
		}
		else
		{
			static u16 wait_counter = 0;
			wait_counter = 0;
		}
		
		//usmart_scan();
		
		//delay_ms(10);  // 延时10ms，避免过快扫描导致CPU占用过高
		
//5. Sip register		
		gSipRamdom1 ++;
		gSipRamdom2 ++;
		//SIP_RegisterTx();

//5. 只有sip注册成功后才执行		
		if(gsSipRxFlag.ok_reg){
			SIP_TxFlow();
		}
		
//6. 收到SDP协议
		if(gsSipRxFlag.sdp ==1){
			gsSipRxFlag.sdp =0;			
			// 根据动态端口重新连接RTP
			if(gpsUaInfo->dynamic_port > 0)
			{
				NET_UdppcbConnect(gpsaUdppcb[1],gpsEeprom->sv_ip,gpsUaInfo->rtp_port,gpsUaInfo->dynamic_port);
			}
			else
			{
				NET_UdppcbConnect(gpsaUdppcb[1],gpsEeprom->sv_ip,gpsUaInfo->rtp_port,RTP_LOCAL_PORT);
			}
		}

//7. 发送rtp
		RTP_tx();		

//8. 写AT24C02 EEPROM   
		EEPROM_Write();	//注：只有在空闲时才写EEPROM？？？？？

//9. 摘挂机下功能
		TEL_OnOffFunc();		

//10. 电池采样
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

			//喂狗
		//	IWDG_ReloadCounter();  // 喂狗，防止复位
		}
	}
}









