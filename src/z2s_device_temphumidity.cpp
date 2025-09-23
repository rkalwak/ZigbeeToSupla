#include "z2s_device_temphumidity.h"

void initZ2SDeviceTempHumidity(int16_t channel_number_slot, bool thermhygrometer) {
  
  if (thermhygrometer) {

    auto Supla_Z2S_VirtualThermHygroMeter = new Supla::Sensor::Z2S_VirtualThermHygroMeter();

    Supla_Z2S_VirtualThermHygroMeter->getChannel()->setChannelNumber(z2s_channels_table[channel_number_slot].Supla_channel);

    if (strlen(z2s_channels_table[channel_number_slot].Supla_channel_name) > 0) 
      Supla_Z2S_VirtualThermHygroMeter->setInitialCaption(z2s_channels_table[channel_number_slot].Supla_channel_name);
  
    if (z2s_channels_table[channel_number_slot].Supla_channel_func != 0) 
      Supla_Z2S_VirtualThermHygroMeter->setDefaultFunction(z2s_channels_table[channel_number_slot].Supla_channel_func);

    if (z2s_channels_table[channel_number_slot].user_data_flags & USER_DATA_FLAG_CORRECTIONS_DISABLED)
      Supla_Z2S_VirtualThermHygroMeter->setApplyCorrections(false);
    else
      Supla_Z2S_VirtualThermHygroMeter->setApplyCorrections(true);

    if (z2s_channels_table[channel_number_slot].user_data_flags & USER_DATA_FLAG_SET_SORWNS_ON_START) {
      
      Supla_Z2S_VirtualThermHygroMeter->getChannel()->setStateOfflineRemoteWakeupNotSupported();
      Supla_Z2S_VirtualThermHygroMeter->setRWNSFlag(true);
    }

    Supla_Z2S_VirtualThermHygroMeter->setTimeoutSecs(z2s_channels_table[channel_number_slot].timeout_secs);


  } else {

    auto Supla_Z2S_VirtualThermometer = new Supla::Sensor::Z2S_VirtualThermometer();

    Supla_Z2S_VirtualThermometer->getChannel()->setChannelNumber(z2s_channels_table[channel_number_slot].Supla_channel);

    if (strlen(z2s_channels_table[channel_number_slot].Supla_channel_name) > 0) 
      Supla_Z2S_VirtualThermometer->setInitialCaption(z2s_channels_table[channel_number_slot].Supla_channel_name);
  
    if (z2s_channels_table[channel_number_slot].Supla_channel_func != 0) 
      Supla_Z2S_VirtualThermometer->setDefaultFunction(z2s_channels_table[channel_number_slot].Supla_channel_func);

    if (z2s_channels_table[channel_number_slot].user_data_flags & USER_DATA_FLAG_CORRECTIONS_DISABLED)
      Supla_Z2S_VirtualThermometer->setApplyCorrections(false);
    else
      Supla_Z2S_VirtualThermometer->setApplyCorrections(true);

    if (z2s_channels_table[channel_number_slot].user_data_flags & USER_DATA_FLAG_SET_SORWNS_ON_START) {
      
      Supla_Z2S_VirtualThermometer->getChannel()->setStateOfflineRemoteWakeupNotSupported();
      Supla_Z2S_VirtualThermometer->setRWNSFlag(true);
    }

    Supla_Z2S_VirtualThermometer->setTimeoutSecs(z2s_channels_table[channel_number_slot].timeout_secs);
  }
  
}

void addZ2SDeviceTempHumidity(zbg_device_params_t *device, uint8_t free_slot, 
                              int8_t sub_id, char *name, uint32_t func,
                              bool thermhygrometer) {

  if (thermhygrometer) {

    auto Supla_Z2S_VirtualThermHygroMeter = new Supla::Sensor::Z2S_VirtualThermHygroMeter();
  
    if (name == nullptr)
      name = (char*)default_temphumi_name;
  
    Z2S_fillChannelsTableSlot(device, free_slot, Supla_Z2S_VirtualThermHygroMeter->getChannelNumber(), 
                              SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR, sub_id, name, func);
  } else {

    auto Z2S_VirtualThermometer = new Supla::Sensor::Z2S_VirtualThermometer();
  
    if (name == nullptr)
      name = (char*)default_temp_name;
  
    Z2S_fillChannelsTableSlot(device, free_slot, Z2S_VirtualThermometer->getChannelNumber(), 
                              SUPLA_CHANNELTYPE_THERMOMETER, sub_id, name, func);
  }
}

Supla::Sensor::Z2S_VirtualThermHygroMeter* getZ2SDeviceTempHumidityPtr(uint8_t Supla_channel) {

  auto element = Supla::Element::getElementByChannelNumber(Supla_channel);
  if (element != nullptr && element->getChannel()->getChannelType() == SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR)  
    return reinterpret_cast<Supla::Sensor::Z2S_VirtualThermHygroMeter *>(element);
  else return nullptr;  
}

void msgZ2SDeviceTempHumidityTemp(int16_t channel_number_slot, double temp) {

  if (channel_number_slot < 0) {
    log_e("msgZ2SDeviceTempHumidityTemp - invalid channel number slot");
    return;
  }

  Z2S_updateZBDeviceLastSeenMs(z2s_channels_table[channel_number_slot].ieee_addr, millis());

  auto element = Supla::Element::getElementByChannelNumber(z2s_channels_table[channel_number_slot].Supla_channel);

  if (element == nullptr)
    return;
  
  switch (element->getChannel()->getChannelType()) {
    
    case SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR: {

      auto Supla_Z2S_VirtualThermHygroMeter = 
        reinterpret_cast<Supla::Sensor::Z2S_VirtualThermHygroMeter *>(element);
    
      Supla_Z2S_VirtualThermHygroMeter->setTemp(temp);
      Supla_Z2S_VirtualThermHygroMeter->Refresh();
    } break;

    case SUPLA_CHANNELTYPE_THERMOMETER: {

      auto Supla_Z2S_VirtualThermometer = 
        reinterpret_cast<Supla::Sensor::Z2S_VirtualThermometer *>(element);
    
      Supla_Z2S_VirtualThermometer->setValue(temp);
      Supla_Z2S_VirtualThermometer->Refresh();
    } break;
  }
}

void msgZ2SDeviceTempHumidityHumi(int16_t channel_number_slot, double humi) {

  if (channel_number_slot < 0) {
    log_e("msgZ2SDeviceTempHumidityHumi - invalid channel number slot");
    return;
  }

  Z2S_updateZBDeviceLastSeenMs(z2s_channels_table[channel_number_slot].ieee_addr, millis());

  auto Supla_Z2S_VirtualThermHygroMeter = getZ2SDeviceTempHumidityPtr(z2s_channels_table[channel_number_slot].Supla_channel);
  
  if (Supla_Z2S_VirtualThermHygroMeter) {
    
    switch (z2s_channels_table[channel_number_slot].model_id) {
      case Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR_HUMIX10: humi *= 10; break;
      default: break;
    }
    Supla_Z2S_VirtualThermHygroMeter->setHumi(humi);
    Supla_Z2S_VirtualThermHygroMeter->Refresh();
  }
}
