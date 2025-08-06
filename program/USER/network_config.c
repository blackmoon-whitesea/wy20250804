/* Simple network configuration for USMART IP conflict detection */
#include "sys.h"
#include "stdio.h"

/* Default network configuration */
u32 current_subnet_mask = 0xFFFFFF00;  /* 255.255.255.0 */
u8 device_mac[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};  /* Default MAC */

/* Simple function to set subnet mask */
void usmart_set_subnet_mask(u32 mask)
{
    current_subnet_mask = mask;
}

/* Simple function to set device MAC */
void usmart_set_device_mac(u8 *mac)
{
    int i;
    if(mac != 0)
    {
        for(i = 0; i < 6; i++)
        {
            device_mac[i] = mac[i];
        }
    }
}
