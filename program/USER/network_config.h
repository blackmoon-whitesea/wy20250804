#ifndef __NETWORK_CONFIG_H
#define __NETWORK_CONFIG_H

#include "sys.h"

/* Global network configuration variables */
extern u32 current_subnet_mask;
extern u8 device_mac[6];

/* Configuration functions */
void usmart_set_subnet_mask(u32 mask);
void usmart_set_device_mac(u8 *mac);

#endif /* __NETWORK_CONFIG_H */
