#include <Arduino.h>
#include "ZigbeeGateway.h"
#include "esp_coexist.h"
#include <map>
#include "supla/sensor/general_purpose_measurement.h"
#include "supla/control/virtual_relay.h"
#include "SuplaDevice.h"
#include <string>
#include <supla/storage/eeprom.h>
#include <supla/storage/littlefs_config.h>
#include <supla/network/esp_wifi.h>
#include "priv_auth_data.h"
#include <supla/sensor/virtual_therm_hygro_meter.h>
#include <Z2S_virtual_relay.h>
#include <supla/sensor/one_phase_electricity_meter.h>
#include <supla/sensor/virtual_binary.h>
#include <Z2S_devices_database.h>
#include <supla/device/supla_ca_cert.h>
#define GATEWAY_ENDPOINT_NUMBER 1

#define BUTTON_PIN                  9  //Boot button for C6/H2

typedef struct z2s_device_params_s {

  bool valid_record;
  uint32_t model_id;
  esp_zb_ieee_addr_t ieee_addr;
  uint8_t endpoint;
  uint16_t cluster_id;
  uint16_t short_addr;
  uint8_t Supla_channel;
  int32_t Supla_channel_type;
} z2s_device_params_t;

static z2s_device_params_t z2s_devices_table[SUPLA_CHANNELMAXCOUNT];


ZigbeeGateway zbGateway = ZigbeeGateway(GATEWAY_ENDPOINT_NUMBER);

Supla::Eeprom             eeprom;
Supla::ESPWifi            wifi(SUPLA_WIFI_SSID, SUPLA_WIFI_SSID);
Supla::LittleFsConfig     configSupla;

uint32_t startTime = 0;
uint32_t printTime = 0;
uint32_t zbInit_delay = 0;

bool zbInit = true;

const static char   Z2S_DEVICES_TABLE []  PROGMEM = "Z2S_devs_table";
const static char   Z2S_DEVICES_TABLE_SIZE []  PROGMEM = "Z2S_devs_ts";

uint32_t Z2S_getDevicesTableSize() {
  uint32_t _z2s_devices_table_size;
  if (Supla::Storage::ConfigInstance()->getUInt32(Z2S_DEVICES_TABLE_SIZE, &_z2s_devices_table_size))
    return _z2s_devices_table_size;
  else
    return 0;
}

uint8_t Z2S_findFirstFreeDevicesTableSlot() {

  for (uint8_t devices_counter = 0; devices_counter < SUPLA_CHANNELMAXCOUNT; devices_counter++) 
      if (!z2s_devices_table[devices_counter].valid_record)
        return devices_counter;
  return 0xFF;
  
}

uint8_t Z2S_printDevicesTableSlots() {

  for (uint8_t devices_counter = 0; devices_counter < SUPLA_CHANNELMAXCOUNT; devices_counter++) 
      log_i("valid %d, ieee addr %s, model_id 0x%x, endpoint 0x%x, cluster 0x%x, channel %d, channel type %d",
        z2s_devices_table[devices_counter].valid_record,
        z2s_devices_table[devices_counter].ieee_addr,
        z2s_devices_table[devices_counter].model_id,
        z2s_devices_table[devices_counter].endpoint,
        z2s_devices_table[devices_counter].cluster_id,
        z2s_devices_table[devices_counter].Supla_channel,
        z2s_devices_table[devices_counter].Supla_channel_type);
  return 0xFF;
  
}


