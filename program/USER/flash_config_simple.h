#ifndef __FLASH_CONFIG_SIMPLE_H
#define __FLASH_CONFIG_SIMPLE_H

#include "sys.h"

// Configuration data structure
typedef struct {
    u32 magic_number;       // Magic number to verify valid config (0x12345678)
    u32 config_version;     // Configuration version
    u32 device_ip;          // Device IP address
    u32 subnet_mask;        // Subnet mask
    u32 gateway_ip;         // Gateway IP (reserved for future use)
    u8  device_mac[6];      // Device MAC address
    u16 reserved1;          // Padding for 4-byte alignment
    u32 crc32;             // CRC32 checksum of the config data
    u8  reserved[100];     // Reserved space for future expansion
} flash_config_t;

// Magic number to identify valid configuration
#define CONFIG_MAGIC_NUMBER     0x12345678
#define CONFIG_VERSION          0x00000001

// Function declarations
u8 flash_config_init(void);
u8 flash_config_save(void);
u8 flash_config_load(void);
u8 flash_config_erase(void);
void flash_config_set_ip(u32 ip);
u32 flash_config_get_ip(void);
void flash_config_set_subnet_mask(u32 mask);
u32 flash_config_get_subnet_mask(void);
void flash_config_set_mac(u8 *mac);
void flash_config_get_mac(u8 *mac);
u8 flash_config_is_valid(void);

// CRC32 calculation function
u32 calculate_crc32(u8 *data, u32 length);

#endif /* __FLASH_CONFIG_SIMPLE_H */
