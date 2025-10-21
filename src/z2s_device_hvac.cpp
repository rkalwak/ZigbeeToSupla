#include "z2s_device_hvac.h"

/*----------------------------------------------------------------------------------------------------------------------------*/
uint8_t getZ2SDeviceHvacCmdSet(uint32_t model_id) {

  switch (model_id) {


    case Z2S_DEVICE_DESC_TS0601_TRV_SASWELL: {  
      
      return saswell_cmd_set; 
    } break;


    case Z2S_DEVICE_DESC_TS0601_TRV_ME167: {
      
      return me167_cmd_set;
    } break;


    case Z2S_DEVICE_DESC_TS0601_TRV_BECA: {
      
      return beca_cmd_set; 
    } break;


    case Z2S_DEVICE_DESC_TS0601_TRV_MOES: {

      return moes_cmd_set;
    } break;


    case Z2S_DEVICE_DESC_TS0601_TRV_TRV601: {

      return trv601_cmd_set;
    } break;


    case Z2S_DEVICE_DESC_TS0601_TRV_TRV603: {

      return trv603_cmd_set;
    } break;


    case Z2S_DEVICE_DESC_TS0601_TRV_GTZ10: {

      return gtz10_cmd_set;
    } break;


    case Z2S_DEVICE_DESC_TS0601_TRV_TRV602Z: {

      return trv602z_cmd_set;
    } break;  


    case Z2S_DEVICE_DESC_TS0601_TRV_TV02: {

      return tv02_cmd_set;
    } break;


    case Z2S_DEVICE_DESC_TS0601_TRV_SITERWELL: {

      return siterwell_cmd_set;
    } break;

    case Z2S_DEVICE_DESC_TS0601_TRV_TRV16: {

      return trv16_cmd_set;
    } break;

    case Z2S_DEVICE_DESC_TS0601_ZWT_ZWT198: {

      return zwt198_cmd_set;
    } break;
    
    case Z2S_DEVICE_DESC_TS0601_MOES_BHT002: {

      return bht002_cmd_set;
    } break;
    
    case Z2S_DEVICE_DESC_SONOFF_TRVZB: {

      return  TRVZB_CMD_SET;
    } break;

    default:
      return 0xFF; break;
  }  
}

/*----------------------------------------------------------------------------------------------------------------------------*/

