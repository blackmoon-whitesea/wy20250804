#ifndef __WAVPLAY_H
#define __WAVPLAY_H
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//WAV �������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/6/29
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved				
//********************************************************************************
//V1.0 ˵��
//1,֧��16λ/24λWAV�ļ�����
//2,��߿���֧�ֵ�192K/24bit��WAV��ʽ. 
////////////////////////////////////////////////////////////////////////////////// 	

#define I2S_DMA_BUF_SIZE    160*4

//���ֲ��ſ�����
typedef __packed struct
{  
	u8 *i2s_txbuf1;		//play buffer(I2S tx)
	u8 *i2s_txbuf2; 	
	u8 *i2s_rxbuf1;		//recorder buffer(I2S rx)
	u8 *i2s_rxbuf2; 
	
	u8 rtp_rx_num;
	u8 rtp_tx_num;
	
}__audiodev; 
extern __audiodev gsAudio;	//���ֲ��ſ�����

void AUDIO_Init(void);
void AUDIO_i2s_dma_rx_callback(void);
void AUDIO_i2s_dma_tx_callback(void);


void AUDIO_recfill(u8 *buf,u16 size);
void AUDIO_playfill(u8 *buf,u16 size);
void AUDIO_rec_mode(void);
void AUDIO_play_mode(void);
void AUDIO_stop_mode(void); 
#endif

