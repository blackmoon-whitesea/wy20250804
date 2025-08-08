#include "lwip_comm.h" 
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/init.h"
#include "ethernetif.h" 
#include "lwip/timers.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "lwip/tcpip.h" 
#include "malloc.h"
#include "delay.h"
#include "usart.h" 
#include "SipConstant.h"
#include <stdio.h>
#include "HhdxMsg.h"
#include "24cxx.h" 

__lwip_dev lwipdev;						//lwip control block 
struct netif lwip_netif;				//network interface structure

extern u32 memp_get_memorysize(void);	//get from memp.c
extern u8_t *memp_memory;				//get from memp.c
extern u8_t *ram_heap;					//get from mem.c

u32 TCPTimer=0;			//TCP timer counter
u32 ARPTimer=0;			//ARP timer counter
u32 lwip_localtime;		//lwip local time, unit: ms

#if LWIP_DHCP
u32 DHCPfineTimer=0;	//DHCP fine timer counter
u32 DHCPcoarseTimer=0;	//DHCP coarse timer counter
#endif

//lwip allocate memory for mem and memp
//return: 0, success;
//        other, error
u8 lwip_comm_mem_malloc(void)
{
	u32 mempsize;
	u32 ramheapsize; 
	mempsize=memp_get_memorysize();			//get memp_memory array size
	memp_memory=mymalloc(SRAMIN,mempsize);	//allocate memory for memp_memory
	ramheapsize=LWIP_MEM_ALIGN_SIZE(MEM_SIZE)+2*LWIP_MEM_ALIGN_SIZE(4*3)+MEM_ALIGNMENT;//get ram heap size
	ram_heap=mymalloc(SRAMIN,ramheapsize);	//allocate memory for ram_heap
	if(!memp_memory||!ram_heap)//allocation failed
	{
		lwip_comm_mem_free();
		return 1;
	}
	return 0;	
}
//lwip free memory for mem and memp
void lwip_comm_mem_free(void)
{ 	
	myfree(SRAMIN,memp_memory);
	myfree(SRAMIN,ram_heap);
}
//lwip set default IP settings
//lwipx: lwip control block pointer
void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
	u32 sn0;
	sn0=*(vu32*)(0x1FFF7A10);//get STM32 unique ID last 24 bits for MAC address generation
	//set remote IP address: 192.168.0.10
	lwipx->remoteip[0]=gpsEeprom->sv_ip[0];	
	lwipx->remoteip[1]=gpsEeprom->sv_ip[1];
	lwipx->remoteip[2]=gpsEeprom->sv_ip[2];
	lwipx->remoteip[3]=gpsEeprom->sv_ip[3];
	//MAC address setting (default manufacturer ID: 2.0.0, unique part from STM32 chip ID)
	lwipx->mac[0]=2;//manufacturer ID (IEEE assigned OUI) default: 2.0.0
	lwipx->mac[1]=0;
	lwipx->mac[2]=0;
	lwipx->mac[3]=(sn0>>16)&0XFF;//unique part from STM32 chip ID
	lwipx->mac[4]=(sn0>>8)&0XFFF;;
	lwipx->mac[5]=sn0&0XFF; 
	//set local IP address: 192.168.0.30
	lwipx->ip[0]=gpsEeprom->own_ip[0];	
	lwipx->ip[1]=gpsEeprom->own_ip[1];
	lwipx->ip[2]=gpsEeprom->own_ip[2];
	lwipx->ip[3]=gpsEeprom->own_ip[3];
	//set subnet mask: 255.255.255.0
	lwipx->netmask[0]=gpsEeprom->mask_ip[0];	
	lwipx->netmask[1]=gpsEeprom->mask_ip[1];
	lwipx->netmask[2]=gpsEeprom->mask_ip[2];
	lwipx->netmask[3]=gpsEeprom->mask_ip[3];
	//set gateway address: 192.168.0.1
	lwipx->gateway[0]=gpsEeprom->gw_ip[0];	
	lwipx->gateway[1]=gpsEeprom->gw_ip[1];
	lwipx->gateway[2]=gpsEeprom->gw_ip[2];
	lwipx->gateway[3]=gpsEeprom->gw_ip[3];	
	lwipx->dhcpstatus=0;//disable DHCP	
} 

