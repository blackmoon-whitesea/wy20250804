#include "wm8978.h"
#include "myiic.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//WM8978 ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/24
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//WM8978�Ĵ���ֵ������(�ܹ�58���Ĵ���,0~57),ռ��116�ֽ��ڴ�
//��ΪWM8978��IIC������֧�ֶ�����,�����ڱ��ر������мĴ���ֵ
//дWM8978�Ĵ���ʱ,ͬ�����µ����ؼĴ���ֵ,���Ĵ���ʱ,ֱ�ӷ��ر��ر���ļĴ���ֵ.
//ע��:WM8978�ļĴ���ֵ��9λ��,����Ҫ��u16���洢. 
static u16 WM8978_REGVAL_TBL[58]=
{
	0X0000,0X0000,0X0000,0X0000,0X0050,0X0000,0X0140,0X0000,
	0X0000,0X0000,0X0000,0X00FF,0X00FF,0X0000,0X0100,0X00FF,
	0X00FF,0X0000,0X012C,0X002C,0X002C,0X002C,0X002C,0X0000,
	0X0032,0X0000,0X0000,0X0000,0X0000,0X0000,0X0000,0X0000,
	0X0038,0X000B,0X0032,0X0000,0X0008,0X000C,0X0093,0X00E9,
	0X0000,0X0000,0X0000,0X0000,0X0003,0X0010,0X0010,0X0100,
	0X0100,0X0002,0X0001,0X0001,0X0039,0X0039,0X0039,0X0039,
	0X0001,0X0001
}; 
//WM8978��ʼ��
//����ֵ:0,��ʼ������
//    ����,�������
u8 WM8978_Init(void)
{
	u8 res;
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, ENABLE);			//ʹ������GPIOB,GPIOCʱ��
     
	//PB12/13 ���ù������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//���� 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��
	
	//PC2/PC3/PC6���ù������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3|GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//���� 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��
	
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource12,GPIO_AF_SPI2); //PB12,AF5  I2S_LRCK
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2);	//PB13,AF5  I2S_SCLK 
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource3,GPIO_AF_SPI2);	//PC3 ,AF5  I2S_DACDATA 
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_SPI2);	//PC6 ,AF5  I2S_MCK
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource2,GPIO_AF6_SPI2);	//PC2 ,AF6  I2S_ADCDATA  I2S2ext_SD��AF6!!!
		
	IIC_Init();//��ʼ��IIC�ӿ�

/*
0x0   
0x1b   0x1b0   0x6f   0x10   0x0    0x0   0x0     0x0     0x0    0x8   
0xff   0xff    0x0    0x8    0xff   0xff  0x0     0x12c   0x2c   0x2c   
0x2c   0x2c    0x0    0x32   0x0    0x0   0x0     0x0     0x0    0x0   
0x0    0x38    0xb    0x32   0x0    0x8   0xc     0x93    0xe9   0x0   
0x0    0x0     0x10   0x0    0x10   0x10  0x100   0x100   0x2    0x1   
0x1    0x28    0x128  0x32   0x132  0x1   0x1
*/
	res=WM8978_Write_Reg(0,0);	//��λWM8978
	if(res)return 1;			//����ָ��ʧ��,WM8978�쳣

//����Ϊͨ������
/*R1
BUFDCOPEN[8] =1(������ʹ�ܣ��涨����1.5x�����ƶ���)
MICBEN[4] =1(MICƫ��ʹ��)
BIASEN[3] =1(ģ��������)
BUFIOEN[2] =1(����/���������ʹ��)
VMIDSEL[1:0]=11(5K)
*/
	WM8978_Write_Reg(1,0x11F);//1B); //
/*R2
R/LOUT1EN[8:7] =11(ROUT1,LOUT1���ʹ��(�������Թ���))
BOOSTENR/L[5:4] =11(����ͨ������BOOSTENR,BOOSTENLʹ��
ADCENR/L[1:0] =00(ADC�رռ��ر�����MIC)
*/
	WM8978_Write_Reg(2,0x1B0);	//R2,ROUT1,LOUT1���ʹ��(�������Թ���),BOOSTENR,BOOSTENLʹ��,�ر�����MIC
/*R3
L/ROUT2EN[6:5] =11(�������2ʹ��(���ȹ���)
R/LMIXEN[3:2] =11(�������(����)���ʹ��)
DACENR/L[1:0] =11(DACENL/Rʹ��)
*/
	WM8978_Write_Reg(3,0x6F);	//R3,LOUT2,ROUT2���ʹ��(���ȹ���),RMIX,LMIX,DACENL/Rʹ��
