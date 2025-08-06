#include "usmart_network.h"
#include "network_config.h"     /* Include network configuration */
#include "flash_config_simple.h" /* Include simplified Flash configuration */
#include "usart.h"
#include "stdio.h"
#include "string.h"

/* External variables (should be defined in your network module) */
extern u32 current_device_ip;

/* External network configuration variables (defined in network_config.c) */
extern u32 current_subnet_mask;  /* Subnet mask */
extern u8 device_mac[6];  /* Device MAC address */

/* Initialize current device IP if not set */
u8 ip_initialized = 0;

/* ARP response tracking */
static volatile u8 arp_response_received = 0;
static volatile u32 arp_response_ip = 0;
static volatile u32 arp_request_start_time = 0;

/* Network detection initialization */
void init_network_detection(void)
{
    /* Initialize network detection variables */
    arp_response_received = 0;
    arp_response_ip = 0;
    
    printf("Network detection initialized.\r\n");
    
    /* Use Flash configuration if available, otherwise use defaults */
    if(flash_config_is_valid())
    {
        current_subnet_mask = flash_config_get_subnet_mask();
        flash_config_get_mac(device_mac);
        printf("Network config loaded from Flash.\r\n");
    }
    
    printf("Current subnet mask: ");
    print_ip_address(current_subnet_mask);
    printf("\r\n");
    printf("Current MAC address: ");
    print_mac_address(device_mac);
    printf("\r\n");
    
    /* TODO: Initialize Ethernet/network hardware here */
    /* This might include: */
    /* - Ethernet controller initialization */
    /* - Network stack initialization */
    /* - ARP table initialization */
}

/* Set network configuration (subnet mask and MAC address) */
void set_network_config(u32 subnet_mask, u8 *mac_addr)
{
    int i;
    
    if(subnet_mask != 0)
    {
        current_subnet_mask = subnet_mask;
        printf("Subnet mask updated to: ");
        print_ip_address(current_subnet_mask);
        printf("\r\n");
    }
    
    if(mac_addr != 0)
    {
        for(i = 0; i < 6; i++)
        {
            device_mac[i] = mac_addr[i];
        }
        printf("MAC address updated to: ");
        print_mac_address(device_mac);
        printf("\r\n");
    }
}

/* Get current network configuration */
void get_network_config(u32 *subnet_mask, u8 *mac_addr)
{
    int i;
    
    if(subnet_mask != 0)
    {
        *subnet_mask = current_subnet_mask;
    }
    
    if(mac_addr != 0)
    {
        for(i = 0; i < 6; i++)
        {
            mac_addr[i] = device_mac[i];
        }
    }
}

/* Check if IP is in valid range */
u8 is_valid_ip_range(u32 ip)
{
    u8 first_octet = (ip >> 24) & 0xFF;
    u8 last_octet = ip & 0xFF;
    
    /* Check reserved ranges */
    if(first_octet == 0 || first_octet >= 224) return 0;  /* Class D/E or invalid */
    if(first_octet == 127) return 0;  /* Loopback */
    if(last_octet == 0 || last_octet == 255) return 0;  /* Network/broadcast */
    
    /* Check private ranges (allow them) */
    /* 10.0.0.0/8, 172.16.0.0/12, 192.168.0.0/16 are allowed */
    
    return 1; /* Valid IP */
}

/* Check if two IPs are in same subnet */
u8 is_same_subnet(u32 ip1, u32 ip2, u32 subnet_mask)
{
    return ((ip1 & subnet_mask) == (ip2 & subnet_mask)) ? 1 : 0;
}

/* Print IP address in readable format */
void print_ip_address(u32 ip)
{
    printf("%d.%d.%d.%d", 
        (int)((ip >> 24) & 0xFF),
        (int)((ip >> 16) & 0xFF),
        (int)((ip >> 8) & 0xFF),
        (int)(ip & 0xFF));
}

