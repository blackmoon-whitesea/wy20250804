#include "cw2015.h" 
#include "delay.h"
#include "usart.h" 
#include "timer.h"
#include "usart3_FSK.h"
#include "tel_func.h"
#include "HhdxMsg.h"
#include "SipFunction.h"
#include "24cxx.h"

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

/*				�û���59.5V		�û���53.2V		�û���48V			�û���43V
1. �һ�: onhook_lv hv 		2.64					2.53					2.45					2.36

2. ����: ring_lv 		2.9~3.2(75V����)

3. ժ��: offhook_lv hv		1.7~1.8V

4. ����绰�ߣ�nophone_hv С��1.68V
*/
void CW2015_BatSample(void)
{
	if(gpsT3->bat_sample >= 200){	
		gpsT3->bat_sample = 0;			
		CW2015_Data();		
		CW2015_BatStatus();
	}
}

//�жϸ���״̬
void CW2015_BatStatus(void)
{
//1. ����绰�ߣ� С��1.68V
	//if(gBatVal <168){
	if(gBatVal<gpsEeprom->nophone_hv){
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
//2. ����:	2.85~3.2(75V����)
	//else if(gBatVal >280){	//ע��������һ��⵽��ѹ����Ч����ժ���һ���ͬ
	else if(gBatVal>gpsEeprom->ring_lv){
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
//3. �һ�: 2.36~2.64
	//else if((gBatVal >236) && (gBatVal <264)){
	else if((gBatVal>gpsEeprom->onhook_lv)&&(gBatVal<gpsEeprom->onhook_hv)){
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
//4. ժ��: 1.7~1.9
	//else if((gBatVal >170) && (gBatVal <190)){
	else if((gBatVal>gpsEeprom->offhook_lv)&&(gBatVal<gpsEeprom->offhook_hv)){
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
					//wy modify
					//BUILD_CID_offhook();
					BUILD_CID_JSON_offhook();
					
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
* ��ʼ��cw2015���ţ�
* IIC'CLK = PF1
*	IIC'SDA = PF0
* QSTRT = PF3(�൱�ڸ�λ��)
***********************************************************/
void CW2015_Init(void)
{		
//2. �ϵ�Ĭ�����ߣ������ڴ˻���	
	CW2015_WriteOneByte(0x0A,0x00);
	delay_ms(100);
	
	gsBatFlag.discon =1;
	gsBatFlag.onhook =0;
	gsBatFlag.offhook =0;
	gsBatFlag.ringing =0;
////3. ��QSTART�����ϸ�һ���ȸߺ�͸�λ�ź�=>�ᵼ�µ�����ʾ��׼
//	QSTART=1;
//	delay_ms(100);
//	QSTART=0; 
//	delay_ms(100);
	
}

/****************************************************************
* ͨ��CW2015�Ĵ�����ȡ��ص�ѹ������
****************************************************************/
void CW2015_Data(void)
{	
	u16 VH,VL;
	u8 SOC;
	float V;
	static u8 old_soc=255;
	
//1. ��ȡ��ص�ѹ:
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
//	V = (float)V*4.3f;//V*4.333;		//4.333 =(11+3.3)/3.3(�����ѹ)
	VL = (int)(V*100);	//Ϊ��ȡ��С�����2λ*100,�Ա����׼
	//printf("B=%d V\r\n",VL);
//	UART5_tx_cmd(UART5_VOL_INT, VL);
	gBatVal =VL;
	
//2. ��ȡ��ش�����SOC(State-Of-Charge)
// SOC(0x04~0x05):	Report 16-bit SOC result 
// In this Register, the high 8bit(0x04) part contains the SOC information in % which can 
// directly used by end user if this accuracy is already good enough for application.
// The low 8bit(0x05) part provides more accurate part of the SOC information until 1/256%.
 	SOC = CW2015_ReadOneByte(0x04);
 	if(SOC !=old_soc){	//�б仯����ʾ
  	old_soc =SOC;
  	CW2015_Display(SOC);
	}
//	printf("SOC=%d%%\r\n",SOC);	
}		

/****************************************************
* ��ص���ͼ����������ʾ
* ���˱����ٽ��ͼ����ͣ���ر䶯��
* �����������������½����ƣ��м���5����Ļ�����
****************************************************/
void CW2015_Display(u8 energy)
{	
////1. ��ʾ��ʱ����ֵ
//	DISP_CreateProtocolInt(0x1002, energy);
////2. ѡ�����ֵ��Ӧͼ��
//	if(energy ==100){
//		DISP_CreateProtocolInt(0x1015, 0);
//	}
//	else{
//		DISP_CreateProtocolInt(0x1015, (100 -energy)/10 +1);
//	}
} 	
	
/*****************************************************************************
* �����������Ĵ�������                             
* ��  ����RegisterAddr:�Ĵ�����ַ                 
* Read:
*			S  0xC4  A  Register Address(8bits)  A  Sr 0xC5 A Register data(8bits)  A P
*****************************************************************************/
u8 CW2015_ReadOneByte(u8 RegisterAddr)
{				  
	u8 temp=0;		  	    																 
  __disable_irq(); 
	IIC1_Start();           				//IIC��ʼ�ź�
	IIC1_Send_Byte(0XC4); 					//����������ַ д���� 	   	
	
	if(IIC1_Wait_Ack())   					//�ȴ�Ӧ��
		printf("Readout1\r\n");
    
	IIC1_Send_Byte(RegisterAddr);	//���ͼĴ�����ַ
	if(IIC1_Wait_Ack())	     					//�ȴ�Ӧ�� 
		printf("Readout2\r\n");
	IIC1_Start();  								//IIC��ʼ�ź�
	IIC1_Send_Byte(0XC5);					//�����ȡģʽ			   
	
	if(IIC1_Wait_Ack())  					//�ȴ�Ӧ��		
		printf("Readout3\r\n");
   
	temp=IIC1_Read_Byte(0);				//��һ�ֽ����ݣ�����Ӧ��     
    IIC1_Stop();     						//����һ��ֹͣ����
	__enable_irq();
	return temp;    							//���ض�ȡ������
}

/*****************************************************************************
* ��������д�Ĵ�������                           
* ��  ����RegisterAddr:�Ĵ�����ַ                 
* Write:
*			S  0xC4  A  Register Address(8bits)  A Write data(8bits)  A P
*****************************************************************************/
void CW2015_WriteOneByte(u16 RegisterAddr,u8 DataToWrite)
{	
	__disable_irq();
	IIC1_Start();                    //��ʼ�ź�
	IIC1_Send_Byte(0XC4);            //����������ַ0XC4,д���� 	 
	
	if(IIC1_Wait_Ack())              //�ȴ�Ӧ��
		printf("WriteOut1\r\n");
   
	IIC1_Send_Byte(RegisterAddr);    //���ͼĴ�����ַ
	
	if(IIC1_Wait_Ack())              //�ȴ�Ӧ��
		printf("WriteOut2\r\n");									  		   
	
	IIC1_Send_Byte(DataToWrite);     //��������							   
	
	if(IIC1_Wait_Ack())              //�ȴ�Ӧ��
		printf("WriteOut3\r\n");			 
    	
	IIC1_Stop();                     //����һ��ֹͣ���� 	  
	__enable_irq();
}