void initZ2SDeviceHvac(ZigbeeGateway *gateway, zbg_device_params_t *device, int16_t channel_number_slot) {
  
  uint8_t trv_commands_set;
  uint8_t trv_external_sensor_mode = EXTERNAL_TEMPERATURE_SENSOR_IGNORE;
  int16_t hvac_room_temperature_min = 500;
  int16_t hvac_room_temperature_max = 3000;

  
  trv_commands_set = getZ2SDeviceHvacCmdSet(z2s_channels_table[channel_number_slot].model_id);

  if ((trv_commands_set >= saswell_cmd_set) && (trv_commands_set < ts0601_cmd_sets_number)) {

    if (ts0601_command_sets_table[trv_commands_set].ts0601_cmd_set_id == trv_commands_set) {

      trv_external_sensor_mode = EXTERNAL_TEMPERATURE_SENSOR_USE_CALIBRATE; 
      hvac_room_temperature_min = 
        ts0601_command_sets_table[trv_commands_set].ts0601_cmd_set_target_heatsetpoint_min;
      hvac_room_temperature_max = 
        ts0601_command_sets_table[trv_commands_set].ts0601_cmd_set_target_heatsetpoint_max;
    } else {

      log_e("ts0601_command_sets_table internal mismatch! %02x <> %02x", 
            ts0601_command_sets_table[trv_commands_set].ts0601_cmd_set_id,
            trv_commands_set);
      return;
    }
  } else {

    switch (trv_commands_set) {


      case TRVZB_CMD_SET: {

        trv_external_sensor_mode = EXTERNAL_TEMPERATURE_SENSOR_USE_INPUT; 
        hvac_room_temperature_min = TRVZB_CMD_SET_HEATSETPOINT_MIN;
        hvac_room_temperature_max = TRVZB_CMD_SET_HEATSETPOINT_MAX;
      } break;
    }
  }

  auto Supla_Z2S_TRVInterface = 
    new Supla::Control::Z2S_TRVInterface(gateway, device, trv_commands_set);

  auto Supla_Z2S_HvacBase = 
    new Supla::Control::HvacBaseEE(Supla_Z2S_TRVInterface);
            
  Supla_Z2S_HvacBase->getChannel()->setChannelNumber(
      z2s_channels_table[channel_number_slot].Supla_channel);

  Supla_Z2S_HvacBase->setMainThermometerChannelNo(
      z2s_channels_table[channel_number_slot].Supla_secondary_channel);

  Supla_Z2S_HvacBase->setBinarySensorChannelNo(
      z2s_channels_table[channel_number_slot].Supla_channel);

  if (strlen(z2s_channels_table[channel_number_slot].Supla_channel_name) > 0) 
    Supla_Z2S_HvacBase->setInitialCaption(
      z2s_channels_table[channel_number_slot].Supla_channel_name);
  
  if (z2s_channels_table[channel_number_slot].Supla_channel_func !=0) 
    Supla_Z2S_HvacBase->setDefaultFunction(
      z2s_channels_table[channel_number_slot].Supla_channel_func);

  Supla_Z2S_TRVInterface->setTimeoutSecs(
    z2s_channels_table[channel_number_slot].timeout_secs);


  Supla_Z2S_HvacBase->addAction(Supla::TURN_OFF, 
                                Supla_Z2S_TRVInterface,
                                Supla::ON_HVAC_MODE_OFF);

  Supla_Z2S_HvacBase->addAction(Supla::TURN_OFF,
                                Supla_Z2S_TRVInterface,
                                Supla::ON_TURN_OFF);

  Supla_Z2S_HvacBase->addAction(Supla::TURN_ON, 
                                Supla_Z2S_TRVInterface,
                                Supla::ON_HVAC_MODE_HEAT);

  Supla_Z2S_HvacBase->addAction(Supla::TURN_ON, 
                                Supla_Z2S_TRVInterface,
                                Supla::ON_TURN_ON);

  Supla_Z2S_HvacBase->addAction(Supla::TURN_ON, 
                                Supla_Z2S_TRVInterface,
                                Supla::ON_HVAC_WEEKLY_SCHEDULE_ENABLED);

  Supla_Z2S_HvacBase->addAction(Supla::TURN_ON, 
                                Supla_Z2S_TRVInterface,
                                Supla::ON_HVAC_WEEKLY_SCHEDULE_DISABLED);

  Supla_Z2S_HvacBase->addAction(Supla::TURN_ON, 
                                Supla_Z2S_TRVInterface,
                                Supla::ON_HVAC_STANDBY);

  Supla_Z2S_HvacBase->addAction(Supla::TURN_ON, 
                                Supla_Z2S_TRVInterface,
                                Supla::ON_HVAC_HEATING);

  //Supla_Z2S_HvacBase->addAction(Supla::TURN_ON, 
  //                                Supla_Z2S_TRVInterface,
  //                                Supla::ON_CHANGE);

  Supla_Z2S_HvacBase->allowWrapAroundTemperatureSetpoints();
  Supla_Z2S_HvacBase->setPrimaryOutputEE(Supla_Z2S_TRVInterface);
  Supla_Z2S_TRVInterface->setTRVHvac(Supla_Z2S_HvacBase);

  Supla_Z2S_HvacBase->setDefaultTemperatureRoomMin(SUPLA_CHANNELFNC_HVAC_THERMOSTAT, 
                                                   hvac_room_temperature_min);

  Supla_Z2S_HvacBase->setDefaultTemperatureRoomMax(SUPLA_CHANNELFNC_HVAC_THERMOSTAT, 
                                                   hvac_room_temperature_max);

  Supla_Z2S_HvacBase->setButtonTemperatureStep(50);
  Supla_Z2S_HvacBase->addLocalUILockCapability(Supla::LocalUILock::Full);
  
  if (z2s_channels_table[channel_number_slot].user_data_flags & 
      USER_DATA_FLAG_TRV_FIXED_CORRECTION) {

    Supla_Z2S_TRVInterface->enableExternalSensorDetection(
      false, 
      EXTERNAL_TEMPERATURE_SENSOR_USE_FIXED, 
      z2s_channels_table[channel_number_slot].Supla_secondary_channel);  

    Supla_Z2S_TRVInterface->setFixedTemperatureCalibration(
      z2s_channels_table[channel_number_slot].hvac_fixed_temperature_correction);
  } else {

    Supla_Z2S_TRVInterface->enableExternalSensorDetection(
      true, 
      trv_external_sensor_mode, 
      z2s_channels_table[channel_number_slot].Supla_secondary_channel); 
  }

  if (z2s_channels_table[channel_number_slot].user_data_flags & 
    USER_DATA_FLAG_TRV_COOPERATIVE_CHILDLOCK)
    Supla_Z2S_TRVInterface->setCooperativeChildLock(true);

  z2s_channels_table[channel_number_slot].user_data_flags &= 
    ~USER_DATA_FLAG_TRV_IGNORE_NEXT_MSG;

  z2s_channels_table[channel_number_slot].user_data_2 = 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/

void addZ2SDeviceHvac(ZigbeeGateway *gateway, 
                      zbg_device_params_t *device, 
                      uint8_t free_slot, 
                      uint8_t trv_thermometer_slot) {
  
  auto Supla_Z2S_HvacBase = new Supla::Control::HvacBaseEE();

  Supla_Z2S_HvacBase->setMainThermometerChannelNo(z2s_channels_table[trv_thermometer_slot].Supla_channel);
  Supla_Z2S_HvacBase->setBinarySensorChannelNo(Supla_Z2S_HvacBase->getChannel()->getChannelNumber());

  Z2S_fillChannelsTableSlot(device, 
                            free_slot, 
                            Supla_Z2S_HvacBase->getChannel()->getChannelNumber(), 
                            SUPLA_CHANNELTYPE_HVAC, 
                            -1, 
                            "THERMOSTAT", 
                            SUPLA_CHANNELFNC_HVAC_THERMOSTAT, 
                            z2s_channels_table[trv_thermometer_slot].Supla_channel); 
}

/*----------------------------------------------------------------------------------------------------------------------------*/

void msgZ2SDeviceHvac(int16_t channel_number_slot, uint8_t msg_id, int32_t msg_value) {

  if (channel_number_slot < 0) {
    
    log_e("msgZ2SDeviceHvac - invalid channel number slot");
    return;
  }

  Z2S_updateZbDeviceLastSeenMs(
    z2s_channels_table[channel_number_slot].ieee_addr, millis());
  
  auto element = 
    Supla::Element::getElementByChannelNumber(
      z2s_channels_table[channel_number_slot].Supla_channel);

  if ((element == nullptr) || 
      (element->getChannel()->getChannelType() != SUPLA_CHANNELTYPE_HVAC))
    return;
  
  auto Supla_Z2S_HvacBase = 
    reinterpret_cast<Supla::Control::HvacBaseEE *>(element);
    
  auto Supla_Z2S_TRVInterface = 
    reinterpret_cast<Supla::Control::Z2S_TRVInterface *>(Supla_Z2S_HvacBase->getPrimaryOutputEE());

  switch (msg_id) {
    
    
    case TRV_HEATING_SETPOINT_MSG: {   //degrees*100

      log_i("msgZ2SDeviceHvac - TRV_HEATING_SETPOINT_MSG: 0x%x", msg_value);
      log_i("HVAC flags: 0x%x", Supla_Z2S_HvacBase->getChannel()->getHvacFlags());

      if (z2s_channels_table[channel_number_slot].user_data_flags & 
          USER_DATA_FLAG_TRV_IGNORE_NEXT_MSG) {

        log_i("Flag set - ignoring TRV_HEATING_SETPOINT_MSG: %d\n\r"
              "Supla_Z2S_HvacBase->getTemperatureSetpointHeat %d", 
              msg_value,
              Supla_Z2S_HvacBase->getTemperatureSetpointHeat());

        /*if (abs(Supla_Z2S_HvacBase->getTemperatureSetpointHeat() - msg_value) > 40)
          break;
        else {

          z2s_channels_table[channel_number_slot].user_data_flags &= 
            ~USER_DATA_FLAG_TRV_IGNORE_NEXT_MSG;
        } */
        if (z2s_channels_table[channel_number_slot].user_data_2 == 0)
          z2s_channels_table[channel_number_slot].user_data_flags &= ~USER_DATA_FLAG_TRV_IGNORE_NEXT_MSG;
        //else
         // z2s_channels_table[channel_number_slot].user_data_1--;*/
          Supla_Z2S_TRVInterface->setTRVTemperatureSetpoint(msg_value);
        break;
      }
       //if (Supla_Z2S_HvacBase->getMode() != SUPLA_HVAC_MODE_OFF) {
      //if (Supla_Z2S_HvacBase->isThermostatDisabled()) {
       // log_i()
      //}
      if (Supla_Z2S_HvacBase->isWeeklyScheduleEnabled() &&
          (abs(Supla_Z2S_HvacBase->getTemperatureSetpointHeat() - msg_value) > 40) &&
          (Supla_Z2S_HvacBase->getCurrentProgramId() != 0)) {

        TWeeklyScheduleProgram program = 
          Supla_Z2S_HvacBase->getProgramById(Supla_Z2S_HvacBase->getCurrentProgramId());

        //Supla_Z2S_HvacBase->setProgram(Supla_Z2S_HvacBase->getCurrentProgramId(), program.Mode, msg_value, program.SetpointTemperatureCool, false);
        //Supla_Z2S_HvacBase->setTemperatureSetpointHeat(msg_value);

        Supla_Z2S_HvacBase->applyNewRuntimeSettings(/*SUPLA_HVAC_MODE_CMD_WEEKLY_SCHEDULE*/SUPLA_HVAC_MODE_NOT_SET, msg_value, 0, 0);
        Supla_Z2S_TRVInterface->setTRVTemperatureSetpoint(msg_value);

        log_i("\n\rChanging weekly schedule program temperature: \n\rprogram id %u"
              "\n\rhvac getTemperatureSetpointHeat %d\n\rmsg value %d", 
              Supla_Z2S_HvacBase->getCurrentProgramId(),
              Supla_Z2S_HvacBase->getTemperatureSetpointHeat(), 
              msg_value);

      } else {

        Supla_Z2S_HvacBase->setTemperatureSetpointHeat(msg_value);
        
        Supla_Z2S_TRVInterface->setTRVTemperatureSetpoint(msg_value);
      }
    } break;


    case TRV_SYSTEM_MODE_MSG: { //0:off, 1:on

      log_i("msgZ2SDeviceHvac - TRV_SYSTEM_MODE_MSG: 0x%x", 
            msg_value);

      log_i("HVAC flags: 0x%x", 
            Supla_Z2S_HvacBase->getChannel()->getHvacFlags());

      switch (msg_value) {


        case 1: 
        
          if (Supla_Z2S_HvacBase->isThermostatDisabled()) 
            Supla_Z2S_HvacBase->setTargetMode(SUPLA_HVAC_MODE_CMD_TURN_ON);

          if (z2s_channels_table[channel_number_slot].user_data_flags & 
              USER_DATA_FLAG_TRV_IGNORE_NEXT_MSG)
            z2s_channels_table[channel_number_slot].user_data_2 = 0;
            
        break;


        case 0: 
        
          if (!Supla_Z2S_HvacBase->isThermostatDisabled())
            Supla_Z2S_HvacBase->setTargetMode(SUPLA_HVAC_MODE_OFF, true); 

          z2s_channels_table[channel_number_slot].user_data_flags &= ~USER_DATA_FLAG_TRV_IGNORE_NEXT_MSG;
          z2s_channels_table[channel_number_slot].user_data_2 = 0;

        break;
      }
      
      Supla_Z2S_TRVInterface->setTRVSystemMode(msg_value);

    } break;


    case TRV_SCHEDULE_MODE_MSG: { //0:off, 1:on

      log_i("msgZ2SDeviceHvac - TRV_SCHEDULE_MODE_MSG: 0x%x", 
            msg_value);

      switch (msg_value) {
   
        
        //case 0: Supla_Z2S_HvacBase->setTargetMode(SUPLA_HVAC_MODE_CMD_TURN_ON); break;
        case 0: {

          if (z2s_channels_table[channel_number_slot].user_data_flags & 
              USER_DATA_FLAG_TRV_IGNORE_NEXT_MSG)
            z2s_channels_table[channel_number_slot].user_data_2 = 0;
        } break;
        case 1: {

          if (z2s_channels_table[channel_number_slot].user_data_flags & 
              USER_DATA_FLAG_TRV_AUTO_TO_SCHEDULE) {
            
            if (z2s_channels_table[channel_number_slot].user_data_flags & 
                USER_DATA_FLAG_TRV_AUTO_TO_SCHEDULE_MANUAL) {
                  
                    log_i("USER_DATA_FLAG_TRV_AUTO_TO_SCHEDULE & USER_DATA_FLAG_TRV_AUTO_TO_SCHEDULE_MANUAL");
                    Supla_Z2S_HvacBase->handleAction(0, Supla::TOGGLE_MANUAL_WEEKLY_SCHEDULE_MODES);
            } else {
              
              log_i("USER_DATA_FLAG_TRV_AUTO_TO_SCHEDULE");
              Supla_Z2S_HvacBase->setTargetMode(SUPLA_HVAC_MODE_CMD_WEEKLY_SCHEDULE, true); 
            }
            
            z2s_channels_table[channel_number_slot].user_data_flags |= 
              USER_DATA_FLAG_TRV_IGNORE_NEXT_MSG;
            z2s_channels_table[channel_number_slot].user_data_2 = 1; //now used for hvac_fixed_temperature
            
            //Supla_Z2S_HvacBase->applyNewRuntimeSettings(SUPLA_HVAC_MODE_CMD_WEEKLY_SCHEDULE, 0);
            Supla_Z2S_TRVInterface->turnOffTRVScheduleMode();
            //Supla_Z2S_TRVInterface->setTRVTemperatureSetpoint(
            //Supla_Z2S_HvacBase->setTemperatureSetpointHeat(msg_value);

          } else {

            Supla_Z2S_HvacBase->setTargetMode(SUPLA_HVAC_MODE_OFF, false); 
            
            Supla_Z2S_TRVInterface->setTRVSystemMode(0);
          }
        } break;
      }        
    } break;


    case TRV_RUNNING_STATE_MSG: { //0:idle, 1:heat

      log_i("msgZ2SDeviceHvac - TRV_RUNNING_STATE_MSG: 0x%x", 
            msg_value);
      
      Supla_Z2S_TRVInterface->setTRVRunningState(msg_value);

      Supla_Z2S_TRVInterface->setOutputValueFromRemote(msg_value * 100);
    } break;


    case TRV_LOCAL_TEMPERATURE_MSG: { //degrees*100

      log_i("msgZ2SDeviceHvac - TRV_LOCAL_TEMPERATURE_MSG: 0x%x", 
            msg_value);
      
      Supla_Z2S_TRVInterface->setTRVLocalTemperature(msg_value);
    } break;


    case TRV_TEMPERATURE_CALIBRATION_MSG: { //degrees*100

      log_i("msgZ2SDeviceHvac - TRV_TEMPERATURE_CALIBRATION_MSG: 0x%x", 
            msg_value);

      Supla_Z2S_TRVInterface->setTRVTemperatureCalibration(msg_value);
    } break; 


    case TRV_LOW_BATTERY_MSG: { //O == OK, 1 - low battery

      log_i("msgZ2SDeviceHvac - TRV_LOW_BATTERY_MSG: 0x%x", msg_value);
    } break;


    case TRV_BATTERY_LEVEL_MSG: { 

      log_i("msgZ2SDeviceHvac - TRV_BATTERY_LEVEL_MSG: 0x%x", msg_value);
    } break;


    case TRV_CHILD_LOCK_MSG: { 

      log_i("msgZ2SDeviceHvac - TRV_CHILD_LOCK_MSG: 0x%x", msg_value);
      Supla_Z2S_TRVInterface->setTRVChildLock(msg_value);
    } break;


    case TRV_WINDOW_DETECT_MSG: { 

      log_i("msgZ2SDeviceHvac - WINDOW_DETECT_MSG: 0x%x", msg_value);
    } break;


    case TRV_ANTI_FREEZE_MSG: { 

      log_i("msgZ2SDeviceHvac - ANTI_FREEZE_MSG: 0x%x", msg_value);
    } break;


    case TRV_LIMESCALE_PROTECT_MSG: { 
      
      log_i("msgZ2SDeviceHvac - LIMESCALE_PROTECT_MSG: 0x%x", msg_value);
    } break;


  case TRV_TEMPERATURE_HISTERESIS_MSG: { 
      
      log_i("msgZ2SDeviceHvac - TRV_TEMPERATURE_HISTERESIS_MSG: 0x%x", 
            msg_value);

      Supla_Z2S_TRVInterface->setTRVTemperatureHisteresis(msg_value);
    } break;


    default: 

      log_i("msgZ2SDeviceHvac - unknown message id: 0x%x, value 0x%x", 
            msg_id, msg_value); 
    break;
  }
}
