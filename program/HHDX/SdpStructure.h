#define SDPSTACKMAX		200

//typedef struct sdp_version_l sdp_version_l;

struct sdp_version_l 
{
	int    value;
	//char value[5];
};

//会话源或会话生成者，以及会话标示符(o)
//typedef struct sdp_origin_l sdp_origin_l;

struct sdp_origin_l 
{
	char calling_number[10];
	char session_id[5];
	char version[5];
	char net_type[5];
	char addr_type[4];
	char addr[20];
};

//会话名称(s)
//typedef struct sdp_session_l sdp_session_l;

struct sdp_session_l 
{
	char name[20];
};

//连接信息(c)
//typedef struct sdp_connect_l sdp_connect_l;

struct sdp_connect_l 
{
	char net_type[5];
	char addr_type[5];
	char addr[20];
};

//带宽信息(b)
//typedef struct sdp_bandwidth_l sdp_bandwidth_l;

struct sdp_bandwidth_l 
{
	char modifier[5];
	int  value;
	//char    value[6];
};

//时间描述(t)
//typedef struct sdp_times_l sdp_times_l;

struct sdp_times_l 
{
	int start;
	int stop;
	//char start[5];
	//char stop[5];
};
//加密密钥(k)
//typedef struct sdp_key_l sdp_key_l;

struct sdp_key_l 
{
	char modifier[10];
	//Length value;
	char value[60];
};

//属性(a)
//typedef struct sdp_attributes_l sdp_attributes_l;

struct sdp_attributes_l 
{
	char attribute[20];
	char fmt[5];
	//int fmt;
	char value[25];
};

//媒体描述(m)
//typedef struct sdp_media_l sdp_media_l;

struct sdp_media_l 
{
	char media[15];
	//char port[8];
	int  port;
	char transport[10];
	char fmtlist[10][10];
	int  fmtnumber;
	//int fmtlist[10];
    struct sdp_attributes_l attributes[10];
};

//typedef struct sdp_packet sdp_packet;

struct sdp_packet
{
	char * sdp_stack[SDPSTACKMAX];
    int medianumber;
	
	struct sdp_version_l version;
    struct sdp_origin_l origin;
    struct sdp_session_l session;
    struct sdp_connect_l connect;
    struct sdp_bandwidth_l bandwidth;
    struct sdp_times_l times;
    struct sdp_media_l media[5];
    struct sdp_key_l key[5];
};