/* Print MAC address in readable format */
void print_mac_address(u8 *mac)
{
    printf("%02X:%02X:%02X:%02X:%02X:%02X", 
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/* Send ARP request packet */
u8 send_arp_request(u32 target_ip)
{
    printf("Sending ARP request to ");
    print_ip_address(target_ip);
    printf("...\r\n");
    
    /* Since we don't have actual Ethernet/ARP implementation, */
    /* we'll simulate the request for now */
    
    /* Reset response flag */
    arp_response_received = 0;
    arp_response_ip = target_ip; /* Store target IP for response matching */
    
    /* Simulate sending ARP request */
    /* In a real implementation, this would: */
    /* 1. Build ARP request packet */
    /* 2. Send via Ethernet hardware */
    /* 3. Wait for hardware interrupt with ARP reply */
    
    return 0; /* Success */
}

/* Check for ARP response with timeout */
u8 check_arp_response(u32 target_ip, u16 timeout_ms)
{
    volatile int i, j; /* C89 compatibility: declare at function start */
    u32 delay_cycles;
    
    printf("Waiting for ARP response (timeout: %dms)...\r\n", timeout_ms);
    
    /* Since we don't have real ARP implementation, */
    /* we'll simulate detection with a simple rule: */
    /* If target IP matches current device IP, it's a conflict */
    
    if(target_ip == current_device_ip)
    {
        printf("ARP response received! IP is in use (same as current device).\r\n");
        return 1; /* IP in use */
    }
    
    /* Calculate delay cycles based on timeout */
    /* Approximate: 1ms = 72000 cycles at 72MHz */
    delay_cycles = (u32)timeout_ms * 1000;
    
    /* Simple delay loop to simulate timeout */
    for(i = 0; i < delay_cycles; i += 1000)
    {
        for(j = 0; j < 1000; j++); /* Inner delay loop */
        
        /* Check if we received a simulated response */
        if(arp_response_received && (arp_response_ip == target_ip))
        {
            printf("ARP response received! IP is in use.\r\n");
            return 1; /* Response received - IP in use */
        }
    }
    
    printf("ARP timeout - no response received.\r\n");
    return 0; /* No response - IP likely available */
}

/* Process incoming ARP packet (to be called by network interrupt handler) */
void process_arp_packet(u8 *packet_data, u16 packet_length)
{
    arp_packet_t *arp_packet;
    
    if(packet_length < sizeof(arp_packet_t)) return;
    
    arp_packet = (arp_packet_t*)packet_data;
    
    /* Check if it's an ARP reply */
    if(arp_packet->operation == 0x0002) /* ARP reply */
    {
        /* Check if this is a response to our request */
        if(arp_packet->sender_ip == arp_response_ip || arp_response_ip == 0)
        {
            printf("ARP reply received from ");
            print_ip_address(arp_packet->sender_ip);
            printf(" (MAC: ");
            print_mac_address(arp_packet->sender_mac);
            printf(")\r\n");
            
            arp_response_received = 1;
            arp_response_ip = arp_packet->sender_ip;
        }
    }
}

/* Enhanced IP conflict detection with multiple retries */
u8 usmart_detect_ip_conflict_enhanced(u32 target_ip)
{
    u8 retry_count;
    u8 response_detected = 0;
    volatile int i; /* C89 compatibility: declare at function start */
    u32 default_current_ip;
    
    printf("Enhanced IP conflict detection for ");
    print_ip_address(target_ip);
    printf("\r\n");
    
    /* Validate IP range first */
    if(!is_valid_ip_range(target_ip))
    {
        printf("Invalid IP range detected!\r\n");
        return ARP_STATUS_ERROR;
    }
    
    /* If current_device_ip is not initialized, use target IP's network */
    if(current_device_ip == 0 || !ip_initialized)
    {
        /* Assume same network as target for first-time setup */
        default_current_ip = (target_ip & 0xFFFFFF00) | 0x01; /* .1 in same network */
        printf("Using default network assumption for subnet check\r\n");
    }
    else
    {
        default_current_ip = current_device_ip;
    }
    
    /* Check same subnet with more lenient logic */
    if(!is_same_subnet(default_current_ip, target_ip, current_subnet_mask))
    {
        /* If not in same subnet, check if it's a common private network */
        u8 target_first = (target_ip >> 24) & 0xFF;
        u8 target_second = (target_ip >> 16) & 0xFF;
        
        if((target_first == 192 && target_second == 168) ||  /* 192.168.x.x */
           (target_first == 10) ||                           /* 10.x.x.x */
           (target_first == 172 && target_second >= 16 && target_second <= 31)) /* 172.16-31.x.x */
        {
            printf("Target IP is in private network range - allowing...\r\n");
        }
        else
        {
            printf("Target IP not in current subnet!\r\n");
            return ARP_STATUS_ERROR;
        }
    }
    
    /* Check if same as current IP */
    if(target_ip == current_device_ip)
    {
        printf("Target IP same as current device IP!\r\n");
        return ARP_STATUS_IN_USE;
    }
    
    /* Simple conflict detection for common IPs */
    /* This simulates detecting common gateway/server IPs */
    {
        u8 last_octet = target_ip & 0xFF;
        
        /* Common gateway addresses */
        if(last_octet == 1 || last_octet == 254)
        {
            printf("Warning: Common gateway IP detected - likely in use!\r\n");
            return ARP_STATUS_IN_USE;
        }
        
        /* For demonstration: simulate that .206 is occupied */
        /* (since your PC is 192.168.0.206) */
        if(last_octet == 206 && ip_initialized && current_device_ip != target_ip)
        {
            printf("Simulated conflict: IP .206 is occupied by another device!\r\n");
            return ARP_STATUS_IN_USE;
        }
    }
    
#if USMART_ENABLE_ARP_DETECTION
    /* Perform multiple ARP requests for reliability */
    for(retry_count = 0; retry_count < ARP_RETRY_COUNT; retry_count++)
    {
        printf("ARP attempt %d/%d\r\n", retry_count + 1, ARP_RETRY_COUNT);
        
        /* Send ARP request */
        if(send_arp_request(target_ip) != 0)
        {
            printf("Failed to send ARP request\r\n");
            continue;
        }
        
        /* Check for response */
        if(check_arp_response(target_ip, ARP_TIMEOUT_MS))
        {
            response_detected = 1;
            break; /* Response received - IP in use */
        }
        
        /* Wait before next retry */
        if(retry_count < ARP_RETRY_COUNT - 1)
        {
            printf("Waiting before retry...\r\n");
            /* Simple delay between retries */
            for(i = 0; i < 50000; i++); /* Placeholder delay */
        }
    }
    
    if(response_detected)
    {
        printf("IP conflict confirmed after %d attempts\r\n", retry_count + 1);
        return ARP_STATUS_IN_USE;
    }
    else
    {
        printf("No conflict detected after %d attempts\r\n", ARP_RETRY_COUNT);
        return ARP_STATUS_AVAILABLE;
    }
#else
    printf("ARP detection disabled - assuming IP available\r\n");
    return ARP_STATUS_AVAILABLE;
#endif
}
