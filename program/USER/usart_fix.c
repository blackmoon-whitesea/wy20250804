// 修改串口中断处理，支持只有回车符结束的命令
// 请将此代码替换USART1_IRQHandler函数中的相应部分

if((USART_RX_STA&0x8000)==0)//接收未完成
{
    if(USART_RX_STA&0x4000)//接收到了0x0d
    {
        // 修改：支持只有回车符的命令结束方式
        USART_RX_STA|=0x8000;	//设置接收完成标志（不再需要等待0x0a）
    }
    else //还没收到0X0D
    {	
        if(Res==0x0d)USART_RX_STA|=0x4000;
        else
        {
            USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
            USART_RX_STA++;
            if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
        }		 
    }
}
