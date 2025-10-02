#include "z2s_device_hvac.h"

void initZ2SDeviceHvac(ZigbeeGateway *gateway, zbg_device_params_t *device, int16_t channel_number_slot) {
  
  uint8_t trv_commands_set;
  uint8_t trv_external_sensor_mode = EXTERNAL_TEMPERATURE_SENSOR_IGNORE;
  int16_t hvac_room_temperature_min = 500;
  int16_t hvac_room_temperature_max = 3000;

  switch (z2s_channels_table[channel_number_slot].model_id) {
    
    case Z2S_DEVICE_DESC_TS0601_TRV_SASWELL: {  
      
      trv_commands_set = SASWELL_CMD_SET; 
      trv_external_sensor_mode = EXTERNAL_TEMPERATURE_SENSOR_USE_CALIBRATE; 

      hvac_room_temperature_min = SASWELL_CMD_SET_HEATSETPOINT_MIN;
      hvac_room_temperature_max = SASWELL_CMD_SET_HEATSETPOINT_MAX;

    } break;

    case Z2S_DEVICE_DESC_TS0601_TRV_ME167: {
      
      trv_commands_set = ME167_CMD_SET; 
      trv_external_sensor_mode = EXTERNAL_TEMPERATURE_SENSOR_USE_CALIBRATE; 

      hvac_room_temperature_min = ME167_CMD_SET_HEATSETPOINT_MIN;
      hvac_room_temperature_max = ME167_CMD_SET_HEATSETPOINT_MAX;

    } break;

    case Z2S_DEVICE_DESC_TS0601_TRV_BECA: {
      
      trv_commands_set = BECA_CMD_SET; 
      trv_external_sensor_mode = EXTERNAL_TEMPERATURE_SENSOR_USE_CALIBRATE; 

      hvac_room_temperature_min = BECA_CMD_SET_HEATSETPOINT_MIN;
      hvac_room_temperature_max = BECA_CMD_SET_HEATSETPOINT_MAX;

    } break;

    case Z2S_DEVICE_DESC_TS0601_TRV_MOES: {

      trv_commands_set = MOES_CMD_SET; 
      trv_external_sensor_mode = EXTERNAL_TEMPERATURE_SENSOR_USE_CALIBRATE; 

      hvac_room_temperature_min = MOES_CMD_SET_HEATSETPOINT_MIN;
      hvac_room_temperature_max = MOES_CMD_SET_HEATSETPOINT_MAX;

    } break;

    case Z2S_DEVICE_DESC_TS0601_TRV_TRV601: {

      trv_commands_set = TRV601_CMD_SET; 
      trv_external_sensor_mode = EXTERNAL_TEMPERATURE_SENSOR_USE_CALIBRATE; 

      hvac_room_temperature_min = TRV601_CMD_SET_HEATSETPOINT_MIN;
      hvac_room_temperature_max = TRV601_CMD_SET_HEATSETPOINT_MAX;

    } break;

    case Z2S_DEVICE_DESC_TS0601_TRV_TRV603: {

      trv_commands_set = TRV603_CMD_SET; 
      trv_external_sensor_mode = EXTERNAL_TEMPERATURE_SENSOR_USE_CALIBRATE; 

      hvac_room_temperature_min = TRV603_CMD_SET_HEATSETPOINT_MIN;
      hvac_room_temperature_max = TRV603_CMD_SET_HEATSETPOINT_MAX;

    } break;

    case Z2S_DEVICE_DESC_TS0601_TRV_GTZ10: {

      trv_commands_set = GTZ10_CMD_SET; 
      trv_external_sensor_mode = EXTERNAL_TEMPERATURE_SENSOR_USE_CALIBRATE; 

      hvac_room_temperature_min = GTZ10_CMD_SET_HEATSETPOINT_MIN;
      hvac_room_temperature_max = GTZ10_CMD_SET_HEATSETPOINT_MAX;

    } break;

    case Z2S_DEVICE_DESC_TS0601_TRV_TRV602Z: {

      trv_commands_set = GTZ10_CMD_SET; 
      trv_external_sensor_mode = EXTERNAL_TEMPERATURE_SENSOR_USE_CALIBRATE; 

      hvac_room_temperature_min = GTZ10_CMD_SET_HEATSETPOINT_MIN;
      hvac_room_temperature_max = TRV602Z_CMD_SET_HEATSETPOINT_MAX;

    } break;  

    case Z2S_DEVICE_DESC_TS0601_TRV_TV02: {

      trv_commands_set = TV02_CMD_SET; 
      trv_external_sensor_mode = EXTERNAL_TEMPERATURE_SENSOR_USE_CALIBRATE; 

      hvac_room_temperature_min = TV02_CMD_SET_HEATSETPOINT_MIN;
      hvac_room_temperature_max = TV02_CMD_SET_HEATSETPOINT_MAX;

    } break;
    
    case Z2S_DEVICE_DESC_SONOFF_TRVZB: {

      trv_commands_set = TRVZB_CMD_SET;
      trv_external_sensor_mode = EXTERNAL_TEMPERATURE_SENSOR_USE_INPUT; 
      hvac_room_temperature_min = TRVZB_CMD_SET_HEATSETPOINT_MIN;
      hvac_room_temperature_max = TRVZB_CMD_SET_HEATSETPOINT_MAX;
    } break;

    default:
      trv_commands_set = 0x00; break;
  }
  
  auto Supla_Z2S_TRVInterface = new Supla::Control::Z2S_TRVInterface(gateway, device, trv_commands_set);

  auto Supla_Z2S_HvacBase = new Supla::Control::HvacBaseEE(Supla_Z2S_TRVInterface);
            
  Supla_Z2S_HvacBase->getChannel()->setChannelNumber(z2s_channels_table[channel_number_slot].Supla_channel);

  if (strlen(z2s_channels_table[channel_number_slot].Supla_channel_name) > 0) 
    Supla_Z2S_HvacBase->setInitialCaption(z2s_channels_table[channel_number_slot].Supla_channel_name);
  
  if (z2s_channels_table[channel_number_slot].Supla_channel_func !=0) 
    Supla_Z2S_HvacBase->setDefaultFunction(z2s_channels_table[channel_number_slot].Supla_channel_func);

  /*if (z2s_channels_table[channel_number_slot].Supla_secondary_channel < 0xFF) 
    Supla_Z2S_HvacBase->setMainThermometerChannelNo(z2s_channels_table[channel_number_slot].Supla_secondary_channel);*/

  /*if (z2s_channels_table[channel_number_slot].user_data_1 > 0) 
    Supla_Z2S_TRVInterface->setTemperatureCalibrationUpdateMs(z2s_channels_table[channel_number_slot].user_data_1 * 1000);

  if (z2s_channels_table[channel_number_slot].user_data_2 > 0) 
    Supla_Z2S_TRVInterface->setTemperatureCalibrationOffsetTrigger(z2s_channels_table[channel_number_slot].user_data_2 * 100);*/
  Supla_Z2S_TRVInterface->setTimeoutSecs(z2s_channels_table[channel_number_slot].timeout_secs);


  Supla_Z2S_HvacBase->addAction(Supla::TURN_OFF, Supla_Z2S_TRVInterface,Supla::ON_HVAC_MODE_OFF);
  Supla_Z2S_HvacBase->addAction(Supla::TURN_OFF, Supla_Z2S_TRVInterface,Supla::ON_TURN_OFF);
  Supla_Z2S_HvacBase->addAction(Supla::TURN_ON, Supla_Z2S_TRVInterface,Supla::ON_HVAC_MODE_HEAT);
  Supla_Z2S_HvacBase->addAction(Supla::TURN_ON, Supla_Z2S_TRVInterface,Supla::ON_TURN_ON);
  Supla_Z2S_HvacBase->addAction(Supla::TURN_ON, Supla_Z2S_TRVInterface,Supla::ON_HVAC_WEEKLY_SCHEDULE_ENABLED);
  Supla_Z2S_HvacBase->addAction(Supla::TURN_ON, Supla_Z2S_TRVInterface,Supla::ON_HVAC_WEEKLY_SCHEDULE_DISABLED);
  Supla_Z2S_HvacBase->addAction(Supla::TURN_ON, Supla_Z2S_TRVInterface,Supla::ON_HVAC_STANDBY);
  Supla_Z2S_HvacBase->addAction(Supla::TURN_ON, Supla_Z2S_TRVInterface,Supla::ON_HVAC_HEATING);
  //Supla_Z2S_HvacBase->addAction(Supla::TURN_ON, Supla_Z2S_TRVInterface,Supla::ON_CHANGE);

  Supla_Z2S_HvacBase->allowWrapAroundTemperatureSetpoints();
  Supla_Z2S_HvacBase->setPrimaryOutputEE(Supla_Z2S_TRVInterface);
  Supla_Z2S_TRVInterface->setTRVHvac(Supla_Z2S_HvacBase);

  Supla_Z2S_HvacBase->setDefaultTemperatureRoomMin(SUPLA_CHANNELFNC_HVAC_THERMOSTAT, hvac_room_temperature_min);
  Supla_Z2S_HvacBase->setDefaultTemperatureRoomMax(SUPLA_CHANNELFNC_HVAC_THERMOSTAT, hvac_room_temperature_max);
  Supla_Z2S_HvacBase->setButtonTemperatureStep(50);
  Supla_Z2S_HvacBase->addLocalUILockCapability(Supla::LocalUILock::Full);
  
  Supla_Z2S_TRVInterface->enableExternalSensorDetection(true, trv_external_sensor_mode, z2s_channels_table[channel_number_slot].Supla_secondary_channel);

  
}

