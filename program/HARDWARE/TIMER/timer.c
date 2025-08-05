#include "timer.h"
#include "lwip_comm.h"
#include "SipFunction.h"
#include "SipStructure.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//定时器 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/4
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

extern u32 lwip_localtime;	//lwip本地时间计数器,单位:ms
extern struct stUaInfo *gpsUaInfo;
extern struct stSipTxFlag gsSipTxFlag;
struct stTimer3 *gpsT3;

//通用定时器3中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///使能TIM3时钟
	
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_Period=arr;   //自动重装载值
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //允许定时器3更新中断
	TIM_Cmd(TIM3,ENABLE); //使能定时器3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

//定时器3中断服务函数: 1ms

void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
	{
// --------- 1ms 基准
		lwip_localtime ++;		
		gpsT3->rtp ++;
		gpsT3->eeprom ++;
		gpsT3->key_reset ++;
		gpsT3->handset ++;
		gpsT3->netlink ++;
		gpsT3->bat_sample++;
		gpsT3->dtmf_low ++;
		gpsT3->dtmf_hi ++;
		
// --------- 100ms 基准
		gpsT3->timer100ms ++;		
		if(gpsT3->timer100ms ==100){	//100ms
			gpsT3->timer100ms = 0;			
			gpsT3->sip_reg ++;
			gpsT3->sysled ++;
			gpsT3->dialnum ++;
			gpsT3->bat_discon++;
			gpsT3->bat_onhook++;
			gpsT3->bat_offhook++;
			gpsT3->bat_ringing++;
		}

		//重传计时器
        if(gpsUaInfo->waiting_response && gpsUaInfo->retry_timer > 0)
        {
            gpsUaInfo->retry_timer--;
            if(gpsUaInfo->retry_timer == 0)
            {
                // 超时，触发重传
                if(gpsUaInfo->retry_count < gpsUaInfo->max_retry)
                {
                    gpsUaInfo->retry_count++;
                    gpsUaInfo->retry_timer = gpsUaInfo->retry_timeout;
                    // 重发消息的标志
                    gsSipTxFlag.retry = 1;
                }
                else
                {
                    // 达到最大重传次数，放弃
                    gpsUaInfo->waiting_response = 0;
                    gpsUaInfo->retry_count = 0;
                }
            }
        }
        
        // 端口请求定时器处理
        if(gpsUaInfo->port_request_timer > 0)
        {
            gpsUaInfo->port_request_timer--;
            if(gpsUaInfo->port_request_timer == 0)
            {
                if(gpsUaInfo->port_request_retry < 3)  // 最多重试3次
                {
                    gpsUaInfo->port_request_retry++;
                    gpsUaInfo->port_request_timer = 30;  // 3秒超时 (100ms*30)
                    
                    // 重新请求端口
                    if(!gpsUaInfo->port_negotiated)
                    {
                        BUILD_port_request();
                    }
                    else if(!gpsUaInfo->port_verified)
                    {
                        BUILD_port_verify(gpsUaInfo->dynamic_port);
                    }
                }
                else
                {
                    // 端口请求失败，使用默认端口
                    printf("Port negotiation failed, using default port\r\n");
                    gpsUaInfo->dynamic_port = 0;  // 使用默认配置
                }
            }
        }

	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位

}