int16_t Z2S_findChannelNumberSlot(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, int32_t channel_type ) {

  log_i("Z2S_findChannelNumberSlot %d:%d:%d:%d:%d:%d:%d:%d, endpoint 0x%x, channel type 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
   ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint, channel_type);
  
  for (uint8_t devices_counter = 0; devices_counter < SUPLA_CHANNELMAXCOUNT; devices_counter++) {
      if (z2s_devices_table[devices_counter].valid_record)
        if ((memcmp(z2s_devices_table[devices_counter].ieee_addr, ieee_addr,8) == 0) && 
        (z2s_devices_table[devices_counter].endpoint == endpoint) &&
        ((channel_type < 0) || (z2s_devices_table[devices_counter].Supla_channel_type == channel_type))) { 
        //&& (z2s_devices_table[devices_counter].cluster_id = cluster)) {
            return devices_counter;
        }

  }  
  return -1;
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

void Z2S_fillDevicesTableSlot(zb_device_params_t *device, uint8_t slot, uint8_t channel, int32_t channel_type) {

  z2s_devices_table[slot].valid_record = true;
  memcpy(z2s_devices_table[slot].ieee_addr,device->ieee_addr,8);
  z2s_devices_table[slot].model_id = device->model_id;
  z2s_devices_table[slot].endpoint = device->endpoint;
  z2s_devices_table[slot].cluster_id = device->cluster_id;
  z2s_devices_table[slot].Supla_channel = channel;
  z2s_devices_table[slot].Supla_channel_type = channel_type; 
  
  Z2S_saveDevicesTable();
  Z2S_printDevicesTableSlots();
}

bool Z2S_loadDevicesTable() {

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
        } else
          return true;
    }
  }
}



void Z2S_initSuplaChannels(){

  for (uint8_t devices_counter = 0; devices_counter < SUPLA_CHANNELMAXCOUNT; devices_counter++) {
      if (z2s_devices_table[devices_counter].valid_record) 
        switch (z2s_devices_table[devices_counter].Supla_channel_type) {
          case SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR: {
            auto Supla_VirtualThermHygroMeter = new Supla::Sensor::VirtualThermHygroMeter();
            Supla_VirtualThermHygroMeter->getChannel()->setChannelNumber(z2s_devices_table[devices_counter].Supla_channel);
            break;
          }
          case SUPLA_CHANNELTYPE_BINARYSENSOR: {
            auto Supla_VirtualBinary = new Supla::Sensor::VirtualBinary();
            Supla_VirtualBinary->getChannel()->setChannelNumber(z2s_devices_table[devices_counter].Supla_channel);
            break;
          }
          case SUPLA_CHANNELTYPE_RELAY: {
            auto Supla_Z2S_VirtualRelay = new Supla::Control::Z2S_VirtualRelay(&zbGateway,z2s_devices_table[devices_counter].ieee_addr );
            Supla_Z2S_VirtualRelay->getChannel()->setChannelNumber(z2s_devices_table[devices_counter].Supla_channel);
            break;
          }
          case SUPLA_CHANNELTYPE_ELECTRICITY_METER: {
            auto Supla_Z2S_OnePhaseElectricityMeter = new Supla::Sensor::OnePhaseElectricityMeter();
            Supla_Z2S_OnePhaseElectricityMeter->getChannel()->setChannelNumber(z2s_devices_table[devices_counter].Supla_channel);
            break;
          }
          default: {
            log_i("Can't create channel for %d channel type", z2s_devices_table[devices_counter].Supla_channel_type);
            break;
          }
          
        }
  }  
}

void Z2S_onTemperatureReceive(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, float temperature) {

  log_i("onTemperatureReceive %d:%d:%d:%d:%d:%d:%d:%d, endpoint 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
   ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint);
  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR);
  if (channel_number_slot < 0)
    log_i("No channel found for address %s", ieee_addr);
  else
  {
    auto element = Supla::Element::getElementByChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
    if (element != nullptr && element->getChannel()->getChannelType() == SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR) {

        auto Supla_VirtualThermHygroMeter = reinterpret_cast<Supla::Sensor::VirtualThermHygroMeter *>(element);
        Supla_VirtualThermHygroMeter->setTemp(temperature);
    }
  }
}

void Z2S_onHumidityReceive(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, float humidity) {

  log_i("onHumidityReceive %d:%d:%d:%d:%d:%d:%d:%d, endopint 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
   ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint);
  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR);
  if (channel_number_slot < 0)
    log_i("No channel found for address %s", ieee_addr);
  else
  {
    auto element = Supla::Element::getElementByChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
    if (element != nullptr && element->getChannel()->getChannelType() == SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR) {

        auto Supla_VirtualThermHygroMeter = reinterpret_cast<Supla::Sensor::VirtualThermHygroMeter *>(element);
        Supla_VirtualThermHygroMeter->setHumi(humidity);
    }
  }
}

