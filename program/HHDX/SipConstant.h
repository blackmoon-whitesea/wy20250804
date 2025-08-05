
#define ETH_ALEN		6		/* Octets in one ethernet addr	 */


//mask address: 255.255.0.0
#define NET_MASK	0xFFFF0000

#define NET_MASK0	255
#define NET_MASK1	255
#define NET_MASK2	0
#define NET_MASK3	0

//------ LOCAL IP: 192.168.1.100
#define HOST_IP		0xC0A80164

#define HOST_IP0	192
#define HOST_IP1	168
#define HOST_IP2	1	
#define HOST_IP3	100

//------ Gateway IP: 192.168.0.1
#define GW_IP		0xC0A80001

#define GW_IP0	192
#define GW_IP1	168
#define GW_IP2	0
#define GW_IP3	1

//------ Server IP: 192.168.1.20
#define SV_IP		0xC0A80114

#define SV_IP0	192
#define SV_IP1	168
#define SV_IP2	1
#define SV_IP3	20

#define RTP_IP0	192
#define RTP_IP1	168
#define RTP_IP2	0
#define RTP_IP3	10



#define SIP_PORT 	5060
#define SIP_EXPIRES	1200

//重传常量
// 添加消息类型定义
#define MSG_TYPE_START      0x01
#define MSG_TYPE_OFFHOOK    0x02
#define MSG_TYPE_STOP       0x03

//动态端口协商常量
#define PORT_REQUEST_TIMEOUT    3000    // 端口请求超时时间 (3秒)
#define PORT_REQUEST_MAX_RETRY  3       // 端口请求最大重试次数
#define FILE_BLOCK_SIZE         1024    // 文件分块大小 (1KB)
#define FILE_TRANSFER_TIMEOUT   5000    // 文件传输超时时间 (5秒)

// 动态端口协商消息类型
#define MSG_TYPE_PORT_REQUEST   0x10    // 端口请求
#define MSG_TYPE_PORT_RESPONSE  0x11    // 端口响应
#define MSG_TYPE_PORT_VERIFY    0x12    // 端口验证
#define MSG_TYPE_FILE_START     0x20    // 文件传输开始
#define MSG_TYPE_FILE_DATA      0x21    // 文件数据块
#define MSG_TYPE_FILE_END       0x22    // 文件传输结束
#define MSG_TYPE_FILE_ACK       0x23    // 文件传输确认

//SIP 常用关键字
//-------- SIP mechanism define
//Normal calling
#define TX_INVITE						0x01	
#define RX_180_RINGING			0x02
#define RX_INVITE_200_OK		0x03
#define TX_ACK							0x04
#define TX_CANCEL						0x05
#define RX_CANCEL_200_OK		0x06
#define RX_487							0x07
#define TX_48X_ACK					0x08
#define RX_486_BUSY					0x09
#define RX_INVITE_100_TRYING 0x8A
	
//Normal Called	
#define RX_INVITE						0x81
#define TX_180_RINGING			0x82	
#define TX_INVITE_200_OK		0x83	
#define RX_ACK							0x84	
#define RX_CANCEL						0x85
#define TX_CANCEL_200_OK		0x86
#define TX_487							0x87
#define RX_487_ACK					0x88
#define TX_486_BUSY					0x89
#define TX_INVITE_100_TRYING 0x8A
	
	
//Normal calling/Called	
#define TX_BYE							0x40
#define RX_BYE							0x41
#define TX_BYE_200_OK				0x42
#define RX_BYE_200_OK				0x43
	

//IP phone hook-off/on tramsmit SIP
#define FLASH_TX_SIP				0xE0	//按Flash键发‘pc’
#define DIAL_NUM_TX_SIP			0xE1	//专用发单个号码
#define HOOKOFF_TX_SIP			0xE2	//拨‘#’号或拨号6s时间到/重拨/专用发‘tj’
#define HOOKON_TX_SIP				0xE3
#define HOTLINE_TX_SIP			0xE4
	
//主叫摘机	
#define PHONE_HKOFF_HFON		0xFE	



