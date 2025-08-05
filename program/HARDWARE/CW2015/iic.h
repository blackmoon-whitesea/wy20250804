#ifndef __IIC_H
#define __IIC_H
#include "sys.h" 

//PF0 =SDA  PF1 =SCL 
#define IIC1_SCL_GPIO   				GPIOF
#define IIC1_SCL_PIN    				GPIO_Pin_1
#define IIC1_SCL0()   					GPIO_ResetBits(IIC1_SCL_GPIO, IIC1_SCL_PIN)
#define IIC1_SCL1()  						GPIO_SetBits(IIC1_SCL_GPIO, IIC1_SCL_PIN)

#define IIC1_SDA_GPIO   				GPIOF
#define IIC1_SDA_PIN    				GPIO_Pin_0
#define IIC1_SDA0()   					GPIO_ResetBits(IIC1_SDA_GPIO, IIC1_SDA_PIN)
#define IIC1_SDA1()  						GPIO_SetBits(IIC1_SDA_GPIO, IIC1_SDA_PIN)
#define IIC1_READ_SDA()   			GPIO_ReadInputDataBit(IIC1_SDA_GPIO, IIC1_SDA_PIN)

#define IIC1_SDA_IN() 	{GPIOF->MODER&=~(3<<(0*2));GPIOF->MODER|=0<<0*2;}	//PF0输入模式
#define IIC1_SDA_OUT() 	{GPIOF->MODER&=~(3<<(0*2));GPIOF->MODER|=1<<0*2;} //PF0输出模式

void IIC1_Init(void);         //初始化IIC的IO口				 
void IIC1_Start(void);				//发送IIC开始信号
void IIC1_Stop(void);	  			//发送IIC停止信号
void IIC1_Send_Byte(u8 txd);	//IIC发送一个字节
	u8 IIC1_Read_Byte(unsigned char ack);//IIC读取一个字节
	u8 IIC1_Wait_Ack(void); 		//IIC等待ACK信号
void IIC1_Ack(void);					//IIC发送ACK信号
void IIC1_NAck(void);					//IIC不发送ACK信号

#endif
















