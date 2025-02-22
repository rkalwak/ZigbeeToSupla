

#include "z2s_devices_table.h"
#include "z2s_device_iaszone.h"
#include "z2s_device_virtual_relay.h"
#include "z2s_device_electricity_meter.h"
#include "z2s_device_tuya_hvac.h"
#include "z2s_device_dimmer.h"
#include "z2s_device_rgb.h"
#include "z2s_device_rgbw.h"
#include "z2s_device_temphumidity.h"
#include "z2s_device_tuya_custom_cluster.h"
#include "z2s_device_general_purpose_measurement.h"

#include <SuplaDevice.h>

#include <supla/control/virtual_relay.h>

#include <Z2S_control/z2s_virtual_relay.h>
#include <Z2S_control/z2s_virtual_relay_scene_switch.h>


extern ZigbeeGateway zbGateway;

z2s_device_params_t z2s_devices_table[Z2S_CHANNELMAXCOUNT];

static uint32_t Styrbar_timer = 0;
static bool     Styrbar_ignore_button_1 = false;

uint32_t Z2S_getDevicesTableSize() {
  uint32_t _z2s_devices_table_size;
  if (Supla::Storage::ConfigInstance()->getUInt32(Z2S_DEVICES_TABLE_SIZE, &_z2s_devices_table_size))
    return _z2s_devices_table_size;
  else
    return 0;
}

uint8_t Z2S_findFirstFreeDevicesTableSlot(uint8_t start_slot) {

  for (uint8_t devices_counter = start_slot; devices_counter < Z2S_CHANNELMAXCOUNT; devices_counter++) 
      if (!z2s_devices_table[devices_counter].valid_record)
        return devices_counter;
  return 0xFF;
  
}

void Z2S_printDevicesTableSlots() {

  for (uint8_t devices_counter = 0; devices_counter < Z2S_CHANNELMAXCOUNT; devices_counter++) 
    if (z2s_devices_table[devices_counter].valid_record)
      log_i("valid %d, ieee addr %s, model_id 0x%x, endpoint 0x%x, cluster 0x%x, channel %d, channel type %d",
        z2s_devices_table[devices_counter].valid_record,
        z2s_devices_table[devices_counter].ieee_addr,
        z2s_devices_table[devices_counter].model_id,
        z2s_devices_table[devices_counter].endpoint,
        z2s_devices_table[devices_counter].cluster_id,
        z2s_devices_table[devices_counter].Supla_channel,
        z2s_devices_table[devices_counter].Supla_channel_type);  
}


int16_t Z2S_findChannelNumberSlot(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, int32_t channel_type, int8_t sub_id) {

  log_i("Z2S_findChannelNumberSlot 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x, endpoint 0x%x, channel type 0x%x", 
        ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3], ieee_addr[2], ieee_addr[1], ieee_addr[0], 
        endpoint, channel_type);
  
  for (uint8_t devices_counter = 0; devices_counter < Z2S_CHANNELMAXCOUNT; devices_counter++) {
      if (z2s_devices_table[devices_counter].valid_record)
        if ((memcmp(z2s_devices_table[devices_counter].ieee_addr, ieee_addr,8) == 0) && 
        (z2s_devices_table[devices_counter].endpoint == endpoint) &&
        ((channel_type < 0) || (z2s_devices_table[devices_counter].Supla_channel_type == channel_type)) &&
        ((sub_id < 0) || (z2s_devices_table[devices_counter].sub_id == sub_id))) { 
        //&& (z2s_devices_table[devices_counter].cluster_id == cluster)) {
            return devices_counter;
        }

  }  
  return -1;
}

int16_t Z2S_findChannelNumberNextSlot(int16_t prev_slot, esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, 
                                      int32_t channel_type, int8_t sub_id) {

  log_i("Z2S_findChannelNumberNextSlot 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x, endpoint 0x%x, channel type 0x%x", 
        ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3], ieee_addr[2], ieee_addr[1], ieee_addr[0], 
        endpoint, channel_type);
  
  for (uint8_t devices_counter = prev_slot + 1; devices_counter < Z2S_CHANNELMAXCOUNT; devices_counter++) {
      if (z2s_devices_table[devices_counter].valid_record)
        if ((memcmp(z2s_devices_table[devices_counter].ieee_addr, ieee_addr,8) == 0) && 
        (z2s_devices_table[devices_counter].endpoint == endpoint) &&
        ((channel_type < 0) || (z2s_devices_table[devices_counter].Supla_channel_type == channel_type)) &&
        ((sub_id < 0) || (z2s_devices_table[devices_counter].sub_id == sub_id))) { 
        //&& (z2s_devices_table[devices_counter].cluster_id == cluster)) {
            return devices_counter;
        }
  }  
  return -1;
}

void Z2S_fillDevicesTableSlot(zbg_device_params_t *device, uint8_t slot, uint8_t channel, int32_t channel_type, int8_t sub_id,
                              char *name, uint32_t func) {

  z2s_devices_table[slot].valid_record = true;
  memcpy(z2s_devices_table[slot].ieee_addr,device->ieee_addr,8);
  z2s_devices_table[slot].model_id = device->model_id;
  z2s_devices_table[slot].endpoint = device->endpoint;
  z2s_devices_table[slot].cluster_id = device->cluster_id;
  z2s_devices_table[slot].Supla_channel = channel;
  z2s_devices_table[slot].Supla_channel_type = channel_type;
  z2s_devices_table[slot].sub_id = sub_id; 
  if (name) strcpy(z2s_devices_table[slot].Supla_channel_name, name);
  z2s_devices_table[slot].Supla_channel_func = func;
  
  Z2S_saveDevicesTable();
  //Z2S_printDevicesTableSlots();
}

