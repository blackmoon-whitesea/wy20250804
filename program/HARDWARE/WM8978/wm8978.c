#include "wm8978.h"
#include "myiic.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//WM8978 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/24
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//WM8978寄存器值缓存区(总共58个寄存器,0~57),占用116字节内存
//因为WM8978的IIC操作不支持读操作,所以在本地保存所有寄存器值
//写WM8978寄存器时,同步更新到本地寄存器值,读寄存器时,直接返回本地保存的寄存器值.
//注意:WM8978的寄存器值是9位的,所以要用u16来存储. 
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
//WM8978初始化
//返回值:0,初始化正常
//    其他,错误代码
u8 WM8978_Init(void)
{
	u8 res;
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, ENABLE);			//使能外设GPIOB,GPIOC时钟
     
	//PB12/13 复用功能输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化
	
	//PC2/PC3/PC6复用功能输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3|GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化
	
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource12,GPIO_AF_SPI2); //PB12,AF5  I2S_LRCK
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2);	//PB13,AF5  I2S_SCLK 
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource3,GPIO_AF_SPI2);	//PC3 ,AF5  I2S_DACDATA 
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_SPI2);	//PC6 ,AF5  I2S_MCK
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource2,GPIO_AF6_SPI2);	//PC2 ,AF6  I2S_ADCDATA  I2S2ext_SD是AF6!!!
		
	IIC_Init();//初始化IIC接口

/*
0x0   
0x1b   0x1b0   0x6f   0x10   0x0    0x0   0x0     0x0     0x0    0x8   
0xff   0xff    0x0    0x8    0xff   0xff  0x0     0x12c   0x2c   0x2c   
0x2c   0x2c    0x0    0x32   0x0    0x0   0x0     0x0     0x0    0x0   
0x0    0x38    0xb    0x32   0x0    0x8   0xc     0x93    0xe9   0x0   
0x0    0x0     0x10   0x0    0x10   0x10  0x100   0x100   0x2    0x1   
0x1    0x28    0x128  0x32   0x132  0x1   0x1
*/
	res=WM8978_Write_Reg(0,0);	//软复位WM8978
	if(res)return 1;			//发送指令失败,WM8978异常

//以下为通用设置
/*R1
BUFDCOPEN[8] =1(缓冲器使能（规定用于1.5x增益推动）)
MICBEN[4] =1(MIC偏置使能)
BIASEN[3] =1(模拟器工作)
BUFIOEN[2] =1(输入/输出缓冲器使能)
VMIDSEL[1:0]=11(5K)
*/
	WM8978_Write_Reg(1,0x11F);//1B); //
/*R2
R/LOUT1EN[8:7] =11(ROUT1,LOUT1输出使能(耳机可以工作))
BOOSTENR/L[5:4] =11(左右通道输入BOOSTENR,BOOSTENL使能
ADCENR/L[1:0] =00(ADC关闭即关闭左右MIC)
*/
	WM8978_Write_Reg(2,0x1B0);	//R2,ROUT1,LOUT1输出使能(耳机可以工作),BOOSTENR,BOOSTENL使能,关闭左右MIC
/*R3
L/ROUT2EN[6:5] =11(左右输出2使能(喇叭工作)
R/LMIXEN[3:2] =11(左右输出(喇叭)混合使能)
DACENR/L[1:0] =11(DACENL/R使能)
*/
	WM8978_Write_Reg(3,0x6F);	//R3,LOUT2,ROUT2输出使能(喇叭工作),RMIX,LMIX,DACENL/R使能
/*R4
BCP[8]:BCLK时钟极性
				0=正常
				1=反转
WL[6:5] =00(16位数据长度)
FMT[4:3] =10(飞利浦标准 I2S)
				 =11(DSP/PCM)
DACLRSWAP[2]:控制DAC数据出现在LRC时钟的“左”或者“右”相
					0=左
					1=右
ADCLRSWAP[1]:控制ADC数据出现在LRC时钟的“左”或者“右”相
					0=左
					1=右
MONO[0]:选择芯片立体声和单声道操作
				0=立体声
				1=单声道，数据在LRC的“左”相出现

*/
//	WM8978_Write_Reg(4,0x011);	 //飞利浦标准 I2S,16位数据长度
	WM8978_Write_Reg(4,0x019);		 // 只有左声道有效
