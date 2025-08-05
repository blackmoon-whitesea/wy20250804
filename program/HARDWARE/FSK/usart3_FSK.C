#include "usart.h"
#include "usart3_FSK.h"
#include "tel_func.h"
#include "HhdxMsg.h"
#include "SipFunction.h"
#include "cw2015.h"
#include "timer.h"

// 全局变量
#define  CID_MIN_LEN		10		//04 LEN 月日时分 号码
struct stCidFlag gsCidFlag;
u8 gRingCount = 0;
u8 gCallerIDBuffer[CID_LEN];
u16 gCidIndex = 0;			//必须是u8类型 0~255
u8 gCidReady = 0;
u8 gFskCidBuf[17];
u8 gFskOwnBuf[17];
u8 gRingingLow_flag =0;
u8 gRingingHi_flag =0;

// USART3初始化 PB11
void USART3_Init(u32 bound) {
	GPIO_InitTypeDef GPIO_InitStruct;	 
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    // PB11作为USART3_RX
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);
   
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
	
    USART_InitStruct.USART_BaudRate = bound;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_Mode = USART_Mode_Rx;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART3, &USART_InitStruct);
    
    USART_Cmd(USART3, ENABLE);
		USART_ClearFlag(USART3, USART_FLAG_TC);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    //Usart3 NVIC 配置
		NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;//串口3中断通道
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//抢占优先级3
		NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		//子优先级3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、

	
//	GPIO_Init(GPIOE,&GPIO_InitStructure);
//		USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
//    NVIC_EnableIRQ(USART3_IRQn);
//    USART_Cmd(USART3, ENABLE);
		GPIO_InitStruct.GPIO_Pin= GPIO_Pin_7;
		GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN;  //输入
		GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;  //上拉输入
		GPIO_InitStruct.GPIO_Speed=GPIO_Speed_100MHz; //高速GPIO
		GPIO_Init(GPIOE,&GPIO_InitStruct);
}

void USART3_IRQHandler(void)                	//串口1中断服务程序
{
	u8 data;
	
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
        data = USART_ReceiveData(USART3);
        
        // 仅在第一/二次振铃间接收数据
        if((gRingCount == 1) && gsBatFlag.ringing) {
            if(gCidIndex < CID_LEN) {
                gCallerIDBuffer[gCidIndex++] = data;
								//printf("%X ",data);
            //}
						}
        }

        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }	 
  } 

	
	