bool Z2S_loadDevicesTable() {

  log_i("before get devices table");
  uint32_t z2s_devices_table_size =  Z2S_getDevicesTableSize(); //3584

  if (z2s_devices_table_size == 0) {

      log_i(" No devices table found, writing empty one with size %d", sizeof(z2s_devices_table));
      
      memset(z2s_devices_table,0,sizeof(z2s_devices_table));
      
      if (!Supla::Storage::ConfigInstance()->setBlob(Z2S_DEVICES_TABLE, (char *)z2s_devices_table, sizeof(z2s_devices_table))) {
        log_i ("Devices table write failed!");
        return false;
      }
      else { 
        if (Supla::Storage::ConfigInstance()->setUInt32(Z2S_DEVICES_TABLE_SIZE, sizeof(z2s_devices_table))) {
          Supla::Storage::ConfigInstance()->commit();
          return true;
        }
        else { 
          log_i ("Devices table size write failed!");
          return false;
        }
      }
  } else
  {
    if (z2s_devices_table_size != sizeof(z2s_devices_table)) {
      
      log_i("Devices table size mismatch %d <> %d", z2s_devices_table_size, sizeof(z2s_devices_table));
      return false;
    }
    else {
        if (!Supla::Storage::ConfigInstance()->getBlob(Z2S_DEVICES_TABLE, (char *)z2s_devices_table, sizeof(z2s_devices_table))) {
          log_i ("Devices table load failed!");
          return false;
        } else {
          log_i ("Devices table load success!");
          return true;
        }
    }
  }
}

bool Z2S_saveDevicesTable() {

  if (!Supla::Storage::ConfigInstance()->setBlob(Z2S_DEVICES_TABLE, (char *)z2s_devices_table, sizeof(z2s_devices_table))) {
    log_i ("Devices table write failed!");
    return false;
  }
  else { 
    if (Supla::Storage::ConfigInstance()->setUInt32(Z2S_DEVICES_TABLE_SIZE, sizeof(z2s_devices_table)))
      return true;
    else { 
      log_i ("Devices table size write failed!");
      return false;
    }
  }
  Supla::Storage::ConfigInstance()->commit();
}

void Z2S_clearDevicesTable() {
  log_i("Clear devices table");
  memset(z2s_devices_table,0,sizeof(z2s_devices_table));
  Z2S_saveDevicesTable();
}

void Z2S_initSuplaChannels(){

  log_i ("initSuplaChannels starting");
  zbg_device_params_t *device = (zbg_device_params_t *)malloc(sizeof(zbg_device_params_t));

  for (uint8_t devices_counter = 0; devices_counter < Z2S_CHANNELMAXCOUNT; devices_counter++) {
      if (z2s_devices_table[devices_counter].valid_record) {

        device->endpoint = z2s_devices_table[devices_counter].endpoint;
        device->cluster_id = z2s_devices_table[devices_counter].cluster_id;
        memcpy(device->ieee_addr, z2s_devices_table[devices_counter].ieee_addr,8);
        device->short_addr = z2s_devices_table[devices_counter].short_addr;
        device->model_id = z2s_devices_table[devices_counter].model_id;

        switch (z2s_devices_table[devices_counter].Supla_channel_type) {

          case SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR: 
            initZ2SDeviceTempHumidity(devices_counter); break;

          case SUPLA_CHANNELTYPE_GENERAL_PURPOSE_MEASUREMENT:
            initZ2SDeviceGeneralPurposeMeasurement(devices_counter); break; 
          
          case SUPLA_CHANNELTYPE_BINARYSENSOR: 
            initZ2SDeviceIASzone(devices_counter); break;
          
          case SUPLA_CHANNELTYPE_RELAY: 
            initZ2SDeviceVirtualRelay(&zbGateway, device, devices_counter); break;
          
          case SUPLA_CHANNELTYPE_ACTIONTRIGGER: {
            //auto Supla_VirtualRelay = new Supla::Control::VirtualRelay();
            auto Supla_VirtualRelay = new Supla::Control::VirtualRelaySceneSwitch(0xFF ^ SUPLA_BIT_FUNC_CONTROLLINGTHEROLLERSHUTTER, 1000);
            Supla_VirtualRelay->setInitialCaption(z2s_devices_table[devices_counter].Supla_channel_name);
            Supla_VirtualRelay->setDefaultFunction(z2s_devices_table[devices_counter].Supla_channel_func);
            Supla_VirtualRelay->getChannel()->setChannelNumber(z2s_devices_table[devices_counter].Supla_channel);
          } break;
          case SUPLA_CHANNELTYPE_ELECTRICITY_METER: 
            initZ2SDeviceElectricityMeter(&zbGateway, device, devices_counter); break;
          
          case SUPLA_CHANNELTYPE_HVAC: initZ2SDeviceTuyaHvac(&zbGateway, device, z2s_devices_table[devices_counter].Supla_channel); break;
          //case SUPLA_CHANNELTYPE_DIMMERANDRGBLED: initZ2SDeviceDimmer(&zbGateway, device, z2s_devices_table[devices_counter].Supla_channel); break;
          
          case SUPLA_CHANNELTYPE_DIMMER: initZ2SDeviceDimmer(&zbGateway, device, devices_counter); break;
          
          case SUPLA_CHANNELTYPE_RGBLEDCONTROLLER: initZ2SDeviceRGB(&zbGateway, device, z2s_devices_table[devices_counter].Supla_channel); break;
          
          case SUPLA_CHANNELTYPE_DIMMERANDRGBLED: initZ2SDeviceRGBW(&zbGateway, device, z2s_devices_table[devices_counter].Supla_channel); break;
          
          default: {
            log_i("Can't create channel for %d channel type", z2s_devices_table[devices_counter].Supla_channel_type);
          } break;
          //case 
        }
      }
  }
  free(device);
}

