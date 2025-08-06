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


// �ⲿ�������� (��SipMsgBuilding.c�ж���)
void BUILD_port_verify(u16 port);

// ���atoi�����ã��ṩһ���򵥵�ʵ��
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

// ��������Ӧ��������
void handle_server_response(void)
{
    // �յ���������Ӧ������ش���־
    if(gpsUaInfo->waiting_response)
    {
        gpsUaInfo->waiting_response = 0;
        gpsUaInfo->retry_count = 0;
        gpsUaInfo->retry_timer = 0;
    }
}

// ��̬�˿���Ӧ��������
void parse_port_response(char* response_data, int data_len)
{
    char *port_start, *port_end;
    char port_str[8];
    int port_len;
    unsigned int temp_port;  // �Ƶ�������ͷ����
    
    // ���� "port":"xxxx" ��ʽ
    port_start = strstr(response_data, "\"port\":\"");
    if(port_start != NULL)
    {
        port_start += 8;  // ���� "port":"
        port_end = strchr(port_start, '"');
        if(port_end != NULL)
        {
            port_len = port_end - port_start;
            if(port_len < 8)
            {
                strncpy(port_str, port_start, port_len);
                port_str[port_len] = '\0';
                
                // ת���˿ںŲ�����
                String2Int(&temp_port, port_str);
                gpsUaInfo->dynamic_port = (u16)temp_port;
                gpsUaInfo->port_negotiated = 1;
                gpsUaInfo->port_request_timer = 0;
                
                printf("Received dynamic port: %d\r\n", gpsUaInfo->dynamic_port);
                
                // ��֤�˿ڿ�����
                BUILD_port_verify(gpsUaInfo->dynamic_port);
            }
        }
    }
}

// �˿���֤��Ӧ��������
void parse_port_verify_response(char* response_data, int data_len)
{
    // ������֤�ɹ���־
    if(strstr(response_data, "\"status\":\"success\"") != NULL)
    {
        gpsUaInfo->port_verified = 1;
        printf("Port verification successful\r\n");
    }
    else
    {
        printf("Port verification failed\r\n");
        // ��������˿�
        gpsUaInfo->port_negotiated = 0;
        gpsUaInfo->port_verified = 0;
    }
}

// �ļ�������Ӧ��������
void parse_file_transfer_response(char* response_data, int data_len)
{
    if(strstr(response_data, "\"type\":\"file_ack\"") != NULL)
    {
        printf("File transfer acknowledged\r\n");
    }
}


/*
1. Request-Line: INVITE sip:3051@172.16.16.60:5060 SIP/2.0
2. Status-Line: SIP/2.0 200 OK
3. Status-Line: SIP/2.0 183 Session Progress
*/
//�ж����������״̬����
int ANALYSIS_line(char *p_line)
{
	unsigned int i,result;
	int j=0;	//�ֶγ���
	int k=0;	//xxx-Line ' '�ո���
	char ibuffer[20]; // SIP���ݻ��� 
	char *lp_line=p_line;	
	for(i=0;i<20;i++)
	{
//1. ��һ���ֽڳ�ʼ��ibuffer		
		if(j==0) ibuffer[0]='\0';
//2. ��һ��' ' ���ж� Request-Line: xxxx��Status-Line: SIP/2.0 			
    if(*(p_line+j)==' '&&k==0)			
		{
			p_line=p_line+j+1;
			ibuffer[j]='\0';
		//2.1. ��"SIP/2.0"���ҵڶ���' '
			if(!strcmp(ibuffer,"SIP/2.0")){
				k++;	//�ڶ���' '
				j=0;	//�ڶ���' '��ַ��0
				continue;
			}
		//2.1. ��Request-Line,��ֱ���˳�for
			else
				break;
		}
//3. �ڶ���' ' 		
    if(*(p_line+j)==' '&&k==1)	//Status-Line �ڶ���' ',��180/183 200
		{
      ibuffer[j]='\0';
      break;
		}		
    ibuffer[j]= *(lp_line+i);
		j++;
		if(j==20)
			j=0;
	}
//ȡ��Request-Line��Status-Line����
	if(i!=20)
	{
		result=ANALYSIS_get_lname(ibuffer);
	}	
	else	result=0;
	return result;
}

