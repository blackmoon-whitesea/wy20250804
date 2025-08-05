#include "cw2015.h" 
#include "delay.h"
#include "usart.h" 
#include "timer.h"
#include "usart3_FSK.h"
#include "tel_func.h"
#include "HhdxMsg.h"
#include "SipFunction.h"
   
unsigned int gBatVal =0;
u8 onhook_delay=0;
struct stBatFlag gsBatFlag;

/**************************************************************************************************
*Register Name   Address          Description      											  Read/Write  Default Value
*  VERSION			0x00				Returns IC version,software version									R				0x6F
*  VCELL				0x02~0x03		Report 14-bit A/D measurement of battery voltage		R				0x00
*  SOC					0x04~0x05 	Report 16-bit SOC result calculated									R				0x00
*  RRT_ALRT			0x06~0x07 	13 bits remaining run time and low SOC alert bit		R/W			0x00
*  CONFIG				0x08				Configure Register, alert threshold set							R/W			0x18
*  MODE					0x0A				Special command for IC state												R/W			0xC0
**************************************************************************************************/
/*****************************************************************************
* Read and Write Command 
*
* Read:
*			S  0xC4  A  Register Address(8bits)  A  Sr 0xC5 A Register data(8bits)  A P
* Write:
*			S  0xC4  A  Register Address(8bits)  A Write data(8bits)  A P
* 
* S = Start
* A = Acknowlage 
* Sr = repeated Start
*****************************************************************************/

/*				用户板59.5V		用户板53.2V		用户板48V			用户板43V
1. 挂机: 		2.64					2.53					2.45					2.36

2. 振铃:		2.9~3.2(75V铃流)

3. 摘机: 		1.7~1.8V

4. 不插电话线： 小于1.68V
*/
void CW2015_BatSample(void)
{
	if(gpsT3->bat_sample >= 200){	
		gpsT3->bat_sample = 0;			
		CW2015_Data();		
		CW2015_BatStatus();
	}
}

