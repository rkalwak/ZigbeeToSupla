/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef SRC_SUPLA_CONTROL_Z2S_TUYA_DIMMER_BULB_H_
#define SRC_SUPLA_CONTROL_Z2S_TUYA_DIMMER_BULB_H_

#include "Z2S_dimmer_base.h"
#include "ZigbeeGateway.h"


namespace Supla {
namespace Control {
class Z2S_TuyaDimmerBulb: public Z2S_DimmerBase {

public:

  Z2S_TuyaDimmerBulb(ZigbeeGateway *gateway, zbg_device_params_t *device);

  void turnOn() override;
  void turnOff() override;
  void toggle() override;
  bool isOn() override;
  
  void onInit() override;
  void iterateAlways() override;

  void setStateOnServer(bool state) override;
  void sendValueToDevice(uint32_t brightness) override;

  bool ping();

protected:

  uint8_t _Tuya_bulb_mode = 0;
  bool _state = false;
  ZigbeeGateway *_gateway = nullptr;
  zbg_device_params_t 	_device;
  uint32_t _last_iterate_ms = 0;

private:

  void setTuyaMode(uint8_t mode);
  bool getTuyaMode();


};

} //Control
}//Supla

#endif