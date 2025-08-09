#include "timer.h"
#include "lwip_comm.h"
//#include "SipFunction.h"
//#include "SipStructure.h"
#include "RecMsgBuilding.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//ALIENTEK STM32F407������
//��ʱ�� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/4
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) �������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

extern u32 lwip_localtime;	//lwip����ʱ�������,��λ:ms
extern struct stUaInfo *gpsUaInfo;
extern struct stSipTxFlag gsSipTxFlag;
struct stTimer3 *gpsT3;

//ͨ�ö�ʱ��3�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///ʹ��TIM3ʱ��
	
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_Period=arr;   //�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //������ʱ��3�����ж�
	TIM_Cmd(TIM3,ENABLE); //ʹ�ܶ�ʱ��3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

//��ʱ��3�жϷ�����: 1ms

void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //����ж�
	{
// --------- 1ms ��׼
		lwip_localtime ++;		
		gpsT3->rtp ++;
		gpsT3->eeprom ++;
		gpsT3->key_reset ++;
		gpsT3->handset ++;
		gpsT3->netlink ++;
		gpsT3->bat_sample++;
		gpsT3->dtmf_low ++;
		gpsT3->dtmf_hi ++;
		
// --------- 100ms ��׼
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

		//�ش���ʱ��
        if(gpsRecInfo->waiting_response && gpsRecInfo->retry_timer > 0)
        {
            gpsRecInfo->retry_timer--;
            if(gpsRecInfo->retry_timer == 0)
            {
                // ��ʱ�������ش�
                if(gpsRecInfo->retry_count < gpsRecInfo->max_retry)
                {
                    gpsRecInfo->retry_count++;
                    gpsRecInfo->retry_timer = gpsRecInfo->retry_timeout;
                    // �ط���Ϣ�ı�־
                    gsRecTxFlag.retry = 1;
                }
                else
                {
                    // �ﵽ����ش�����������
                    gpsRecInfo->waiting_response = 0;
                    gpsRecInfo->retry_count = 0;
                }
            }
        }
        
//         // �˿�����ʱ������
//         if(gpsUaInfo->port_request_timer > 0)
//         {
//             gpsUaInfo->port_request_timer--;
//             if(gpsUaInfo->port_request_timer == 0)
//             {
//                 if(gpsUaInfo->port_request_retry < 3)  // �������3��
//                 {
//                     gpsUaInfo->port_request_retry++;
//                     gpsUaInfo->port_request_timer = 30;  // 3�볬ʱ (100ms*30)
                    
//                     // ��������˿�
//                     if(!gpsUaInfo->port_negotiated)
//                     {
//                         BUILD_port_request();
//                     }
//                     else if(!gpsUaInfo->port_verified)
//                     {
//                         BUILD_port_verify(gpsUaInfo->dynamic_port);
//                     }
//                 }
//                 else
//                 {
//                     // �˿�����ʧ�ܣ�ʹ��Ĭ�϶˿�
//                     printf("Port negotiation failed, using default port\r\n");
//                     gpsUaInfo->dynamic_port = 0;  // ʹ��Ĭ������
//                 }
//             }
//         }

// 	}

 	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ

 }

}