//-------- SIP header
#define SIP_MSG_HDR_TO			0x01	
#define SIP_MSG_HDR_FROM		0x02
#define SIP_MSG_HDR_ID			0x03
#define SIP_MSG_HDR_CSEQ		0x04
#define SIP_MSG_HDR_CONTACT	0x05
#define SIP_MSG_HDR_CLEN		0x06
#define SIP_MSG_HDR_VIA			0x07
#define SIP_MSG_HDR_RECORD	0x08
	
	

//请求方法
#define INVITE         11
#define ACK            12
#define BYE            13            
#define OPTION         14
#define CANCEL         15
#define REGISTER       16

//SIP版本
#define SIPVERSION     17

//SIP 常规首部
#define ACCEPT           51
#define ACCEPT_ENCODING  52
#define ACCEPT_Language  53
#define CALL_ID          54
#define CONTACT          55
#define CSEQ             56
#define DATE             57
#define ENCRYPTION       58
#define FROM             59
#define ORGANIZATION     60
#define RECORD_ROUTE     61
#define REQUIRE          62
#define SUPPORTED        63
#define TIMESTAMP        64
#define TO               65
#define USER_AGENT       66
#define VIA              67
#define MIME_VERSION     68

//100~699 为状态码
#define TRYING                  100
#define RINGING                 180
#define CALL_IS_BEING_FORWARDED 181
#define QUEUED                  182
#define SESSION_PROGRESS        183
#define OK                      200
#define MULTIPLE_CHOICES        300
#define MOVED_PERMANENTLY       301
#define MOVED_TEMPORARILY       302
#define USE_PROXY               305
#define ALTERNATIVE_SERVICE     380
#define BAD_REQUEST             400
#define UNAUTHORIZED            401
#define PAYMENT_REQUIRED        402
#define FORBIDDEN               403
#define NOT_FOUND               404
#define METHOD_NOT_ALLOWED      405
#define NOT_ACCEPTABLE_INVITE   406
#define PROXY								    407	//Status-Line: SIP/2.0 407 Proxy Authentication Required
#define REQUEST_TIMEOUT         408
#define GONE                    410
#define REQUEST_ENTITY_TOO_LARGE   411
#define REQUEST_URI_TOO_LONG       412
#define UNSUPPORTED_MEDIA_TYPE     415
#define UNSUPPORTD_URI_SCHEME      416
#define BAD_EXTENSION              420
#define EXTENSION_REQUIRED         421
#define INTERVAL_TOO_BRIEF         423
#define TEMPORARILY_UNAVAILABLE    480
#define CALL_TRANSACTION_DOES_NOT_EXIST   481
#define LOOP_DETECTED              482
#define TOO_MANY_HOPS              483
#define ADDRESS_INCOMPLETE         484
#define AMBIGUOUS                  485
#define BUSY_HERE                  486
#define REQUEST_TERMINATED         487
#define NOT_ACCEPTABLE_HERE        488
#define REQUEST_PENDING            491
#define UNDECIPHERABLE             493
#define SERVER_INTERNAL_ERROR      500
#define NOT_IMPLEMENTED            501
#define BAD_GATEWAY                502
#define SERVICE_UNAVAILABLE        503
#define SERVICE_TIME_OUT           504
#define VERSION_NOT_SUPPORTED      505
#define MESSAGE_TOO_LARGE          513
#define BUSY_EVERYWHERE            600
#define DECLINE                    603
#define DOES_NOT_EXIST_ANYWHERE    604
#define NOT_ACCEPTABLE_GLOBAL      606           

//SIP 请求首部
#define HIDE                700
#define IN_REPLY_TO         701
#define MAX_FORWARDS        702
#define PRIORITY            703
#define PROXY_AUTHORIZATION 104
#define PROXY_REQUIRE       705
#define ROUTE               706
#define RESPONSE_KEY        707
#define SUBJECT             708

//SIP 回应首部
#define PROXY_AUTHENTICATE 750
#define RETRY_AFTER        751
#define SERVER             752
#define UNSUPPORTED        753
#define WARNING            754
#define WWW_AUTHENTICATE   755

//SIP 实体首部
#define ALLOW                 800
#define CONTENT_DISPOSITION   801
#define CONTENT_ENCODING      802
#define CONTENT_LANGUAGE      803
#define CONTENT_LENGTH        804
#define CONTENT_TYPE          805
#define EXPIRES               806