void Z2S_onOnOffReceive(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, bool state) {

  log_i("onOnOffReceive %d:%d:%d:%d:%d:%d:%d:%d, endopint 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
   ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint);
  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_RELAY);
  if (channel_number_slot < 0)
    log_i("No channel found for address %s", ieee_addr);
  else
  {
    auto element = Supla::Element::getElementByChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
    if (element != nullptr && element->getChannel()->getChannelType() == SUPLA_CHANNELTYPE_RELAY) {

        auto Supla_Z2S_VirtualRelay = reinterpret_cast<Supla::Control::Z2S_VirtualRelay *>(element);
        Supla_Z2S_VirtualRelay->Z2S_setOnOff(state); 
    }
  }
}

void Z2S_onRMSVoltageReceive(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, uint16_t voltage) {

  log_i("onRMSVoltageReceive %d:%d:%d:%d:%d:%d:%d:%d, endopint 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
   ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint);
  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_ELECTRICITY_METER);
  if (channel_number_slot < 0)
    log_i("No channel found for address %s", ieee_addr);
  else
  {
    auto element = Supla::Element::getElementByChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
    if (element != nullptr && element->getChannel()->getChannelType() == SUPLA_CHANNELTYPE_ELECTRICITY_METER) {

        auto Supla_OnePhaseElectricityMeter = reinterpret_cast<Supla::Sensor::OnePhaseElectricityMeter *>(element);
        Supla_OnePhaseElectricityMeter->setVoltage(0, voltage * 100);
    }
  }
}

void Z2S_onRMSCurrentReceive(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, uint16_t current) {

  log_i("onRMSCurrentReceive %d:%d:%d:%d:%d:%d:%d:%d, endopint 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
   ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint);
  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_ELECTRICITY_METER);
  if (channel_number_slot < 0)
    log_i("No channel found for address %s", ieee_addr);
  else
  {
    auto element = Supla::Element::getElementByChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
    if (element != nullptr && element->getChannel()->getChannelType() == SUPLA_CHANNELTYPE_ELECTRICITY_METER) {

        auto Supla_OnePhaseElectricityMeter = reinterpret_cast<Supla::Sensor::OnePhaseElectricityMeter *>(element);
        Supla_OnePhaseElectricityMeter->setCurrent(0, current * 1000);
    }
  }
}

void Z2S_onRMSActivePowerReceive(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, uint16_t active_power) {

  log_i("onRMSVoltageReceive %d:%d:%d:%d:%d:%d:%d:%d, endopint 0x%x", ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3],
   ieee_addr[2], ieee_addr[1], ieee_addr[0], endpoint);
  int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_ELECTRICITY_METER);
  if (channel_number_slot < 0)
    log_i("No channel found for address %s", ieee_addr);
  else
  {
    auto element = Supla::Element::getElementByChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
    if (element != nullptr && element->getChannel()->getChannelType() == SUPLA_CHANNELTYPE_ELECTRICITY_METER) {

        auto Supla_OnePhaseElectricityMeter = reinterpret_cast<Supla::Sensor::OnePhaseElectricityMeter *>(element);
        Supla_OnePhaseElectricityMeter->setPowerActive(0, active_power * 100000);
    }
  }
}

void Z2S_onIASzoneStatusChangeNotification(esp_zb_ieee_addr_t ieee_addr, uint16_t endpoint, uint16_t cluster, int iaszone_status) {
  
int16_t channel_number_slot = Z2S_findChannelNumberSlot(ieee_addr, endpoint, cluster, SUPLA_CHANNELTYPE_BINARYSENSOR);
  if (channel_number_slot < 0)
    log_i("No channel found for address %s", ieee_addr);
  else
  {
    auto element = Supla::Element::getElementByChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
    if (element != nullptr && element->getChannel()->getChannelType() == SUPLA_CHANNELTYPE_BINARYSENSOR) {

        auto Supla_VirtualBinary = reinterpret_cast<Supla::Sensor::VirtualBinary *>(element);
        if (iaszone_status == 0) Supla_VirtualBinary->set();
        else Supla_VirtualBinary->clear();
    }
  }
}



