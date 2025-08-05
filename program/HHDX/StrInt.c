#include <math.h>
#include <string.h>
#include "Usart.h"
//#include "Misc.h"
#include "malloc.h"

/*
 判断16位数据中第几个bit =0，如果有多个为0，则返回0，否则返回0的位数
 1. val: 要判断的数据
 2. bit_num: 总共判断多少位
 3. 如果有
*/
unsigned char MISC_uint16_bit_0(unsigned int val, unsigned char bit_num)
{
	unsigned char i,num,times;
	unsigned int val_temp;	
	times =0;
	val_temp = val;
	for(i =0; i < bit_num; i++){
		val_temp = val >> i;
		if((val_temp & 0x0001) == 0){
			num = i+1;	//bit_i =0
			times ++;
		}
	}
	if(times ==1){
		return num;
	}
	else
		return 0;
}


//动态分配内存并复制
char *MymallocCopy(char *p_dst,int len,char *p_src)
{
	if(p_dst != NULL){
		myfree(SRAMIN,p_dst);
	}
	p_dst=mymalloc(SRAMIN,len+3);	//申请内存
	strcpy(p_dst,p_src);
	return p_dst;
}
/*******************************************************************************
* Function Name	: 
* Description	: 将Int转成String
* Input			: -string : 转换后的字符串
*				  -value : 要被转换的整形数据
* Return		: None
*******************************************************************************/
void Int2String(char *string,unsigned short value)
{
	unsigned short i;
	unsigned short temp[10]; //unsigned short max = 65536
	unsigned short k;
	for(i=0;i<10;i++)
	{ // value =unsigned short 12345 => temp[i] = unsigned short 5,4,3,2,1
		temp[i]=value%10; // 取余运算
		value=value/10;
		if(value==0)
			break;
	}
	k=i+1; // i = unsigned short 位数
	for(i=0;i<k;i++)
		// temp[i] = unsigned short 5,4,3,2,1 => char *string = 12345
		*(string+i)=(char)(48+temp[k-i-1]);
	*(string+k)='\0';
	return;
}

/*******************************************************************************
* Function Name	: 
* Description	: 将String转成Int
* Input			: -string : 要被转换的字符串
*				  -value : 转换后的整形数据
* Return		: None
*******************************************************************************/
void String2Int(unsigned int *value,char *string)
{
	unsigned short i;
	unsigned short k;
	k=strlen(string); // 得字符串string长度
	*value=0;
	for(i=0;i<k;i++)
	// int = *string + (*(string +1)) x 10^(k-i-1)
		*value=*value+(unsigned int)(*(string+i)-48)*(unsigned int)pow(10,k-i-1);
    return;
}

//-----------------------------------------------------------------------------
//【  版          本  】v1.0
//【  函  数  名  称  】
//【 创建人及创建时间 】snowwind 2010-01-08
//【 修改人及修改时间 】
//【  修  改  原  因  】
//【  功  能  描  述  】HEX(char,shor,int) => ASCII
//						如:  0x3B = 33,42
//-----------------------------------------------------------------------------
void Hex2Asc(void *hex,unsigned char *asc,unsigned char len)
{	
	unsigned char hex_to_asc[]="0123456789ABCDEF",i;
	unsigned int *hex_int;
	unsigned short *hex_short;
	unsigned char *hex_char;
	
	if(len==(sizeof(unsigned int)<<1))
		hex_int=hex;
	if(len==(sizeof(unsigned short)<<1))
		hex_short=hex;
	if(len==(sizeof(char)<<1))
		hex_char=hex;
			
	for(i=0;i<len;i++)
	{
		if(len==(sizeof(unsigned int)<<1))
			*(asc+i)=hex_to_asc[(((*hex_int)>>((len-1-i)<<2))&0x0f)];
		if(len==(sizeof(unsigned short)<<1))
			*(asc+i)=hex_to_asc[(((*hex_short)>>((len-1-i)<<2))&0x0f)];
		if(len==(sizeof(unsigned char)<<1))
			*(asc+i)=hex_to_asc[(((*hex_char)>>((len-1-i)<<2))&0x0f)];		
	}
	*(asc+len)='\0';
}

