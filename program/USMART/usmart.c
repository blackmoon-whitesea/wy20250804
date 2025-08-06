#include "usmart.h"
#include "usart.h"
#include "sys.h" 
#include "stdio.h"
#include "string.h"
//#include "../USER/usmart_network.h"  // Include network detection functions
#include <24cxx.h>
#include <HhdxMsg.h>

//////////////////////////////////////////////////////////////////////////////////	 
//This program is only for learning use, without author's permission, 
//it cannot be used for any other purpose
//ALIENTEK STM32 Development Board	   
//ALIENTEK@ALIENTEK
//Technical Forum:www.openedv.com 
//Modified Date:V3.1
//All rights reserved, piracy will be prosecuted.
//Copyright(C) ALIENTEK 2011-2021
//All rights reserved
//********************************************************************************
//Version History:
//V1.4 - Added string parameter support with enhanced functionality
//V2.0 - Added list and id commands, improved parameter matching
//V2.1 - Added dec/hex conversion commands  
//V2.2 - Added void function support, default hex display
//V2.3 - Fixed function parsing bugs
//V2.4 - Improved command name parsing and TIM2 scan support
//V2.5 - Modified usmart_init() with system clock parameter
//V2.6 - Added read_addr/write_addr functions for memory access
//V2.7 - Fixed return value display bugs
//V2.8 - Fixed list command display issues
//V2.9 - Fixed void*xxx(void) function recognition
//V3.0 - Added enhanced string and conversion support
//V3.1 - Added runtime measurement system for function execution timing
//Usage: runtime 1 (enable timing), runtime 0 (disable timing)
//Note: Runtime statistics require USMART_ENTIMX_SCAN = 1
/////////////////////////////////////////////////////////////////////////////////////

// Case-insensitive string comparison function
// Return value: 0 means equal, non-0 means different
static u8 usmart_strcasecmp_impl(u8 *str1, u8 *str2)
{
	u8 c1, c2;
	
	while(*str1 && *str2)
	{
		c1 = *str1;
		c2 = *str2;
		
		// Convert to lowercase for comparison
		if(c1 >= 'A' && c1 <= 'Z') c1 += 32;
		if(c2 >= 'A' && c2 <= 'Z') c2 += 32;
		
		if(c1 != c2) return 1;
		
		str1++;
		str2++;
	}
	
	// Check if the end characters are also equal
	c1 = *str1;
	c2 = *str2;
	if(c1 >= 'A' && c1 <= 'Z') c1 += 32;
	if(c2 >= 'A' && c2 <= 'Z') c2 += 32;
	
	return (c1 == c2) ? 0 : 1;
}

u8 usmart_strcasecmp(u8 *str1, u8 *str2)
{
	return usmart_strcasecmp_impl(str1, str2);
}

//System command table
u8 *sys_cmd_tab[]=
{
	"?",
	"help",
	"list",
	"id",
	"hex",
	"dec",
	"runtime",	  
	"getip",//local:192.168.0.30 netmask:255.255.255.0 gw:192.168.0.1  server:192.168.0.10 port:5000
	"setip",//192.168.0.30 2555.255.255.0 192.168.0.1
	"setsvip",//192.168.0.10 5000
};	    