void Z2S_onBoundDevice(zb_device_params_t *device, bool last_cluster) {
  
  
  Z2S_printDevicesTableSlots();

  int16_t channel_number_slot = Z2S_findChannelNumberSlot(device->ieee_addr, device->endpoint, device->cluster_id, -1);
  
  if (channel_number_slot < 0) {
    log_i("No channel found for address %s, adding new one", device->ieee_addr);
    
    uint8_t first_free_slot = Z2S_findFirstFreeDevicesTableSlot();
    
    if (first_free_slot == 0xFF) {
        log_i("Devices table full");
        return;
    }
    log_i("model id %d, first free slot %d", device->model_id, first_free_slot);
    
    switch (device->model_id) {
      case 0x0000: break;
      
      case Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR: {
        auto Supla_VirtualThermHygroMeter = new Supla::Sensor::VirtualThermHygroMeter();
        Z2S_fillDevicesTableSlot(device, first_free_slot, Supla_VirtualThermHygroMeter->getChannelNumber(), SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR);
      } break;
      case Z2S_DEVICE_DESC_IAS_ZONE_SENSOR: {
        auto Supla_VirtualBinary = new Supla::Sensor::VirtualBinary();
        Z2S_fillDevicesTableSlot(device, first_free_slot, Supla_VirtualBinary->getChannelNumber(), SUPLA_CHANNELTYPE_BINARYSENSOR); 
      } break;
      case Z2S_DEVICE_DESC_RELAY: {
        auto Supla_Z2S_VirtualRelay = new Supla::Control::Z2S_VirtualRelay(&zbGateway,device->ieee_addr);
        Z2S_fillDevicesTableSlot(device, first_free_slot, Supla_Z2S_VirtualRelay->getChannelNumber(), SUPLA_CHANNELTYPE_RELAY); 
      } break;
      case Z2S_DEVICE_DESC_RELAY_ELECTRICITY_METER: {
        auto Supla_Z2S_VirtualRelay = new Supla::Control::Z2S_VirtualRelay(&zbGateway,device->ieee_addr);
        Z2S_fillDevicesTableSlot(device, first_free_slot, Supla_Z2S_VirtualRelay->getChannelNumber(), SUPLA_CHANNELTYPE_RELAY); 
        first_free_slot = Z2S_findFirstFreeDevicesTableSlot();
        if (first_free_slot == 0xFF) {
          log_i("Devices table full");
          return;
        }
        auto Supla_Z2S_OnePhaseElectricityMeter = new Supla::Sensor::OnePhaseElectricityMeter();
        Z2S_fillDevicesTableSlot(device, first_free_slot, Supla_Z2S_OnePhaseElectricityMeter->getChannelNumber(), 
                                SUPLA_CHANNELTYPE_ELECTRICITY_METER);

      } break;
    }
  }
  else
  {
    switch (device->model_id) {
      case 0x0000: break;
      
      case Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR: {
        auto Supla_channel = Supla::Channel::GetByChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
        if (!Supla_channel) {
          auto Supla_VirtualThermHygroMeter = new Supla::Sensor::VirtualThermHygroMeter();
          Supla_VirtualThermHygroMeter->getChannel()->setChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
        }
      } break;

      case Z2S_DEVICE_DESC_IAS_ZONE_SENSOR: {
        auto Supla_channel = Supla::Channel::GetByChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
        if (!Supla_channel) {
          auto Supla_VirtualBinary = new Supla::Sensor::VirtualBinary();
          Supla_VirtualBinary->getChannel()->setChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
        }
      } break;
      
      case Z2S_DEVICE_DESC_RELAY: {
        auto Supla_channel = Supla::Channel::GetByChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
        if (!Supla_channel) {
          auto Supla_Z2S_VirtualRelay = new Supla::Control::Z2S_VirtualRelay(&zbGateway, z2s_devices_table[channel_number_slot].ieee_addr);
          Supla_Z2S_VirtualRelay->getChannel()->setChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
        }
      } break;
      
      case Z2S_DEVICE_DESC_RELAY_ELECTRICITY_METER: {
        channel_number_slot = Z2S_findChannelNumberSlot(device->ieee_addr, device->endpoint, device->cluster_id, SUPLA_CHANNELTYPE_RELAY);
        auto Supla_channel = Supla::Channel::GetByChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
        if (!Supla_channel) {
          auto Supla_Z2S_VirtualRelay = new Supla::Control::Z2S_VirtualRelay(&zbGateway, z2s_devices_table[channel_number_slot].ieee_addr);
          Supla_Z2S_VirtualRelay->getChannel()->setChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
        }
        channel_number_slot = Z2S_findChannelNumberSlot(device->ieee_addr, device->endpoint, device->cluster_id, SUPLA_CHANNELTYPE_ELECTRICITY_METER);
        Supla_channel = Supla::Channel::GetByChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
        if (!Supla_channel) {
          auto Supla_Z2S_OnePhaseElectricityMeter = new Supla::Sensor::OnePhaseElectricityMeter();
          Supla_Z2S_OnePhaseElectricityMeter->getChannel()->setChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
        }
      } break;
    }
  }
}