/*
 ȡ��Request-Line��Status-Line����
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
 ���ݽ��յ�line��,�ж�Ҫ������Щmessage header
��ڲ��� 
	line_name: ��һ������
	p_msg: ��һ�е�һ�ֽڵ�ַ
	msg_len: ����sip������
*/
void ANALYSIS_line_to_msg(int line_name,char *p_msg,int msg_len)
{
	int len;
	switch(line_name)
	{
		case 0:
			break;
//1. �����յ�INVITE
		case INVITE:
//			ANALYSIS_RxParamInit();			
			gsSipRxFlag.invite =1;
			gSipMechanism = RX_INVITE;
			gSipTxFlow =TX_INVITE_100_TRYING;
			len =ANALYSIS_message(p_msg, msg_len);
			ANALYSIS_cseq(gsMsg.cseq,0);	//ȡ��CSeq��xxx
			if(msg_len-len>20){	//˵����sdp����
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
				if(msg_len-len>20){	//˵����sdp����
					ANALYSIS_sdp(p_msg+len, msg_len-len);
				}
			}
			break;
		case REQUEST_TERMINATED:		//487	
			gSipMechanism =RX_487;			
			gSipTxFlow =TX_ACK;	
			len =ANALYSIS_message(p_msg, msg_len);
			ANALYSIS_cseq(gsMsg.cseq,0);	//ȡ��CSeq��xxx
		case PROXY:
			gsSipRxFlag.proxy =1;
			break;	
			
			
		default:
			break;
	}
}

/*
 200 OK�������¼������
*/
void ANALYSIS_cseq_type(char *p_msg_name)
{
	if(!strcmp(p_msg_name,"INVITE")){		
		gSipMechanism = RX_INVITE_200_OK;
		gSipTxFlow =TX_ACK;
		gsSipRxFlag.ok_inv =1;
		gsSipRxFlag.rec =1;
		//���ӷ�������Ӧ����
		handle_server_response();
	}
  	if(!strcmp(p_msg_name,"BYE")){
  		gSipMechanism = RX_BYE_200_OK;
  		gsSipRxFlag.ok_bye =1;
		// ���ӷ�������Ӧ����
    	handle_server_response();
	}
	if(!strcmp(p_msg_name,"CANCEL")){
		gSipMechanism = RX_CANCEL_200_OK;
		gsSipRxFlag.ok_cel =1;
		// ���ӷ�������Ӧ����
    	handle_server_response();
	}
	if(!strcmp(p_msg_name,"REGISTER")){
		gSipMechanism = RX_BYE_200_OK;
		gsSipRxFlag.ok_reg =1;
		// ���ӷ�������Ӧ����
    	handle_server_response();
	}
	// ������̬�˿���Ӧ��Ϣ
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
1. ��������Message Header
2. ����"\r\n"Ϊ�ֽ�������ȡ��ÿ��ibuffer[]
3. �ٷ���ÿ�ֶ�
4. ���ػ�ʣ����
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
	int j=0;	//ÿ�г��ȣ���������'\r\n'���ֽ�
	int k=0;	//�ڼ���'\r\n'
	char ibuffer[300]; //ÿ��message header���ݻ��� 
	char *lp_msg=p_msg;	//��һ��line�׵�ַ	
	gsMsg.via_num =0;
	gsMsg.re_route_num =0;
	for(i=0;i<msg_len;i++)
	{
		if(j==0)
			ibuffer[0]='\0';
		if((*(p_msg+j)=='\r')&&(*(p_msg+j+1)=='\n'))
		{
			p_msg=p_msg+j+2;	//��һ��line�׵�ַ
	    if(j==0)					//����'\r\n\r\n': ��ʾMessage Header����
				break;					
			ibuffer[j]='\0';
//			printf("%s",ibuffer);
			if(k!=0)		//k=0��Request-Line��Status-Line�ѷ���
			   ANALYSIS_msghdr(ibuffer,j);
			/////////////////////////////
			k++;
			i++;
			j=0;
		  continue;
		}
    ibuffer[j]= *(lp_msg+i); // ��ÿline���ݰᵽibuffer[j]��
		j++;
		if(j==300)
			j=0;
	}
  ibuffer[0]='\0';
	return i;			//���س���
}


/*
1. ����ÿ���ֶ�
    Via: SIP/2.0/UDP 172.16.66.80:5060;branch=z9hG4bK3606831015
    From: <sip:7000@172.16.1.135>;tag=1302917275
    To: <sip:5000@172.16.1.135:5060>
    Call-ID: 1578683025@172.16.66.80
    ...
    
2. ��¼Via��From��To��Call-ID��CSeq��Record-Route����
*/
void ANALYSIS_msghdr(char *p_msg, int len)
{
	int i=0;
	char ibuffer[20]; //message header���ƻ���	
	while(i<len)
	{
		if((*(p_msg+i)==':')&&(*(p_msg+i+1)==' ')){
	// ÿһ����Ϣͷ����ѭ���¸�ʽ��������ͷ�ֶ�������Via��From�ȣ���
	// ������ð�ţ�Ȼ�����Ϊһ������ǰ���ո񣬺���Ϊ�ֶ�ֵ:
	// header-header = field-name: SP [field-value] CRLF
			ibuffer[i]='\0'; 				//�ַ���������
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
			break;	//����ÿ���ֶξͽ���
		}
   	ibuffer[i]=*(p_msg+i);	// ���ֶ���copy��ibuffer[i]
		i++;
	}
	ibuffer[0]='\0';
	return;
}