//Execute system commands
//Return: 0,success; other,error code;
u8 usmart_sys_cmd_exe(u8 *str)
{
	u8 i;
	u8 sfname[MAX_FNAME_LEN];//Temporary function name buffer
	u8 pnum;
	u8 rval;
	u32 res;
	int j;        // 添加变量声明用于for循环
	char *p;      // 添加指针变量声明  
	res=usmart_get_cmdname(str,sfname,&i,MAX_FNAME_LEN);//Get command name and length
	if(res)return USMART_FUNCERR;//Error getting command 
	str+=i;	 	 			    
	for(i=0;i<sizeof(sys_cmd_tab)/4;i++)//Support system commands
	{
		if(usmart_strcasecmp(sfname,sys_cmd_tab[i])==0)break;
	}
	switch(i)
	{					   
		case 0:
		case 1://Help command
			printf("\r\n");
#if USMART_USE_HELP
			printf("------------------------USMART V3.1------------------------ \r\n");
			printf("    USMART is a smart serial debugging component developed by \r\n");
			printf("ALIENTEK. Through it, you can call any function in the program \r\n");
			printf("through serial assistant and execute it. Therefore, you can \r\n");
			printf("arbitrarily change the input parameters of the function \r\n");
			printf("(supporting numbers (10/16 base), strings, function entry \r\n");	  
			printf("addresses, etc. as parameters). A single function supports up \r\n");
			printf("to 10 input parameters and supports function return value \r\n");
			printf("display. New feature: perfect support for function name and \r\n");
			printf("parameter association. Convenient for user input.\r\n");
			printf("Technical support: www.openedv.com\r\n");
			printf("USMART has 9 system functions (case-insensitive input):\r\n");
			printf("?:      Get help information\r\n");
			printf("help:   Get help information\r\n");
			printf("list:   Function list\r\n\n");
			printf("id:     Function ID list\r\n\n");
			printf("hex:    16-bit hexadecimal display, followed by space+number\r\n");
			printf("        to perform base conversion\r\n\n");
			printf("dec:    10-bit decimal display, followed by space+number\r\n");
			printf("        to perform base conversion\r\n\n");
			printf("runtime:1,enable function execution time statistics;\r\n");
			printf("        0,disable function execution time statistics;\r\n\n");
			printf("getip:  Get local IP address\r\n");
			printf("        localip netmask gateway server\r\n");
			printf("setip:  Set local IP address\r\n");
			printf("        localip netmask gateway\r\n");
			printf("setsvip:Set server IP address and port\r\n");
			printf("Note: IP address must be valid, otherwise setting fails.\r\n");
			printf("System commands support case-insensitive input.\r\n");
			printf("USMART Usage:\r\n");
			printf("Please input function name and parameters in C function\r\n");
			printf("format, and end with Enter key.\r\n");    
			printf("--------------------------ALIENTEK------------------------- \r\n");
#else
			printf("Command disabled\r\n");	
#endif
			break;
		case 2://Query command
			printf("\r\n");
			printf("-------------------------Function List--------------------------- \r\n");
			for(i=0;i<usmart_dev.fnum;i++)printf("%s\r\n",usmart_dev.funs[i].name);
			printf("\r\n");
			break;	 
		case 3://Query ID
			printf("\r\n");
			printf("-------------------------Function ID --------------------------- \r\n");
			for(i=0;i<usmart_dev.fnum;i++)
			{
				usmart_get_fname((u8*)usmart_dev.funs[i].name,sfname,&pnum,&rval);//get function name and parameters 
				printf("%s id is:\r\n0X%08X\r\n",sfname,usmart_dev.funs[i].func); //show ID
			}
			printf("\r\n");
			break;
		case 4://hex command
			printf("\r\n");
			usmart_get_aparm(str,sfname,&i);
			if(i==0)//get parameters
			{
				i=usmart_str2num(sfname,&res);	   	//record parameters	
				if(i==0)						  	//conversion succeeds
				{
					printf("HEX:0X%X\r\n",res);	   	//convert to hexadecimal
				}else if(i!=4)return USMART_PARMERR;//parameter error.
				else 				   				//no parameter, set display parameter
				{
					printf("16-bit hex parameter display!\r\n");
					usmart_dev.sptype=SP_TYPE_HEX;  
				}

			}else return USMART_PARMERR;			//parameter error.
			printf("\r\n"); 
			break;
		case 5://dec command
			printf("\r\n");
			usmart_get_aparm(str,sfname,&i);
			if(i==0)//get parameters
			{
				i=usmart_str2num(sfname,&res);	   	//record parameters	
				if(i==0)						   	//conversion succeeds
				{
					printf("DEC:%lu\r\n",res);	   	//convert to decimal
				}else if(i!=4)return USMART_PARMERR;//parameter error.
				else 				   				//no parameter, set display parameter
				{
					printf("10-bit decimal parameter display!\r\n");
					usmart_dev.sptype=SP_TYPE_DEC;  
				}

			}else return USMART_PARMERR;			//parameter error. 
			printf("\r\n"); 
			break;	 
		case 6://runtime command, control whether to display function execution time
			printf("\r\n");
			res=usmart_get_aparm(str,sfname,&i);
			if(i==0)//get parameters
			{
				i=usmart_str2num(sfname,&res);	   		//record parameters	
				if(i==0)						   		//get command address data succeeded
				{
					if(USMART_ENTIMX_SCAN==0)printf("\r\nError! \r\nTo EN RunTime function,Please set USMART_ENTIMX_SCAN = 1 first!\r\n");//error
					else
					{
						usmart_dev.runtimeflag=res;
						if(usmart_dev.runtimeflag)printf("Run Time Calculation ON\r\n");
						else printf("Run Time Calculation OFF\r\n"); 
					}
				}else return USMART_PARMERR;   			//not set, or parameter error	 
 			}else return USMART_PARMERR;				//parameter error. 
			printf("\r\n"); 
			break;	    
		case 7://getip command, get IP address
			printf("\r\n");
			printf("Local IP Address: %d.%d.%d.%d\r\n",gpsEeprom->own_ip[0],gpsEeprom->own_ip[1],gpsEeprom->own_ip[2],gpsEeprom->own_ip[3]);
			printf("Netmask: %d.%d.%d.%d\r\n",gpsEeprom->mask_ip[0],gpsEeprom->mask_ip[1],gpsEeprom->mask_ip[2],gpsEeprom->mask_ip[3]);
			printf("Gateway: %d.%d.%d.%d\r\n",gpsEeprom->gw_ip[0],gpsEeprom->gw_ip[1],gpsEeprom->gw_ip[2],gpsEeprom->gw_ip[3]);
			printf("Server: %d.%d.%d.%d Port: %d\r\n",gpsEeprom->sv_ip[0],gpsEeprom->sv_ip[1],gpsEeprom->sv_ip[2],gpsEeprom->sv_ip
				[3],gpsEeprom->port);
			printf("\r\n");	
			break;
		case 8://setip command, set IP addresss
			printf("\r\n");
			//localip
			res=usmart_get_ipparm(str,sfname,&i);
			str+=res; //point to next parameter
			if(res==0)return USMART_FUNCERR;		//error getting parameters
			if(i==1)//get parameters
			{
				//处理sfname中ip地址
				for(j=0;j<4;j++){
					p=strchr(sfname,'.');
					if(p!=NULL)
					{
						*p='\0';//将分隔符替换为结束符
						p++;
						usmart_str2num(p,&res);
					}
					gpsEeprom->own_ip[j]=res;
					if(j==3) break;
					strcpy(sfname,p);
				}

			}else return USMART_PARMERR;			//parameter error.		

			//netmask
			res=usmart_get_ipparm(str,sfname,&i);
			str+=res; //point to next parameter
			if(res==0)return USMART_FUNCERR;
			if(i==1)//get parameters
			{
				//处理sfname中netmask地址
				for(j=0;j<4;j++){
					p=strchr(sfname,'.');
					if(p!=NULL)
					{
						*p='\0';//将分隔符替换为结束符
						p++;
						usmart_str2num(p,&res);
					}
					gpsEeprom->mask_ip[j]=res;
					if(j==3) break;
					strcpy(sfname,p);
				}
			}else return USMART_PARMERR;			//parameter error.

			//gw
			res=usmart_get_ipparm(str,sfname,&i);
			//if(res!=0)return USMART_FUNCERR;
			if(i==1)//get parameters
			{
				//处理sfname中ip地址
				for(j=0;j<4;j++){
					p=strchr(sfname,'.');
					if(p!=NULL)
					{
						*p='\0';//将分隔符替换为结束符
						p++;
						usmart_str2num(p,&res);
					}
					gpsEeprom->gw_ip[j]=res;
					if(j==3) break;
					strcpy(sfname,p);
				}
			}else return USMART_PARMERR;		 	//parameter error.

			//save to eeprom
			//AT24CXX_WriteLenByte(EEPROM_GPS_FLAG_ADDR,(u8*)&gpsEeprom->flag,sizeof(gpsEeprom->flag));
			gsHhdxFlag.eepromwr =1;

			// if(USMART_ENTIMX_SCAN==0)printf("\r\nError! \r\nTo EN Set IP function,Please set USMART_ENTIMX_SCAN = 1 first!\r\n");//error
			// else
			// {
			// 	if(usmart_set_ip(str)==0)	//set IP address
			// 	{
			// 		printf("Set IP Address Success!\r\n");	//set IP address success
			// 	}
			// 	else
			// 	{
			// 		printf("Set IP Address Error!\r\n");	//set IP address error
			// 	}
			// }
			printf("\r\n");
			break;
		case 9:
			printf("\r\n");
			//svip
			res=usmart_get_ipparm(str,sfname,&i);
			str+=res; //point to next parameter
			printf("ip%s %d %d\r\n",sfname,i,res);
			if(res==0)return USMART_FUNCERR;
			
			if(i==1)//get parameters
			{
				printf("ip%s\r\n",sfname);
				//处理sfname中svip地址
				for(j=0;j<4;j++){
					p=strchr(sfname,'.');
					if(p!=NULL)
					{
						*p='\0';//将分隔符替换为结束符
						p++;
						usmart_str2num(p,&res);
						printf("res=%d\r\n",res);
					}
					gpsEeprom->sv_ip[j]=res;
					if(j==3) break;
					strcpy(sfname,p);
				}
			}else return USMART_PARMERR;			//parameter error.
			//port
			res=usmart_get_aparm(str,sfname,&i);
			//if(res!=0)return USMART_FUNCERR;
			if(i==0)//get parameters
			{
				i=usmart_str2num(sfname,&res);	   		//record parameters
				if(i==0)						   		//get command address data succeeded
				{
					gpsEeprom->port=res;
				}else return USMART_PARMERR;   			//not set, or parameter error
 			}else return USMART_PARMERR;				//parameter error.
			//save to eeprom
			//AT24CXX_WriteLenByte(EEPROM_GPS_FLAG_ADDR,(u8*)&gpsEeprom->flag,sizeof(gpsEeprom->flag));
	        gsHhdxFlag.eepromwr =1;
			break;
		default://illegal command
			//return USMART_FUNCERR;
			return 0;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////
//Porting notes: This is tested on STM32, for other MCUs, corresponding changes needed.
//usmart_reset_runtime: Clear function timing, set timer counter and overflow flag.
//Timer reload value is maximum, to avoid overflow during measurement.
//usmart_get_runtime: Get function execution time by reading CNT value.
//Since usmart is called through interrupts, timing interrupts should be disabled
//during timing to get accurate results. Time unit: 2 timer cycles * 0.1ms.
//For STM32, max time: 13.1s
//Note: TIM4_IRQHandler and Timer4_Init need to be modified for different MCUs.
//Make sure timer frequency is 10Khz. Also, timing requires proper interrupt priority!

#if USMART_ENTIMX_SCAN==1
//Reset runtime timing
//This function needs to be modified for different MCU timer implementations
void usmart_reset_runtime(void)
{
	TIM4->SR&=~(1<<0);	//Clear interrupt flag 
	TIM4->ARR=0XFFFF;	//Set reload value to maximum
	TIM4->CNT=0;		//Reset counter CNT
	usmart_dev.runtime=0;	
}
//Get runtime timing
//Return: execution time, unit: 0.1ms, calculated as timer CNT value * 2 * 0.1ms
//This function needs to be modified for different MCU timer implementations
u32 usmart_get_runtime(void)
{
	if(TIM4->SR&0X0001)//Overflow occurred, add overflow time
	{
		usmart_dev.runtime+=0XFFFF;
	}
	usmart_dev.runtime+=TIM4->CNT;
	return usmart_dev.runtime;		//Return total timing value
}  
//Timer4 interrupt handler, used by USMART, do not modify, needs MCU porting. 
//Timer4 interrupt function	 
void TIM4_IRQHandler(void)
{ 		    		  			    
	if(TIM4->SR&0X0001)//Overflow interrupt
	{
		usmart_dev.scan();	//Execute usmart scan	
		TIM4->CNT=0;		//Reset counter CNT
		TIM4->ARR=1000;		//Restore original reload value		    				   				     	    	
	}				   
	TIM4->SR&=~(1<<0);//Clear interrupt flag 	    
}
//Enable Timer4, enable interrupts.
void Timer4_Init(u16 arr,u16 psc)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC->APB1ENR|=1<<2;   	//TIM4 clock enable      
 	TIM4->ARR=arr;  		//Set auto-reload value  
	TIM4->PSC=psc;  		//Prescaler 7200, get 10Khz timer clock 
	TIM4->DIER|=1<<0;   	//Enable update interrupt	 
	TIM4->CR1|=0x01;    	//Enable Timer4
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);		//Preemption priority 3, sub priority 3 (lower than group 2)							 
}
#endif
////////////////////////////////////////////////////////////////////////////////////////
//Initialize USMART
//sysclk: system clock in MHz
void usmart_init(u8 sysclk)
{
#if USMART_ENTIMX_SCAN==1
	Timer4_Init(1000,(u32)sysclk*100-1);//Timer freq 10K, 100ms interrupt, note: timer freq must be 10Khz for runtime unit (0.1ms) consistency.
#endif
	usmart_dev.sptype=1;	//Hexadecimal parameter display mode
	
	
	printf("USMART initialized with network detection.\r\n");
}		
//Get function name, id, and parameter info from str
//*str: string pointer.
//Return: 0,recognized successfully; other,error code.
u8 usmart_cmd_rec(u8*str) 
{
	u8 sta,i,rval;//status	 
	u8 rpnum,spnum;
	u8 rfname[MAX_FNAME_LEN];//Received buffer, used to store received function name  
	u8 sfname[MAX_FNAME_LEN];//Temporary function name buffer
	sta=usmart_get_fname(str,rfname,&rpnum,&rval);//Get function name, parameter count and return value from command string	  
	if(sta)return sta;//Error
	for(i=0;i<usmart_dev.fnum;i++)
	{
		sta=usmart_get_fname((u8*)usmart_dev.funs[i].name,sfname,&spnum,&rval);//Get function name and parameters from function table
		if(sta)return sta;//Error getting function info	  
		if(usmart_strcmp(sfname,rfname)==0)//Match found
		{
			if(spnum>rpnum)return USMART_PARMERR;//Parameter error (received parameters less than required)
			usmart_dev.id=i;//Record function ID.
			break;//Found match.
		}	
	}
	if(i==usmart_dev.fnum)return USMART_NOFUNCFIND;	//No matching function found
 	sta=usmart_get_fparam(str,&i);					//Get function parameters	
	if(sta)return sta;								//Parameter error
	usmart_dev.pnum=i;								//Record parameter count
    return USMART_OK;
}
//usmart execution function
//This function calls the function defined in the function table and executes it.
//Maximum 10 input parameters supported, parameter values are converted to the appropriate type.
//It will also print function execution info in the format: "function_name(param1,param2...paramN)=return_value".
//If the function has no return value, only the semicolon is printed.
void usmart_exe(void)
{
	u8 id,i;
	u32 res;		   
	u32 temp[MAX_PARM];//Temporary conversion to support string parameters 
	u8 sfname[MAX_FNAME_LEN];//Temporary function name buffer
	u8 pnum,rval;
	id=usmart_dev.id;
	if(id>=usmart_dev.fnum)return;//Invalid function ID, do not execute.
	usmart_get_fname((u8*)usmart_dev.funs[id].name,sfname,&pnum,&rval);//Get function name, parameter count and return flag 
	printf("\r\n%s(",sfname);//Print function name being executed
	for(i=0;i<pnum;i++)//Print parameters
	{
		if(usmart_dev.parmtype&(1<<i))//String parameter
		{
			printf("%c",'"');			 
			printf("%s",usmart_dev.parm+usmart_get_parmpos(i));
			printf("%c",'"');
			temp[i]=(u32)&(usmart_dev.parm[usmart_get_parmpos(i)]);
		}else						  //Numeric parameter
		{
			temp[i]=*(u32*)(usmart_dev.parm+usmart_get_parmpos(i));
			if(usmart_dev.sptype==SP_TYPE_DEC)printf("%lu",temp[i]);//Decimal display
			else printf("0X%X",temp[i]);//Hexadecimal display 	   
		}
		if(i!=pnum-1)printf(",");
	}
	printf(")");
	usmart_reset_runtime();	//Reset timing, start measurement
	switch(usmart_dev.pnum)
	{
		case 0://No parameters (void function)											  
			res=(*(u32(*)())usmart_dev.funs[id].func)();
			break;
	    case 1://1 parameter
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0]);
			break;
	    case 2://2 parameters
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1]);
			break;
	    case 3://3 parameters
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2]);
			break;
	    case 4://4 parameters
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3]);
			break;
	    case 5://5 parameters
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4]);
			break;
	    case 6://6 parameters
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5]);
			break;
	    case 7://7 parameters
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5],temp[6]);
			break;
	    case 8://8 parameters
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5],temp[6],temp[7]);
			break;
	    case 9://9 parameters
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5],temp[6],temp[7],temp[8]);
			break;
	    case 10://10 parameters
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5],temp[6],temp[7],temp[8],temp[9]);
			break;
	}
	usmart_get_runtime();//Get function execution time
	if(rval==1)//Need to display return value.
	{
		if(usmart_dev.sptype==SP_TYPE_DEC)printf("=%lu;\r\n",res);//Display return value in decimal format
		else printf("=0X%X;\r\n",res);//Display return value in hexadecimal format	   
	}else printf(";\r\n");		//No return value needed, directly print semicolon
	if(usmart_dev.runtimeflag)	//Need to display function execution time
	{ 
		printf("Function Run Time:%d.%1dms\r\n",usmart_dev.runtime/10,usmart_dev.runtime%10);//Print function execution time 
	}	
}
//usmart scan function
//Through this function, usmart functionality is realized. This function needs to be called
//periodically or when there's time to implement real-time functionality.
//This function is automatically called when using timer interrupts.
//Note: This function is provided by ALIENTEK, USART_RX_STA and USART_RX_BUF[] need to be implemented by the user
void usmart_scan(void)
{
	u8 sta,len;  
	if(USART_RX_STA&0x8000)//Data reception complete
	{					   
		len=USART_RX_STA&0x3fff;	//Get length of received data
		USART_RX_BUF[len]='\0';	//Add string terminator at the end. 
		sta=usmart_dev.cmd_rec(USART_RX_BUF);//Get command recognition info
		if(sta==0)usmart_dev.exe();	//Execute function 
		else 
		{  
			len=usmart_sys_cmd_exe(USART_RX_BUF);
			if(len!=USMART_FUNCERR)sta=len;
			if(sta)
			{
				switch(sta)
				{
					case USMART_FUNCERR:
						printf("Function Error!\r\n");   			
						break;	
					case USMART_PARMERR:
						printf("Parameter Error!\r\n");   			
						break;				
					case USMART_PARMOVER:
						printf("Too Many Parameters!\r\n");   			
						break;		
					case USMART_NOFUNCFIND:
						printf("Function Not Found!\r\n");   			
						break;		
				}
			}
		}
		USART_RX_STA=0;//Reset status flag	    
	}
}