void Z2S_onBTCBoundDevice(zb_device_params_t *device) {

  log_i("BTC bound device 0x%x on endpoint 0x%x cluster id 0x%x", device->short_addr, device->endpoint, device->cluster_id );
  if (device->cluster_id == ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT) {
    log_i("Trying to wake up device 0x%x on endpoint 0x%x cluster id 0x%x", device->short_addr, device->endpoint, device->cluster_id );
    zbGateway.setClusterReporting(device->short_addr, device->endpoint, device->cluster_id, 
                                  ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID, ESP_ZB_ZCL_ATTR_TYPE_S16, 30, 120, 10);
  } else
  if (device->cluster_id == ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT) {
    log_i("Trying to wake up device 0x%x on endpoint 0x%x cluster id 0x%x", device->short_addr, device->endpoint, device->cluster_id );
    zbGateway.setClusterReporting(device->short_addr, device->endpoint, device->cluster_id, 
                                  ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID, ESP_ZB_ZCL_ATTR_TYPE_U16, 30, 120, 10);
  } else
  if (device->cluster_id == ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG) {
    log_i("Trying to wake up device 0x%x on endpoint 0x%x cluster id 0x%x", device->short_addr, device->endpoint, device->cluster_id );
    zbGateway.setClusterReporting(device->short_addr, device->endpoint, device->cluster_id, 
                                  0x0021, ESP_ZB_ZCL_ATTR_TYPE_U8, 30, 120, 10);
  }
  if (device->cluster_id == ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT) {
    zbGateway.sendAttributeRead(device, ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT, ESP_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_ACVOLTAGE_MULTIPLIER_ID);
    zbGateway.sendAttributeRead(device, ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT, ESP_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_ACVOLTAGE_DIVISOR_ID);
    zbGateway.sendAttributeRead(device, ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT, ESP_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_ACCURRENT_MULTIPLIER_ID);
    zbGateway.sendAttributeRead(device, ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT, ESP_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_ACCURRENT_DIVISOR_ID);
    zbGateway.sendAttributeRead(device, ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT, ESP_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_ACPOWER_MULTIPLIER_ID);
    zbGateway.sendAttributeRead(device, ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT,ESP_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_ACPOWER_DIVISOR_ID);

    log_i("Trying to wake up device 0x%x on endpoint 0x%x cluster id 0x%x", device->short_addr, device->endpoint, device->cluster_id );
    zbGateway.setClusterReporting(device->short_addr, device->endpoint, device->cluster_id, 
                                  ESP_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_RMSVOLTAGE_ID, ESP_ZB_ZCL_ATTR_TYPE_U16, 5, 5, 1);
  }if (device->cluster_id == ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT) {
    log_i("Trying to wake up device 0x%x on endpoint 0x%x cluster id 0x%x", device->short_addr, device->endpoint, device->cluster_id );
    zbGateway.setClusterReporting(device->short_addr, device->endpoint, device->cluster_id, 
                                  ESP_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_RMSCURRENT_ID, ESP_ZB_ZCL_ATTR_TYPE_U16, 5, 5, 1);
  }if (device->cluster_id == ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT) {
    log_i("Trying to wake up device 0x%x on endpoint 0x%x cluster id 0x%x", device->short_addr, device->endpoint, device->cluster_id );
    zbGateway.setClusterReporting(device->short_addr, device->endpoint, device->cluster_id, 
                                  ESP_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_ID, ESP_ZB_ZCL_ATTR_TYPE_U16, 5, 5, 1);
  }
  Z2S_onBoundDevice(device, true);
}