//判断各种状态
void CW2015_BatStatus(void)
{
//1. 不插电话线： 小于1.68V
	if(gBatVal <168){
		if(gsBatFlag.discon ==0){
			if(gsBatFlag.flag1 ==1){
				if(gpsT3->bat_discon >30){
					gsBatFlag.discon =1;
					gsBatFlag.onhook =0;
					gsBatFlag.offhook =0;
					gsBatFlag.ringing =0;
					gsBatFlag.flag1 =0;
					gRingCount =0;
					gpsT3->bat_discon =0;
					gpsT3->bat_onhook =0;
					gpsT3->bat_offhook =0;
					gpsT3->bat_ringing =0;
					printf("disconnect\r\n");
				}
			}
			else{
				gsBatFlag.flag1 =1;
				gsBatFlag.flag2 =0;
				gsBatFlag.flag3 =0;
				gsBatFlag.flag4 =0;
				//printf("1\r\n");
				gpsT3->bat_discon =0;
			}				
		}		
	}
//2. 振铃:	2.85~3.2(75V铃流)
	else if(gBatVal >280){	//注：振铃是一检测到电压就有效，与摘，挂机不同
		gpsT3->bat_onhook =0;
		gpsT3->bat_offhook =0;
//		if(gsBatFlag.flag4 ==0){
//			gsBatFlag.flag4 =1;
////			printf("ring vol=%d,t=%d,f=%d\r\n",gBatVal,gpsT3->bat_ringing,gsBatFlag.ringing);
//		}
//		else{
//			gsBatFlag.flag4 =0;
//		}
//		
//		if(gsBatFlag.ringing ==0){
//			gRingCount =1;
//			gsCidFlag.parse =0;
//			gCidIndex =0;
//			gsBatFlag.discon =0;
//			gsBatFlag.onhook =0;
//			gsBatFlag.offhook =0;
////			gsBatFlag.ringing =1;
//			
//			gsBatFlag.flag1 =0;
//			gsBatFlag.flag2 =0;
//			gsBatFlag.flag3 =0;
//			//gsBatFlag.flag4 =1;
//			gpsT3->bat_discon =0;
//			gpsT3->bat_onhook =0;
//			gpsT3->bat_offhook =0;
//			gpsT3->bat_ringing =0;
//			
//			
//			gsSipTxFlag.rtp =0;		
//			//printf("ringing =%d\r\n",gRingCount);	
//		}
//		else{
//			if(gpsT3->bat_ringing >10){
//				gpsT3->bat_ringing =0;
//				gRingCount ++;
//				//printf("ringing =%d\r\n",gRingCount);	
//				gpsT3->bat_discon =0;
//				gpsT3->bat_onhook =0;
//				gpsT3->bat_offhook =0;
//				gpsT3->bat_ringing =0;
//			}
//		}
	}
//3. 挂机: 2.36~2.64
	else if((gBatVal >236) && (gBatVal <264)){
		gpsT3->bat_offhook =0;
		gsBatFlag.flag3=0;
		if((gsBatFlag.offhook ==1)||(gRingCount !=0)){
			if(gsBatFlag.flag2 ==1){
				if(gpsT3->bat_onhook >onhook_delay){
					gsBatFlag.discon =0;
					gsBatFlag.onhook =1;
					gsBatFlag.offhook =0;
					gsBatFlag.ringing =0;
					gsBatFlag.flag2 =0;
					gRingCount =0;					
					gpsT3->bat_discon =0;
					gpsT3->bat_onhook =0;
					gpsT3->bat_offhook =0;
					gpsT3->bat_ringing =0;
					printf("onhook\r\n");
				}
			}
			else{
				gsBatFlag.flag1 =0;
				gsBatFlag.flag2 =1;
				gsBatFlag.flag3 =0;
				gsBatFlag.flag4 =0;
				//printf("2\r\n");
				gpsT3->bat_onhook =0;
			}				
		}		
	}
//4. 摘机: 1.7~1.9
	else if((gBatVal >170) && (gBatVal <190)){
		gpsT3->bat_onhook =0;
		gsBatFlag.flag2 =0;
		if(gsBatFlag.offhook ==0){
			if(gsBatFlag.flag3 ==1){
				if(gpsT3->bat_offhook >=12){
					gsBatFlag.discon =0;
					gsBatFlag.onhook =0;
					gsBatFlag.offhook =1;
					gsBatFlag.ringing =0;
					gsBatFlag.flag3 =0;
					gRingCount =0;					
					gpsT3->bat_discon =0;
					gpsT3->bat_onhook =0;
					gpsT3->bat_offhook =0;
					gpsT3->bat_ringing =0;
					printf("offhook\r\n");
					BUILD_CID_offhook();
					onhook_delay =12;
				}
			}
			else{
				gsBatFlag.flag1 =0;
				gsBatFlag.flag2 =0;
				gsBatFlag.flag3 =1;
				gsBatFlag.flag4 =0;
				//printf("3\r\n");
				gpsT3->bat_offhook =0;
			}				
		}		
	}
	
}
/***********************************************************
* 初始化cw2015引脚：
* IIC'CLK = PF1
*	IIC'SDA = PF0
* QSTRT = PF3(相当于复位脚)
***********************************************************/
void CW2015_Init(void)
{		
//2. 上电默认休眠，我们在此唤醒	
	CW2015_WriteOneByte(0x0A,0x00);
	delay_ms(100);
	
	gsBatFlag.discon =1;
	gsBatFlag.onhook =0;
	gsBatFlag.offhook =0;
	gsBatFlag.ringing =0;
////3. 在QSTART引脚上给一个先高后低复位信号=>会导致电量显示不准
//	QSTART=1;
//	delay_ms(100);
//	QSTART=0; 
//	delay_ms(100);
	
}