#if USMART_USE_WRFUNS==1 	//If read/write functions are enabled
//Read value from specified address		 
u32 read_addr(u32 addr)
{
	return *(u32*)addr;//	
}
//Write specified value to specified address		 
void write_addr(u32 addr,u32 val)
{
	*(u32*)addr=val; 	
}
#endif



// // ARP detection function to check if IP is already in use
// // Returns: 0 = IP available, 1 = IP in use, 2 = detection failed
// u8 usmart_detect_ip_conflict(u32 target_ip)
// {
// 	extern u32 current_device_ip;
// 	extern u32 current_subnet_mask;  // Assume this is defined in SipMsgAnalysis.c
	
// 	u8 detection_result;
	
// 	printf("Starting IP conflict detection...\r\n");
	
// 	// Use the enhanced detection function
// 	detection_result = usmart_detect_ip_conflict_enhanced(target_ip);
	
// 	switch(detection_result)
// 	{
// 		case ARP_STATUS_AVAILABLE:
// 			printf("IP conflict detection: IP available\r\n");
// 			return 0; // Available
			
// 		case ARP_STATUS_IN_USE:
// 			printf("IP conflict detection: IP already in use\r\n");
// 			return 1; // In use
			
// 		case ARP_STATUS_TIMEOUT:
// 			printf("IP conflict detection: Timeout (assuming available)\r\n");
// 			return 0; // Assume available on timeout
			
