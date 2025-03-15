#include "z2s_device_temphumidity.h"

void initZ2SDeviceTempHumidity(int16_t channel_number_slot) {
  
  uint8_t timeout = 0;
  if (z2s_devices_table[channel_number_slot].user_data_1 & USER_DATA_FLAG_SED_TIMEOUT == USER_DATA_FLAG_SED_TIMEOUT)
    timeout = z2s_devices_table[channel_number_slot].user_data_2;
  auto Supla_Z2S_VirtualThermHygroMeter = new Supla::Sensor::Z2S_VirtualThermHygroMeter(timeout);

  Supla_Z2S_VirtualThermHygroMeter->getChannel()->setChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
}

void addZ2SDeviceTempHumidity(zbg_device_params_t *device, uint8_t free_slot) {

  auto Supla_Z2S_VirtualThermHygroMeter = new Supla::Sensor::Z2S_VirtualThermHygroMeter();
  
  Z2S_fillDevicesTableSlot(device, free_slot, Supla_Z2S_VirtualThermHygroMeter->getChannelNumber(), SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR, -1);
}

Supla::Sensor::Z2S_VirtualThermHygroMeter* getZ2SDeviceTempHumidityPtr(uint8_t Supla_channel) {

  auto element = Supla::Element::getElementByChannelNumber(Supla_channel);
  if (element != nullptr && element->getChannel()->getChannelType() == SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR)  
    return reinterpret_cast<Supla::Sensor::Z2S_VirtualThermHygroMeter *>(element);
  else return nullptr;  
}

void msgZ2SDeviceTempHumidityTemp(int16_t channel_number_slot, double temp, signed char rssi) {

  if (channel_number_slot < 0) {
    log_e("msgZ2SDeviceTempHumidityTemp - invalid channel number slot");
    return;
  }

  auto Supla_Z2S_VirtualThermHygroMeter = getZ2SDeviceTempHumidityPtr(z2s_devices_table[channel_number_slot].Supla_channel);
  
  if (Supla_Z2S_VirtualThermHygroMeter) {
    
    Supla_Z2S_VirtualThermHygroMeter->setTemp(temp);
    Supla_Z2S_VirtualThermHygroMeter->Refresh();
  }
}

void msgZ2SDeviceTempHumidityHumi(int16_t channel_number_slot, double humi, signed char rssi) {

  if (channel_number_slot < 0) {
    log_e("msgZ2SDeviceTempHumidityHumi - invalid channel number slot");
    return;
  }

  auto Supla_Z2S_VirtualThermHygroMeter = getZ2SDeviceTempHumidityPtr(z2s_devices_table[channel_number_slot].Supla_channel);
  
  if (Supla_Z2S_VirtualThermHygroMeter) {
    
    Supla_Z2S_VirtualThermHygroMeter->setHumi(humi);
    Supla_Z2S_VirtualThermHygroMeter->Refresh();
  }
}

void msgZ2SDeviceTempHumidityBatteryLevel(int16_t channel_number_slot, uint8_t battery_level, signed char rssi) {

  if (channel_number_slot < 0) {
    log_e("msgZ2SDeviceTempHumidityBatteryLevel - invalid channel number slot");
    return;
  }
  
  auto Supla_Z2S_VirtualThermHygroMeter = getZ2SDeviceTempHumidityPtr(z2s_devices_table[channel_number_slot].Supla_channel);
  
  if (Supla_Z2S_VirtualThermHygroMeter) {
    
    Supla_Z2S_VirtualThermHygroMeter->getChannel()->setBatteryLevel(battery_level);
    Supla_Z2S_VirtualThermHygroMeter->Refresh();
  }
}
