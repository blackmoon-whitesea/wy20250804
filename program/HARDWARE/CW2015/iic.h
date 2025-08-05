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

#define IIC1_SDA_IN() 	{GPIOF->MODER&=~(3<<(0*2));GPIOF->MODER|=0<<0*2;}	//PF0����ģʽ
#define IIC1_SDA_OUT() 	{GPIOF->MODER&=~(3<<(0*2));GPIOF->MODER|=1<<0*2;} //PF0���ģʽ

void IIC1_Init(void);         //��ʼ��IIC��IO��				 
void IIC1_Start(void);				//����IIC��ʼ�ź�
void IIC1_Stop(void);	  			//����IICֹͣ�ź�
void IIC1_Send_Byte(u8 txd);	//IIC����һ���ֽ�
	u8 IIC1_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
	u8 IIC1_Wait_Ack(void); 		//IIC�ȴ�ACK�ź�
void IIC1_Ack(void);					//IIC����ACK�ź�
void IIC1_NAck(void);					//IIC������ACK�ź�

#endif
















