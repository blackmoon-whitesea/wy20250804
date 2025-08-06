#ifndef __USMART_NETWORK_H
#define __USMART_NETWORK_H

#include "sys.h"

// Network configuration structures
typedef struct {
    u32 ip;
    u32 subnet_mask;
    u32 gateway;
    u8 mac[6];
} network_config_t;

// ARP packet structure
typedef struct {
    u16 hw_type;        // Hardware type (Ethernet = 1)
    u16 proto_type;     // Protocol type (IP = 0x0800)
    u8 hw_len;          // Hardware address length (6 for MAC)
    u8 proto_len;       // Protocol address length (4 for IP)
    u16 operation;      // Operation (1=request, 2=reply)
    u8 sender_mac[6];   // Sender MAC address
    u32 sender_ip;      // Sender IP address
    u8 target_mac[6];   // Target MAC address
    u32 target_ip;      // Target IP address
} __attribute__((packed)) arp_packet_t;

// ARP detection status
#define ARP_STATUS_AVAILABLE    0   // IP available
#define ARP_STATUS_IN_USE      1   // IP already in use
#define ARP_STATUS_TIMEOUT     2   // Detection timeout
#define ARP_STATUS_ERROR       3   // Detection error

// Function declarations for ARP conflict detection
u8 usmart_detect_ip_conflict(u32 target_ip);
u8 usmart_detect_ip_conflict_enhanced(u32 target_ip);
u8 send_arp_request(u32 target_ip);
u8 check_arp_response(u32 target_ip, u16 timeout_ms);
void init_network_detection(void);

// Network configuration functions
void set_network_config(u32 subnet_mask, u8 *mac_addr);
void get_network_config(u32 *subnet_mask, u8 *mac_addr);

// Network utility functions
u8 is_valid_ip_range(u32 ip);
u8 is_same_subnet(u32 ip1, u32 ip2, u32 subnet_mask);
void print_ip_address(u32 ip);
void print_mac_address(u8 *mac);

// Timeout and retry configurations
#define ARP_TIMEOUT_MS          200     // ARP response timeout
#define ARP_RETRY_COUNT         3       // Number of retries
#define ARP_REQUEST_INTERVAL    50      // Interval between requests (ms)

// Network detection enable/disable
#define USMART_ENABLE_ARP_DETECTION  1  // Set to 0 to disable ARP detection

// External variables
extern u8 ip_initialized;  // IP initialization flag

#endif /* __USMART_NETWORK_H */
