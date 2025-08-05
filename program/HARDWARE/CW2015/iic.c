
#include "iic.h"
#include "delay.h"

u8 clk_num=0;
/***********************************************************
* ��ʼ��IIC�ӿ�: CW2015
* 	IIC'CLK = PF1
*		IIC'SDA = PF0
***********************************************************/
void IIC1_Init(void)
{		
	
	GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);//ʹ��GPIOFʱ��

  //GPIOF0,F1��ʼ������
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(GPIOF, &GPIO_InitStructure);//��ʼ��
	
	IIC1_SCL1();   //SCL����
	IIC1_SDA1();   //SDA����
}
/*****************************************************
* IIC��ʼ�ź�: SDA���
* 	CLK: 1 -> 0
* 	SDA: 1 -> 0	����4us
*****************************************************/
void IIC1_Start(void)
{
	IIC1_SDA1();	   //SDA���� 
	IIC1_SCL1();     //SCL����	
	IIC1_SDA_OUT();     //SDA�����ģʽ
//	IIC1_SDA1();	   //SDA����  	  
	delay_us(4);   //��ʱ
 	IIC1_SDA0();     //SCL�ߵ�ƽ��ʱ��SDA�ɸߵ��ͣ�����һ����ʼ�ź� 	
	delay_us(4);   //��ʱ
	
//	IIC1_SCL0();     //SCL����
}

/*****************************************************
* IICֹͣ�ź�: SDA���
* 	CLK: 0 -> 1
* 	SDA: 0 -> 1	
*   ����4us
*****************************************************/
void IIC1_Stop(void)
{
	IIC1_SDA_OUT();     //SDA�����ģʽ
	IIC1_SCL1();     //SCL����
	delay_us(4);   //��ʱ
//	IIC1_SCL0();     //SCL����
	IIC1_SDA0();     //SDA����//STOP:when CLK is high DATA change form low to high
 	delay_us(4);   //��ʱ
	IIC1_SDA1();     //SDA���� SCL�ߵ�ƽ��SDA�ɵ͵��ߣ�����ֹͣ�ź�	
	delay_us(4);   //��ʱ
}

/*****************************************************
* �ȴ�Ӧ��: 
*  ������SDA���� CLK =1 SDA =1;
*  Ȼ��ȴ�SDA =0;
*****************************************************/
u8 IIC1_Wait_Ack(void)
{
	u8 timeout=0;
	IIC1_SDA_IN();            //SDA�����ģʽ 
//	IIC1_SDA1();delay_us(1);	//SDA���� ��ʱ  
	IIC1_SCL1();delay_us(1);	//SCL���� ��ʱ 
	
	while(IIC1_READ_SDA())         //�ȴ�SDA��ͣ���ʾӦ���������Ȼһֱwhileѭ����ֱ����ʱ
	{
		timeout++;            //��ʱ����+1
		if(timeout > 250)     //�������250
		{
//			printf("ackout\r\n");
			IIC1_Stop();					//����ֹͣ�ź�
			return 1;						//����1����ʾʧ��
		}
	}	
	IIC1_SCL0();							//SCL����
	return 0;								//����0����ʾ�ɹ�
} 

/*****************************************************
* ����Ӧ��: SDA���
* 	CLK: 0 -> 1 ->0
* 	SDA = 0 
*   ����2us
*****************************************************/
void IIC1_Ack(void)
{
	IIC1_SCL0();   //SCL����
	IIC1_SDA_OUT();   //SDA�����ģʽ 	
	IIC1_SDA0();   //SDA���ͣ���ʾӦ��
	delay_us(2); //��ʱ	
	IIC1_SCL1();   //SCL����
	delay_us(2); //��ʱ
	IIC1_SCL0();   //SCL����
	delay_us(2); //��ʱ	
}

/*****************************************************
* ������Ӧ��: SDA���
* 	CLK: 0 -> 1 ->0
* 	SDA = 1 
*   ����2us
*****************************************************/
void IIC1_NAck(void)
{
	IIC1_SCL0();   //SCL����
	IIC1_SDA_OUT();   //SDA�����ģʽ 
	IIC1_SDA1();   //SDA���ߣ���ʾ��Ӧ��
	delay_us(2); //��ʱ
	IIC1_SCL1();   //SCL����
	delay_us(2); //��ʱ
	IIC1_SCL0();   //SCL����
}	

/*****************************************************
* ����һ���ֽ�: SDA���
* 	������CLK: 0 -> 1��������SDA =X
*   ����5us
*****************************************************/  
void IIC1_Send_Byte(u8 txd)
{                        
	u8 t;   
	IIC1_SDA_OUT();							//SDA�����ģʽ 
	IIC1_SCL0();           			//SCL���ͣ���ʼ���ݴ���
	for(t=0;t<8;t++)       			//forѭ����һλһλ�ķ��ͣ������λ λ7��ʼ
	{ 		
		//IIC1_SDA=(txd&0x80)>>7; 	//����λ7�⣬����ȫ����Ϊ0��Ȼ�����Ƶ�λ0����SDA������
		if((txd&0x80)>>7)
			IIC1_SDA1();
		else
			IIC1_SDA0();
		txd<<=1; 	              //����һλ��׼����һ�η���

		delay_us(5);          	//��ʱ
		IIC1_SCL1();            	//SCL����
		delay_us(5);         		//��ʱ
		IIC1_SCL0();	         		//SCL����
		delay_us(5);        		//��ʱ
//		IIC1_SCL0();	         		//SCL����
	}	 
} 	    

/*****************************************************
* ��ȡһ���ֽ�: SDA����
* 	������CLK: 0 -> 1 ��ȡ����SDA =X
*   ����5us
* ack���Ƿ���Ӧ�� 1:�� 0������
*****************************************************/  
u8 IIC1_Read_Byte(unsigned char ack)
{
	u8 i,receive=0;
	
	IIC1_SDA_IN();        		//SDA����Ϊ����
	for(i=0;i<8;i++ ) 	//forѭ����һλһλ�Ķ�ȡ�������λ λ7��ʼ
	{
		IIC1_SCL0();        //SCL����    
		delay_us(5);      //��ʱ
		IIC1_SCL1();        //SCL���� 
		receive<<=1;      //����һλ��׼���´εĶ�ȡ
		if(IIC1_READ_SDA())
			receive++;     	//�����ȡ���Ǹߵ�ƽ��Ҳ����1��receive+1
		delay_us(5);      //��ʱ
	}	
	if (!ack)          	//����Ҫ����
		IIC1_NAck();    		//����nACK
	else               	//��Ҫ����
		IIC1_Ack();     		//����ACK   
	return receive;
}



