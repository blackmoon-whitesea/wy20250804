#include "flash_config_simple.h"
#include "stdio.h"
#include "string.h"

// Global configuration data in RAM (simulates Flash storage)
static flash_config_t g_config;
static u8 config_loaded = 0;
static u8 config_saved = 0;  // Flag to track if config was saved

// Simple CRC32 calculation (polynomial: 0xEDB88320)
u32 calculate_crc32(u8 *data, u32 length)
{
    u32 crc = 0xFFFFFFFF;
    u32 i, j;
    
    for(i = 0; i < length; i++)
    {
        crc ^= data[i];
        for(j = 0; j < 8; j++)
        {
            if(crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc = crc >> 1;
        }
    }
    
    return crc ^ 0xFFFFFFFF;
}

// Initialize default configuration
static void flash_config_set_defaults(void)
{
    int i;
    
    /* Set default values */
    g_config.magic_number = CONFIG_MAGIC_NUMBER;
    g_config.config_version = CONFIG_VERSION;
    g_config.device_ip = 0xC0A80001;      /* 192.168.0.1 */
    g_config.subnet_mask = 0xFFFFFF00;    /* 255.255.255.0 */
    g_config.gateway_ip = 0xC0A80001;     /* 192.168.0.1 */
    
    /* Default MAC address */
    g_config.device_mac[0] = 0x02;
    g_config.device_mac[1] = 0x00;
    g_config.device_mac[2] = 0x00;
    g_config.device_mac[3] = 0x00;
    g_config.device_mac[4] = 0x00;
    g_config.device_mac[5] = 0x01;
    
    g_config.reserved1 = 0;
    
    /* Clear reserved area */
    for(i = 0; i < sizeof(g_config.reserved); i++)
    {
        g_config.reserved[i] = 0;
    }
    
    /* Calculate CRC32 (excluding the CRC field itself) */
    g_config.crc32 = calculate_crc32((u8*)&g_config, sizeof(g_config) - sizeof(g_config.crc32));
    
    printf("Default configuration initialized.\r\n");
}

// Initialize flash configuration system
u8 flash_config_init(void)
{
    printf("Initializing Flash configuration system (RAM simulation)...\r\n");
    
    /* Set defaults */
    flash_config_set_defaults();
    config_loaded = 1;
    config_saved = 0;  /* Mark as not saved yet */
    
    printf("Configuration system ready.\r\n");
    return 0;
}

// Load configuration from "Flash" (actually RAM simulation)
u8 flash_config_load(void)
{
    if(!config_saved)
    {
        printf("No saved configuration found, using defaults.\r\n");
        return 1;
    }
    
    printf("Configuration loaded from memory.\r\n");
    printf("Device IP: %d.%d.%d.%d\r\n", 
           (int)((g_config.device_ip >> 24) & 0xFF),
           (int)((g_config.device_ip >> 16) & 0xFF),
           (int)((g_config.device_ip >> 8) & 0xFF),
           (int)(g_config.device_ip & 0xFF));
    
    return 0;
}

// Save configuration to "Flash" (actually RAM simulation)
u8 flash_config_save(void)
{
    printf("Saving configuration to memory simulation...\r\n");
    
    /* Calculate CRC32 before saving */
    g_config.crc32 = calculate_crc32((u8*)&g_config, sizeof(g_config) - sizeof(g_config.crc32));
    
    /* Mark as saved */
    config_saved = 1;
    
    printf("Configuration saved successfully.\r\n");
    printf("Note: This is RAM simulation. In real implementation, data would persist after reset.\r\n");
    return 0;
}

// Erase configuration from "Flash" (actually RAM simulation)
u8 flash_config_erase(void)
{
    printf("Erasing configuration from memory simulation...\r\n");
    
    /* Reset to defaults */
    flash_config_set_defaults();
    config_saved = 0;  /* Mark as not saved */
    
    printf("Configuration erased successfully.\r\n");
    return 0;
}

// Configuration access functions
void flash_config_set_ip(u32 ip)
{
    g_config.device_ip = ip;
    printf("IP set in configuration: %d.%d.%d.%d\r\n",
           (int)((ip >> 24) & 0xFF),
           (int)((ip >> 16) & 0xFF),
           (int)((ip >> 8) & 0xFF),
           (int)(ip & 0xFF));
}

u32 flash_config_get_ip(void)
{
    if(!config_loaded) return 0xC0A80001; /* Default IP */
    return g_config.device_ip;
}

void flash_config_set_subnet_mask(u32 mask)
{
    g_config.subnet_mask = mask;
}

u32 flash_config_get_subnet_mask(void)
{
    if(!config_loaded) return 0xFFFFFF00; /* Default mask */
    return g_config.subnet_mask;
}

void flash_config_set_mac(u8 *mac)
{
    int i;
    if(mac != 0)
    {
        for(i = 0; i < 6; i++)
        {
            g_config.device_mac[i] = mac[i];
        }
    }
}

void flash_config_get_mac(u8 *mac)
{
    int i;
    if(mac != 0)
    {
        if(!config_loaded)
        {
            /* Default MAC */
            mac[0] = 0x02; mac[1] = 0x00; mac[2] = 0x00;
            mac[3] = 0x00; mac[4] = 0x00; mac[5] = 0x01;
        }
        else
        {
            for(i = 0; i < 6; i++)
            {
                mac[i] = g_config.device_mac[i];
            }
        }
    }
}

u8 flash_config_is_valid(void)
{
    return config_loaded;
}