void Z2S_onTemperatureReceive(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, float temperature, signed char rssi) {

  log_i("onTemperatureReceive 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x, endpoint 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
   ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint);
  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR, NO_CUSTOM_CMD_SID);
  if (channel_number_slot < 0)
    log_i("No channel found for address %s", ieee_addr);
  else
    msgZ2SDeviceTempHumidityTemp(channel_number_slot, temperature, rssi);
}

void Z2S_onHumidityReceive(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, float humidity, signed char rssi) {

  log_i("onHumidityReceive 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x, endpoint 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
   ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint);
  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR, NO_CUSTOM_CMD_SID);
  if (channel_number_slot < 0)
    log_i("No channel found for address %s", ieee_addr);
  else
    msgZ2SDeviceTempHumidityHumi(channel_number_slot, humidity, rssi);
}

void Z2S_onIlluminanceReceive(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, uint16_t illuminance, signed char rssi) {

  log_i("onIlluminanceReceive 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x, endpoint 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
   ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint);
  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_GENERAL_PURPOSE_MEASUREMENT, NO_CUSTOM_CMD_SID);
  if (channel_number_slot < 0)
    log_i("No channel found for address %s", ieee_addr);
  else
  msgZ2SDeviceGeneralPurposeMeasurement(channel_number_slot, illuminance, rssi);
}

void Z2S_onOnOffReceive(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, bool state, signed char rssi) {

  log_i("onOnOffReceive 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x, endpoint 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
   ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint);

  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_RELAY, NO_CUSTOM_CMD_SID);
  if (channel_number_slot >= 0) {
    msgZ2SDeviceVirtualRelay(channel_number_slot, state, rssi);
    return;
  }

  channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_BINARYSENSOR, NO_CUSTOM_CMD_SID);
  if (channel_number_slot >= 0) {
    msgZ2SDeviceIASzone(channel_number_slot, !state, rssi); //AQARA magnet
    return;
  }

  channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_DIMMER, NO_CUSTOM_CMD_SID);
  if (channel_number_slot >= 0) {
    msgZ2SDeviceDimmer(channel_number_slot, -1, state, rssi);
    return;
  }
  
  channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_RGBLEDCONTROLLER, NO_CUSTOM_CMD_SID);
  if (channel_number_slot >= 0) {
    msgZ2SDeviceRGB(z2s_devices_table[channel_number_slot].model_id, z2s_devices_table[channel_number_slot].Supla_channel, 0xFF, 0xFF, state, rssi);
    return;
  }
  
  channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_DIMMERANDRGBLED, NO_CUSTOM_CMD_SID);
  if (channel_number_slot >= 0) {
    msgZ2SDeviceRGBW(z2s_devices_table[channel_number_slot].model_id, z2s_devices_table[channel_number_slot].Supla_channel, 0xFF, 0xFF, 0xFFFF, state, rssi);
    return;
  }
  log_i("No channel found for address %s", ieee_addr);
}

void Z2S_onRMSVoltageReceive(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, uint16_t voltage, signed char rssi) {

  log_i("onRMSVoltageReceive 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x, endpoint 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
   ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint);
  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_ELECTRICITY_METER, NO_CUSTOM_CMD_SID);
  if (channel_number_slot < 0)
    log_i("No channel found for address %s", ieee_addr);
  else
    msgZ2SDeviceElectricityMeter(channel_number_slot, Z2S_EM_VOLTAGE_SEL, voltage, rssi);
}

void Z2S_onRMSCurrentReceive(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, uint16_t current, signed char rssi) {

  log_i("onRMSCurrentReceive 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x, endpoint 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
   ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint);
  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_ELECTRICITY_METER, NO_CUSTOM_CMD_SID);
  if (channel_number_slot < 0)
    log_i("No channel found for address %s", ieee_addr);
  else
    msgZ2SDeviceElectricityMeter(channel_number_slot, Z2S_EM_CURRENT_SEL, current, rssi);
}

void Z2S_onRMSActivePowerReceive(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, uint16_t active_power, signed char rssi) {

  log_i("onRMSVoltageReceive 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x, endpoint 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
   ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint);
  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_ELECTRICITY_METER, NO_CUSTOM_CMD_SID);
  if (channel_number_slot < 0)
    log_i("No channel found for address %s", ieee_addr);
  else {
    msgZ2SDeviceElectricityMeter(z2s_devices_table[channel_number_slot].Supla_channel, Z2S_EM_ACTIVE_POWER_SEL, active_power, rssi);
  }
}

