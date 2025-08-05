
#include "iic.h"
#include "delay.h"

u8 clk_num=0;
/***********************************************************
* 初始化IIC接口: CW2015
* 	IIC'CLK = PF1
*		IIC'SDA = PF0
***********************************************************/
void IIC1_Init(void)
{		
	
	GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);//使能GPIOF时钟

  //GPIOF0,F1初始化设置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOF, &GPIO_InitStructure);//初始化
	
	IIC1_SCL1();   //SCL拉高
	IIC1_SDA1();   //SDA拉高
}
/*****************************************************
* IIC起始信号: SDA输出
* 	CLK: 1 -> 0
* 	SDA: 1 -> 0	保持4us
*****************************************************/
void IIC1_Start(void)
{
	IIC1_SDA1();	   //SDA拉高 
	IIC1_SCL1();     //SCL拉高	
	IIC1_SDA_OUT();     //SDA线输出模式
//	IIC1_SDA1();	   //SDA拉高  	  
	delay_us(4);   //延时
 	IIC1_SDA0();     //SCL高电平的时候，SDA由高到低，发出一个起始信号 	
	delay_us(4);   //延时
	
//	IIC1_SCL0();     //SCL拉低
}

/*****************************************************
* IIC停止信号: SDA输出
* 	CLK: 0 -> 1
* 	SDA: 0 -> 1	
*   保持4us
*****************************************************/
void IIC1_Stop(void)
{
	IIC1_SDA_OUT();     //SDA线输出模式
	IIC1_SCL1();     //SCL拉高
	delay_us(4);   //延时
//	IIC1_SCL0();     //SCL拉低
	IIC1_SDA0();     //SDA拉低//STOP:when CLK is high DATA change form low to high
 	delay_us(4);   //延时
	IIC1_SDA1();     //SDA拉高 SCL高电平，SDA由低到高，发出停止信号	
	delay_us(4);   //延时
}

/*****************************************************
* 等待应答: 
*  先设置SDA输入 CLK =1 SDA =1;
*  然后等待SDA =0;
*****************************************************/
u8 IIC1_Wait_Ack(void)
{
	u8 timeout=0;
	IIC1_SDA_IN();            //SDA线输出模式 
//	IIC1_SDA1();delay_us(1);	//SDA拉高 延时  
	IIC1_SCL1();delay_us(1);	//SCL拉高 延时 
	
	while(IIC1_READ_SDA())         //等待SDA变低，表示应答带来，不然一直while循环，直到超时
	{
		timeout++;            //超时计数+1
		if(timeout > 250)     //如果大于250
		{
//			printf("ackout\r\n");
			IIC1_Stop();					//发送停止信号
			return 1;						//返回1，表示失败
		}
	}	
	IIC1_SCL0();							//SCL拉低
	return 0;								//返回0，表示成功
} 

/*****************************************************
* 发送应答: SDA输出
* 	CLK: 0 -> 1 ->0
* 	SDA = 0 
*   保持2us
*****************************************************/
void IIC1_Ack(void)
{
	IIC1_SCL0();   //SCL拉低
	IIC1_SDA_OUT();   //SDA线输出模式 	
	IIC1_SDA0();   //SDA拉低，表示应答
	delay_us(2); //延时	
	IIC1_SCL1();   //SCL拉高
	delay_us(2); //延时
	IIC1_SCL0();   //SCL拉低
	delay_us(2); //延时	
}

/*****************************************************
* 不发送应答: SDA输出
* 	CLK: 0 -> 1 ->0
* 	SDA = 1 
*   保持2us
*****************************************************/
void IIC1_NAck(void)
{
	IIC1_SCL0();   //SCL拉低
	IIC1_SDA_OUT();   //SDA线输出模式 
	IIC1_SDA1();   //SDA拉高，表示不应答
	delay_us(2); //延时
	IIC1_SCL1();   //SCL拉高
	delay_us(2); //延时
	IIC1_SCL0();   //SCL拉低
}	

/*****************************************************
* 发送一个字节: SDA输出
* 	上升沿CLK: 0 -> 1发送数据SDA =X
*   保持5us
*****************************************************/  
void IIC1_Send_Byte(u8 txd)
{                        
	u8 t;   
	IIC1_SDA_OUT();							//SDA线输出模式 
	IIC1_SCL0();           			//SCL拉低，开始数据传输
	for(t=0;t<8;t++)       			//for循环，一位一位的发送，从最高位 位7开始
	{ 		
		//IIC1_SDA=(txd&0x80)>>7; 	//除了位7外，其余全屏蔽为0，然后右移到位0，给SDA数据线
		if((txd&0x80)>>7)
			IIC1_SDA1();
		else
			IIC1_SDA0();
		txd<<=1; 	              //左移一位，准备下一次发送

		delay_us(5);          	//延时
		IIC1_SCL1();            	//SCL拉高
		delay_us(5);         		//延时
		IIC1_SCL0();	         		//SCL拉低
		delay_us(5);        		//延时
//		IIC1_SCL0();	         		//SCL拉低
	}	 
} 	    

/*****************************************************
* 读取一个字节: SDA输入
* 	上升沿CLK: 0 -> 1 读取数据SDA =X
*   保持5us
* ack：是否发送应答 1:发 0：不发
*****************************************************/  
u8 IIC1_Read_Byte(unsigned char ack)
{
	u8 i,receive=0;
	
	IIC1_SDA_IN();        		//SDA设置为输入
	for(i=0;i<8;i++ ) 	//for循环，一位一位的读取，从最高位 位7开始
	{
		IIC1_SCL0();        //SCL拉低    
		delay_us(5);      //延时
		IIC1_SCL1();        //SCL拉高 
		receive<<=1;      //左移一位，准备下次的读取
		if(IIC1_READ_SDA())
			receive++;     	//如果读取的是高电平，也就是1，receive+1
		delay_us(5);      //延时
	}	
	if (!ack)          	//不需要发送
		IIC1_NAck();    		//发送nACK
	else               	//需要发送
		IIC1_Ack();     		//发送ACK   
	return receive;
}