/*R5
WL8[5] =0(关闭以8位模式操作)
DAC_COMP[4:3] =00(DAC压缩关，即线性模式) 
							=11(a-law)
ADC_COMP[2:1] =00(ADC压缩关，即线性模式) 
							=11(a-law)
LOOPBACK[0] =0(不环回)
						=1(环回使能，ADC数据输出直接送到DAC数据输入) 
*/
//	WM8978_Write_Reg(5,0x00);	
	WM8978_Write_Reg(5,0x03E);
/*R6
CLKSEL[8] =0(MCLK由外部提供))
MCLKDIV[7:5] =000(1分频)
BCLKDIV[4:2] =000(1分频BCLK=MCLK)
*/
	WM8978_Write_Reg(6,0);		//R6,MCLK由外部提供
/*R7
SR[3:1] =000(48kHz)大约的采样率（配置内部数字滤波器的系数）
		    =101(8kHz)
*/
	//WM8978_Write_Reg(7,0);
	 WM8978_Write_Reg(7,0x00A);		//8K采样率
/*R10
DACOSR128[3] =1(128x(最高性能) DAC过采样率
*/	
	WM8978_Write_Reg(10,1<<3);	//R10,SOFTMUTE关闭,128x采样,最佳SNR 

/*
R11 左DAC数字音量控制
R12 左DAC数字音量控制
*/	

/*R14
ADCOSR128[3] =1(128x(最高性能) ADC过采样率
*/	
	WM8978_Write_Reg(14,1<<3);	//R14,ADC 128x采样率

/*
R15 左ADC数字音量控制
R16 左ADC数字音量控制
*/	

/*R36 PLL N value
PLLRESCALE[4] =1(输入到PLL前MCLK除以2)
PLLN[3:0]	=1000(PLL输入/输出频率比率整数部分，值大于5小于13)
*/	
	WM8978_Write_Reg(36,0x0018);                    //由于使用12MHz和内部的PLL，因此一定要注意设置这个寄存器，不然Fs帧信号会抖，具体方法见手册

/*R43
INVROUT2[4] =1(反向驱动喇叭)
*/
	WM8978_Write_Reg(43,1<<4);	//R43,INVROUT2反向,驱动喇叭

/*R44
	关闭线路输入：LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA.
*/		
	WM8978_Write_Reg(44,0);//设置R44: 关闭LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA.

/*R47
PGABOOSTL[8] =1(左通道输入PGA推动使能,PGA输出以+20dB增益通过输入BOOST)
*/	
	WM8978_Write_Reg(47,1<<8);	//R47设置,PGABOOSTL,左通道MIC获得20倍增益
/*R48
PGABOOSTR[8] =1(右通道输入PGA推动使能,PGA输出以+20dB增益通过输入BOOST)
*/	
	WM8978_Write_Reg(48,1<<8);	//R48设置,PGABOOSTR,右通道MIC获得20倍增益
/*R49
TSDEN[1] =1: 开启过热保护
*/
	WM8978_Write_Reg(49,1<<1);	//R49,TSDEN,开启过热保护 
/*R50
DACL2LMIX[0] =1(左DAC输出到左输出混合器)
*/
	WM8978_Write_Reg(50,0x1);
/*R51
DACR2LMIX[0] =1(右DAC输出到右输出混合器)
*/
	WM8978_Write_Reg(51,0x1);
/*R52,3
HPVU[8] =1(直到一个1写入到HPVU才更新LOUT1和ROUT1音量)
L/ROUT1MUTE[6]:左右耳机输出消声
								0=正常操作
								1=消声
ROUT1VOL/R[5:0] =101000(设置左右耳机音量)
								=111001(0dB)
								...
								=111111=+6dB
*/		
	WM8978_Write_Reg(52,0x28);
	WM8978_Write_Reg(53,0x128);