void addZ2SDeviceHvac(ZigbeeGateway * gateway, zbg_device_params_t *device, uint8_t free_slot, uint8_t trv_thermometer_slot) {
  
  auto Supla_Z2S_HvacBase = new Supla::Control::HvacBaseEE();

  Supla_Z2S_HvacBase->setMainThermometerChannelNo(z2s_channels_table[trv_thermometer_slot].Supla_channel);
  Supla_Z2S_HvacBase->setBinarySensorChannelNo(Supla_Z2S_HvacBase->getChannel()->getChannelNumber());

  Z2S_fillChannelsTableSlot(device, free_slot, Supla_Z2S_HvacBase->getChannel()->getChannelNumber(), SUPLA_CHANNELTYPE_HVAC, -1, 
                           "THERMOSTAT", SUPLA_CHANNELFNC_HVAC_THERMOSTAT, z2s_channels_table[trv_thermometer_slot].Supla_channel); 


}

void msgZ2SDeviceHvac(int16_t channel_number_slot, uint8_t msg_id, int32_t msg_value) {

  if (channel_number_slot < 0) {
    
    log_e("msgZ2SDeviceHvac - invalid channel number slot");
    return;
  }

  Z2S_updateZbDeviceLastSeenMs(z2s_channels_table[channel_number_slot].ieee_addr, millis());
  
  auto element = Supla::Element::getElementByChannelNumber(z2s_channels_table[channel_number_slot].Supla_channel);

  if ((element == nullptr) || (element->getChannel()->getChannelType() != SUPLA_CHANNELTYPE_HVAC))
    return;
  
  auto Supla_Z2S_HvacBase = reinterpret_cast<Supla::Control::HvacBaseEE *>(element);
  auto Supla_Z2S_TRVInterface = reinterpret_cast<Supla::Control::Z2S_TRVInterface *>(Supla_Z2S_HvacBase->getPrimaryOutputEE());

  switch (msg_id) {
    case TRV_HEATING_SETPOINT_MSG: {   //degrees*100

      log_i("msgZ2SDeviceHvac - TRV_HEATING_SETPOINT_MSG: 0x%x", msg_value);
      log_i("HVAC flags: 0x%x", Supla_Z2S_HvacBase->getChannel()->getHvacFlags());

      if (z2s_channels_table[channel_number_slot].user_data_flags & USER_DATA_FLAG_TRV_IGNORE_NEXT_MSG) {

        log_i("msgZ2SDeviceHvac - ignoring TRV_HEATING_SETPOINT_MSG: 0x%x", msg_value);

        if (abs(Supla_Z2S_HvacBase->getTemperatureSetpointHeat() - msg_value) > 40)
          break;
        else {

          z2s_channels_table[channel_number_slot].user_data_flags &= ~USER_DATA_FLAG_TRV_IGNORE_NEXT_MSG;
        } 
        /*if (z2s_channels_table[channel_number_slot].user_data_1 == 0)
          z2s_channels_table[channel_number_slot].user_data_flags &= ~USER_DATA_FLAG_TRV_IGNORE_NEXT_MSG;
        else
          z2s_channels_table[channel_number_slot].user_data_1--;*/

        //break;
      }
       //if (Supla_Z2S_HvacBase->getMode() != SUPLA_HVAC_MODE_OFF) {
      //if (Supla_Z2S_HvacBase->isThermostatDisabled()) {
       // log_i()
      //}
      if (Supla_Z2S_HvacBase->isWeeklyScheduleEnabled() &&
          (abs(Supla_Z2S_HvacBase->getTemperatureSetpointHeat() - msg_value) > 40) &&
          (Supla_Z2S_HvacBase->getCurrentProgramId() != 0)) {

        TWeeklyScheduleProgram program = Supla_Z2S_HvacBase->getProgramById(Supla_Z2S_HvacBase->getCurrentProgramId());
        //Supla_Z2S_HvacBase->setProgram(Supla_Z2S_HvacBase->getCurrentProgramId(), program.Mode, msg_value, program.SetpointTemperatureCool, false);
        //Supla_Z2S_HvacBase->setTemperatureSetpointHeat(msg_value);
        Supla_Z2S_HvacBase->applyNewRuntimeSettings(/*SUPLA_HVAC_MODE_CMD_WEEKLY_SCHEDULE*/SUPLA_HVAC_MODE_NOT_SET, msg_value, 0, 0);
        Supla_Z2S_TRVInterface->setTRVTemperatureSetpoint(msg_value);

        log_i("Changing weekly schedule program temperature: program id %u,hvac getTemperatureSetpointHeat %d, msg value %d", Supla_Z2S_HvacBase->getCurrentProgramId(),
              Supla_Z2S_HvacBase->getTemperatureSetpointHeat(), msg_value);

      } else {

        Supla_Z2S_HvacBase->setTemperatureSetpointHeat(msg_value);
        Supla_Z2S_TRVInterface->setTRVTemperatureSetpoint(msg_value);
      }
    } break;

    case TRV_SYSTEM_MODE_MSG: { //0:off, 1:on

      log_i("msgZ2SDeviceHvac - TRV_SYSTEM_MODE_MSG: 0x%x", msg_value);
      log_i("HVAC flags: 0x%x", Supla_Z2S_HvacBase->getChannel()->getHvacFlags());

      switch (msg_value) {

        case 1: if (Supla_Z2S_HvacBase->isThermostatDisabled()) 
                  Supla_Z2S_HvacBase->setTargetMode(SUPLA_HVAC_MODE_CMD_TURN_ON); break;

        case 0: if (!Supla_Z2S_HvacBase->isThermostatDisabled())
                  Supla_Z2S_HvacBase->setTargetMode(SUPLA_HVAC_MODE_OFF, true); break;
      }
        Supla_Z2S_TRVInterface->setTRVSystemMode(msg_value);

    } break;

    case TRV_SCHEDULE_MODE_MSG: { //0:off, 1:on

      log_i("msgZ2SDeviceHvac - TRV_SCHEDULE_MODE_MSG: 0x%x", msg_value);

      switch (msg_value) {
        //case 0: Supla_Z2S_HvacBase->setTargetMode(SUPLA_HVAC_MODE_CMD_TURN_ON); break;
        case 1: {
          if (z2s_channels_table[channel_number_slot].user_data_flags & USER_DATA_FLAG_TRV_AUTO_TO_SCHEDULE) {

            Supla_Z2S_HvacBase->setTargetMode(SUPLA_HVAC_MODE_CMD_WEEKLY_SCHEDULE, true); 
            z2s_channels_table[channel_number_slot].user_data_flags |= USER_DATA_FLAG_TRV_IGNORE_NEXT_MSG;
            z2s_channels_table[channel_number_slot].user_data_1 = 1;
            //Supla_Z2S_HvacBase->applyNewRuntimeSettings(SUPLA_HVAC_MODE_CMD_WEEKLY_SCHEDULE, 0);
            Supla_Z2S_TRVInterface->turnOffTRVScheduleMode();

          } else {

            Supla_Z2S_HvacBase->setTargetMode(SUPLA_HVAC_MODE_OFF, false); 
            Supla_Z2S_TRVInterface->setTRVSystemMode(0);
          }
        } break;
      }        
    } break;

    case TRV_RUNNING_STATE_MSG: { //0:idle, 1:heat

      log_i("msgZ2SDeviceHvac - TRV_RUNNING_STATE_MSG: 0x%x", msg_value);
      
      Supla_Z2S_TRVInterface->setTRVRunningState(msg_value);
      Supla_Z2S_TRVInterface->setOutputValueFromRemote(msg_value*100);
    } break;
  
    case TRV_LOCAL_TEMPERATURE_MSG: { //degrees*100

      log_i("msgZ2SDeviceHvac - TRV_LOCAL_TEMPERATURE_MSG: 0x%x", msg_value);
        Supla_Z2S_TRVInterface->setTRVLocalTemperature(msg_value);
    } break;
    
    case TRV_TEMPERATURE_CALIBRATION_MSG: { //degrees*100

      log_i("msgZ2SDeviceHvac - TRV_TEMPERATURE_CALIBRATION_MSG: 0x%x", msg_value);
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

    default: log_i("msgZ2SDeviceHvac - unknown message id: 0x%x, value 0x%x", msg_id, msg_value); break;
  }
}

