// USMART串口接收诊断代码
// 请在main.c的主循环中临时添加此代码来检查问题

void usmart_debug_check(void)
{
    static u32 last_rx_sta = 0;
    static u32 debug_counter = 0;
    
    debug_counter++;
    
    // 每5秒打印一次状态（假设主循环延时10ms）
    if(debug_counter % 500 == 0) 
    {
        printf("USMART Debug: USART_RX_STA=0x%04X, Buffer[0-9]: ", USART_RX_STA);
        for(int i = 0; i < 10 && i < USART_REC_LEN; i++)
        {
            printf("%02X ", USART_RX_BUF[i]);
        }
        printf("\r\n");
    }
    
    // 检查是否有新的接收数据
    if(USART_RX_STA != last_rx_sta)
    {
        printf("USMART: RX_STA changed from 0x%04X to 0x%04X\r\n", last_rx_sta, USART_RX_STA);
        last_rx_sta = USART_RX_STA;
    }
    
    // 检查是否有部分接收的数据（没有完成标志但有数据）
    if((USART_RX_STA & 0x3FFF) > 0 && (USART_RX_STA & 0x8000) == 0)
    {
        printf("USMART: Partial receive: %d bytes\r\n", USART_RX_STA & 0x3FFF);
    }
}

// 在main.c的主循环中添加这一行：
// usmart_debug_check();
