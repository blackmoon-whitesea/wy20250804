#ifndef __WAVPLAY_H
#define __WAVPLAY_H
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//WAV 解码代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/6/29
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved				
//********************************************************************************
//V1.0 说明
//1,支持16位/24位WAV文件播放
//2,最高可以支持到192K/24bit的WAV格式. 
////////////////////////////////////////////////////////////////////////////////// 	

#define I2S_DMA_BUF_SIZE    160*4

//音乐播放控制器
typedef __packed struct
{  
	u8 *i2s_txbuf1;		//play buffer(I2S tx)
	u8 *i2s_txbuf2; 	
	u8 *i2s_rxbuf1;		//recorder buffer(I2S rx)
	u8 *i2s_rxbuf2; 
	
	u8 rtp_rx_num;
	u8 rtp_tx_num;
	
}__audiodev; 
extern __audiodev gsAudio;	//音乐播放控制器

void AUDIO_Init(void);
void AUDIO_i2s_dma_rx_callback(void);
void AUDIO_i2s_dma_tx_callback(void);


void AUDIO_recfill(u8 *buf,u16 size);
void AUDIO_playfill(u8 *buf,u16 size);
void AUDIO_rec_mode(void);
void AUDIO_play_mode(void);
void AUDIO_stop_mode(void); 
#endif