/*R4
BCP[8]:BCLKʱ�Ӽ���
				0=����
				1=��ת
WL[6:5] =00(16λ���ݳ���)
FMT[4:3] =10(�����ֱ�׼ I2S)
				 =11(DSP/PCM)
DACLRSWAP[2]:����DAC���ݳ�����LRCʱ�ӵġ��󡱻��ߡ��ҡ���
					0=��
					1=��
ADCLRSWAP[1]:����ADC���ݳ�����LRCʱ�ӵġ��󡱻��ߡ��ҡ���
					0=��
					1=��
MONO[0]:ѡ��оƬ�������͵���������
				0=������
				1=��������������LRC�ġ��������

*/
//	WM8978_Write_Reg(4,0x011);	 //�����ֱ�׼ I2S,16λ���ݳ���
	WM8978_Write_Reg(4,0x019);		 // ֻ����������Ч
/*R5
WL8[5] =0(�ر���8λģʽ����)
DAC_COMP[4:3] =00(DACѹ���أ�������ģʽ) 
							=11(a-law)
ADC_COMP[2:1] =00(ADCѹ���أ�������ģʽ) 
							=11(a-law)
LOOPBACK[0] =0(������)
						=1(����ʹ�ܣ�ADC�������ֱ���͵�DAC��������) 
*/
//	WM8978_Write_Reg(5,0x00);	
	WM8978_Write_Reg(5,0x03E);
/*R6
CLKSEL[8] =0(MCLK���ⲿ�ṩ))
MCLKDIV[7:5] =000(1��Ƶ)
BCLKDIV[4:2] =000(1��ƵBCLK=MCLK)
*/
	WM8978_Write_Reg(6,0);		//R6,MCLK���ⲿ�ṩ
/*R7
SR[3:1] =000(48kHz)��Լ�Ĳ����ʣ������ڲ������˲�����ϵ����
		    =101(8kHz)
*/
	//WM8978_Write_Reg(7,0);
	 WM8978_Write_Reg(7,0x00A);		//8K������
/*R10
DACOSR128[3] =1(128x(�������) DAC��������
*/	
	WM8978_Write_Reg(10,1<<3);	//R10,SOFTMUTE�ر�,128x����,���SNR 

/*
R11 ��DAC������������
R12 ��DAC������������
*/	

/*R14
ADCOSR128[3] =1(128x(�������) ADC��������
*/	
	WM8978_Write_Reg(14,1<<3);	//R14,ADC 128x������

/*
R15 ��ADC������������
R16 ��ADC������������
*/	

/*R36 PLL N value
PLLRESCALE[4] =1(���뵽PLLǰMCLK����2)
PLLN[3:0]	=1000(PLL����/���Ƶ�ʱ����������֣�ֵ����5С��13)
*/	
	WM8978_Write_Reg(36,0x0018);                    //����ʹ��12MHz���ڲ���PLL�����һ��Ҫע����������Ĵ�������ȻFs֡�źŻᶶ�����巽�����ֲ�

/*R43
INVROUT2[4] =1(������������)
*/
	WM8978_Write_Reg(43,1<<4);	//R43,INVROUT2����,��������

/*R44
	�ر���·���룺LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA.
*/		
	WM8978_Write_Reg(44,0);//����R44: �ر�LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA.

/*R47
PGABOOSTL[8] =1(��ͨ������PGA�ƶ�ʹ��,PGA�����+20dB����ͨ������BOOST)
*/	
	WM8978_Write_Reg(47,1<<8);	//R47����,PGABOOSTL,��ͨ��MIC���20������
/*R48
PGABOOSTR[8] =1(��ͨ������PGA�ƶ�ʹ��,PGA�����+20dB����ͨ������BOOST)
*/	
	WM8978_Write_Reg(48,1<<8);	//R48����,PGABOOSTR,��ͨ��MIC���20������
/*R49
TSDEN[1] =1: �������ȱ���
*/
	WM8978_Write_Reg(49,1<<1);	//R49,TSDEN,�������ȱ��� 
/*R50
DACL2LMIX[0] =1(��DAC���������������)
*/
	WM8978_Write_Reg(50,0x1);
/*R51
DACR2LMIX[0] =1(��DAC���������������)
*/
	WM8978_Write_Reg(51,0x1);
/*R52,3
HPVU[8] =1(ֱ��һ��1д�뵽HPVU�Ÿ���LOUT1��ROUT1����)
L/ROUT1MUTE[6]:���Ҷ����������
								0=��������
								1=����
ROUT1VOL/R[5:0] =101000(�������Ҷ�������)
								=111001(0dB)
								...
								=111111=+6dB
*/		
	WM8978_Write_Reg(52,0x28);
	WM8978_Write_Reg(53,0x128);
