#include "stdio.h"
#include "string.h"
//#include "Misc.h"
#include "SipConstant.h"
#include "SipFunction.h"
/*********************************************
;v=0
;o=CallingNumber 0246810 1357911 IN IP4 SourceIP
;s=A call
;c=IN IP4 SourceIP
;t=0 0
;m=audio 6000 RTP/AVP 8
;a=rtpmap:8 PCMA/8000
;a=sendrecv
;*********************************************
;*********************
;1.R0 = SIP_NET_DATA_DPL
;2.R1: IP Address
;3.R2: IP length
;4.SDP field=>PUBLIC_XRAM_DPTR
;注：要先在PUBLIC XRAM中构建SDP，这样才可以得到Content-Length: XXX < 256 bytes
;    然后再将PUBLIC XRAM=>NET DATA BUFFER
;*********************/

int SDP_building(char *SendMsg,struct stUaInfo *UAinfo,int j)
{
	j+=sprintf(SendMsg+j,"v=0\r\n");
	j+=sprintf(SendMsg+j,"o=%s 0246810 1357911 IN IP4 %s\r\n",UAinfo->calling_number,UAinfo->src_ip);
  j+=sprintf(SendMsg+j,"s=A call\r\n");
	j+=sprintf(SendMsg+j,"c=IN IP4 %s\r\n",UAinfo->src_ip);
	j+=sprintf(SendMsg+j,"t=0 0\r\n"
											 "m=audio 6000 RTP/AVP 8\r\n"
											 "a=rtpmap:8 PCMA/8000\r\n"
											 "a=sendrecv\r\n");
	return j;
}