/*
 ȡ��Message Header����
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
	int k=0;	//�ֶα�־
	char ibuffer[20]; //message header���ƻ���
	for(i=0;i<sdp_len;i++)
	{
		if(*(p_sdp+i)=='\n'){
			j=0;			//ÿ���ֶ�����ʼ
			k=0;			//ÿ��' '��0��ʼ����
			continue;	//����'\n'��������д��ibuffer[j]
		}	
//1. ������������
    if(*(p_sdp+i)==' '&&k==0){	//��һ��' '
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
			j=0;			//ÿ���ֶ�����ʼ
			continue;	//����' '
		}
//2. c=IN IP4 172.16.66.185		
		if(*(p_sdp+i)==' '&&k==1){	//c=IN IP4 
		  k=2;
    	j=0;			//ÿ���ֶ�����ʼ
			continue;	//����' '
		}
    if(*(p_sdp+i)=='\r'&&k==2){	//c=IN IP4 172.16.66.185
		  ibuffer[j] ='\0';
		  if(j<16){
		  	strcpy(gpsUaInfo->rtp_ip,ibuffer);
		  //	printf("ip=%s\r\n",gpsUaInfo->rtp_ip);
		  }
		  k=0;
    	j=0;			//ÿ���ֶ�����ʼ
			continue;	//����' '
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
		ibuffer[j]=*(p_sdp+i);	// ���ֶ���copy��ibuffer[j]		
		j++;
		if(j==20)
			j=0;
	}
}


/*
 1. �Ƚ�����message header
 2. �ҳ�CSeq: 
 3. ����CSeq: ��ַ
*/

void ANALYSIS_get_cseq(char *p_msg,int msg_len)
{
	int i=0;
	int j=0;
	char ibuffer[20]; //message header���ƻ���
	for(i=0;i<msg_len;i++)
	{
		if(*(p_msg+i)=='\n'){
			j=0;	//ÿ���ֶ�����ʼ
			continue;
		}
		
		if((*(p_msg+i)==':')&&(*(p_msg+i+1)==' ')){
	// ÿһ����Ϣͷ����ѭ���¸�ʽ��������ͷ�ֶ�������Via��From�ȣ���
	// ������ð�ţ�Ȼ�����Ϊһ������ǰ���ո񣬺���Ϊ�ֶ�ֵ:
	// header-header = field-name: SP [field-value] CRLF
			ibuffer[j]='\0'; 				//�ַ���������
			if(!strcmp(ibuffer,"CSeq")){
				ANALYSIS_cseq(p_msg+i+1,1);	//��' '��ʼ
				break;
			}	
		}			
		ibuffer[j]=*(p_msg+i);	// ���ֶ���copy��ibuffer[j]		
		j++;
		if(j==20)
			j=0;
	}
}

/*
	CSeq: 1 INVITE

	1. num_name=0: ֻ��cseq num 
*/
void ANALYSIS_cseq(char *p_cseq,char num_name)
{
	unsigned int i;
	int k=0;	//xxx-Line ' '�ո���
	int j=0;	//�ֶγ���
	char ibuffer[20]; // cseq��������
	for(i=0;i<30;i++)	// �ٶ�CSeq:�в����ܴ���30
	{
//1. ��һ��' '����CSeq: 		
    if(*(p_cseq+i)==' '&&k==0)			
		{
			k++;	//�ڶ���' '
			j =0;
			continue;
		}
//2. �ڶ���' ' Ϊxxx 	
    if(*(p_cseq+i)==' '&&k==1)
		{
			ibuffer[j] ='\0';	
			if(j<6){
				String2Int(&gsMsg.cseq_num,ibuffer);	
			}
			j=0;
      continue;
		}		
//3. �н���	
    if(*(p_cseq+i)=='\r')
		{
			ibuffer[j] ='\0';
			if(num_name==1){	//=1: ��200 OK
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
 ��ʼ�� sip rx/tx flag
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
