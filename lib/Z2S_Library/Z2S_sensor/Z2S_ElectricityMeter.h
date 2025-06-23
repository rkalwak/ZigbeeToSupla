#ifndef Z2S_ELECTRICITY_METER_H_
#define Z2S_ELECTRICITY_METER_H_


#include <supla/log_wrapper.h>
#include <supla/sensor/electricity_meter.h>
#include "ZigbeeGateway.h"

namespace Supla {
namespace Sensor {

class Z2S_ElectricityMeter : public ElectricityMeter {
 public:
  Z2S_ElectricityMeter(ZigbeeGateway *gateway, zbg_device_params_t *device, uint64_t *data_counter, bool isTuya, 
		       bool active_query = false, bool one_phase = true) : _gateway(gateway), _data_counter(data_counter) {
	memcpy(&_device, device, sizeof(zbg_device_params_t));
	_active_query = active_query;
        _isTuya = isTuya;
	if (_active_query) refreshRateSec = 30;
	if (one_phase) {
	  extChannel.setFlag(SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED);
          extChannel.setFlag(SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED);
        }

  }

void setVoltageMultiplier(uint16_t voltage_multiplier) {
  
  if (voltage_multiplier > 0)
    _voltage_multiplier = voltage_multiplier;
}

uint16_t getVoltageMultiplier() {

  return _voltage_multiplier;
}

void setVoltageDivisor(uint16_t voltage_divisor) {
  
  if (voltage_divisor > 0)
    _voltage_divisor = voltage_divisor;
}

uint16_t getVoltageDivisor() {
	
  return _voltage_divisor;
}

void setCurrentMultiplier(uint16_t current_multiplier) {

  if (current_multiplier > 0)
    _current_multiplier = current_multiplier;
}

uint16_t getCurrentMultiplier() {

  return _current_multiplier;
}

void setCurrentDivisor(uint16_t current_divisor) {
  
  if (current_divisor > 0)
    _current_divisor = current_divisor;
}

uint16_t getCurrentDivisor() {
  
  return _voltage_divisor;
}

void setActivePowerMultiplier(uint16_t active_power_multiplier) {
  
  if (active_power_multiplier > 0)
    _active_power_multiplier = active_power_multiplier;
}

uint16_t getActivePowerMultiplier() {
  
  return _active_power_multiplier;
} 

void setActivePowerDivisor(uint16_t active_power_divisor) {
  
  if (active_power_divisor > 0)  
    _active_power_divisor = active_power_divisor;
}

uint16_t getActivePowerDivisor() {
  
  return _active_power_divisor;
} 

 
void setVoltage2(int phase, uint32_t voltage) {

  if ((_voltage_multiplier == 0) || (_voltage_divisor == 0)) 
    setVoltage(phase, voltage * 100);
  else 
    setVoltage(phase, (voltage * _voltage_multiplier * 100) / _voltage_divisor);
}

void setCurrent2(int phase, uint32_t current) {

  if ((_current_multiplier == 0) || (_current_divisor == 0))
    setCurrent(phase, current * 1);
  else	
   setCurrent(phase, (current * _current_multiplier * 1000) / _current_divisor);
}

void setPowerActive2(int phase, int64_t power) {

  if ((_active_power_multiplier == 0) || (_active_power_divisor == 0))
    setPowerActive(phase, power * 100000);
  else
    setPowerActive(phase, (power * _active_power_multiplier * 100000) / _active_power_divisor);
}

void setFwdActEnergy2(int phase, unsigned _supla_int64_t energy) {

  if (energy > _energy_initial_counter)
    energy -= _energy_initial_counter;
  else
    energy = 0;

  setFwdActEnergy(phase, energy);
}

void onInit() override {
  //readValuesFromDevice();
  //updateChannelValues();
  if (_timeout_enabled)
    getChannel()->setStateOffline();
}

virtual void readValuesFromDevice() {

  return; //rest is invalid for now
  if (_gateway && Zigbee.started()) {
    if ((_active_query || 0 /*(emValue.m[0].voltage[0] == 0)*/) && _gateway->sendAttributeRead(&_device, ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT,      			ESP_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_RMSVOLTAGE_ID, true))
         setVoltage(0, (*(uint16_t *)_gateway->getReadAttrLastResult()->data.value) * 100);
    if ((_active_query || 0/*(!currentMeasurementAvailable)*/) && _gateway->sendAttributeRead(&_device, ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT, 		ESP_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_RMSCURRENT_ID, true))
         setCurrent(0, (*(uint16_t *)_gateway->getReadAttrLastResult()->data.value) * 1);
    if ((_active_query || 0/*(!powerActiveMeasurementAvailable)*/) && _gateway->sendAttributeRead(&_device, ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT, 	ESP_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_ID, true))
         setPowerActive(0, (*(uint16_t *)_gateway->getReadAttrLastResult()->data.value)*100000*_active_power_multiplier);
       
    if (_active_query && _gateway->sendAttributeRead(&_device,ESP_ZB_ZCL_CLUSTER_ID_METERING, ESP_ZB_ZCL_ATTR_METERING_CURRENT_SUMMATION_DELIVERED_ID, true)) {
      esp_zb_uint48_t *value = (esp_zb_uint48_t *)_gateway->getReadAttrLastResult()->data.value;
      _supla_int64_t energy = ((_supla_int64_t)value->high << 32) + value->low;
      setFwdActEnergy(0, energy * 1000);
    }
  }
}

void resetStorage() {
    //if (_gateway && Zigbee.started()) {

      //_gateway->sendDeviceFactoryReset(&_device, _isTuya);
      //_gateway->sendAttributeRead(&_device,ESP_ZB_ZCL_CLUSTER_ID_METERING, ESP_ZB_ZCL_ATTR_METERING_CURRENT_SUMMATION_DELIVERED_ID, false); // ) {
     //}
    _energy_initial_counter = emValue.total_forward_active_energy[0];
    if (_data_counter) {
      *_data_counter = _energy_initial_counter;
      _gateway->requestDataSave(0);
    }
  }


