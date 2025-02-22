#ifndef Z2S_DEVICE_DIMMER_H_
#define Z2S_DEVICE_DIMMER_H_

#include "z2s_devices_table.h"
#include <Z2S_control/Z2S_Tuya_dimmer_bulb.h>
#include <Z2S_control/Z2S_Tuya_dimmer_switch.h>

void initZ2SDeviceDimmer(ZigbeeGateway *gateway, zbg_device_params_t *device, int16_t channel_number_slot);

void addZ2SDeviceDimmer(ZigbeeGateway *gateway, zbg_device_params_t *device, uint8_t free_slot, int8_t sub_id, char *name = nullptr, uint32_t func = 0);

void addZ2SDeviceDimmer(ZigbeeGateway *gateway, zbg_device_params_t *device, uint8_t free_slot, char *name = nullptr, uint32_t func = 0);

void msgZ2SDeviceDimmer(int16_t channel_number_slot, int16_t level, bool state, signed char rssi);


#endif