/************************************ 单数据 **************************************
1．同步引导字符: 55H 55H AAH AAH 55H 55H 55H,为150个0/1相间的波形
2．数据消息格式标识符: 80H表示复合数据消息格式 04H表示单数据消息格式 
3．总数据长度: 16H 从其后的第一个字符到BCC字符之前（不包括BCC）的所有字符的总长度。
4．时间串：30H 33H 31H 37H 31H 36H 35H 36H 30H--36H为03171656的时间串，表示3月17日16时56分 
5．来电号码：32H 31H 35H 36H 34H 33H 30H 31H 34H 34H 32H--34H为电话号码2156430144 	
6．BCC校验字：BAH 为所有字符的（所有带下划线的）按256的模求和取补	
*/
/************************************ 单数据 **************************************
55H 55H AAH AAH 55H 55H 55H 80H 16H 01H 08H 30H 33H 31H 37H 31H 36H 35H 36H 02H 0AH 32H 31H 35H 36H 34H 33H 30H 31H 34H 34H BAH 
数据分析: 
1．同步引导字符: 55H 55H AAH AAH 55H 55H 55H,为150个0/1相间的波形
2．数据消息格式标识符: 80H表示复合数据消息格式 04H表示单数据消息格式 
3．总数据长度: 16H 从其后的第一个字符到BCC字符之前（不包括BCC）的所有字符的总长度。
4．第一个消息: 01H 08H 30H 33H 31H 37H 31H 36H 35H 36H 
	01H--消息类型,呼叫时间 08H--参数长度 30H--36H为03171656的时间串，表示3月17日16时56分 
5．第二个消息: 02H 0AH 32H 31H 35H 36H 34H 33H 30H 31H 34H 34H 
	02H--消息类型,主中号码 0AH--参数长度 32H--34H为电话号码2156430144 
6．BCC校验字：BAH 为所有字符的（所有带下划线的）按256的模求和取补 
*/
u8 FSK_parse(void)	
{
	u8 i,j,byte55,num_len,type;
	//if((gRingCount >0) && (gRingCount <=2) &&(gCidIndex >(CID_LEN-CID_MIN_LEN))&& (gCidIndex <CID_LEN)){
	if((gRingCount == 2) && (gsCidFlag.parse ==0)){
		gsCidFlag.parse =1;
		
		printf("\r\n");
		for(i=0; i<128; i++){
			printf("%X ",gCallerIDBuffer[i]);
		}
		printf("\r\n");
		
		byte55 =0;
//1．同步引导字符:
		for(i =0; i<(CID_LEN-CID_MIN_LEN); i++){
//			printf("%X \r\n",gCallerIDBuffer[i]);
			if((gCallerIDBuffer[i] ==0x55) || (gCallerIDBuffer[i] ==0xAA)){
				byte55++;
//				printf("byte55=%d\r\n",byte55);
				if(byte55 >10){	//10个55H或0AAH
					break;
				}
			}
			else{
				byte55 =0;
//				printf("byte55=%d\r\n",byte55);
			}
		}
		//printf("byte55_i=%d\r\n",i);
		if(i >=(CID_LEN-CID_MIN_LEN))
			return 0;
		type =0;
//2．数据消息格式标识符: 80H表示复合数据消息格式 04H表示单数据消息格式
		for(; i<(CID_LEN-CID_MIN_LEN); i++){
			if(gCallerIDBuffer[i] ==0x04)
			{
				type =0x04;
				break;
			}
			else if(gCallerIDBuffer[i] ==0x80)
			{
				type =0x80;
				break;
			}
		}
		//printf("type_i=%d\r\n",i);
		if(i >=(CID_LEN-CID_MIN_LEN))
			return 0;
//--------------- 单数据 --------------		
		if(type ==0x04){
	//3．总数据长度: 16H 从其后的第一个字符到BCC字符之前（不包括BCC）的所有字符的总长度。
			i++;
			num_len =gCallerIDBuffer[i];
			//printf("num_len[%d]=%d\r\n",i,num_len);
			if(num_len <9)
				return 0;
	//4．时间串：		
			i +=9;
			num_len -=8;
			if(num_len >MAX_NUM_LEN)
				num_len =num_len;
			gDialNumLen=0;
			printf("number: ");
			for(j=0; j<num_len;j ++){
				gpsUaInfo->called_number[gDialNumLen] =gCallerIDBuffer[i];
				printf("%c",gpsUaInfo->called_number[gDialNumLen]);
				i++;
				gDialNumLen ++;
			}		
			gpsUaInfo->called_number[gDialNumLen] =0;
			printf("\r\n");
	//5. 清空buf
			for(j=0; j<gCidIndex;j ++){
				gCallerIDBuffer[j] =0;
			}
			BUILD_CID_start();
		}
//--------------- 复合数据 --------------		
		if(type ==0x80){
	//3．总数据长度: 16H 从其后的第一个字符到BCC字符之前（不包括BCC）的所有字符的总长度。
			i++;
			num_len =gCallerIDBuffer[i];
			//printf("num_len[%d]=%d\r\n",i,num_len);
			if(num_len <13)
				return 0;
	//4．时间串：		
			i +=12;
			num_len =gCallerIDBuffer[i];
			printf("len=%d\r\n",num_len);
			if(num_len >MAX_NUM_LEN)
				num_len =num_len;
			gDialNumLen=0;
			i++;
			printf("number: ");
			for(j=0; j< num_len;j ++){
				gpsUaInfo->called_number[gDialNumLen] =gCallerIDBuffer[i];
				printf("%c",gpsUaInfo->called_number[gDialNumLen]);
				i++;
				gDialNumLen ++;
			}		
			gpsUaInfo->called_number[gDialNumLen] =0;
			printf("\r\n");
	//5. 清空buf
			for(j=0; j<gCidIndex;j ++){
				gCallerIDBuffer[j] =0;
			}
			BUILD_CID_start();
		}		
		
	}
	return 0;
}
// 发送结束协议
void FSK_RTP_stop(void)	
{
	if((gsSipTxFlag.rtp ==1) && (gsBatFlag.onhook==1)){
		gsSipTxFlag.rtp =0;
		BUILD_CID_stop();
	}
}
//判断振铃
void FSK_ringing(void)	
{
	if(CID_RINGING ==0){
		if(gRingingLow_flag ==0){
			gRingingLow_flag =1;
			gRingingHi_flag =0;
			gsBatFlag.discon =0;
			gsBatFlag.onhook =0;
			gsBatFlag.offhook =0;
			gsBatFlag.flag1 =0;
			gsBatFlag.flag2 =0;			
			gsBatFlag.flag3 =0;			
			onhook_delay =60;
			if(gRingCount ==0){
				gRingCount =1;
				gsCidFlag.parse =0;
				gCidIndex =0;
				gsBatFlag.discon =0;
				gsBatFlag.onhook =0;
				gsBatFlag.offhook =0;
				gsBatFlag.ringing =0;
				
				gsBatFlag.flag1 =0;
				gsBatFlag.flag2 =0;
				gsBatFlag.flag3 =0;
				//gsBatFlag.flag4 =1;
				gpsT3->bat_discon =0;
				gpsT3->bat_onhook =0;
				gpsT3->bat_offhook =0;
				gpsT3->bat_ringing =0;
				
				
				gsSipTxFlag.rtp =0;		
				printf("ringing =%d\r\n",gRingCount);	
			}
			else{
				gRingCount ++;
				printf("ringing =%d\r\n",gRingCount);	
				gpsT3->bat_discon =0;
				gpsT3->bat_onhook =0;
				gpsT3->bat_offhook =0;
				gpsT3->bat_ringing =0;
			}
		}
	}
	else{		
		gRingingHi_flag =1;
		gRingingLow_flag =0;
	}
		
	if((gRingCount ==1) && (gpsT3->bat_ringing >=10)){
		gpsT3->bat_ringing =0;
		if(gsBatFlag.ringing ==0){
			gsBatFlag.ringing =1;
		}
	}
}
void CID_print(void)
{
//	u16 i;
//	if(gRingCount == 2){
//		printf("\r\n");
//		gRingCount =12;
//		for(i=0; i<128; i++){
//			printf("%X ",gCallerIDBuffer[i]);
//		}
//		printf("\r\n");
//	}
}