// 		case ARP_STATUS_ERROR:
// 		default:
// 			printf("IP conflict detection: Error occurred\r\n");
// 			return 2; // Error
// 	}
// }

// u8 usmart_set_ip(u8 *ip_cmd)
// {
// 	extern u32 current_device_ip;  // Defined in SipMsgAnalysis.c for current device IP
// 	u8 ip_parts[4];
// 	int part_count = 0;
// 	int current_part = 0;
// 	int i;
// 	u32 new_ip;
// 	u8 conflict_result;
// 	static u8 setting_in_progress = 0; /* Prevent re-entrance */
	
// 	/* Prevent re-entrance during setting process */
// 	if(setting_in_progress)
// 	{
// 		printf("IP setting already in progress, please wait...\r\n");
// 		return 1;
// 	}
	
// 	setting_in_progress = 1; /* Set flag */
	
// 	if(ip_cmd == 0) 
// 	{
// 		setting_in_progress = 0;
// 		return 1; // Parameter error
// 	}
	
// 	// Skip leading spaces
// 	while(*ip_cmd == ' ') ip_cmd++;
	
// 	// Parse IP address format (example: "192.168.1.100")
// 	for(i = 0; ip_cmd[i] != '\0' && part_count < 4; i++)
// 	{
// 		if(ip_cmd[i] >= '0' && ip_cmd[i] <= '9')
// 		{
// 			current_part = current_part * 10 + (ip_cmd[i] - '0');
// 			if(current_part > 255) 
// 			{
// 				printf("Invalid IP: Part %d out of range (0-255)\r\n", current_part);
// 				setting_in_progress = 0;
// 				return 1; // IP part value out of range
// 			}
// 		}
// 		else if(ip_cmd[i] == '.')
// 		{
// 			ip_parts[part_count++] = current_part;
// 			current_part = 0;
// 		}
// 		else
// 		{
// 			printf("Invalid character in IP address: '%c'\r\n", ip_cmd[i]);
// 			setting_in_progress = 0;
// 			return 1; // Invalid character
// 		}
// 	}
	
