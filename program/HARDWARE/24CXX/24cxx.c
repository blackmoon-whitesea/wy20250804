#include "24cxx.h" 
#include "delay.h" 	
#include "usart.h"
#include "timer.h"
#include "HhdxMsg.h" 			 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//24CXX 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/6
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

struct stEeprom *gpsEeprom;
u8 gEeprom_len=0;

//初始化IIC接口
void AT24CXX_Init(void)
{
	IIC_Init();//IIC初始化
}


int AT24C02_SequentialRead(u8 Addr, u8 *Data, u8 len)	//连续读取
{         
	IIC_Start(); 					//开始信号
	IIC_Send_Byte(0xA0);	//发送设备地址确定操作哪个IIC从设备
	IIC_Wait_Ack();
	IIC_Send_Byte(Addr);  //读取的起始地址
	IIC_Wait_Ack();
	IIC_Start();          //开始信号(重启)
	IIC_Send_Byte(0xA1); 	//进入接收模式
	IIC_Wait_Ack();	
	while(len--)          //根据数组长度计算循环次数
	{ 
		*Data = IIC_Read_Byte(1); //读取数组每一个元素		
		Data++;						//数组的元素地址加1
	}
	IIC_NAck();
	IIC_Stop();					//正常接收完毕，停止信号
	return 0;
}

//每次写8bytes(一页)
void AT24C02_PageWrite(u8 Addr, u8 *Data)
{ 
	u8 i; 
	IIC_Start(); 					//开始信号
	IIC_Send_Byte(0xA0);	//发送设备地址确定操作哪个IIC从设备
  IIC_Wait_Ack();
  IIC_Send_Byte(Addr);  //写入的起始地址
  IIC_Wait_Ack(); 
	for(i=0; i<8; i++)		//每次写8bytes(一页)
  { 
		IIC_Send_Byte(*Data);
		IIC_Wait_Ack();
    Data++; 
	}
  IIC_Stop();
//  delay_ms(10); 	     //停止时间需超过5ms.
}
 

void EEPROM_Read(void)
{
	AT24C02_SequentialRead(0, (u8 *)gpsEeprom, gEeprom_len);
}

/*
1. 每写一页，中间要delay 10ms
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




