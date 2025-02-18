#ifndef Z2S_DEVICE_VIRTUAL_RELAY_H_
#define Z2S_DEVICE_VIRTUAL_RELAY_H_

#include "z2s_devices_table.h"
//#include <supla/control/virtual_relay.h>
#include <Z2S_control/z2s_virtual_relay.h>


void initZ2SDeviceVirtualRelay(ZigbeeGateway *gateway, zbg_device_params_t *device, uint8_t Supla_channel, char *name = nullptr, uint32_t func = 0);

void addZ2SDeviceVirtualRelay(ZigbeeGateway *gateway, zbg_device_params_t *device, uint8_t free_slot, char *name = nullptr, uint32_t func = 0);

void msgZ2SDeviceVirtualRelay(uint8_t Supla_channel, bool state, signed char rssi);

#endif