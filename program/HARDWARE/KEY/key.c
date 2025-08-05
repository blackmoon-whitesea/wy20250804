#include "key.h"
#include "delay.h"
#include "usart.h" 
#include "timer.h" 
#include "net_rxtx.h"
#include "HhdxMsg.h" 

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//����������������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

u8 gHandsetFlag;
u8 gHfFlag;

//������ʼ������
void KEY_Init(void)
{
	//ʹ��GPIOʱ��
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOG,ENABLE);  //ʹ��GPIOG GPIOFʱ��
	
	//Reset
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_15;  //PF15����
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;  //����
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;  //��������
	GPIO_Init(GPIOF,&GPIO_InitStructure); 			//��ʼ��GPIOE
	
	//PG0(LINK_LED): ���NET����û����
	GPIO_InitStructure.GPIO_Pin= GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;  //����
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;  //��������

	//Ӧ������
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	
}


/*******************
* ɨ���ֱ�ժ�һ�
* 1: �һ�			0: ժ��
*******************/
void KEY_reset_scan(void)
{	
	static u8 step =0;
	static u8 handset_old =0;		//��¼�ֱ�ԭ��״̬
	static u8 long_press =0;
	switch(step)
	{
//1. ȷ�������һ�����л�ɨ��һ��
		case 0: 
			handset_old = KEY_RST;
			handset_old = (~handset_old);	//��һ�ο϶�������ͬ
			step =1;
			break;
			
//2. �ж��ֱ�״̬��û�ı䣬�в���¼ 
		case 1: 
			if(handset_old != KEY_RST){
				handset_old = KEY_RST;
				gpsT3->key_reset =0;
				step =2;
				long_press =0;	
			}
			break;

//3. ȥ����״̬�Ƿ���ͬ		
		case 2: 
			if(gpsT3->key_reset >=100){			//ȥ��ʱ�䵽	
				if(handset_old == KEY_RST){	//�����					
					if(handset_old == 1){			//�ɿ�
						step =1;										//ѭ��ɨ��
					}
					else{
						gpsT3->key_reset =0;
						long_press ++;
						if(long_press >=50){		//����5s						
							step =1;										//ѭ��ɨ��
							HHDX_AnalyseJsonValue(JS_RESET,0);
						}							
					}
				}
				else					
					step =1;										//ѭ��ɨ��
			}
			break;
	
		default:
			step =0;
		break;		
	}	
}	

/***************************
* ɨ����û����������
* 1: �һ�			0: ժ��
***************************/
void KEY_netlink_scan(void)
{	
	static u8 step =0;
	static u8 net_cnt_old =0;		//��¼����ԭ��״̬
	switch(step)
	{
//1. ȷ�������һ�����л�ɨ��һ��
		case 0: 
			if(gsNetFlag.netlink !=KEY_NETLINK){
				net_cnt_old = KEY_NETLINK;
				net_cnt_old = (~net_cnt_old);	//��һ�ο϶�������ͬ
				step =1;
			}
			break;
			
//2. �ж��ֱ�״̬��û�ı䣬�в���¼ 
		case 1: 
			if(net_cnt_old != KEY_NETLINK){
				net_cnt_old = KEY_NETLINK;	
				gpsT3->netlink =0;
				step =2;
			}
			break;

//3. ȥ����״̬�Ƿ���ͬ		
		case 2: 
			if(gpsT3->netlink >=100){			//ȥ��ʱ�䵽
				step =1;										//ѭ��ɨ��		
				if(net_cnt_old == KEY_NETLINK){	//�����
					gsNetFlag.netlink =KEY_NETLINK;
					if(net_cnt_old == 1){	
						if(gsNetFlag.netok ==0){		//�����ɹ������ظ���ʼ��
							NET_VariateInit();
							if(lwip_comm_init())				//��Ϊ������ظ������ڴ�
							{ //lwip��ʼ��
								printf("LWIP��ʼ��ʧ�ܣ�\r\n");
							}
							else
							{
								gsNetFlag.netok =1;
								printf("LWIP��ʼ���ɹ���\r\n");
							}							
						#if LWIP_DHCP
							if(sNetFlag.netok){
								while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF))//�ȴ�DHCP��ȡ�ɹ�/��ʱ���
								{
									lwip_periodic_handle();
								}
							}
						#endif	
							HHDX_VariateInit();
						}
					}
					else{
						printf("net =0\r\n");
					}
				}
			}
			break;
	
		default:
			step =0;
		break;		
	}	
}	