// 	// Check last part
// 	if(current_part > 255) 
// 	{
// 		printf("Invalid IP: Last part %d out of range (0-255)\r\n", current_part);
// 		setting_in_progress = 0;
// 		return 1;
// 	}
// 	ip_parts[part_count++] = current_part;
	
// 	// Check if we have exactly 4 parts
// 	if(part_count != 4) 
// 	{
// 		printf("Invalid IP format: Expected 4 parts, got %d\r\n", part_count);
// 		setting_in_progress = 0;
// 		return 1;
// 	}
	
// 	// Validate IP ranges (avoid reserved addresses)
// 	if(ip_parts[0] == 0 || ip_parts[0] >= 224)
// 	{
// 		printf("Invalid IP: First octet %d not allowed\r\n", ip_parts[0]);
// 		setting_in_progress = 0;
// 		return 1;
// 	}
	
// 	if(ip_parts[3] == 0 || ip_parts[3] == 255)
// 	{
// 		printf("Invalid IP: Host part cannot be 0 or 255\r\n");
// 		setting_in_progress = 0;
// 		return 1;
// 	}
	
// 	// Construct new IP address (Big Endian format)
// 	new_ip = ((u32)ip_parts[0] << 24) | 
// 			 ((u32)ip_parts[1] << 16) | 
// 			 ((u32)ip_parts[2] << 8) | 
// 			 ((u32)ip_parts[3]);
	
