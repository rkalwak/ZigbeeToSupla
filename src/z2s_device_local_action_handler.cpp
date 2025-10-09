#include "z2s_device_local_action_handler.h"

const char* getZ2SDeviceLocalActionHandlerTypeName(int16_t channel_number_slot){

  switch (z2s_channels_table[channel_number_slot].local_channel_type) {

    case LOCAL_CHANNEL_TYPE_ACTION_HANDLER: {
      
      return "Local action handler";
    } break;

    case LOCAL_CHANNEL_TYPE_VIRTUAL_RELAY:
      
      return "Local virtual relay";
    break;

    default: 
      return "";
    break;
  }

  return "";
}

const char* getZ2SDeviceLocalActionHandlerLogicOperatorName(int16_t channel_number_slot) {

  switch (z2s_channels_table[channel_number_slot].local_channel_type) {

    case LOCAL_CHANNEL_TYPE_ACTION_HANDLER: {
      
      return 
        ACTION_HANDLERS_DEFAULT_NAMES[z2s_channels_table[channel_number_slot].local_action_handler_data.logic_operator];
    } break;

    case LOCAL_CHANNEL_TYPE_VIRTUAL_RELAY:
      
      return "No special functions";
    break;
  }

  return "";
}


void initZ2SDeviceLocalActionHandler(int16_t channel_number_slot)  {

  switch (z2s_channels_table[channel_number_slot].local_channel_type) {

    case LOCAL_CHANNEL_TYPE_ACTION_HANDLER: {
      
      auto Supla_LocalActionHandlerWithTrigger = 
        new Supla::LocalActionHandlerWithTrigger(
          z2s_channels_table[channel_number_slot].local_action_handler_data.logic_operator); 

      z2s_channels_table[channel_number_slot].local_action_handler_data.Supla_element =
        Supla_LocalActionHandlerWithTrigger;

      Supla_LocalActionHandlerWithTrigger->setPostponedTurnOnSecs(  
        z2s_channels_table[channel_number_slot].keep_alive_secs);
    } break;

    case LOCAL_CHANNEL_TYPE_VIRTUAL_RELAY: {
      
      uint8_t Supla_channel = z2s_channels_table[channel_number_slot].Supla_channel;
      auto Supla_VirtualRelay = new Supla::Control::VirtualRelay(); 
      
      Supla_VirtualRelay->getChannel()->setChannelNumber(Supla_channel);
    }
    break;
  } 
}

/*---------------------------------------------------------------------------------------------------------------------------*/

bool addZ2SDeviceLocalActionHandler(uint8_t local_channel_type, 
                                    uint32_t local_channel_func,
                                    uint8_t logic_operator) {

  uint8_t first_free_slot = Z2S_findFirstFreeChannelsTableSlot();

  if (first_free_slot == 0xFF)
    return false;

  z2s_channels_table[first_free_slot].valid_record = true;

  z2s_channels_table[first_free_slot].extended_data_type = CHANNEL_EXTENDED_DATA_TYPE_NULL;
  z2s_channels_table[first_free_slot].local_channel_type = local_channel_type;

  memset(z2s_channels_table[first_free_slot].ieee_addr, 0, sizeof(esp_zb_ieee_addr_t));
  z2s_channels_table[first_free_slot].short_addr = 0;

  z2s_channels_table[first_free_slot].model_id = Z2S_DEVICE_DESC_LOCAL_ACTION_HANDLER;

  z2s_channels_table[first_free_slot].endpoint = 0;
  z2s_channels_table[first_free_slot].cluster_id = 0;

  switch(local_channel_type) {

    case LOCAL_CHANNEL_TYPE_ACTION_HANDLER: {

      z2s_channels_table[first_free_slot].Supla_channel = 
        Z2S_findFirstFreeLocalActionHandlerId();

      strcpy(z2s_channels_table[first_free_slot].Supla_channel_name, 
             ACTION_HANDLERS_DEFAULT_NAMES[logic_operator]);

      z2s_channels_table[first_free_slot].local_action_handler_data.logic_operator = logic_operator;

      z2s_channels_table[first_free_slot].local_action_handler_data.Supla_element = 
        new Supla::LocalActionHandlerWithTrigger(logic_operator);   
    } break;

    case LOCAL_CHANNEL_TYPE_VIRTUAL_RELAY: {

      SuplaDevice.saveStateToStorage();
      
      auto Supla_VirtualRelay = new Supla::Control::VirtualRelay(); 

      z2s_channels_table[first_free_slot].Supla_channel = Supla_VirtualRelay->getChannelNumber();
      strcpy(z2s_channels_table[first_free_slot].Supla_channel_name, "LOCAL VIRTUAL RELAY");
      
      Supla_VirtualRelay->setInitialCaption(z2s_channels_table[first_free_slot].Supla_channel_name);
      Supla_VirtualRelay->setDefaultFunction(local_channel_func);
      
    } break;
  }

  z2s_channels_table[first_free_slot].Supla_secondary_channel = 0xFF;

  z2s_channels_table[first_free_slot].Supla_channel_type = 0x0000;

  z2s_channels_table[first_free_slot].sub_id = 0; 
  
  z2s_channels_table[first_free_slot].Supla_channel_func = local_channel_func;

  //z2s_channels_table[first_free_slot].local_action_handler_data.Supla_element = nullptr;
  z2s_channels_table[first_free_slot].ZB_device_id = 0xFF;
  
  return Z2S_saveChannelsTable();
}