//LWIP initialization (LWIP must be called before using LWIP)
//return: 0, success
//        1, memory error
//        2, LAN8720 initialization error
//        3, network interface initialization error.
u8 lwip_comm_init(void)
{
	struct netif *Netif_Init_Flag;		//return value of netif_add() function, used to check initialization status
	struct ip_addr ipaddr;  			//ip address
	struct ip_addr netmask; 			//subnet mask
	struct ip_addr gw;      			//gateway address
	if(LAN8720_Init())return 2;			//initialize LAN8720 
	lwip_init();								//initialize LWIP stack
	lwip_comm_default_ip_set(&lwipdev);	//set default IP settings
#if LWIP_DHCP		//use DHCP
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else				//use static IP
	IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]);
	IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
	printf("Ethernet MAC Address:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
	printf("Local IP Address........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	printf("Subnet Mask..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
	printf("Gateway Address..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
#endif
	Netif_Init_Flag=netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&ethernet_input);//add network interface
	
#if LWIP_DHCP			//enable DHCP function
	lwipdev.dhcpstatus=0;	//DHCP status = 0
	dhcp_start(&lwip_netif);	//start DHCP service
#endif
	
	if(Netif_Init_Flag==NULL)return 3;//network interface initialization failed 
	else//network interface initialization successful, set netif as default, enable netif
	{
		netif_set_default(&lwip_netif); //set netif as default interface
		netif_set_up(&lwip_netif);		//enable netif interface
	}
	return 0;//initialization OK.
}   

//receive packet handling function 
void lwip_pkt_handle(void)
{
  //read ethernet packet and pass to LWIP stack for processing
 ethernetif_input(&lwip_netif);
}

//LWIP periodic processing function
void lwip_periodic_handle()
{
#if LWIP_TCP
	//call tcp_tmr() function every 250ms
  if (lwip_localtime - TCPTimer >= TCP_TMR_INTERVAL)
  {
    TCPTimer =  lwip_localtime;
    tcp_tmr();
  }
#endif
  //ARP processing every 5s
  if ((lwip_localtime - ARPTimer) >= ARP_TMR_INTERVAL)
  {
    ARPTimer =  lwip_localtime;
    etharp_tmr();
  }

#if LWIP_DHCP //when using DHCP
  //call dhcp_fine_tmr() function every 500ms
  if (lwip_localtime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
  {
    DHCPfineTimer =  lwip_localtime;
    dhcp_fine_tmr();
    if ((lwipdev.dhcpstatus != 2)&&(lwipdev.dhcpstatus != 0XFF))
    { 
      lwip_dhcp_process_handle();  //DHCP process handling
    }
  }

  //call DHCP coarse timer every 60s
  if (lwip_localtime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
  {
    DHCPcoarseTimer =  lwip_localtime;
    dhcp_coarse_tmr();
  }  
#endif
}


//DHCP process when using DHCP
#if LWIP_DHCP

//DHCP status processing function
void lwip_dhcp_process_handle(void)
{
	u32 ip=0,netmask=0,gw=0;
	switch(lwipdev.dhcpstatus)
	{
		case 0: 	//start DHCP
			dhcp_start(&lwip_netif);
			lwipdev.dhcpstatus = 1;		//enter DHCP status wait mode
			printf("Starting DHCP IP allocation, please wait...........\r\n");  
			break;
		case 1:		//waiting for DHCP to get IP address
		{
			ip=lwip_netif.ip_addr.addr;		//read current IP address
			netmask=lwip_netif.netmask.addr;//read current subnet mask
			gw=lwip_netif.gw.addr;			//read current gateway address
			
			if(ip!=0)			//successfully acquired IP address
			{
				lwipdev.dhcpstatus=2;	//DHCP success
				printf("Ethernet MAC Address:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
				//update local device with DHCP assigned IP address
				lwipdev.ip[3]=(uint8_t)(ip>>24); 
				lwipdev.ip[2]=(uint8_t)(ip>>16);
				lwipdev.ip[1]=(uint8_t)(ip>>8);
				lwipdev.ip[0]=(uint8_t)(ip);
				printf("DHCP Assigned IP Address..............%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
				//update local device with DHCP assigned subnet mask
				lwipdev.netmask[3]=(uint8_t)(netmask>>24);
				lwipdev.netmask[2]=(uint8_t)(netmask>>16);
				lwipdev.netmask[1]=(uint8_t)(netmask>>8);
				lwipdev.netmask[0]=(uint8_t)(netmask);
				printf("DHCP Assigned Subnet Mask............%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
				//update local device with DHCP assigned gateway address
				lwipdev.gateway[3]=(uint8_t)(gw>>24);
				lwipdev.gateway[2]=(uint8_t)(gw>>16);
				lwipdev.gateway[1]=(uint8_t)(gw>>8);
				lwipdev.gateway[0]=(uint8_t)(gw);
				printf("DHCP Assigned Gateway Address..........%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			}else if(lwip_netif.dhcp->tries>LWIP_MAX_DHCP_TRIES) //DHCP timeout, failed to get IP address, use static IP configuration
			{
				lwipdev.dhcpstatus=0XFF;//DHCP failed.
				//use static IP settings
				IP4_ADDR(&(lwip_netif.ip_addr),lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
				IP4_ADDR(&(lwip_netif.netmask),lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
				IP4_ADDR(&(lwip_netif.gw),lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
				printf("DHCP timeout, using static IP configuration!\r\n");
				printf("Ethernet MAC Address:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
				printf("Local IP Address........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
				printf("Subnet Mask..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
				printf("Gateway Address..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			}
		}
		break;
		default : break;
	}
}
#endif 


void LWIP_NetConfigUpdate(void)
{
	struct ip_addr ipaddr;  			//ip address
	struct ip_addr netmask; 			//subnet mask
	struct ip_addr gw;      			//gateway address
	
//	lwip_comm_default_ip_set(&lwipdev);	//set default IP settings
	
	//set local IP address: 192.168.0.30
	lwipdev.ip[0]=gpsEeprom->own_ip[0];	
	lwipdev.ip[1]=gpsEeprom->own_ip[1];
	lwipdev.ip[2]=gpsEeprom->own_ip[2];
	lwipdev.ip[3]=gpsEeprom->own_ip[3];
	//set subnet mask: 255.255.0.0
	lwipdev.netmask[0]=255;	
	lwipdev.netmask[1]=255;
	lwipdev.netmask[2]=0;
	lwipdev.netmask[3]=0;
	//set gateway address: 192.168.0.1
	lwipdev.gateway[0]=gpsEeprom->own_ip[0];	
	lwipdev.gateway[1]=gpsEeprom->own_ip[1];
	lwipdev.gateway[2]=0;
	lwipdev.gateway[3]=1;	
	
	IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]);
	IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
	printf("Ethernet MAC Address:................%x.%x.%x.%x.%x.%x\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
	printf("Local IP Address........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	printf("Subnet Mask..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
	printf("Gateway Address..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
	
	
	netif_set_down(&lwip_netif); //disable network interface
	
	netif_set_gw(&lwip_netif, &gw);        //set new gateway address
	netif_set_netmask(&lwip_netif, &netmask); //set new subnet mask
	netif_set_ipaddr(&lwip_netif, &ipaddr);    //set new IP address
//  netif_set_addr(&u_sNetif, &ip_update, &Mask_update, &GW_updata);

  netif_set_up(&lwip_netif);  //enable network interface
  
	
/*	
	int i;
	err_t err;
	struct ip_addr ip_update;
	struct ip_addr Mask_update;
	struct ip_addr GW_updata;
	
	IP4_ADDR(&ip_update,sLocalMesg->aSrcIP[0],sLocalMesg->aSrcIP[1],
	        sLocalMesg->aSrcIP[2],sLocalMesg->aSrcIP[3]);
	IP4_ADDR(&Mask_update, sLocalMesg->aNetMask[0], sLocalMesg->aNetMask[1],
	        sLocalMesg->aNetMask[2], sLocalMesg->aNetMask[3]);
	IP4_ADDR(&GW_updata, sLocalMesg->aGateWay[0], sLocalMesg->aGateWay[1],
	        sLocalMesg->aGateWay[2], sLocalMesg->aGateWay[3]);
	
	u_byLocalIP[0]=sLocalMesg->aSrcIP[0];
	u_byLocalIP[1]=sLocalMesg->aSrcIP[1];
	u_byLocalIP[2]=sLocalMesg->aSrcIP[2];
	u_byLocalIP[3]=sLocalMesg->aSrcIP[3];
	
	for(i = 0; i < LWIP_MAX_CONNECT; i++)
	{
		if(u_sTcp_pcb[i] != NULL)
		{
			err = tcp_close(u_sTcp_pcb[i]);
			if(err == ERR_OK)
				u_sTcp_pcb[i] = NULL;
	  }
	}
	netif_set_down(&u_sNetif); //disable network interface
	
	netif_set_gw(&u_sNetif, &GW_updata);        //set new gateway address
	netif_set_netmask(&u_sNetif, &Mask_update); //set new subnet mask
	netif_set_ipaddr(&u_sNetif, &ip_update);    //set new IP address
//  netif_set_addr(&u_sNetif, &ip_update, &Mask_update, &GW_updata);

  netif_set_up(&u_sNetif);  //enable network interface
*/
}

