// 	printf("Validating IP: %d.%d.%d.%d (0x%08X)\r\n", 
// 		ip_parts[0], ip_parts[1], ip_parts[2], ip_parts[3], new_ip);
	
// 	// Check for IP conflicts on network
// 	conflict_result = usmart_detect_ip_conflict(new_ip);
	
// 	switch(conflict_result)
// 	{
// 		case 0: // IP available
// 			printf("IP conflict check passed. Setting new IP...\r\n");
// 			break;
			
// 		case 1: // IP in use
// 			printf("Error: IP address already in use on network!\r\n");
// 			printf("Cannot set IP %d.%d.%d.%d - conflict detected.\r\n",
// 				ip_parts[0], ip_parts[1], ip_parts[2], ip_parts[3]);
// 			setting_in_progress = 0;
// 			return 1;
			
// 		case 2: // Detection failed
// 			printf("Warning: IP conflict detection failed.\r\n");
// 			printf("Proceeding with caution...\r\n");
// 			break;
			
// 		default:
// 			printf("Unknown conflict detection result: %d\r\n", conflict_result);
// 			setting_in_progress = 0;
// 			return 1;
// 	}
	
// 	// Update global IP address
// 	current_device_ip = new_ip;
	
// 	/* Mark IP as initialized */
// 	ip_initialized = 1;
	
// 	printf("IP successfully set: %d.%d.%d.%d (0x%08X)\r\n", 
// 		ip_parts[0], ip_parts[1], ip_parts[2], ip_parts[3], new_ip);
	
// 	setting_in_progress = 0; /* Clear flag */
// 	return 0; // Success
// }













