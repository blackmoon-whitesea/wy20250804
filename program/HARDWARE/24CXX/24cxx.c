#include "24cxx.h" 
#include "delay.h" 	
#include "usart.h"
#include "timer.h"
#include "HhdxMsg.h" 			 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//24CXX ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/6
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

struct stEeprom *gpsEeprom;
u8 gEeprom_len=0;

//��ʼ��IIC�ӿ�
void AT24CXX_Init(void)
{
	IIC_Init();//IIC��ʼ��
}


int AT24C02_SequentialRead(u8 Addr, u8 *Data, u8 len)	//������ȡ
{         
	IIC_Start(); 					//��ʼ�ź�
	IIC_Send_Byte(0xA0);	//�����豸��ַȷ�������ĸ�IIC���豸
	IIC_Wait_Ack();
	IIC_Send_Byte(Addr);  //��ȡ����ʼ��ַ
	IIC_Wait_Ack();
	IIC_Start();          //��ʼ�ź�(����)
	IIC_Send_Byte(0xA1); 	//�������ģʽ
	IIC_Wait_Ack();	
	while(len--)          //�������鳤�ȼ���ѭ������
	{ 
		*Data = IIC_Read_Byte(1); //��ȡ����ÿһ��Ԫ��		
		Data++;						//�����Ԫ�ص�ַ��1
	}
	IIC_NAck();
	IIC_Stop();					//����������ϣ�ֹͣ�ź�
	return 0;
}

//ÿ��д8bytes(һҳ)
void AT24C02_PageWrite(u8 Addr, u8 *Data)
{ 
	u8 i; 
	IIC_Start(); 					//��ʼ�ź�
	IIC_Send_Byte(0xA0);	//�����豸��ַȷ�������ĸ�IIC���豸
  IIC_Wait_Ack();
  IIC_Send_Byte(Addr);  //д�����ʼ��ַ
  IIC_Wait_Ack(); 
	for(i=0; i<8; i++)		//ÿ��д8bytes(һҳ)
  { 
		IIC_Send_Byte(*Data);
		IIC_Wait_Ack();
    Data++; 
	}
  IIC_Stop();
//  delay_ms(10); 	     //ֹͣʱ���賬��5ms.
}
 

void EEPROM_Read(void)
{
	AT24C02_SequentialRead(0, (u8 *)gpsEeprom, gEeprom_len);
}

/*
1. ÿдһҳ���м�Ҫdelay 10ms
*/
void EEPROM_Write(void)
{
	static u8 writed_len =0;
	if(gsHhdxFlag.eepromwr ==1){
		if(gpsT3->eeprom >=10){
			gpsT3->eeprom =0;
			if(gEeprom_len >= writed_len){
				AT24C02_PageWrite(writed_len, (u8 *)(gpsEeprom)+writed_len);		//err: (u8 *)(gpsEeprom+writed_len)
				writed_len +=8;
			}
			else{
				gsHhdxFlag.eepromwr =0;
				writed_len =0;
			}
		}
	}
}