//-----------------------------------------------------------------------------
//【  版          本  】v1.0
//【  函  数  名  称  】
//【 创建人及创建时间 】snowwind 2010-01-07
//【 修改人及修改时间 】
//【  修  改  原  因  】
//【  功  能  描  述  】unsigned long => char *string
//						如:  ip = 172.16.1.133
//						ip[0]~[3] => 31 37 32 . 31 36 . 31 . 31 33 33
//-----------------------------------------------------------------------------
void IpInt2Str(char *string,u8 *ip_addr)
{
	char buff[10];
	Int2String(buff,*(ip_addr+0));
	strcpy(string,buff);
	strcat(string,".");	
	Int2String(buff,*(ip_addr+1));
	strcat(string,buff);
	strcat(string,".");
	Int2String(buff,*(ip_addr+2));
	strcat(string,buff);
	strcat(string,".");
	Int2String(buff,*(ip_addr+3));
	strcat(string,buff);
}


//-----------------------------------------------------------------------------
//【  版          本  】v1.0
//【  函  数  名  称  】
//【 创建人及创建时间 】snowwind 2023-06-24
//【 修改人及修改时间 】
//【  修  改  原  因  】
//【  功  能  描  述  】 char *string => ip_array[0]~[3]
//						如:  172.16.1.133 => [0xAC],[0x10],[0x01],[0x85]
//-----------------------------------------------------------------------------
void Str2IpArray(u8 *ip_array,char *string)
{
	u8 len,val,i=3,j=0;
	len=(u8)strlen(string);
	while(len--)
	{
		if((*(string+len)) =='.'){	//碰到'.'则记录一个数
			ip_array[i] =val;
//			printf("ip[%d]=%d ",i,ip_array[i]);
			i--;
			j=0;
		}
		else{
			 if(j==0){
			 	val = ((*(string+len))&0x0f);		//个位
			 	j++;
			}
			else if(j ==1){
				val = ((*(string+len))&0x0f)*10 +val;		//十位
			 	j++;
			}
			else if(j ==2){
				val = ((*(string+len))&0x0f)*100 +val;	//百位
			 	j++;
			}
			else{
				break;
			}
		}				
		if((i==0)&&(len==0)){		//最后数
			ip_array[i] =val;
//			printf("ip[%d]=%d\r\n",i,ip_array[i]);
		}			
	}
}


///*******************************************************************************
//* Function Name	: String2Addr
//* Description	: 将String转成address
//* Input			: -clientaddr : 转换后的结果
//*				  -dst_ip : 要转换的IP
//*				  -dst_port : 要转换的Port
//* Return		: Note
//* 如：31 37 32 . 31 36 . 31 . 31 33 33 => 172.16.1.133
//*******************************************************************************/
//void String2Addr(char *dst_ip)
//{
//    u8 i,j=0,len,k=0;
//    u16 m;
//    char *temp;
//    char *ldst_ip=dst_ip;
//    len=(u8)strlen(dst_ip);
//	for(i=0;i<len;i++)
//	{
//		if(j==0)
//			ibuffer[0]='\0';
//		if(*(dst_ip+j)=='.')
//		{   
//            k++;
//			dst_ip=dst_ip+j+1;
//			ibuffer[j]='\0';
//            temp=ibuffer;
//			String2Int(m,temp);
//            if(k==1){
//            	uip_udp_conn->ripaddr[0] = m;		// 16
//            }
//            if(k==2){
//            	uip_udp_conn->ripaddr[0] |= (m<<8);	// 172
//            }
//            if(k==3){
//            	uip_udp_conn->ripaddr[1] = m;		// 81
//            	String2Int(m,dst_ip);
//            	uip_udp_conn->ripaddr[1] |= (m<<8);	// 66
//            }
//			j=0;
//		    continue;
//		}
//        ibuffer[j]= *(ldst_ip+i);
//		j++;
//	}
//    ibuffer[0]='\0';
//    return;
//} 
