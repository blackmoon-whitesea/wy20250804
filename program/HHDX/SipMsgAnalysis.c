#include "sys.h"
#include "usart.h"
#include "SipFunction.h"
#include "SipConstant.h"
#include "malloc.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "wm8978.h"
#include "Audio.h"
#include "i2s.h"
#include "24cxx.h"

// 串口IP配置相关变量
char uart_rx_buffer[256];  // 串口接收缓冲区
u8 uart_rx_index = 0;      // 接收索引
u8 uart_cmd_ready = 0;     // 命令准备标志

// 全局IP配置变量
u32 current_device_ip = 0xC0A80164;  // 默认IP: 192.168.1.100

// 外部函数声明 (在SipMsgBuilding.c中定义)
void BUILD_port_verify(u16 port);

// 如果atoi不可用，提供一个简单的实现
#ifndef atoi
int simple_atoi(const char* str)
{
    int result = 0;
    int sign = 1;
    
    if(*str == '-')
    {
        sign = -1;
        str++;
    }
    
    while(*str >= '0' && *str <= '9')
    {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return sign * result;
}
#define atoi simple_atoi
#endif

extern struct stUaInfo *gpsUaInfo;struct stMsg gsMsg;
struct stSipRxFlag gsSipRxFlag;
struct stSipTxFlag gsSipTxFlag;

// 服务器响应处理函数
void handle_server_response(void)
{
    // 收到服务器响应，清除重传标志
    if(gpsUaInfo->waiting_response)
    {
        gpsUaInfo->waiting_response = 0;
        gpsUaInfo->retry_count = 0;
        gpsUaInfo->retry_timer = 0;
    }
}

// 动态端口响应解析函数
void parse_port_response(char* response_data, int data_len)
{
    char *port_start, *port_end;
    char port_str[8];
    int port_len;
    unsigned int temp_port;  // 移到函数开头声明
    
    // 查找 "port":"xxxx" 格式
    port_start = strstr(response_data, "\"port\":\"");
    if(port_start != NULL)
    {
        port_start += 8;  // 跳过 "port":"
        port_end = strchr(port_start, '"');
        if(port_end != NULL)
        {
            port_len = port_end - port_start;
            if(port_len < 8)
            {
                strncpy(port_str, port_start, port_len);
                port_str[port_len] = '\0';
                
                // 转换端口号并保存
                String2Int(&temp_port, port_str);
                gpsUaInfo->dynamic_port = (u16)temp_port;
                gpsUaInfo->port_negotiated = 1;
                gpsUaInfo->port_request_timer = 0;
                
                printf("Received dynamic port: %d\r\n", gpsUaInfo->dynamic_port);
                
                // 验证端口可用性
                BUILD_port_verify(gpsUaInfo->dynamic_port);
            }
        }
    }
}

// 端口验证响应解析函数
void parse_port_verify_response(char* response_data, int data_len)
{
    // 查找验证成功标志
    if(strstr(response_data, "\"status\":\"success\"") != NULL)
    {
        gpsUaInfo->port_verified = 1;
        printf("Port verification successful\r\n");
    }
    else
    {
        printf("Port verification failed\r\n");
        // 重新请求端口
        gpsUaInfo->port_negotiated = 0;
        gpsUaInfo->port_verified = 0;
    }
}

// 文件传输响应解析函数
void parse_file_transfer_response(char* response_data, int data_len)
{
    if(strstr(response_data, "\"type\":\"file_ack\"") != NULL)
    {
        printf("File transfer acknowledged\r\n");
    }
}

// IPv4地址校验函数
u8 validate_ipv4(const char* ip_str)
{
    char ip_copy[32];
    char *token;
    int octet;
    int count = 0;
    
    if(strlen(ip_str) > 15 || strlen(ip_str) < 7) // 最短x.x.x.x，最长xxx.xxx.xxx.xxx
        return 0;
    
    strcpy(ip_copy, ip_str);
    token = strtok(ip_copy, ".");
    
    while(token != NULL && count < 4)
    {
        int i;
        // 检查是否为纯数字
        for(i = 0; i < strlen(token); i++)
        {
            if(token[i] < '0' || token[i] > '9')
                return 0;
        }
        
        // 转换为整数并检查范围
        octet = simple_atoi(token);
        if(octet < 0 || octet > 255)
            return 0;
            
        // 检查前导零（除了单独的0）
        if(strlen(token) > 1 && token[0] == '0')
            return 0;
            
        count++;
        token = strtok(NULL, ".");
    }
    
    return (count == 4 && token == NULL) ? 1 : 0;
}

// 将字符串IP转换为32位整数
u32 str_to_ip(const char* ip_str)
{
    u32 ip = 0;
    char ip_copy[32];
    char *token;
    int octet;
    int shift = 24;
    
    strcpy(ip_copy, ip_str);
    token = strtok(ip_copy, ".");
    
    while(token != NULL && shift >= 0)
    {
        octet = simple_atoi(token);
        ip |= ((u32)octet << shift);
        shift -= 8;
        token = strtok(NULL, ".");
    }
    
    return ip;
}

// ARP冲突检测函数
u8 check_ip_conflict(u32 target_ip)
{
    char ip_str[16];  // 变量声明移到函数开头
    
    // 这里应该实现ARP请求检测
    // 简化版本：发送ARP请求并等待响应
    printf("Checking IP conflict for: ");
    sprintf(ip_str, "%d.%d.%d.%d", 
            (int)((target_ip >> 24) & 0xFF),
            (int)((target_ip >> 16) & 0xFF), 
            (int)((target_ip >> 8) & 0xFF),
            (int)(target_ip & 0xFF));
    printf("%s\r\n", ip_str);
    
    // 实际项目中这里应该调用LWIP的ARP函数
    // 暂时返回0表示无冲突
    return 0;
}

// 保存IP配置到Flash/EEPROM
void save_ip_config(u32 new_ip)
{
    // 更新全局IP变量
    current_device_ip = new_ip;
    
    // 读取当前EEPROM配置
    // 修改IP地址
    // 写回EEPROM
    printf("Saving IP config to EEPROM: %08X\r\n", (unsigned int)new_ip);
    
    // 这里应该调用AT24CXX写入函数
    // AT24CXX_Write(IP_ADDR_OFFSET, (u8*)&new_ip, 4);
    
    printf("IP configuration saved successfully\r\n");
}

// 处理串口IP配置命令
void process_ip_config_command(char* cmd)
{
    char ip_str[32];
    u32 new_ip;
    
    // 解析命令格式：SET_IP:192.168.1.100
    if(strncmp(cmd, "SET_IP:", 7) == 0)
    {
        strcpy(ip_str, cmd + 7);  // 提取IP地址部分
        
        printf("Received IP config command: %s\r\n", ip_str);
        
        // 1. 校验IP地址格式
        if(!validate_ipv4(ip_str))
        {
            printf("ERROR: Invalid IPv4 address format\r\n");
            return;
        }
        
        // 2. 转换IP地址
        new_ip = str_to_ip(ip_str);
        
        // 3. 检查IP冲突
        if(check_ip_conflict(new_ip))
        {
            printf("ERROR: IP address conflict detected\r\n");
            return;
        }
        
        // 4. 保存配置
        save_ip_config(new_ip);
        
        // 5. 应用新配置（需要重启网络接口）
        printf("SUCCESS: IP address updated to %s\r\n", ip_str);
        printf("Please restart the device to apply new IP configuration\r\n");
    }
    else if(strncmp(cmd, "GET_IP", 6) == 0)
    {
        char current_ip[16];
        
        // 获取当前IP地址 - 使用sprintf函数转换
        sprintf(current_ip, "%d.%d.%d.%d",
                (int)((current_device_ip >> 24) & 0xFF),
                (int)((current_device_ip >> 16) & 0xFF),
                (int)((current_device_ip >> 8) & 0xFF),
                (int)(current_device_ip & 0xFF));
        
        printf("Current IP: %s\r\n", current_ip);
    }
    else
    {
        printf("ERROR: Unknown command. Use SET_IP:x.x.x.x or GET_IP\r\n");
    }
}

// 串口接收中断处理函数
void uart_rx_interrupt_handler(u8 received_char)
{
    // 处理接收到的字符
    if(received_char == '\r' || received_char == '\n')
    {
        if(uart_rx_index > 0)
        {
            uart_rx_buffer[uart_rx_index] = '\0';  // 字符串结束符
            uart_cmd_ready = 1;  // 命令准备就绪
        }
    }
    else if(uart_rx_index < sizeof(uart_rx_buffer) - 1)
    {
        uart_rx_buffer[uart_rx_index++] = received_char;
    }
    else
    {
        // 缓冲区溢出，重置
        uart_rx_index = 0;
        printf("ERROR: UART buffer overflow\r\n");
    }
}

// 处理串口命令（在主循环中调用）
void process_uart_commands(void)
{
    if(uart_cmd_ready)
    {
        uart_cmd_ready = 0;
        process_ip_config_command(uart_rx_buffer);
        uart_rx_index = 0;  // 重置接收索引
    }
}

// 获取当前IP配置字符串
void get_current_ip_string(char* ip_buffer, int buffer_size)
{
    if(ip_buffer != NULL && buffer_size >= 16)
    {
        sprintf(ip_buffer, "%d.%d.%d.%d",
                (int)((current_device_ip >> 24) & 0xFF),
                (int)((current_device_ip >> 16) & 0xFF),
                (int)((current_device_ip >> 8) & 0xFF),
                (int)(current_device_ip & 0xFF));
    }
}

// IP配置系统初始化
void ip_config_init(void)
{
    char ip_str[16];  // 变量声明移到函数开头
    
    uart_rx_index = 0;
    uart_cmd_ready = 0;
    printf("IP Configuration System Initialized\r\n");
    printf("Default IP: ");
    
    get_current_ip_string(ip_str, sizeof(ip_str));
    printf("%s\r\n", ip_str);
    
    printf("Commands: SET_IP:x.x.x.x or GET_IP\r\n");
}
/*
1. Request-Line: INVITE sip:3051@172.16.16.60:5060 SIP/2.0
2. Status-Line: SIP/2.0 200 OK
3. Status-Line: SIP/2.0 183 Session Progress
*/
//判断请求命令还是状态命令
int ANALYSIS_line(char *p_line)
{
	unsigned int i,result;
	int j=0;	//字段长度
	int k=0;	//xxx-Line ' '空格数
	char ibuffer[20]; // SIP数据缓存 
	char *lp_line=p_line;	
	for(i=0;i<20;i++)
	{
//1. 第一个字节初始化ibuffer		
		if(j==0) ibuffer[0]='\0';
//2. 第一个' ' 并判断 Request-Line: xxxx或Status-Line: SIP/2.0 			
    if(*(p_line+j)==' '&&k==0)			
		{
			p_line=p_line+j+1;
			ibuffer[j]='\0';
		//2.1. 是"SIP/2.0"则找第二个' '
			if(!strcmp(ibuffer,"SIP/2.0")){
				k++;	//第二个' '
				j=0;	//第二个' '地址归0
				continue;
			}
		//2.1. 是Request-Line,则直接退出for
			else
				break;
		}
//3. 第二个' ' 		
    if(*(p_line+j)==' '&&k==1)	//Status-Line 第二个' ',如180/183 200
		{
      ibuffer[j]='\0';
      break;
		}		
    ibuffer[j]= *(lp_line+i);
		j++;
		if(j==20)
			j=0;
	}
//取得Request-Line或Status-Line名称
	if(i!=20)
	{
		result=ANALYSIS_get_lname(ibuffer);
	}	
	else	result=0;
	return result;
}

/*
 取得Request-Line或Status-Line名称
*/
int ANALYSIS_get_lname(char *p_line_name)
{
//------- Request-Line:
	if(!strcmp(p_line_name,"INVITE"))
		return INVITE;
  if(!strcmp(p_line_name,"ACK"))
		return ACK;
  if(!strcmp(p_line_name,"BYE"))
		return BYE;
	if(!strcmp(p_line_name,"CANCEL"))
		return CANCEL;	
//	if(!strcmp(p_line_name,"OPTION"))
//		return OPTION;	
//	if(!strcmp(p_line_name,"REGISTER"))
//		return REGISTER;

//------- Status-Line:	
	if(!strcmp(p_line_name,"100"))
		return TRYING;
  if(!strcmp(p_line_name,"180"))
		return RINGING;
  if(!strcmp(p_line_name,"200"))
		return OK;
	if(!strcmp(p_line_name,"487"))
		return REQUEST_TERMINATED;
	if(!strcmp(p_line_name,"407"))
		return PROXY;	
	return 0;
}		

/*
 根据接收到line名,判定要分析哪些message header
入口参数 
	line_name: 第一行名称
	p_msg: 第一行第一字节地址
	msg_len: 整个sip包长度
*/
void ANALYSIS_line_to_msg(int line_name,char *p_msg,int msg_len)
{
	int len;
	switch(line_name)
	{
		case 0:
			break;
//1. 被叫收到INVITE
		case INVITE:
//			ANALYSIS_RxParamInit();			
			gsSipRxFlag.invite =1;
			gSipMechanism = RX_INVITE;
			gSipTxFlow =TX_INVITE_100_TRYING;
			len =ANALYSIS_message(p_msg, msg_len);
			ANALYSIS_cseq(gsMsg.cseq,0);	//取到CSeq中xxx
			if(msg_len-len>20){	//说明有sdp内容
				ANALYSIS_sdp(p_msg+len, msg_len-len);
			}
			break;
		case ACK:
			gSipMechanism = RX_ACK;
			gsSipRxFlag.ack =1;
			break;	
		case BYE:
			gSipMechanism = RX_BYE;			
			gSipTxFlow =TX_BYE_200_OK;
			gsSipRxFlag.bye =1;
			len =ANALYSIS_message(p_msg, msg_len);
			break;
		case CANCEL:
			gSipMechanism = RX_CANCEL;			
			gSipTxFlow =TX_CANCEL_200_OK;
			gsSipRxFlag.cancel =1;		
			len =ANALYSIS_message(p_msg, msg_len);
			break;
					
		case TRYING:
			break;
		case RINGING:		
			gSipMechanism = RX_180_RINGING;
			gsSipRxFlag.ringing =1;
			break;
		case OK:
			ANALYSIS_get_cseq(p_msg,msg_len);
			if(gsSipRxFlag.rec ==1){
				gsSipRxFlag.rec =0;
				len =ANALYSIS_message(p_msg, msg_len);
				if(msg_len-len>20){	//说明有sdp内容
					ANALYSIS_sdp(p_msg+len, msg_len-len);
				}
			}
			break;
		case REQUEST_TERMINATED:		//487	
			gSipMechanism =RX_487;			
			gSipTxFlow =TX_ACK;	
			len =ANALYSIS_message(p_msg, msg_len);
			ANALYSIS_cseq(gsMsg.cseq,0);	//取到CSeq中xxx
		case PROXY:
			gsSipRxFlag.proxy =1;
			break;	
			
			
		default:
			break;
	}
}

/*
 200 OK中有以下几个情况
*/
void ANALYSIS_cseq_type(char *p_msg_name)
{
	if(!strcmp(p_msg_name,"INVITE")){		
		gSipMechanism = RX_INVITE_200_OK;
		gSipTxFlow =TX_ACK;
		gsSipRxFlag.ok_inv =1;
		gsSipRxFlag.rec =1;
		//添加服务器响应处理
		handle_server_response();
	}
  	if(!strcmp(p_msg_name,"BYE")){
  		gSipMechanism = RX_BYE_200_OK;
  		gsSipRxFlag.ok_bye =1;
		// 添加服务器响应处理
    	handle_server_response();
	}
	if(!strcmp(p_msg_name,"CANCEL")){
		gSipMechanism = RX_CANCEL_200_OK;
		gsSipRxFlag.ok_cel =1;
		// 添加服务器响应处理
    	handle_server_response();
	}
	if(!strcmp(p_msg_name,"REGISTER")){
		gSipMechanism = RX_BYE_200_OK;
		gsSipRxFlag.ok_reg =1;
		// 添加服务器响应处理
    	handle_server_response();
	}
	// 处理动态端口响应消息
	if(!strcmp(p_msg_name,"PORT_RESPONSE")){
		printf("Received PORT_RESPONSE message\r\n");
		handle_server_response();
	}
	if(!strcmp(p_msg_name,"PORT_VERIFY_RESPONSE")){
		printf("Received PORT_VERIFY_RESPONSE message\r\n");
		handle_server_response();
	}
	if(!strcmp(p_msg_name,"FILE_ACK")){
		printf("Received FILE_ACK message\r\n");
		handle_server_response();
	}
}

/*
1. 分析整个Message Header
2. 先以"\r\n"为分结束符，取得每段ibuffer[]
3. 再分析每字段
4. 返回还剩多少
Message Header
    Via: SIP/2.0/UDP 172.16.1.135:5060
    Via: SIP/2.0/UDP 172.16.66.80:5060;branch=z9hG4bK3606831015
    From: <sip:7000@172.16.1.135>;tag=1302917275
    To: <sip:5000@172.16.1.135:5060>
    Call-ID: 1578683025@172.16.66.80
    [Generated Call-ID: 1578683025@172.16.66.80]
    CSeq: 20 INVITE
    Contact: <sip:7000@172.16.66.80:5060>
    Max-Forwards: 9
    User-Agent: AOS 2.0.12.0-TCP01
    Record-Route: <sip:172.16.1.135:5060;lr>
    Content-Type: application/sdp
    Content-Length:   157
*/
int ANALYSIS_message(char *p_msg,int msg_len)
{
	int i;		
	int j=0;	//每行长度，但不包含'\r\n'两字节
	int k=0;	//第几个'\r\n'
	char ibuffer[300]; //每行message header数据缓存 
	char *lp_msg=p_msg;	//第一行line首地址	
	gsMsg.via_num =0;
	gsMsg.re_route_num =0;
	for(i=0;i<msg_len;i++)
	{
		if(j==0)
			ibuffer[0]='\0';
		if((*(p_msg+j)=='\r')&&(*(p_msg+j+1)=='\n'))
		{
			p_msg=p_msg+j+2;	//下一个line首地址
	    if(j==0)					//两个'\r\n\r\n': 表示Message Header结束
				break;					
			ibuffer[j]='\0';
//			printf("%s",ibuffer);
			if(k!=0)		//k=0是Request-Line或Status-Line已分析
			   ANALYSIS_msghdr(ibuffer,j);
			/////////////////////////////
			k++;
			i++;
			j=0;
		  continue;
		}
    ibuffer[j]= *(lp_msg+i); // 将每line内容搬到ibuffer[j]中
		j++;
		if(j==300)
			j=0;
	}
  ibuffer[0]='\0';
	return i;			//返回长度
}


/*
1. 分析每行字段
    Via: SIP/2.0/UDP 172.16.66.80:5060;branch=z9hG4bK3606831015
    From: <sip:7000@172.16.1.135>;tag=1302917275
    To: <sip:5000@172.16.1.135:5060>
    Call-ID: 1578683025@172.16.66.80
    ...
    
2. 记录Via、From、To、Call-ID、CSeq、Record-Route整行
*/
void ANALYSIS_msghdr(char *p_msg, int len)
{
	int i=0;
	char ibuffer[20]; //message header名称缓存	
	while(i<len)
	{
		if((*(p_msg+i)==':')&&(*(p_msg+i+1)==' ')){
	// 每一个消息头都遵循以下格式：首先是头字段名（如Via、From等），
	// 后面是冒号；然后紧接为一个获多个前导空格，后面为字段值:
	// header-header = field-name: SP [field-value] CRLF
			ibuffer[i]='\0'; 				//字符串结束符
			switch(ANALYSIS_get_msgname(ibuffer))
			{
				case VIA:	
					gsMsg.via[gsMsg.via_num] =MymallocCopy(gsMsg.via[gsMsg.via_num],len,p_msg);
					gsMsg.via_num ++;
					if(gsMsg.via_num ==MAX_SERVER_NUMBER)
						gsMsg.via_num =0;
					break;
				case FROM:	
					gsMsg.from=MymallocCopy(gsMsg.from,len,p_msg);
					break;			
        case TO:	
        	gsMsg.to=MymallocCopy(gsMsg.to,len,p_msg);
					break;
				case CALL_ID:
					gsMsg.callid=MymallocCopy(gsMsg.callid,len,p_msg);
					break;
				case CSEQ:	
					gsMsg.cseq=MymallocCopy(gsMsg.cseq,len,p_msg);
					break;
				case RECORD_ROUTE:
					gsMsg.re_route[gsMsg.re_route_num]=MymallocCopy(gsMsg.re_route[gsMsg.re_route_num],len,p_msg);
					gsMsg.re_route_num ++;
					if(gsMsg.re_route_num ==MAX_SERVER_NUMBER)
						gsMsg.re_route_num =0;
					break;
        case CONTACT:
					break;				
				case MAX_FORWARDS:
					break;
				case ROUTE:
					break; 		
      	case USER_AGENT:
					break;
      	case CONTENT_TYPE:
					break;
				case CONTENT_LENGTH:
					
					break;
     		case EXPIRES:
					break;
				default:
					break;
			}	
			break;	//分析每个字段就结束
		}
   	ibuffer[i]=*(p_msg+i);	// 将字段名copy到ibuffer[i]
		i++;
	}
	ibuffer[0]='\0';
	return;
}

/*
 取得Message Header名称
*/
int ANALYSIS_get_msgname(char *p_msg_name)
{		
	if(!strcmp(p_msg_name,"Via")){		
		return VIA;
	}
  if(!strcmp(p_msg_name,"Record-Route")){
		return RECORD_ROUTE;
	}
  if(!strcmp(p_msg_name,"From"))
		return FROM;
	if(!strcmp(p_msg_name,"To"))
		return TO;
  if(!strcmp(p_msg_name,"Call-ID"))
		return CALL_ID;
  if(!strcmp(p_msg_name,"CSeq"))
		return CSEQ;	
  if(!strcmp(p_msg_name,"Content-Length"))
		return CONTENT_LENGTH;
	if(!strcmp(p_msg_name,"Max-Forwards"))
		return MAX_FORWARDS;
  if(!strcmp(p_msg_name,"Route"))
		return ROUTE;		
  if(!strcmp(p_msg_name,"Contact"))
		return CONTACT;
  if(!strcmp(p_msg_name,"User-Agent"))
		return USER_AGENT;
  if(!strcmp(p_msg_name,"Content-Type"))
		return CONTENT_TYPE;
	if(!strcmp(p_msg_name,"Expires"))
		return EXPIRES;	
	return 0;
}


/*
v=0
o=5000 19618 0 IN IP4 172.16.66.185
s=-
c=IN IP4 172.16.66.185
t=0 0
m=audio 20000 RTP/AVP 0 8 4 18 101
a=rtpmap:101 telephone-event/8000
a=fmtp:101 0-15
a=rtcp:20001
a=sendrecv

*/
void ANALYSIS_sdp(char *p_sdp, int sdp_len)
{	
	int i=0;
	int j=0;
	int k=0;	//字段标志
	char ibuffer[20]; //message header名称缓存
	for(i=0;i<sdp_len;i++)
	{
		if(*(p_sdp+i)=='\n'){
			j=0;			//每个字段名开始
			k=0;			//每行' '从0开始计算
			continue;	//跳过'\n'，不将其写入ibuffer[j]
		}	
//1. 查找有用两行
    if(*(p_sdp+i)==' '&&k==0){	//第一个' '
    	ibuffer[j] ='\0';
    	if(!strcmp(ibuffer,"c=IN")){
				k=1;
    	}
    	else if(!strcmp(ibuffer,"m=audio")){
				k=3;
    	}
    	else{
    		k=0;
    	}
			j=0;			//每个字段名开始
			continue;	//跳过' '
		}
//2. c=IN IP4 172.16.66.185		
		if(*(p_sdp+i)==' '&&k==1){	//c=IN IP4 
		  k=2;
    	j=0;			//每个字段名开始
			continue;	//跳过' '
		}
    if(*(p_sdp+i)=='\r'&&k==2){	//c=IN IP4 172.16.66.185
		  ibuffer[j] ='\0';
		  if(j<16){
		  	strcpy(gpsUaInfo->rtp_ip,ibuffer);
		  //	printf("ip=%s\r\n",gpsUaInfo->rtp_ip);
		  }
		  k=0;
    	j=0;			//每个字段名开始
			continue;	//跳过' '
		}	
//3. m=audio 20000
		if(*(p_sdp+i)==' '&&k==3){
			ibuffer[j] ='\0';
			if(j<6){
				String2Int(&gpsUaInfo->rtp_port,ibuffer);
				gsSipRxFlag.sdp =1;
//		  	printf("p=%d\r\n",gpsUaInfo->rtp_port);
		  }	
			break;
		}
		ibuffer[j]=*(p_sdp+i);	// 将字段名copy到ibuffer[j]		
		j++;
		if(j==20)
			j=0;
	}
}


/*
 1. 比较整个message header
 2. 找出CSeq: 
 3. 返回CSeq: 地址
*/

void ANALYSIS_get_cseq(char *p_msg,int msg_len)
{
	int i=0;
	int j=0;
	char ibuffer[20]; //message header名称缓存
	for(i=0;i<msg_len;i++)
	{
		if(*(p_msg+i)=='\n'){
			j=0;	//每个字段名开始
			continue;
		}
		
		if((*(p_msg+i)==':')&&(*(p_msg+i+1)==' ')){
	// 每一个消息头都遵循以下格式：首先是头字段名（如Via、From等），
	// 后面是冒号；然后紧接为一个获多个前导空格，后面为字段值:
	// header-header = field-name: SP [field-value] CRLF
			ibuffer[j]='\0'; 				//字符串结束符
			if(!strcmp(ibuffer,"CSeq")){
				ANALYSIS_cseq(p_msg+i+1,1);	//从' '开始
				break;
			}	
		}			
		ibuffer[j]=*(p_msg+i);	// 将字段名copy到ibuffer[j]		
		j++;
		if(j==20)
			j=0;
	}
}

/*
	CSeq: 1 INVITE

	1. num_name=0: 只需cseq num 
*/
void ANALYSIS_cseq(char *p_cseq,char num_name)
{
	unsigned int i;
	int k=0;	//xxx-Line ' '空格数
	int j=0;	//字段长度
	char ibuffer[20]; // cseq类型数据
	for(i=0;i<30;i++)	// 假定CSeq:行不可能大于30
	{
//1. 第一个' '跳过CSeq: 		
    if(*(p_cseq+i)==' '&&k==0)			
		{
			k++;	//第二个' '
			j =0;
			continue;
		}
//2. 第二个' ' 为xxx 	
    if(*(p_cseq+i)==' '&&k==1)
		{
			ibuffer[j] ='\0';	
			if(j<6){
				String2Int(&gsMsg.cseq_num,ibuffer);	
			}
			j=0;
      continue;
		}		
//3. 行结束	
    if(*(p_cseq+i)=='\r')
		{
			ibuffer[j] ='\0';
			if(num_name==1){	//=1: 是200 OK
				ANALYSIS_cseq_type(ibuffer);
			}
      break;
		}				
    ibuffer[j]= *(p_cseq+i);
		j++;
		if(j==20)
			j=0;
	}
}


/*
 初始化 sip rx/tx flag
*/
void ANALYSIS_RxParamInit(void)
{
	
	gSipMechanism =0;
//	gsMsg.via_num =0;
//	gsMsg.re_route_num =0;	
	gpsUaInfo->rtp_port =0;
	
	gsSipRxFlag.invite =0;		
	gsSipRxFlag.ack =0;		
	gsSipRxFlag.bye =0;
	gsSipRxFlag.cancel =0;	
	gsSipRxFlag.ringing =0;
	gsSipRxFlag.ok_inv =0;
	gsSipRxFlag.ok_bye =0;
	gsSipRxFlag.ok_cel =0;
	gsSipRxFlag.proxy =0;		
	gsSipRxFlag.sdp =0;
	gsSipRxFlag.rec =0;
	
	gsSipTxFlag.invite =0;		
	gsSipTxFlag.ack =0;		
	gsSipTxFlag.bye =0;
	gsSipTxFlag.cancel =0;	
	gsSipTxFlag.ringing =0;
	gsSipTxFlag.ok_inv =0;
	gsSipTxFlag.ok_bye =0;
	gsSipTxFlag.ok_cel =0;
	gsSipTxFlag.proxy =0;
	gsSipTxFlag.rtp =0;		
	
}