void setup()
{
  Serial.begin(115200);
 //pinMode(BUTTON_PIN, INPUT);

  eeprom.setStateSavePeriod(5000);

  Supla::Storage::Init();

  auto cfg = Supla::Storage::ConfigInstance();

  cfg->commit();

  cfg->setGUID(GUID);
  cfg->setAuthKey(AUTHKEY);
  cfg->setWiFiSSID(SUPLA_WIFI_SSID);
  cfg->setWiFiPassword(SUPLA_WIFI_PASS);
  cfg->setSuplaServer(SUPLA_SVR);
  cfg->setEmail(SUPLA_EMAIL);

  Z2S_loadDevicesTable();

  Z2S_initSuplaChannels();

  //  Zigbee Gateway notifications

  zbGateway.onTemperatureReceive(Z2S_onTemperatureReceive);
  zbGateway.onHumidityReceive(Z2S_onHumidityReceive);
  zbGateway.onOnOffReceive(Z2S_onOnOffReceive);
  zbGateway.onRMSVoltageReceive(Z2S_onRMSVoltageReceive);
  zbGateway.onRMSCurrentReceive(Z2S_onRMSCurrentReceive);
  zbGateway.onRMSActivePowerReceive(Z2S_onRMSActivePowerReceive);

  zbGateway.onIASzoneStatusChangeNotification(Z2S_onIASzoneStatusChangeNotification);

  zbGateway.onBoundDevice(Z2S_onBoundDevice);
  zbGateway.onBTCBoundDevice(Z2S_onBTCBoundDevice);

  zbGateway.setManufacturerAndModel("Supla", "Z2SGateway");
  zbGateway.allowMultipleBinding(true);

  Zigbee.addEndpoint(&zbGateway);

  //Open network for 180 seconds after boot
  Zigbee.setRebootOpenNetwork(ZG_OPEN_NETWORK);

  //Supla
  
  SuplaDevice.setSuplaCACert(suplaCACert);
  SuplaDevice.setSupla3rdPartyCACert(supla3rdCACert);
  
  SuplaDevice.setName("Zigbee to Supla");
  //wifi.enableSSL(true);

  SuplaDevice.begin(GUID,             // Global Unique Identifier 
                    SUPLA_SVR,        // SUPLA server address
                    SUPLA_EMAIL,      // Email address used to login to Supla Cloud
                    AUTHKEY);         // Authorization key
  
  startTime = millis();
  printTime = millis();
  zbInit_delay = millis();
}

zb_device_params_t *gateway_device;
zb_device_params_t *joined_device;

char zbd_model_name[32];
char zbd_manuf_name[32];