/****************************************************************
* 通过CW2015寄存器获取电池电压及容量
****************************************************************/
void CW2015_Data(void)
{	
	u16 VH,VL;
	u8 SOC;
	float V;
	static u8 old_soc=255;
	
//1. 读取电池电压:
// VCELL(0x02~0x03):	Report 14-bit A/D measurement of battery voltage
	VL = CW2015_ReadOneByte(0x03);		
//	printf("VL=%x\r\n",VL);
 	VH = CW2015_ReadOneByte(0x02);
//	printf("VH=%x\r\n",VH); 
// A 14bit sigma-delta A/D converter is used and the voltage resolution is  305uV for CW2015
	VH =(VH*256+VL);
// 	printf("Voltage=%f V\r\n",(VH*305)/1000000.0);
	V = (VH*305)/1000000.0f;	
//	VL = (int)(V);
//	printf("V=%f,%d V\r\n",V,VL);
//	V = (float)V*4.3f;//V*4.333;		//4.333 =(11+3.3)/3.3(电阻分压)
	VL = (int)(V*100);	//为了取得小数点后2位*100,以便更精准
	//printf("B=%d V\r\n",VL);
//	UART5_tx_cmd(UART5_VOL_INT, VL);
	gBatVal =VL;
	
//2. 读取电池储存量SOC(State-Of-Charge)
// SOC(0x04~0x05):	Report 16-bit SOC result 
// In this Register, the high 8bit(0x04) part contains the SOC information in % which can 
// directly used by end user if this accuracy is already good enough for application.
// The low 8bit(0x05) part provides more accurate part of the SOC information until 1/256%.
 	SOC = CW2015_ReadOneByte(0x04);
 	if(SOC !=old_soc){	//有变化才显示
  	old_soc =SOC;
  	CW2015_Display(SOC);
	}
//	printf("SOC=%d%%\r\n",SOC);	
}		

/****************************************************
* 电池电量图形与数字显示
* 发了避免临界点图案不停来回变动，
* 可以区分是上升，下降趋势，中间有5个点的缓冲区
****************************************************/
void CW2015_Display(u8 energy)
{	
////1. 显示当时电量值
//	DISP_CreateProtocolInt(0x1002, energy);
////2. 选择电量值对应图案
//	if(energy ==100){
//		DISP_CreateProtocolInt(0x1015, 0);
//	}
//	else{
//		DISP_CreateProtocolInt(0x1015, (100 -energy)/10 +1);
//	}
} 	
	
/*****************************************************************************
* 函数名：读寄存器数据                             
* 参  数：RegisterAddr:寄存器地址                 
* Read:
*			S  0xC4  A  Register Address(8bits)  A  Sr 0xC5 A Register data(8bits)  A P
*****************************************************************************/
u8 CW2015_ReadOneByte(u8 RegisterAddr)
{				  
	u8 temp=0;		  	    																 
  __disable_irq(); 
	IIC1_Start();           				//IIC开始信号
	IIC1_Send_Byte(0XC4); 					//发送器件地址 写数据 	   	
	
	if(IIC1_Wait_Ack())   					//等待应答
		printf("Readout1\r\n");
    
	IIC1_Send_Byte(RegisterAddr);	//发送寄存器地址
	if(IIC1_Wait_Ack())	     					//等待应答 
		printf("Readout2\r\n");
	IIC1_Start();  								//IIC开始信号
	IIC1_Send_Byte(0XC5);					//进入读取模式			   
	
	if(IIC1_Wait_Ack())  					//等待应答		
		printf("Readout3\r\n");
   
	temp=IIC1_Read_Byte(0);				//读一字节数据，发送应答     
    IIC1_Stop();     						//产生一个停止条件
	__enable_irq();
	return temp;    							//返回读取的数据
}

/*****************************************************************************
* 函数名：写寄存器数据                           
* 参  数：RegisterAddr:寄存器地址                 
* Write:
*			S  0xC4  A  Register Address(8bits)  A Write data(8bits)  A P
*****************************************************************************/
void CW2015_WriteOneByte(u16 RegisterAddr,u8 DataToWrite)
{	
	__disable_irq();
	IIC1_Start();                    //开始信号
	IIC1_Send_Byte(0XC4);            //发送器件地址0XC4,写数据 	 
	
	if(IIC1_Wait_Ack())              //等待应答
		printf("WriteOut1\r\n");
   
	IIC1_Send_Byte(RegisterAddr);    //发送寄存器地址
	
	if(IIC1_Wait_Ack())              //等待应答
		printf("WriteOut2\r\n");									  		   
	
	IIC1_Send_Byte(DataToWrite);     //发送数据							   
	
	if(IIC1_Wait_Ack())              //等待应答
		printf("WriteOut3\r\n");			 
    	
	IIC1_Stop();                     //产生一个停止条件 	  
	__enable_irq();
}