void Z2S_onCurrentSummationReceive(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, uint64_t active_fwd_energy, signed char rssi) {

  log_i("onCurrentSummationReceive 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x, endpoint 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
   ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint);
  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_ELECTRICITY_METER, NO_CUSTOM_CMD_SID);
  if (channel_number_slot < 0)
    log_i("No channel found for address %s", ieee_addr);
  else
    msgZ2SDeviceElectricityMeter(channel_number_slot, Z2S_ACT_FWD_ENERGY_SEL, active_fwd_energy, rssi);
}

void Z2S_onCurrentLevelReceive(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, uint8_t level, signed char rssi) {

  log_i("onCurrentLevelReceive 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x, endpoint 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
   ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint);

  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_DIMMER, NO_CUSTOM_CMD_SID);
  if (channel_number_slot >= 0) {
    msgZ2SDeviceDimmer(channel_number_slot, level, true, rssi);
    return;
  }

  /*channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_DIMMERANDRGBLED, NO_CUSTOM_CMD_SID);
  if (channel_number_slot >= 0) {
    msgZ2SDeviceRGBW(z2s_devices_table[channel_number_slot].model_id, z2s_devices_table[channel_number_slot].Supla_channel, 0xFF, 0xFF, level, true);
    return;
  }*/
  log_i("No channel found for address %s", ieee_addr);
}

void Z2S_onColorHueReceive(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, uint8_t hue, signed char rssi) {

  log_i("onColorHueReceive 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x, endpoint 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
        ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint);

  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_RGBLEDCONTROLLER, NO_CUSTOM_CMD_SID);
  if (channel_number_slot >= 0) {
    msgZ2SDeviceRGB(z2s_devices_table[channel_number_slot].model_id, z2s_devices_table[channel_number_slot].Supla_channel, hue, 0xFF, true, rssi);
    return;
  }

  /*channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_DIMMERANDRGBLED, NO_CUSTOM_CMD_SID);
  if (channel_number_slot >= 0) {
    msgZ2SDeviceRGBW(z2s_devices_table[channel_number_slot].model_id, z2s_devices_table[channel_number_slot].Supla_channel, hue, 0xFF, 0xFFFF, true);
    return;
  }*/
  log_i("No channel found for address %s", ieee_addr);
}

void Z2S_onColorSaturationReceive(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, uint8_t saturation, signed char rssi) {

  log_i("onColorHueReceive 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x, endpoint 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
        ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint);
  
  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_RGBLEDCONTROLLER, NO_CUSTOM_CMD_SID);
  if (channel_number_slot >= 0) {
    msgZ2SDeviceRGB(z2s_devices_table[channel_number_slot].model_id, z2s_devices_table[channel_number_slot].Supla_channel, 0xFF, saturation,true, rssi);
    return;
  }
  
  /*channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_DIMMERANDRGBLED, NO_CUSTOM_CMD_SID);
  if (channel_number_slot >= 0) {
    msgZ2SDeviceRGBW(z2s_devices_table[channel_number_slot].model_id, z2s_devices_table[channel_number_slot].Supla_channel, 0xFF, saturation, 0xFFFF, true);
    return;
  }*/
  log_i("No channel found for address %s", ieee_addr);
}

void Z2S_onBatteryPercentageReceive(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, uint8_t battery_remaining) {

  log_i("onBatteryPercentageReceive 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x, endpoint 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
   ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint);
  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, ALL_SUPLA_CHANNEL_TYPES, NO_CUSTOM_CMD_SID);
  if (channel_number_slot < 0)
    log_i("No channel found for address %s", ieee_addr);
  else
  while (channel_number_slot >= 0)
  {
    auto element = Supla::Element::getElementByChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
    if (element) element->getChannel()->setBatteryLevel(battery_remaining);
    channel_number_slot = Z2S_findChannelNumberNextSlot(channel_number_slot, ieee_addr, endpoint, cluster, ALL_SUPLA_CHANNEL_TYPES, NO_CUSTOM_CMD_SID);
  }
}

void Z2S_onIASzoneStatusChangeNotification(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, int iaszone_status, signed char rssi) {
  
int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_BINARYSENSOR, NO_CUSTOM_CMD_SID);
  if (channel_number_slot < 0)
    log_i("No channel found for address %s", ieee_addr);
  else
    msgZ2SDeviceIASzone(channel_number_slot, (iaszone_status == 0), rssi);
}

void Z2S_onOnOffCustomCmdReceive( esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint8_t command_id, uint8_t command_data, signed char rssi) {
  
  log_i("Z2S_onOnOffCustomCmdReceive command id 0x%x, command data 0x%x", command_id, command_data);
  if ((command_id == TUYA_ON_OFF_CUSTOM_CMD_BUTTON_PRESS_ID) || (command_id == TUYA_ON_OFF_CUSTOM_CMD_BUTTON_ROTATE_ID)) {

    int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, ESP_ZB_ZCL_CLUSTER_ID_ON_OFF, 
                                                            SUPLA_CHANNELTYPE_ACTIONTRIGGER, command_data);
    if (channel_number_slot < 0)
      log_i("No channel found for address %s", ieee_addr);
    else {
      log_i("z2s_devices_table[channel_number_slot].Supla_channel 0x%x", z2s_devices_table[channel_number_slot].Supla_channel);
      auto element = Supla::Element::getElementByChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
      if (element) log_i("element->getChannel()->getChannelType() 0x%x", element->getChannel()->getChannelType());
      else log_i("element not found");
      if (element) { //(element != nullptr && element->getChannel()->getChannelType() == SUPLA_CHANNELTYPE_ACTIONTRIGGER) {
        log_i("trying to toggle");
      auto Supla_VirtualRelay = reinterpret_cast<Supla::Control::VirtualRelaySceneSwitch *>(element);
      Supla_VirtualRelay->toggle();
      Supla_VirtualRelay->getChannel()->setBridgeSignalStrength(Supla::rssiToSignalStrength(rssi));
    }
  }
}
}