  void ping() {

  uint16_t attributes[3] = {ESP_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_RMSVOLTAGE_ID, ESP_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_RMSCURRENT_ID,
                            ESP_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_ID};

  if (_gateway && Zigbee.started())
    _gateway->sendAttributesRead(&_device, ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT, 3, &attributes[0]);
}

void pong() {

  _last_ping_ms = millis();
  _last_seen_ms = _last_ping_ms;
  
  if (!getChannel()->isStateOnline()) 
	  getChannel()->setStateOnline();
}


void setKeepAliveSecs(uint32_t keep_alive_secs) {

  _keep_alive_ms = keep_alive_secs * 1000;
  if (_keep_alive_ms == 0) {
    _keep_alive_enabled = false;
    //getChannel()->setStateOnline();
  } else
    _keep_alive_enabled = true;
}

  void setTimeoutSecs(uint32_t timeout_secs) {

  _timeout_ms = timeout_secs * 1000;
  if (_timeout_ms == 0) {
    _timeout_enabled = false;
    getChannel()->setStateOnline();
  } else
    _timeout_enabled = true;
}

void setRefreshSecs(uint32_t refresh_secs) {

  _refresh_ms = refresh_secs * 1000;
  if (_refresh_ms == 0) 
    _refresh_enabled = false; 
  else 
    _refresh_enabled = true;
 
}

void setEnergyInitialCounter(uint64_t energy_initial_counter) {

  _energy_initial_counter = energy_initial_counter;
}

uint32_t getKeepAliveSecs() {

  return _keep_alive_ms * 1000;
}

uint32_t getTimeoutSecs() {

  return _timeout_ms * 1000;
}

uint32_t getRefreshSecs() {

  return _refresh_ms * 1000;
}

uint64_t getEnergyInitialCounter() {

  return _energy_initial_counter;
}


void iterateAlways() override {

  Supla::Sensor::ElectricityMeter::iterateAlways();

  if ((_refresh_enabled) && ((millis() - _last_refresh_ms) > _refresh_ms)) {
    if (_gateway) {
      
      _last_seen_ms = _gateway->getZbgDeviceUnitLastSeenMs(_device.short_addr);
      //if ((millis() - _last_seen_ms) > _keep_alive_ms) {
      	ping();
        _last_ping_ms = millis();
      //} else {
        //_last_ping_ms = _last_seen_ms;
        if ((!getChannel()->isStateOnline()) && ((millis() - _last_seen_ms) < _timeout_ms)) 
	  getChannel()->setStateOnline();
      //}
    }
    _last_refresh_ms = millis();
  } else
  if (_keep_alive_enabled && ((millis() - _last_ping_ms) > _keep_alive_ms)) {
    if (_gateway) {
      
      _last_seen_ms = _gateway->getZbgDeviceUnitLastSeenMs(_device.short_addr);
      if ((millis() - _last_seen_ms) > _keep_alive_ms) {
      	ping();
        _last_ping_ms = millis();
      } else {
        _last_ping_ms = _last_seen_ms;
        if (!getChannel()->isStateOnline()) 
	  getChannel()->setStateOnline();
      }
    }
  }
  if (_timeout_enabled && getChannel()->isStateOnline() && ((millis() - _last_seen_ms) > _timeout_ms)) {
	  log_i("current_millis %u, _last_seen_ms %u", millis(), _last_seen_ms);
    _last_seen_ms = _gateway->getZbgDeviceUnitLastSeenMs(_device.short_addr);
    log_i("current_millis %u, _last_seen_ms(updated) %u", millis(), _last_seen_ms);
    if ((millis() - _last_seen_ms) > _timeout_ms)
      getChannel()->setStateOffline();
  }
}

  protected:
    ZigbeeGateway 	 *_gateway = nullptr;
    zbg_device_params_t  _device;  
    uint64_t		 *_data_counter = nullptr;
    bool                 _active_query = false;
    esp_zb_uint48_t      _write_mask;
    bool                 _isTuya = false;

    uint16_t _voltage_multiplier = 0;
    uint16_t _voltage_divisor 	= 0;
    uint16_t _current_multiplier = 0;
    uint16_t _current_divisor = 0;
    uint16_t _active_power_multiplier = 0;
    uint16_t _active_power_divisor = 0;

    bool _keep_alive_enabled = true;
    bool _timeout_enabled    = true;
    bool _refresh_enabled    = true;
 
    uint64_t _energy_initial_counter = 0;

    uint32_t _refresh_ms    = 30000;
    uint32_t _keep_alive_ms = 30000;
    uint32_t _timeout_ms    = 60000;
    
    uint32_t _last_ping_ms    = 0;
    uint32_t _last_seen_ms    = 0;
    uint32_t _last_refresh_ms = 0;

};

};  // namespace Sensor
};  // namespace Supla

#endif  //Z2S_ONE_PHASE_ELECTRICITY_METER_H_