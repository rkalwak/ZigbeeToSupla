#include "z2s_device_iaszone.h"

void initZ2SDeviceIASzone(int16_t channel_number_slot) {
  
  uint8_t timeout = 0;

  if (z2s_devices_table[channel_number_slot].user_data_1 & USER_DATA_FLAG_SED_TIMEOUT == USER_DATA_FLAG_SED_TIMEOUT)
    timeout = z2s_devices_table[channel_number_slot].user_data_2;
  
  auto Supla_Z2S_VirtualBinary = new Supla::Sensor::Z2S_VirtualBinary(true, timeout);
  
  Supla_Z2S_VirtualBinary->getChannel()->setChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);

  if (z2s_devices_table[channel_number_slot].Supla_channel_name) 
    Supla_Z2S_VirtualBinary->setInitialCaption(z2s_devices_table[channel_number_slot].Supla_channel_name);
  
  if (z2s_devices_table[channel_number_slot].Supla_channel_func != 0) 
    Supla_Z2S_VirtualBinary->setDefaultFunction(z2s_devices_table[channel_number_slot].Supla_channel_func);
  
}

void addZ2SDeviceIASzone(zbg_device_params_t *device, uint8_t free_slot, int8_t sub_id, char *name, uint32_t func) {
  
  auto Supla_Z2S_VirtualBinary = new Supla::Sensor::Z2S_VirtualBinary(true);
  Z2S_fillDevicesTableSlot(device, free_slot, Supla_Z2S_VirtualBinary->getChannelNumber(), SUPLA_CHANNELTYPE_BINARYSENSOR, sub_id, name, func);

  if (name) 
    Supla_Z2S_VirtualBinary->setInitialCaption(name);
  
  if (func !=0) 
    Supla_Z2S_VirtualBinary->setDefaultFunction(func);
  
}

void msgZ2SDeviceIASzone(int16_t channel_number_slot, bool state, signed char rssi) {

  if (channel_number_slot < 0) {
    log_e("msgZ2SDeviceIASzone - invalid channel number slot");
    return;
  }

  auto element = Supla::Element::getElementByChannelNumber(z2s_devices_table[channel_number_slot].Supla_channel);
  if (element != nullptr && element->getChannel()->getChannelType() == SUPLA_CHANNELTYPE_BINARYSENSOR) {

        auto Supla_Z2S_VirtualBinary = reinterpret_cast<Supla::Sensor::Z2S_VirtualBinary *>(element);
        if (state) Supla_Z2S_VirtualBinary->set();
        else Supla_Z2S_VirtualBinary->clear();
        Supla_Z2S_VirtualBinary->Refresh();
    }
}
