#ifndef __REC_STRUCTURE_H__
#define __REC_STRUCTURE_H__

#include "sys.h"

#define MAX_NUM_LEN     16
#define MAX_SERVER_NUMBER   5

//重传常量
// 添加消息类型定义
#define MSG_TYPE_NONE      0x00
#define MSG_TYPE_START      0x01
#define MSG_TYPE_OFFHOOK    0x02
#define MSG_TYPE_STOP       0x03

//typedef struct stUaInfo stUaInfo;
struct stRecInfo
{
    char calling_number[MAX_NUM_LEN];
	char called_number[MAX_NUM_LEN];
// �����ش������ֶ�
    u8 retry_count;          // ��ǰ�ش�����
    u8 max_retry;            // ����ش����� (3��)
    u16 retry_timeout;       // �ش���ʱʱ�� (2�� = 2000ms)
    u16 retry_timer;         // �ش���ʱ��
    u8 last_msg_type;        // ����͵���Ϣ����
    u8 waiting_response;     // �ȴ��ظ���־

// ������̬�˿�Э���ֶ�
    u16 dynamic_port;        // ����������Ķ�̬�˿�
    // u8 port_negotiated;      // �˿�Э����ɱ�־ (0=δЭ��, 1=��Э��)
    // u8 port_verified;        // �˿���֤��ɱ�־ (0=δ��֤, 1=����֤)
    // u16 port_request_timer;  // �˿�����ʱ��ʱ��
    // u8 port_request_retry;   // �˿��������Դ���

};

extern struct stRecInfo *gpsRecInfo;


struct stRecTxFlag
{	
	unsigned short retry:		1;		// 重传标志
};

extern struct stRecTxFlag gsRecTxFlag;

void BUILD_CID_JSON_offhook(void);
void BUILD_CID_JSON_start(void);
void BUILD_CID_JSON_stop(void);
void Rec_TxFlow(void);
void RecInfoInit(void); 
int HHDX_RecDivideJson(int name_index, char *addr, int len);
int HHDX_RecCommand(char *addr);
char *HHDX_RecGetFieldName(int cmd_type,char *p_msg);
int HHDX_RecFuncRx(int cmd_type,char *p_msg_name);
char *HHDX_RecGetFieldValue(char name,char *p_msg);
char HHDX_RecAnalyseJsonName(int cmd_type,char *p_msg);
void HHDX_RecAnalyseJsonValue(char name,char *p_msg);
// JSON发送函数
void SEND_CID_JSON(const char *event, const char *action, const char *data);

#endif