bool Z2S_onCustomCmdReceive( esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster_id, uint8_t command_id, uint8_t buffer_size, uint8_t *buffer, signed char  rssi){
  log_i("Z2S_onCustomCmdReceive cluster 0x%x, command id 0x%x, rssi: %d", cluster_id, command_id, rssi);
  //if (cluster_id == ESP_ZB_ZCL_CLUSTER_ID_BASIC) return true;
  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, ESP_ZB_ZCL_CLUSTER_ID_ON_OFF, 
                                                            SUPLA_CHANNELTYPE_ACTIONTRIGGER, NO_CUSTOM_CMD_SID);
  if (channel_number_slot < 0) {
    log_i("No channel found for address %s", ieee_addr);
    return false;
  }
  log_i("z2s_devices_table[channel_number_slot].Supla_channel 0x%x", z2s_devices_table[channel_number_slot].Supla_channel);

  switch (z2s_devices_table[channel_number_slot].model_id) {
    case Z2S_DEVICE_DESC_IKEA_SMART_BUTTON: {
      log_i("IKEA command: cluster(0x%x), command id(0x%x), ", cluster_id, command_id);
      uint8_t sub_id = 0xFF;
      if ((cluster_id == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF) && (command_id == 0x01)) {
        if ((Styrbar_ignore_button_1) && (millis() - Styrbar_timer < 1500)) return true;
        else {
          sub_id = IKEA_CUSTOM_CMD_BUTTON_1_PRESSED_SID;
          Styrbar_ignore_button_1 = false;
        }
      }
      else if ((cluster_id == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF) && (command_id == 0x00))
        sub_id = IKEA_CUSTOM_CMD_BUTTON_2_PRESSED_SID;
      else if ((cluster_id == ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL) && (command_id == 0x05))
        sub_id = IKEA_CUSTOM_CMD_BUTTON_1_HELD_SID;
      else if ((cluster_id == ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL) && (command_id == 0x01) && ((*(int32_t *)buffer) == 0x00005301))
        sub_id = IKEA_CUSTOM_CMD_BUTTON_2_HELD_SID;
      else if ((cluster_id == ESP_ZB_ZCL_CLUSTER_ID_SCENES) && (command_id == 0x09) && ((*(int16_t *)buffer) == 0x0000)) {
        Styrbar_ignore_button_1 = true;
        Styrbar_timer = millis();
        return true; //
      }
      else if ((cluster_id == ESP_ZB_ZCL_CLUSTER_ID_SCENES) && (command_id == 0x08) && (*(buffer) == 0x01))
        sub_id = IKEA_CUSTOM_CMD_BUTTON_3_HELD_SID;
      else if ((cluster_id == ESP_ZB_ZCL_CLUSTER_ID_SCENES) && (command_id == 0x08) && (*(buffer) == 0x00))
        sub_id = IKEA_CUSTOM_CMD_BUTTON_4_HELD_SID;
      else if ((cluster_id == ESP_ZB_ZCL_CLUSTER_ID_SCENES) && (command_id == 0x07)) {
        if ((*(int32_t *)buffer) == 0x000D0101)
          sub_id = IKEA_CUSTOM_CMD_BUTTON_3_PRESSED_SID;
        else if ((*(int32_t *)buffer) == 0x000D0100)
          sub_id = IKEA_CUSTOM_CMD_BUTTON_4_PRESSED_SID;
      }  
      if (sub_id == 0xFF) return false;

      channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, ESP_ZB_ZCL_CLUSTER_ID_ON_OFF, 
                                                            SUPLA_CHANNELTYPE_ACTIONTRIGGER, sub_id);
      if (channel_number_slot < 0)
        log_i("No IKEA device channel found for address %s", ieee_addr);
      else {
        log_i("IKEA device custom command Supla channel 0x%x", z2s_devices_table[channel_number_slot].Supla_channel);
        auto element = Supla::Element::getElementByChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
        if (element) log_i("element->getChannel()->getChannelType() 0x%x", element->getChannel()->getChannelType());
        else log_i("element not found");
        if (element) { //(element != nullptr && element->getChannel()->getChannelType() == SUPLA_CHANNELTYPE_ACTIONTRIGGER) {
          log_i("trying to toggle");
          auto Supla_VirtualRelay = reinterpret_cast<Supla::Control::VirtualRelaySceneSwitch *>(element);
          Supla_VirtualRelay->toggle();
          Supla_VirtualRelay->getChannel()->setBridgeSignalStrength(Supla::rssiToSignalStrength(rssi));
        }
      }
      return true;
    } break;
    case Z2S_DEVICE_DESC_TUYA_SWITCH_4X3:
    case Z2S_DEVICE_DESC_TUYA_SMART_BUTTON_5F:
    case Z2S_DEVICE_DESC_TUYA_SMART_BUTTON_3F:
    case Z2S_DEVICE_DESC_TUYA_SMART_BUTTON_2F: {
      log_i("TUYA command: cluster(0x%x), command id(0x%x)" ,  cluster_id, command_id);   
      //process Tuya command
      if ((cluster_id == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF) &&
        ((command_id == TUYA_ON_OFF_CUSTOM_CMD_BUTTON_PRESS_ID) || (command_id == TUYA_ON_OFF_CUSTOM_CMD_BUTTON_ROTATE_ID))) {

        int8_t sub_id = (command_id == TUYA_ON_OFF_CUSTOM_CMD_BUTTON_ROTATE_ID) ? TUYA_CUSTOM_CMD_BUTTON_ROTATE_RIGHT_SID + (*buffer) : (*buffer);
        channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, ESP_ZB_ZCL_CLUSTER_ID_ON_OFF, 
                                                            SUPLA_CHANNELTYPE_ACTIONTRIGGER, sub_id);
        if (channel_number_slot < 0)
          log_i("No Tuya device channel found for address %s", ieee_addr);
        else {
        log_i("Tuya device custom command Supla channel 0x%x", z2s_devices_table[channel_number_slot].Supla_channel);
        auto element = Supla::Element::getElementByChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
        if (element) log_i("element->getChannel()->getChannelType() 0x%x", element->getChannel()->getChannelType());
        else log_i("element not found");
        if (element) { //(element != nullptr && element->getChannel()->getChannelType() == SUPLA_CHANNELTYPE_ACTIONTRIGGER) {
          log_i("trying to toggle");
          auto Supla_VirtualRelay = reinterpret_cast<Supla::Control::VirtualRelaySceneSwitch *>(element);
          Supla_VirtualRelay->toggle();
          Supla_VirtualRelay->getChannel()->setBridgeSignalStrength(Supla::rssiToSignalStrength(rssi));
        }
      }
      return true;
    }
  } break;
      return false; //true;
  }
  return false;
}