/*R54,5
SPKVU[8] =1(直到一个1写入到SPKVU才更新LOUT2和ROUT2音量)
L/ROUT2MUTE[6]:左右喇叭输出消声
								0=正常操作
								1=消声
LOUT2VOL/R[5:0] =110010(设置左右喇叭音量)
								=111001(0dB)
								...
								=111111=+6dB
*/	
	WM8978_Write_Reg(54,0x32);//3F);//
	WM8978_Write_Reg(55,0x132);//13F);//


	WM8978_ADDA_Cfg(1,1);			//开启ADC,DAC
	WM8978_Input_Cfg(1,0,0);	//开启输入通道(MIC&LINE IN),关闭AUX
	WM8978_Output_Cfg(1,0);		//开启DAC,BYPASS禁止 
	WM8978_MIC_Gain(30);			//MIC
	WM8978_HPvol_Set(30,30);	//手柄听筒
	WM8978_SPKvol_Set(40);		//免提喇叭

	return 0;
} 
//WM8978写寄存器
//reg:寄存器地址
//val:要写入寄存器的值 
//返回值:0,成功;
//    其他,错误代码
u8 WM8978_Write_Reg(u8 reg,u16 val)
{ 
	IIC_Start(); 
	IIC_Send_Byte((WM8978_ADDR<<1)|0);//发送器件地址+写命令	 
	if(IIC_Wait_Ack())return 1;	//等待应答(成功?/失败?) 
    IIC_Send_Byte((reg<<1)|((val>>8)&0X01));//写寄存器地址+数据的最高位
	if(IIC_Wait_Ack())return 2;	//等待应答(成功?/失败?) 
	IIC_Send_Byte(val&0XFF);	//发送数据
	if(IIC_Wait_Ack())return 3;	//等待应答(成功?/失败?) 
    IIC_Stop();
	WM8978_REGVAL_TBL[reg]=val;	//保存寄存器值到本地
	return 0;	
}  
//WM8978读寄存器
//就是读取本地寄存器值缓冲区内的对应值
//reg:寄存器地址 
//返回值:寄存器值
u16 WM8978_Read_Reg(u8 reg)
{  
	return WM8978_REGVAL_TBL[reg];	
} 
//WM8978 DAC/ADC配置
//adcen:adc使能(1)/关闭(0)
//dacen:dac使能(1)/关闭(0)
void WM8978_ADDA_Cfg(u8 dacen,u8 adcen)
{
	u16 regval;
	regval=WM8978_Read_Reg(3);	//读取R3
	if(dacen)regval|=3<<0;		//R3最低2个位设置为1,开启DACR&DACL
	else regval&=~(3<<0);		//R3最低2个位清零,关闭DACR&DACL.
	WM8978_Write_Reg(3,regval);	//设置R3
	regval=WM8978_Read_Reg(2);	//读取R2
	if(adcen)regval|=3<<0;		//R2最低2个位设置为1,开启ADCR&ADCL
	else regval&=~(3<<0);		//R2最低2个位清零,关闭ADCR&ADCL.
	WM8978_Write_Reg(2,regval);	//设置R2	
}
//WM8978 输入通道配置 
//micen:MIC开启(1)/关闭(0)
//lineinen:Line In开启(1)/关闭(0)
//auxen:aux开启(1)/关闭(0) 
void WM8978_Input_Cfg(u8 micen,u8 lineinen,u8 auxen)
{
	u16 regval;  
	regval=WM8978_Read_Reg(2);	//读取R2
	if(micen)regval|=3<<2;		//开启INPPGAENR,INPPGAENL(MIC的PGA放大)
	else regval&=~(3<<2);		//关闭INPPGAENR,INPPGAENL.
 	WM8978_Write_Reg(2,regval);	//设置R2 
	
	regval=WM8978_Read_Reg(44);	//读取R44
	if(micen)regval|=3<<4|3<<0;	//开启LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA.
	else regval&=~(3<<4|3<<0);	//关闭LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA.
	WM8978_Write_Reg(44,regval);//设置R44
	
	if(lineinen)WM8978_LINEIN_Gain(1);//LINE IN 0dB增益
	else WM8978_LINEIN_Gain(0);	//关闭LINE IN
	if(auxen)WM8978_AUX_Gain(7);//AUX 6dB增益
	else WM8978_AUX_Gain(0);	//关闭AUX输入  
}
//WM8978 输出配置 
//dacen:DAC输出(放音)开启(1)/关闭(0)
//bpsen:Bypass输出(录音,包括MIC,LINE IN,AUX等)开启(1)/关闭(0): MIC语音直接送到喇叭
void WM8978_Output_Cfg(u8 dacen,u8 bpsen)
{
	u16 regval=0;
	if(dacen)regval|=1<<0;	//DAC输出使能
	if(bpsen)
	{
		regval|=1<<1;		//BYPASS使能
		regval|=5<<2;		//0dB增益
	} 
	WM8978_Write_Reg(50,regval);//R50设置
	WM8978_Write_Reg(51,regval);//R51设置 
}
//WM8978 MIC增益设置(不包括BOOST的20dB,MIC-->ADC输入部分的增益)
//gain:0~63,对应-12dB~35.25dB,0.75dB/Step
//注：只有左声道有效
void WM8978_MIC_Gain(u8 gain)
{
	gain&=0X3F;
	if(gain ==0){
		WM8978_Write_Reg(45,1<<6|1<<8);		//0：左通道禁音
//		WM8978_Write_Reg(46,gain|1<<6|1<<8);
//		WM8978_Write_Reg(46,gain|1<<6|1<<8);	//R46,右通道PGA设置
	}
	else{
		WM8978_Write_Reg(45,gain|1<<8);		//R45,左通道PGA设置 
	}
//	WM8978_Write_Reg(45,gain);
//	WM8978_Write_Reg(46,gain|1<<8);	//R46,右通道PGA设置
}
//WM8978 L2/R2(也就是Line In)增益设置(L2/R2-->ADC输入部分的增益)
//gain:0~7,0表示通道禁止,1~7,对应-12dB~6dB,3dB/Step
void WM8978_LINEIN_Gain(u8 gain)
{
	u16 regval;
	gain&=0X07;
	regval=WM8978_Read_Reg(47);	//读取R47
	regval&=~(7<<4);			//清除原来的设置 
 	WM8978_Write_Reg(47,regval|gain<<4);//设置R47
	regval=WM8978_Read_Reg(48);	//读取R48
	regval&=~(7<<4);			//清除原来的设置 
 	WM8978_Write_Reg(48,regval|gain<<4);//设置R48
} 
//WM8978 AUXR,AUXL(PWM音频部分)增益设置(AUXR/L-->ADC输入部分的增益)
//gain:0~7,0表示通道禁止,1~7,对应-12dB~6dB,3dB/Step
void WM8978_AUX_Gain(u8 gain)
{
	u16 regval;
	gain&=0X07;
	regval=WM8978_Read_Reg(47);	//读取R47
	regval&=~(7<<0);			//清除原来的设置 
 	WM8978_Write_Reg(47,regval|gain<<0);//设置R47
	regval=WM8978_Read_Reg(48);	//读取R48
	regval&=~(7<<0);			//清除原来的设置 
 	WM8978_Write_Reg(48,regval|gain<<0);//设置R48
}  
//设置I2S工作模式
//fmt:0,LSB(右对齐);1,MSB(左对齐);2,飞利浦标准I2S;3,PCM/DSP;
//len:0,16位;1,20位;2,24位;3,32位;  
void WM8978_I2S_Cfg(u8 fmt,u8 len)
{
	fmt&=0X03;
	len&=0X03;//限定范围
	WM8978_Write_Reg(4,(fmt<<3)|(len<<5));	//R4,WM8978工作模式设置	
}	

//设置耳机左右声道音量
//voll:左声道音量(0~63)
//volr:右声道音量(0~63)
void WM8978_HPvol_Set(u8 voll,u8 volr)
{
	voll&=0X3F;
	volr&=0X3F;//限定范围
	if(voll==0)voll|=1<<6;//音量为0时,直接mute
	if(volr==0)volr|=1<<6;//音量为0时,直接mute 
	WM8978_Write_Reg(52,voll);			//R52,耳机左声道音量设置
	WM8978_Write_Reg(53,volr|(1<<8));	//R53,耳机右声道音量设置,同步更新(HPVU=1)
}
//设置喇叭音量
//voll:左声道音量(0~63) 
void WM8978_SPKvol_Set(u8 volx)
{ 
	volx&=0X3F;//限定范围
	if(volx==0)volx|=1<<6;//音量为0时,直接mute 
 	WM8978_Write_Reg(54,volx);			//R54,喇叭左声道音量设置
	WM8978_Write_Reg(55,volx|(1<<8));	//R55,喇叭右声道音量设置,同步更新(SPKVU=1)	
}













