#ifndef Z2S_ONE_PHASE_ELECTRICITY_METER_H_
#define Z2S_ONE_PHASE_ELECTRICITY_METER_H_


#include <supla/log_wrapper.h>
#include <supla/sensor/one_phase_electricity_meter.h>
#include "ZigbeeGateway.h"

namespace Supla {
namespace Sensor {

class Z2S_OnePhaseElectricityMeter : public OnePhaseElectricityMeter {
 public:
  Z2S_OnePhaseElectricityMeter(ZigbeeGateway *gateway, zb_device_params_t *device, bool active_query = false) : _gateway(gateway) {
	memcpy(&_device, device, sizeof(zb_device_params_t));
	_active_query = active_query;

  }

  void onInit() override {
    readValuesFromDevice();
    updateChannelValues();
  }

  virtual void readValuesFromDevice() {
    if (_gateway && Zigbee.started()) {

       if ((_active_query || (emValue.m[0].voltage[0] == 0)) && _gateway->sendAttributeRead(&_device, ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT, ESP_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_RMSVOLTAGE_ID, true))
         setVoltage(0, (*(uint16_t *)_gateway->getReadAttrLastResult()->data.value) * 100);
       if ((_active_query || (!currentMeasurementAvailable)) && _gateway->sendAttributeRead(&_device, ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT, ESP_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_RMSCURRENT_ID, true))
         setCurrent(0, (*(uint16_t *)_gateway->getReadAttrLastResult()->data.value) * 1);
       if ((_active_query || (!powerActiveMeasurementAvailable)) && _gateway->sendAttributeRead(&_device, ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT, ESP_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_ID, true))
         setPowerActive(0, (*(uint16_t *)_gateway->getReadAttrLastResult()->data.value)*10000);
       
	if (_active_query && _gateway->sendAttributeRead(&_device,ESP_ZB_ZCL_CLUSTER_ID_METERING, ESP_ZB_ZCL_ATTR_METERING_CURRENT_SUMMATION_DELIVERED_ID, true)) {
         esp_zb_uint48_t *value = (esp_zb_uint48_t *)_gateway->getReadAttrLastResult()->data.value;
         //esp_zb_uint24_t *multiplier;
         //esp_zb_uint24_t *divisor;

         //if (Gateway->sendAttributeRead(&Device,ESP_ZB_ZCL_CLUSTER_ID_METERING, ESP_ZB_ZCL_ATTR_METERING_MULTIPLIER_ID, true))
         //  multiplier = (esp_zb_uint24_t *)Gateway->getReadAttrLastResult()->data.value;
	//if (Gateway->sendAttributeRead(&Device,ESP_ZB_ZCL_CLUSTER_ID_METERING, ESP_ZB_ZCL_ATTR_METERING_DIVISOR_ID, true))
        //   divisor = (esp_zb_uint24_t *)Gateway->getReadAttrLastResult()->data.value;

	 //log_i("value high 0x%x and low 0x%x, multiplier high 0x%x low 0x%x, divisor high 0x%x low 0x%x", value->high, value->low,
           //    multiplier->high, multiplier->low, divisor->high, divisor->low);

          _supla_int64_t energy = ((_supla_int64_t)value->high << 32) + value->low;
	//	log_i("energy after shit 0x%x", energy);
         //_supla_int64_t multiplier64 = ((_supla_int64_t)multiplier->high << 16) + multiplier->low;
	//_supla_int64_t divisor64 = ((_supla_int64_t)divisor->high << 16) + divisor->low;
	//if (multiplier64 !=0) energy = energy * multiplier64;
	//if (divisor64 != 0) energy = energy / divisor64;
	 setFwdActEnergy(0, energy);
       }
    }
  }

 protected:
   ZigbeeGateway 	*_gateway = nullptr;
   zb_device_params_t 	_device;  
   bool                 _active_query = false;
};

};  // namespace Sensor
};  // namespace Supla

#endif  //Z2S_ONE_PHASE_ELECTRICITY_METER_H_