void loop() 
{
  
  SuplaDevice.iterate();

  if (millis() - printTime > 10000) {

    printTime = millis();
  }

  if (zbInit && wifi.isReady()) {
    Serial.println("zbInit");
    
    esp_coex_wifi_i154_enable();
  
    if (!Zigbee.begin(ZIGBEE_COORDINATOR)) {
      Serial.println("Zigbee failed to start!");
      Serial.println("Rebooting...");
      ESP.restart();
    }
    zbInit = false;
    startTime = millis();
 }
  
  if (digitalRead(BUTTON_PIN) == LOW) {  // Push button pressed
    // Key debounce handling
    delay(100);
    
    while (digitalRead(BUTTON_PIN) == LOW) {
      delay(50);
      if ((millis() - startTime) > 5000) {
        // If key pressed for more than 5 secs, factory reset Zigbee and reboot
        Serial.printf("Resetting Zigbee to factory settings, reboot.\n");
        Zigbee.factoryReset();
      }
    }
    Zigbee.openNetwork(180);
  }
  delay(100);

  if (zbGateway.isNewDeviceJoined()) {

    zbGateway.clearNewDeviceJoined();
    zbGateway.printJoinedDevices();

    while (!zbGateway.getJoinedDevices().empty())
    {
      joined_device = zbGateway.getLastJoinedDevice();
      
      strcpy(zbd_manuf_name,zbGateway.readManufacturer(joined_device->endpoint, joined_device->short_addr, joined_device->ieee_addr));
      log_i("manufacturer %s ", zbd_manuf_name);
      strcpy(zbd_model_name,zbGateway.readModel(joined_device->endpoint, joined_device->short_addr, joined_device->ieee_addr));
      log_i("model %s ", zbd_model_name);

      uint16_t devices_list_table_size = sizeof(Z2S_DEVICES_LIST)/sizeof(Z2S_DEVICES_LIST[0]);
      uint16_t devices_desc_table_size = sizeof(Z2S_DEVICES_DESC)/sizeof(Z2S_DEVICES_DESC[0]);
      bool device_recognized = false;

          for (int i = 0; i < devices_list_table_size; i++) {
            if ((strcmp(zbd_model_name, Z2S_DEVICES_LIST[i].model_name) == 0) &&
            (strcmp(zbd_manuf_name, Z2S_DEVICES_LIST[i].manufacturer_name) == 0)) {
              log_i("LIST matched %s::%s, entry # %d",Z2S_DEVICES_LIST[i].manufacturer_name, Z2S_DEVICES_LIST[i].model_name, i);
              for (int j = 0; j < devices_desc_table_size; j++) {
                if (Z2S_DEVICES_LIST[i].z2s_device_desc_id == Z2S_DEVICES_DESC[j].z2s_device_desc_id) {
                  log_i("DESC matched 0x%x, %d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x ",
                        Z2S_DEVICES_DESC[j].z2s_device_desc_id,   
                        Z2S_DEVICES_DESC[j].z2s_device_clusters_count,
                        Z2S_DEVICES_DESC[j].z2s_device_clusters[0],
                        Z2S_DEVICES_DESC[j].z2s_device_clusters[1],
                        Z2S_DEVICES_DESC[j].z2s_device_clusters[2],
                        Z2S_DEVICES_DESC[j].z2s_device_clusters[3],
                        Z2S_DEVICES_DESC[j].z2s_device_clusters[4],
                        Z2S_DEVICES_DESC[j].z2s_device_clusters[5],
                        Z2S_DEVICES_DESC[j].z2s_device_clusters[6],
                        Z2S_DEVICES_DESC[j].z2s_device_clusters[7]);

                        device_recognized = true;

                        esp_zb_lock_acquire(portMAX_DELAY);
                        joined_device->model_id = Z2S_DEVICES_DESC[j].z2s_device_desc_id;
                        zbGateway.setClusters2Bind(Z2S_DEVICES_DESC[j].z2s_device_clusters_count);
                        for (int k = 0; k <Z2S_DEVICES_DESC[j].z2s_device_clusters_count; k++)
                          zbGateway.bindDeviceCluster(joined_device, Z2S_DEVICES_DESC[j].z2s_device_clusters[k]);
                        esp_zb_lock_release();
                }  
                else 
                  log_i("DESC checking 0x%x, %d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x ",
                        Z2S_DEVICES_DESC[j].z2s_device_desc_id,   
                        Z2S_DEVICES_DESC[j].z2s_device_clusters_count,
                        Z2S_DEVICES_DESC[j].z2s_device_clusters[0],
                        Z2S_DEVICES_DESC[j].z2s_device_clusters[1],
                        Z2S_DEVICES_DESC[j].z2s_device_clusters[2],
                        Z2S_DEVICES_DESC[j].z2s_device_clusters[3],
                        Z2S_DEVICES_DESC[j].z2s_device_clusters[4],
                        Z2S_DEVICES_DESC[j].z2s_device_clusters[5],
                        Z2S_DEVICES_DESC[j].z2s_device_clusters[6],
                        Z2S_DEVICES_DESC[j].z2s_device_clusters[7]);        
              }
            }
            else log_i("LIST checking %s::%s, entry # %d",Z2S_DEVICES_LIST[i].manufacturer_name, Z2S_DEVICES_LIST[i].model_name, i);
          }
      if (!device_recognized) log_d("Unknown model %s::%s, no binding is possible", zbd_manuf_name, zbd_model_name);
    }
  }
}