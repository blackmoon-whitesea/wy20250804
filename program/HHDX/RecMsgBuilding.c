#include <stdio.h>
#include "usart.h"
#include "malloc.h"
#include "net_rxtx.h"
#include "stdio.h"
#include "string.h" 
#include "timer.h"
#include "HhdxMsg.h"
#include "24cxx.h" 
#include "RecMsgBuilding.h"
#include "usart3_FSK.h"
#include "SipFunction.h"

struct stRecInfo *gpsRecInfo;
struct stRecTxFlag gsRecTxFlag;
// 函数声明
void SEND_CID_JSON(const char* msg_type, const char* status, const char* call_number);

/***********************************************************************
"Send_Start={" + 编号(4位) + 本机号(16位) + 来电号(16位) }
***********************************************************************/
void BUILD_CID_JSON_start(void)
{
	int i;
	
	strcpy((char *)gFskCidBuf,gpsRecInfo->called_number);	//来电号码
	for(i =strlen((char *)gFskCidBuf); i <MAX_NUM_LEN; i++){
		gFskCidBuf[i] =0x20;
	}
	gFskCidBuf[MAX_NUM_LEN] =0;
	
	// 每次都从EEPROM获取最新的本机号码
	// strcpy((char *)gFskOwnBuf,(const char*)gpsEeprom->own_num);	//本机号码
	// for(i =strlen((char *)gFskOwnBuf); i <MAX_NUM_LEN; i++){
	// 	gFskOwnBuf[i] =0x20;
	// }
	// gFskOwnBuf[MAX_NUM_LEN] =0;
	
	// 使用新的JSON发送函数
	SEND_CID_JSON("incoming_call", "Start", (char*)gFskCidBuf);

	//重传设置
    if(!gsRecTxFlag.retry){
	    gpsRecInfo->waiting_response = 1;
        gpsRecInfo->retry_count = 0;
        gpsRecInfo->max_retry = 2;
        gpsRecInfo->retry_timeout = 2000;  // 2秒
        gpsRecInfo->retry_timer = 2000;
        gpsRecInfo->last_msg_type = MSG_TYPE_START;
    }
}

/***********************************************************************
"Send_Stop" + 编号(4位)
***********************************************************************/
void BUILD_CID_JSON_stop(void)
{
	// 使用新的JSON发送函数
	SEND_CID_JSON("call_end", "Stop", NULL);

	//重传设置
    if(!gsRecTxFlag.retry){
	    gpsRecInfo->waiting_response = 1;
        gpsRecInfo->retry_count = 0;
        gpsRecInfo->max_retry = 2;
        gpsRecInfo->retry_timeout = 2000;  // 2秒
        gpsRecInfo->retry_timer = 2000;
        gpsRecInfo->last_msg_type = MSG_TYPE_STOP;
        gsRecTxFlag.retry = 0; // 清除重传标志
    }
}

/***********************************************************************
"Send_Offhook={}" + 编号(4位)
***********************************************************************/
void BUILD_CID_JSON_offhook(void)
{
	// 使用新的JSON发送函数
	SEND_CID_JSON("call_answer", "OffHook", NULL);

	//重传设置
    if(!gsRecTxFlag.retry){
	    gpsRecInfo->waiting_response = 1;
        gpsRecInfo->retry_count = 0;
        gpsRecInfo->max_retry = 2;
        gpsRecInfo->retry_timeout = 2000;  // 2秒
        gpsRecInfo->retry_timer = 2000;
        gpsRecInfo->last_msg_type = MSG_TYPE_OFFHOOK;
        gsRecTxFlag.retry = 0; // 清除重传标志
    }
}