/*R54,5
SPKVU[8] =1(ֱ��һ��1д�뵽SPKVU�Ÿ���LOUT2��ROUT2����)
L/ROUT2MUTE[6]:���������������
								0=��������
								1=����
LOUT2VOL/R[5:0] =110010(����������������)
								=111001(0dB)
								...
								=111111=+6dB
*/	
	WM8978_Write_Reg(54,0x32);//3F);//
	WM8978_Write_Reg(55,0x132);//13F);//


	WM8978_ADDA_Cfg(1,1);			//����ADC,DAC
	WM8978_Input_Cfg(1,0,0);	//��������ͨ��(MIC&LINE IN),�ر�AUX
	WM8978_Output_Cfg(1,0);		//����DAC,BYPASS��ֹ 
	WM8978_MIC_Gain(30);			//MIC
	WM8978_HPvol_Set(30,30);	//�ֱ���Ͳ
	WM8978_SPKvol_Set(40);		//��������

	return 0;
} 
//WM8978д�Ĵ���
//reg:�Ĵ�����ַ
//val:Ҫд��Ĵ�����ֵ 
//����ֵ:0,�ɹ�;
//    ����,�������
u8 WM8978_Write_Reg(u8 reg,u16 val)
{ 
	IIC_Start(); 
	IIC_Send_Byte((WM8978_ADDR<<1)|0);//����������ַ+д����	 
	if(IIC_Wait_Ack())return 1;	//�ȴ�Ӧ��(�ɹ�?/ʧ��?) 
    IIC_Send_Byte((reg<<1)|((val>>8)&0X01));//д�Ĵ�����ַ+���ݵ����λ
	if(IIC_Wait_Ack())return 2;	//�ȴ�Ӧ��(�ɹ�?/ʧ��?) 
	IIC_Send_Byte(val&0XFF);	//��������
	if(IIC_Wait_Ack())return 3;	//�ȴ�Ӧ��(�ɹ�?/ʧ��?) 
    IIC_Stop();
	WM8978_REGVAL_TBL[reg]=val;	//����Ĵ���ֵ������
	return 0;	
}  
//WM8978���Ĵ���
//���Ƕ�ȡ���ؼĴ���ֵ�������ڵĶ�Ӧֵ
//reg:�Ĵ�����ַ 
//����ֵ:�Ĵ���ֵ
u16 WM8978_Read_Reg(u8 reg)
{  
	return WM8978_REGVAL_TBL[reg];	
} 
//WM8978 DAC/ADC����
//adcen:adcʹ��(1)/�ر�(0)
//dacen:dacʹ��(1)/�ر�(0)
void WM8978_ADDA_Cfg(u8 dacen,u8 adcen)
{
	u16 regval;
	regval=WM8978_Read_Reg(3);	//��ȡR3
	if(dacen)regval|=3<<0;		//R3���2��λ����Ϊ1,����DACR&DACL
	else regval&=~(3<<0);		//R3���2��λ����,�ر�DACR&DACL.
	WM8978_Write_Reg(3,regval);	//����R3
	regval=WM8978_Read_Reg(2);	//��ȡR2
	if(adcen)regval|=3<<0;		//R2���2��λ����Ϊ1,����ADCR&ADCL
	else regval&=~(3<<0);		//R2���2��λ����,�ر�ADCR&ADCL.
	WM8978_Write_Reg(2,regval);	//����R2	
}
//WM8978 ����ͨ������ 
//micen:MIC����(1)/�ر�(0)
//lineinen:Line In����(1)/�ر�(0)
//auxen:aux����(1)/�ر�(0) 
void WM8978_Input_Cfg(u8 micen,u8 lineinen,u8 auxen)
{
	u16 regval;  
	regval=WM8978_Read_Reg(2);	//��ȡR2
	if(micen)regval|=3<<2;		//����INPPGAENR,INPPGAENL(MIC��PGA�Ŵ�)
	else regval&=~(3<<2);		//�ر�INPPGAENR,INPPGAENL.
 	WM8978_Write_Reg(2,regval);	//����R2 
	
	regval=WM8978_Read_Reg(44);	//��ȡR44
	if(micen)regval|=3<<4|3<<0;	//����LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA.
	else regval&=~(3<<4|3<<0);	//�ر�LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA.
	WM8978_Write_Reg(44,regval);//����R44
	
	if(lineinen)WM8978_LINEIN_Gain(1);//LINE IN 0dB����
	else WM8978_LINEIN_Gain(0);	//�ر�LINE IN
	if(auxen)WM8978_AUX_Gain(7);//AUX 6dB����
	else WM8978_AUX_Gain(0);	//�ر�AUX����  
}
//WM8978 ������� 
//dacen:DAC���(����)����(1)/�ر�(0)
//bpsen:Bypass���(¼��,����MIC,LINE IN,AUX��)����(1)/�ر�(0): MIC����ֱ���͵�����
void WM8978_Output_Cfg(u8 dacen,u8 bpsen)
{
	u16 regval=0;
	if(dacen)regval|=1<<0;	//DAC���ʹ��
	if(bpsen)
	{
		regval|=1<<1;		//BYPASSʹ��
		regval|=5<<2;		//0dB����
	} 
	WM8978_Write_Reg(50,regval);//R50����
	WM8978_Write_Reg(51,regval);//R51���� 
}
//WM8978 MIC��������(������BOOST��20dB,MIC-->ADC���벿�ֵ�����)
//gain:0~63,��Ӧ-12dB~35.25dB,0.75dB/Step
//ע��ֻ����������Ч
void WM8978_MIC_Gain(u8 gain)
{
	gain&=0X3F;
	if(gain ==0){
		WM8978_Write_Reg(45,1<<6|1<<8);		//0����ͨ������
//		WM8978_Write_Reg(46,gain|1<<6|1<<8);
//		WM8978_Write_Reg(46,gain|1<<6|1<<8);	//R46,��ͨ��PGA����
	}
	else{
		WM8978_Write_Reg(45,gain|1<<8);		//R45,��ͨ��PGA���� 
	}
//	WM8978_Write_Reg(45,gain);
//	WM8978_Write_Reg(46,gain|1<<8);	//R46,��ͨ��PGA����
}
//WM8978 L2/R2(Ҳ����Line In)��������(L2/R2-->ADC���벿�ֵ�����)
//gain:0~7,0��ʾͨ����ֹ,1~7,��Ӧ-12dB~6dB,3dB/Step
void WM8978_LINEIN_Gain(u8 gain)
{
	u16 regval;
	gain&=0X07;
	regval=WM8978_Read_Reg(47);	//��ȡR47
	regval&=~(7<<4);			//���ԭ�������� 
 	WM8978_Write_Reg(47,regval|gain<<4);//����R47
	regval=WM8978_Read_Reg(48);	//��ȡR48
	regval&=~(7<<4);			//���ԭ�������� 
 	WM8978_Write_Reg(48,regval|gain<<4);//����R48
} 
//WM8978 AUXR,AUXL(PWM��Ƶ����)��������(AUXR/L-->ADC���벿�ֵ�����)
//gain:0~7,0��ʾͨ����ֹ,1~7,��Ӧ-12dB~6dB,3dB/Step
void WM8978_AUX_Gain(u8 gain)
{
	u16 regval;
	gain&=0X07;
	regval=WM8978_Read_Reg(47);	//��ȡR47
	regval&=~(7<<0);			//���ԭ�������� 
 	WM8978_Write_Reg(47,regval|gain<<0);//����R47
	regval=WM8978_Read_Reg(48);	//��ȡR48
	regval&=~(7<<0);			//���ԭ�������� 
 	WM8978_Write_Reg(48,regval|gain<<0);//����R48
}  
//����I2S����ģʽ
//fmt:0,LSB(�Ҷ���);1,MSB(�����);2,�����ֱ�׼I2S;3,PCM/DSP;
//len:0,16λ;1,20λ;2,24λ;3,32λ;  
void WM8978_I2S_Cfg(u8 fmt,u8 len)
{
	fmt&=0X03;
	len&=0X03;//�޶���Χ
	WM8978_Write_Reg(4,(fmt<<3)|(len<<5));	//R4,WM8978����ģʽ����	
}	

//���ö���������������
//voll:����������(0~63)
//volr:����������(0~63)
void WM8978_HPvol_Set(u8 voll,u8 volr)
{
	voll&=0X3F;
	volr&=0X3F;//�޶���Χ
	if(voll==0)voll|=1<<6;//����Ϊ0ʱ,ֱ��mute
	if(volr==0)volr|=1<<6;//����Ϊ0ʱ,ֱ��mute 
	WM8978_Write_Reg(52,voll);			//R52,������������������
	WM8978_Write_Reg(53,volr|(1<<8));	//R53,������������������,ͬ������(HPVU=1)
}
//������������
//voll:����������(0~63) 
void WM8978_SPKvol_Set(u8 volx)
{ 
	volx&=0X3F;//�޶���Χ
	if(volx==0)volx|=1<<6;//����Ϊ0ʱ,ֱ��mute 
 	WM8978_Write_Reg(54,volx);			//R54,������������������
	WM8978_Write_Reg(55,volx|(1<<8));	//R55,������������������,ͬ������(SPKVU=1)	
}













