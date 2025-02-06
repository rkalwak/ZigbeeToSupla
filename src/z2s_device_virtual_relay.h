#ifndef Z2S_DEVICE_VIRTUAL_RELAY_H_
#define Z2S_DEVICE_VIRTUAL_RELAY_H_

#include "z2s_devices_table.h"
//#include <supla/control/virtual_relay.h>
#include <Z2S_control/Z2S_virtual_relay.h>


void initZ2SDeviceVirtualRelay(ZigbeeGateway *gateway, zb_device_params_t *device, uint8_t Supla_channel);

void addZ2SDeviceVirtualRelay(ZigbeeGateway *gateway, zb_device_params_t *device, uint8_t free_slot);

void msgZ2SDeviceVirtualRelay(uint8_t Supla_channel, bool state);

#endif