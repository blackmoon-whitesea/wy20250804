#include "key.h"
#include "delay.h"
#include "usart.h" 
#include "timer.h" 
#include "net_rxtx.h"
#include "HhdxMsg.h" 

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//按键输入驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

u8 gHandsetFlag;
u8 gHfFlag;

//按键初始化函数
void KEY_Init(void)
{
	//使用GPIO时钟
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOG,ENABLE);  //使能GPIOG GPIOF时钟
	
	//Reset
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_15;  //PF15引脚
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;  //输入
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;  //上拉输入
	GPIO_Init(GPIOF,&GPIO_InitStructure); 			//初始化GPIOE
	
	//PG0(LINK_LED): 检测NET线有没插入
	GPIO_InitStructure.GPIO_Pin= GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;  //输入
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;  //上拉输入

	//应用配置
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	
}


/*******************
* 扫描手柄摘挂机
* 1: 挂机			0: 摘机
*******************/
void KEY_reset_scan(void)
{	
	static u8 step =0;
	static u8 handset_old =0;		//记录手柄原有状态
	static u8 long_press =0;
	switch(step)
	{
//1. 确保程序第一次运行会扫描一次
		case 0: 
			handset_old = KEY_RST;
			handset_old = (~handset_old);	//第一次肯定不会相同
			step =1;
			break;
			
//2. 判断手柄状态有没改变，有并记录 
		case 1: 
			if(handset_old != KEY_RST){
				handset_old = KEY_RST;
				gpsT3->key_reset =0;
				step =2;
				long_press =0;	
			}
			break;

//3. 去抖后，状态是否还相同		
		case 2: 
			if(gpsT3->key_reset >=100){			//去抖时间到	
				if(handset_old == KEY_RST){	//还相等					
					if(handset_old == 1){			//松开
						step =1;										//循环扫描
					}
					else{
						gpsT3->key_reset =0;
						long_press ++;
						if(long_press >=50){		//长按5s						
							step =1;										//循环扫描
							HHDX_AnalyseJsonValue(JS_RESET,0);
						}							
					}
				}
				else					
					step =1;										//循环扫描
			}
			break;
	
		default:
			step =0;
		break;		
	}	
}	

/***************************
* 扫描有没插入网络线
* 1: 挂机			0: 摘机
***************************/
void KEY_netlink_scan(void)
{	
	static u8 step =0;
	static u8 net_cnt_old =0;		//记录网线原有状态
	switch(step)
	{
//1. 确保程序第一次运行会扫描一次
		case 0: 
			if(gsNetFlag.netlink !=KEY_NETLINK){
				net_cnt_old = KEY_NETLINK;
				net_cnt_old = (~net_cnt_old);	//第一次肯定不会相同
				step =1;
			}
			break;
			
//2. 判断手柄状态有没改变，有并记录 
		case 1: 
			if(net_cnt_old != KEY_NETLINK){
				net_cnt_old = KEY_NETLINK;	
				gpsT3->netlink =0;
				step =2;
			}
			break;

//3. 去抖后，状态是否还相同		
		case 2: 
			if(gpsT3->netlink >=100){			//去抖时间到
				step =1;										//循环扫描		
				if(net_cnt_old == KEY_NETLINK){	//还相等
					gsNetFlag.netlink =KEY_NETLINK;
					if(net_cnt_old == 1){	
						if(gsNetFlag.netok ==0){		//初化成功则不能重复初始化
							NET_VariateInit();
							if(lwip_comm_init())				//因为这里会重复分配内存
							{ //lwip初始化
								printf("LWIP初始化失败！\r\n");
							}
							else
							{
								gsNetFlag.netok =1;
								printf("LWIP初始化成功！\r\n");
							}							
						#if LWIP_DHCP
							if(sNetFlag.netok){
								while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF))//等待DHCP获取成功/超时溢出
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

