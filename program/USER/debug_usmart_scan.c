// USMART串口接收状态调试
// 请将此函数替换usmart_scan函数中的内容

void usmart_scan(void)
{
	u8 sta,len;  
	
	// 调试：检查串口接收状态
	static u32 debug_counter = 0;
	debug_counter++;
	if(debug_counter % 1000 == 0) // 每1000次打印一次状态
	{
		printf("USMART Debug: USART_RX_STA=0x%04X\r\n", USART_RX_STA);
	}
	
	if(USART_RX_STA&0x8000)//串口接收完成？
	{					   
		len=USART_RX_STA&0x3fff;	//得到此次接收到的数据长度
		printf("USMART: Received %d bytes: [%s]\r\n", len, USART_RX_BUF); // 调试输出
		USART_RX_BUF[len]='\0';	//在末尾加入结束符. 
		sta=usmart_dev.cmd_rec(USART_RX_BUF);//得到函数各个信息
		if(sta==0)usmart_dev.exe();	//执行函数 
		else 
		{  
			len=usmart_sys_cmd_exe(USART_RX_BUF);
			if(len!=USMART_FUNCERR)sta=len;
			if(sta)
			{
				switch(sta)
				{
					case USMART_FUNCERR:
						printf("函数错误!\r\n");   			
						break;	
					case USMART_PARMERR:
						printf("参数错误!\r\n");   			
						break;				
					case USMART_PARMOVER:
						printf("参数太多!\r\n");   			
						break;		
					case USMART_NOFUNCFIND:
						printf("未找到匹配的函数!\r\n");   			
						break;		
				}
			}
		}
		USART_RX_STA=0;//状态寄存器清空	    
	}
}
