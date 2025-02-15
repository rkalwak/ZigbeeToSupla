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


#include "Z2S_Tuya_dimmer_switch.h"
#include <supla/log_wrapper.h>


Supla::Control::Z2S_TuyaDimmerSwitch::Z2S_TuyaDimmerSwitch(ZigbeeGateway *gateway, zbg_device_params_t *device)
    : _gateway(gateway){
    memcpy(&_device, device, sizeof(zbg_device_params_t)); 
}

void Supla::Control::Z2S_TuyaDimmerSwitch::turnOn() {
}

void Supla::Control::Z2S_TuyaDimmerSwitch::turnOff() {
}

void Supla::Control::Z2S_TuyaDimmerSwitch::toggle() {
}

bool Supla::Control::Z2S_TuyaDimmerSwitch::isOn() {
    return false;
}

void Supla::Control::Z2S_TuyaDimmerSwitch::sendValueToDevice(uint32_t brightness) {
}
