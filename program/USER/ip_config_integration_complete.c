// 完整的IP配置系统集成示例
// 此文件展示如何在main.c中集成串口IP配置功能

#include "sys.h"
#include "usart.h"
#include "ip_config.h"  // 包含IP配置头文件

// 在main.c中的完整集成示例:

/*
// 1. 在main.c头部包含头文件
#include "ip_config.h"

// 2. 修改串口中断服务函数
void USART1_IRQHandler(void)
{
    u8 received_char;
    
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        received_char = USART_ReceiveData(USART1);
        
        // 调用IP配置处理函数
        uart_rx_interrupt_handler(received_char);
        
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

// 3. 在main函数中初始化和使用
int main(void)
{
    // ... 系统初始化代码 ...
    
    delay_init(168);       // 延时初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    uart_init(115200);     // 串口初始化
    
    // 启用串口接收中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    
    // 初始化IP配置系统
    ip_config_init();
    
    printf("System initialized. IP configuration ready.\r\n");
    
    while(1)
    {
        // ... 其他主循环代码 ...
        
        // 处理IP配置命令 (必须在主循环中调用)
        process_uart_commands();
        
        // 可选: 定期显示当前IP
        static u32 ip_display_counter = 0;
        ip_display_counter++;
        if(ip_display_counter > 1000000)  // 大约每几秒显示一次
        {
            char current_ip[16];
            get_current_ip_string(current_ip, sizeof(current_ip));
            printf("Current IP: %s\r\n", current_ip);
            ip_display_counter = 0;
        }
        
        // ... 其他代码 ...
        
        delay_ms(1);  // 防止主循环过快
    }
}
*/

// 使用方法说明:
/*
=== 串口命令使用 ===

1. 设置新的IP地址:
   发送: SET_IP:192.168.1.200
   响应: 
   Received IP config command: 192.168.1.200
   Checking IP conflict for: 192.168.1.200
   Saving IP config to EEPROM: C0A801C8
   IP configuration saved successfully
   SUCCESS: IP address updated to 192.168.1.200
   Please restart the device to apply new IP configuration

2. 查询当前IP地址:
   发送: GET_IP
   响应: Current IP: 192.168.1.200

3. 错误处理示例:
   发送: SET_IP:999.999.999.999
   响应: ERROR: Invalid IPv4 address format
   
   发送: UNKNOWN_CMD
   响应: ERROR: Unknown command. Use SET_IP:x.x.x.x or GET_IP

=== 系统特性 ===
✅ 实时IP地址验证
✅ IPv4格式检查 (0-255范围)
✅ ARP冲突检测框架
✅ EEPROM配置保存
✅ 串口中断驱动接收
✅ 缓冲区溢出保护
✅ 完整错误处理
✅ C89标准兼容

=== 技术规格 ===
- 串口波特率: 115200 (可配置)
- 命令缓冲区: 256字节
- IP地址格式: x.x.x.x (标准IPv4)
- 默认IP: 192.168.1.100
- 命令结束符: \r 或 \n
- 最大命令长度: 255字符

=== 安全特性 ===
- IP地址格式严格验证
- 数值范围检查 (0-255)
- 前导零检测
- 缓冲区溢出保护
- ARP冲突检测 (可扩展)
- 配置持久化存储

=== 扩展功能建议 ===
1. 子网掩码配置: SET_MASK:255.255.255.0
2. 网关配置: SET_GW:192.168.1.1
3. DNS配置: SET_DNS:8.8.8.8
4. DHCP开关: SET_DHCP:ON/OFF
5. 网络状态查询: GET_NET_STATUS
6. IP配置重置: RESET_IP_CONFIG

这个IP配置系统提供了完整的网络配置管理功能，
可以方便地通过串口动态调整设备的网络参数。
*/