// JSON格式来电信息发送函数
void SEND_CID_JSON(const char* msg_type, const char* status, const char* call_number)
{
    char json_msg[256];
    char timestamp[16];
    char *SendMsg;
    int msg_len;
    int i;
    // 获取当前时间戳
    sprintf(timestamp, "%04d%02d%02d%02d%02d%02d", 
        gsDate.year, gsDate.month, gsDate.day, 
        gsDate.hour, gsDate.min, gsDate.sec);
    
	// 每次都从EEPROM获取最新的本机号码
	strcpy((char *)gFskOwnBuf,(const char*)gpsEeprom->own_num);	//本机号码
	for(i =strlen((char *)gFskOwnBuf); i <MAX_NUM_LEN; i++){
		gFskOwnBuf[i] =0x20;
	}
	gFskOwnBuf[MAX_NUM_LEN] =0;

    // 构建JSON消息
    if(call_number != NULL)
    {
        // 包含来电号码的消息（呼入事件）
        // msg_len = sprintf(json_msg, "Send_%s"
        //     "{\"id\":\"%s\",\"number\":\"%s\",\"call\":\"%s\",\"timestamp\":\"%s\",\"type\":\"%s\",\"status\":\"%s\"}\r\n",
        //     status,gpsEeprom->own_id,gFskOwnBuf, call_number, timestamp, msg_type, status);
   
        msg_len = sprintf(json_msg, "Send_%s"
            "{\"id\":\"%s\",\"number\":\"%s\",\"call\":\"%s\"}\r\n",
            status,gpsEeprom->own_id,gFskOwnBuf, call_number);
    
    }
    else
    {
        // 不包含来电号码的消息（接听/挂断事件）
        // msg_len = sprintf(json_msg, "Send_%s"
        //     "{\"id\":\"%s\",\"number\":\"%s\",\"timestamp\":\"%s\",\"type\":\"%s\",\"status\":\"%s\"}\r\n",
        //     status,gpsEeprom->own_id,gFskOwnBuf, timestamp, msg_type, status);
        msg_len = sprintf(json_msg, "Send_%s"
            "{\"id\":\"%s\",\"number\":\"%s\"}\r\n",
            status,gpsEeprom->own_id,gFskOwnBuf);
    
    }
    
    // 网络发送
    SendMsg = (char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
    strcpy(SendMsg, json_msg);
    NET_TxPcbLenWr(SIP_TX, msg_len);
    
    // 串口发送
    printf("CID_JSON: %s", json_msg);
}

// // 初始化动态端口协商
// void init_dynamic_port_negotiation(void)
// {
//     printf("Initializing dynamic port negotiation...\r\n");
    
//     // 启动端口请求
//     if(gpsRecInfo->dynamic_port == 0)
//     {
//         transfer_recording_file("recording_001.wav");
//     }
// }

//Rec发送流程
void Rec_TxFlow(void)
{
// 检查重传标志
    if(gsRecTxFlag.retry)
    {
    
        //printf("retry times: %d %d %d\r\n", gpsRecInfo->retry_count, gpsRecInfo->max_retry, gpsRecInfo->retry_timer);

        // 根据最后发送的消息类型重发
        switch(gpsRecInfo->last_msg_type)
        {
            case MSG_TYPE_START:
                BUILD_CID_JSON_start();
                break;
            case MSG_TYPE_OFFHOOK:
                BUILD_CID_JSON_offhook();
                break;
            case MSG_TYPE_STOP:
                BUILD_CID_JSON_stop();
                break;
            // case MSG_TYPE_PORT_REQUEST:
            //     BUILD_port_request();
            //     break;
            // case MSG_TYPE_PORT_VERIFY:
            //     BUILD_port_verify(gpsUaInfo->dynamic_port);
            //     break;
        }

        gsRecTxFlag.retry = 0;

        return;
    }
}

void RecInfoInit(){
    gpsRecInfo->retry_count = 0;
    gpsRecInfo->max_retry = 3;
    gpsRecInfo->retry_timeout = 2000;   // 2秒
    gpsRecInfo->retry_timer = 0;
    gpsRecInfo->last_msg_type = MSG_TYPE_NONE;
    gpsRecInfo->waiting_response = 0;
    gsRecTxFlag.retry = 0; // 清除重传标志
    gpsRecInfo->dynamic_port = gpsEeprom->port; // 初始化动态端口为0
    // gpsRecInfo->port_negotiated = 0; // 初始化端口协商状态
    // gpsRecInfo->port_verified = 0;   // 初始化端口验证状态
    // gpsRecInfo->port_request_timer = 0; // 初始化端口请求计时器
    // gpsRecInfo->port_request_retry = 0; // 初始化端口请求重试计数器
}