void Z2S_onCmdCustomClusterReceive( esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, uint8_t command_id,
                                     uint16_t payload_size, uint8_t *payload, signed char rssi) {

  processTuyaCustomCluster(ieee_addr, endpoint, command_id, payload_size, payload, rssi);
}

void Z2S_onBTCBoundDevice(zbg_device_params_t *device) {
  log_i("BTC bound device(0x%x) on endpoint(0x%x), cluster id(0x%x)", device->short_addr, device->endpoint, device->cluster_id );
  int16_t channel_number_slot = Z2S_findChannelNumberSlot(device->ieee_addr, device->endpoint, device->cluster_id, ALL_SUPLA_CHANNEL_TYPES, NO_CUSTOM_CMD_SID);
  if (channel_number_slot < 0)
    log_i("No channel found for address %s", device->ieee_addr);
  else
  while (channel_number_slot >= 0)
  {
    device->model_id = z2s_devices_table[channel_number_slot].model_id;
    device->user_data = z2s_devices_table[channel_number_slot].Supla_channel;
  
    channel_number_slot = Z2S_findChannelNumberNextSlot(channel_number_slot, device->ieee_addr, device->endpoint, device->cluster_id, ALL_SUPLA_CHANNEL_TYPES, NO_CUSTOM_CMD_SID);
  } 
}


void Z2S_onBoundDevice(zbg_device_params_t *device, bool last_cluster) {
}

