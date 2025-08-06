// IP配置系统集成示例
// 此文件演示如何在主程序中集成串口IP配置功能

#include "sys.h"
#include "usart.h"

// 外部函数声明 (在SipMsgAnalysis.c中实现)
extern void process_uart_commands(void);
extern void uart_rx_interrupt_handler(u8 received_char);

// 全局变量 (在SipMsgAnalysis.c中定义)
extern u32 current_device_ip;

// 在主循环中调用的函数
void handle_ip_configuration(void)
{
    // 处理串口接收到的IP配置命令
    process_uart_commands();
}

// 在main.c中的集成示例:
/*
int main(void)
{
    // ... 系统初始化代码 ...
    
    // 启用串口接收中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    
    while(1)
    {
        // ... 其他主循环代码 ...
        
        // 处理IP配置命令
        handle_ip_configuration();
        
        // ... 其他代码 ...
    }
}
*/

// 使用方法:
/*
1. 通过串口发送命令配置IP地址:
   SET_IP:192.168.1.200

2. 查询当前IP地址:
   GET_IP

3. 系统响应示例:
   Received IP config command: 192.168.1.200
   Checking IP conflict for: 192.168.1.200
   Saving IP config to EEPROM: C0A801C8
   IP configuration saved successfully
   SUCCESS: IP address updated to 192.168.1.200
   Please restart the device to apply new IP configuration
*/
