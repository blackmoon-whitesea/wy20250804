// IP配置系统头文件声明
// 在需要访问IP配置的其他文件中包含此声明

#ifndef IP_CONFIG_H
#define IP_CONFIG_H

#include "sys.h"

// 全局IP配置变量声明 (定义在SipMsgAnalysis.c中)
extern u32 current_device_ip;

// IP配置相关函数声明
extern void process_uart_commands(void);
extern void uart_rx_interrupt_handler(u8 received_char);
extern u8 validate_ipv4(const char* ip_str);
extern u32 str_to_ip(const char* ip_str);
extern u8 check_ip_conflict(u32 target_ip);
extern void save_ip_config(u32 new_ip);
extern void process_ip_config_command(char* cmd);

// IP配置系统初始化函数
void ip_config_init(void);

// 获取当前IP配置
void get_current_ip_string(char* ip_buffer, int buffer_size);

// 应用新的IP配置到网络接口
void apply_ip_config(u32 new_ip);

#endif // IP_CONFIG_H

/*
使用示例:

1. 在main.c中包含此头文件:
   #include "ip_config.h"

2. 在主循环中调用:
   process_uart_commands();

3. 在串口中断中调用:
   uart_rx_interrupt_handler(received_char);

4. 访问当前IP:
   printf("Current IP: %08X\n", current_device_ip);

5. 获取IP字符串:
   char ip_str[16];
   get_current_ip_string(ip_str, sizeof(ip_str));
   printf("Current IP: %s\n", ip_str);
*/