uint8_t Z2S_addZ2SDevice(zbg_device_params_t *device, int8_t sub_id) {

  int16_t channel_number_slot = Z2S_findChannelNumberSlot(device->ieee_addr, device->endpoint, device->cluster_id, ALL_SUPLA_CHANNEL_TYPES, sub_id);
  
  if (channel_number_slot < 0) {
    log_i("No channel found for address %s, adding new one", device->ieee_addr);
    
    uint8_t first_free_slot = Z2S_findFirstFreeDevicesTableSlot();
    
    if (first_free_slot == 0xFF) {
        log_i("Devices table full");
        return ADD_Z2S_DEVICE_STATUS_DT_FULL;
    }
    log_i("model id %d, first free slot %d", device->model_id, first_free_slot);
    
    switch (device->model_id) {
      case 0x0000: break;
      
      case Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR:
      case Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR_1:
      case Z2S_DEVICE_DESC_TUYA_SOIL_TEMPHUMIDITY_SENSOR:
      case Z2S_DEVICE_DESC_TUYA_TEMPHUMIDITY_SENSOR: 
        addZ2SDeviceTempHumidity(device, first_free_slot); break;

      case Z2S_DEVICE_DESC_IAS_ZONE_SENSOR: 
      case Z2S_DEVICE_DESC_LUMI_MAGNET_SENSOR: addZ2SDeviceIASzone(device, first_free_slot); break;

      case Z2S_DEVICE_DESC_RELAY:
      case Z2S_DEVICE_DESC_RELAY_1: addZ2SDeviceVirtualRelay( &zbGateway,device, first_free_slot, "POWER SWITCH", 
                                                              SUPLA_CHANNELFNC_POWERSWITCH); break;

      case Z2S_DEVICE_DESC_TUYA_2GANG_SWITCH_1:
      case Z2S_DEVICE_DESC_TUYA_2GANG_SWITCH_2: addZ2SDeviceVirtualRelay( &zbGateway,device, first_free_slot, "2GANG SWITCH", 
                                                                          SUPLA_CHANNELFNC_LIGHTSWITCH); break;

      case Z2S_DEVICE_DESC_ON_OFF:
      case Z2S_DEVICE_DESC_ON_OFF_1: {
        auto Supla_Z2S_VirtualRelay = new Supla::Control::VirtualRelay();
        Z2S_fillDevicesTableSlot(device, first_free_slot, Supla_Z2S_VirtualRelay->getChannelNumber(), SUPLA_CHANNELTYPE_ACTIONTRIGGER, sub_id); 
      } break;

      case Z2S_DEVICE_DESC_TUYA_SWITCH_4X3: {
        //auto Supla_Z2S_VirtualRelay = new Supla::Control::VirtualRelay();
        auto Supla_Z2S_VirtualRelay = new Supla::Control::VirtualRelaySceneSwitch(0xFF ^ SUPLA_BIT_FUNC_CONTROLLINGTHEROLLERSHUTTER, 1000);
        char button_name_function[30];
        char button_function[][15] = {"PRESSED", "DOUBLE PRESSED","HELD"};
        sprintf(button_name_function, "BUTTON #%d %s", device->endpoint, button_function[sub_id]);
        Z2S_fillDevicesTableSlot(device, first_free_slot, Supla_Z2S_VirtualRelay->getChannelNumber(), SUPLA_CHANNELTYPE_ACTIONTRIGGER, sub_id,
        button_name_function, SUPLA_CHANNELFNC_POWERSWITCH); 
      } break;

      case Z2S_DEVICE_DESC_TUYA_SMART_BUTTON_2F:
      case Z2S_DEVICE_DESC_TUYA_SMART_BUTTON_3F:
      case Z2S_DEVICE_DESC_TUYA_SMART_BUTTON_5F: {
        auto Supla_Z2S_VirtualRelay = new Supla::Control::VirtualRelay();
        char button_name_function[30];
        char button_function_press[][15] = {"PRESSED", "DOUBLE PRESSED","HELD"};
        char button_function_rotate[][15] = {"ROTATED RIGHT", "ROTATED LEFT"};
        if (sub_id < TUYA_CUSTOM_CMD_BUTTON_ROTATE_RIGHT_SID)
          sprintf(button_name_function, "BUTTON %s", button_function_press[sub_id]);
        else
          sprintf(button_name_function, "BUTTON %s", button_function_rotate[sub_id - TUYA_CUSTOM_CMD_BUTTON_ROTATE_RIGHT_SID]);
        Z2S_fillDevicesTableSlot(device, first_free_slot, Supla_Z2S_VirtualRelay->getChannelNumber(), SUPLA_CHANNELTYPE_ACTIONTRIGGER, sub_id,
        button_name_function, SUPLA_CHANNELFNC_POWERSWITCH); 
      } break;

      case Z2S_DEVICE_DESC_IKEA_SMART_BUTTON: {
        auto Supla_Z2S_VirtualRelay = new Supla::Control::VirtualRelay();
        char button_name_function[30];
        sprintf(button_name_function, IKEA_STYRBAR_BUTTONS[sub_id]);
        Z2S_fillDevicesTableSlot(device, first_free_slot, Supla_Z2S_VirtualRelay->getChannelNumber(), SUPLA_CHANNELTYPE_ACTIONTRIGGER, sub_id,
        button_name_function, SUPLA_CHANNELFNC_POWERSWITCH);
      } break;
      case Z2S_DEVICE_DESC_RELAY_ELECTRICITY_METER:
      case Z2S_DEVICE_DESC_RELAY_ELECTRICITY_METER_1:
      case Z2S_DEVICE_DESC_RELAY_ELECTRICITY_METER_2:
      case Z2S_DEVICE_DESC_TUYA_RELAY_ELECTRICITY_METER:
      case Z2S_DEVICE_DESC_TUYA_RELAY_ELECTRICITY_METER_1:
      case Z2S_DEVICE_DESC_TUYA_RELAY_ELECTRICITY_METER_2: {
        addZ2SDeviceVirtualRelay(&zbGateway,device, first_free_slot);
        first_free_slot = Z2S_findFirstFreeDevicesTableSlot();
        if (first_free_slot == 0xFF) {
          log_i("ERROR! Devices table full!");
          return ADD_Z2S_DEVICE_STATUS_DT_FWA;
        }
        if (device->model_id == Z2S_DEVICE_DESC_TUYA_RELAY_ELECTRICITY_METER_2)
              addZ2SDeviceElectricityMeter(&zbGateway, device, true, true, first_free_slot);
            else if (device->model_id == Z2S_DEVICE_DESC_TUYA_RELAY_ELECTRICITY_METER)
              addZ2SDeviceElectricityMeter(&zbGateway, device, true, false, first_free_slot);
            else
              addZ2SDeviceElectricityMeter(&zbGateway, device, false, false, first_free_slot);
      } break;

      case Z2S_DEVICE_DESC_TUYA_HVAC:
      case Z2S_DEVICE_DESC_TUYA_HVAC_6567C:
      case Z2S_DEVICE_DESC_TUYA_HVAC_23457:
      case Z2S_DEVICE_DESC_TUYA_HVAC_LEGACY: addZ2SDeviceTuyaHvac(&zbGateway, device, first_free_slot); break;
      
      case Z2S_DEVICE_DESC_TUYA_DIMMER_BULB: {

        addZ2SDeviceDimmer(&zbGateway,device, first_free_slot, "DIMMER BULB", SUPLA_CHANNELFNC_DIMMER); 
      } break; 

      case Z2S_DEVICE_DESC_TUYA_RGB_BULB: {

        addZ2SDeviceRGB(&zbGateway,device, first_free_slot, "RGB BULB", SUPLA_CHANNELFNC_RGBLIGHTING);
      } break;

      case Z2S_DEVICE_DESC_TUYA_RGBW_BULB: {

        addZ2SDeviceDimmer(&zbGateway,device, first_free_slot, "DIMMER", SUPLA_CHANNELFNC_DIMMER);

        first_free_slot = Z2S_findFirstFreeDevicesTableSlot();
        if (first_free_slot == 0xFF) {
          log_i("ERROR! Devices table full!");
          return ADD_Z2S_DEVICE_STATUS_DT_FWA;
        }
        addZ2SDeviceRGB(&zbGateway,device, first_free_slot, "RGB", SUPLA_CHANNELFNC_RGBLIGHTING); 
      } break; 

      case Z2S_DEVICE_DESC_TUYA_DIMMER_DOUBLE_SWITCH:
        addZ2SDeviceDimmer(&zbGateway,device, first_free_slot, sub_id, "DIMMER SWITCH", SUPLA_CHANNELFNC_DIMMER); break;

      case Z2S_DEVICE_DESC_TUYA_SMOKE_DETECTOR: {

        addZ2SDeviceIASzone(device, first_free_slot, -1, "SMOKE DETECT", SUPLA_CHANNELFNC_ALARM); break;

        first_free_slot = Z2S_findFirstFreeDevicesTableSlot();
        if (first_free_slot == 0xFF) {
          log_i("ERROR! Devices table full!");
          return ADD_Z2S_DEVICE_STATUS_DT_FWA;
        }
        addZ2SDeviceGeneralPurposeMeasurement(device, first_free_slot, "SMOKE CONC.", SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT, "ppm");
      } break;

      case Z2S_DEVICE_DESC_ILLUTEMPHUMIZONE_SENSOR: {
        
        addZ2SDeviceIASzone(device, first_free_slot, -1, "LS ZONE", SUPLA_CHANNELFNC_ALARM); break;

        first_free_slot = Z2S_findFirstFreeDevicesTableSlot();
        if (first_free_slot == 0xFF) {
          log_i("ERROR! Devices table full!");
          return ADD_Z2S_DEVICE_STATUS_DT_FWA;
        }
        addZ2SDeviceGeneralPurposeMeasurement(device, first_free_slot, "LIGHT ILLU.", SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT, "lx");
        
        first_free_slot = Z2S_findFirstFreeDevicesTableSlot();
        if (first_free_slot == 0xFF) {
          log_i("ERROR! Devices table full!");
          return ADD_Z2S_DEVICE_STATUS_DT_FWA;
        }
        addZ2SDeviceTempHumidity(device, first_free_slot);
      } break;

      case Z2S_DEVICE_DESC_ILLUZONE_SENSOR: {
        
        addZ2SDeviceIASzone(device, first_free_slot, -1, "LS ZONE", SUPLA_CHANNELFNC_ALARM); break;

        first_free_slot = Z2S_findFirstFreeDevicesTableSlot();
        if (first_free_slot == 0xFF) {
          log_i("ERROR! Devices table full!");
          return ADD_Z2S_DEVICE_STATUS_DT_FWA;
        }

        addZ2SDeviceGeneralPurposeMeasurement(device, first_free_slot, "LIGHT ILLU.", SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT, "lx");
      } break;

      case Z2S_DEVICE_DESC_TUYA_PRESENCE_SENSOR: {
        
        switch (sub_id) {
          case 0x01 : //presence
            addZ2SDeviceIASzone(device, first_free_slot, sub_id, "PRESENCE", SUPLA_CHANNELFNC_ALARM); break;

          case 0x65:  //motion_state
            addZ2SDeviceGeneralPurposeMeasurement(device, first_free_slot, "MOTION STATE", SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT, "[0..5]");
            break;
      
          case 0x6A:  //illuminance
            addZ2SDeviceGeneralPurposeMeasurement(device, first_free_slot, "ILLUMINANCE", SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT, "lx");
        }
      } break;
      
      default : {
        
        log_i("Device (0x%x), endpoint (0x%x), model (0x%x) unknown", device->short_addr, device->endpoint, device->model_id);
        return ADD_Z2S_DEVICE_STATUS_DUN;
      } break;
    }
    return ADD_Z2S_DEVICE_STATUS_OK;
  } else {
    log_i("Device (0x%x), endpoint (0x%x) already in z2s_devices_table (index 0x%x)", device->short_addr, device->endpoint, channel_number_slot);
    return ADD_Z2S_DEVICE_STATUS_DAP; 
  }
}
