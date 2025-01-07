#include <Arduino.h>
#include "ZigbeeGateway.h"

//#include "esp_coexist.h"

//#include "supla/sensor/general_purpose_measurement.h"
//#include "supla/control/virtual_relay.h"
#define GATEWAY_ENDPOINT_NUMBER 1

#define BUTTON_PIN 9 // Boot button for C6/
ZigbeeGateway zbGateway = ZigbeeGateway(GATEWAY_ENDPOINT_NUMBER);

void sz_ias_zone_notification(int status, uint8_t *ieee_addr_64)
{
  Serial.println("in sz_ias_zone_nitification");
  Serial.println(status);
  Serial.println(ieee_addr_64[0]);
}

uint32_t startTime = 0;
uint32_t printTime = 0;
uint32_t zbInit_delay = 0;

bool zbInit = true;
zb_device_params_t *gateway_device;
zb_device_params_t *joined_device;
char zbd_model_name[64];

/*
void addSuplaChannel(esp_zb_ieee_addr_t longAddress, uint16_t shortAddress, esp_zb_zcl_cluster_id_t clusterId)
{

  switch (clusterId)
  {
  case ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT:
  {
    auto sensor = new Supla::Sensor::GeneralPurposeMeasurement();
    sensor->setDefaultUnitAfterValue("%");
    sensor->setInitialCaption("Humidity");
        // sensor->setValue();
    break;
  }

  case ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT:
  {
    auto sensor = new Supla::Sensor::GeneralPurposeMeasurement();
    sensor->setDefaultUnitAfterValue("C");
    sensor->setInitialCaption("Temperature");
    // sensor->setValue();
    break;
  }

  case ESP_ZB_ZCL_CLUSTER_ID_ON_OFF:
  {
    auto relay = new Supla::Control::VirtualRelay();
    relay->setInitialCaption("Power switch");
    relay->setDefaultFunction(SUPLA_CHANNELFNC_POWERSWITCH);
    
    // relay->turnOff();
    // relay->turnOn();
    break;
  }
  default:
    break;
  }
}

*/

void setup()
{
  Serial.begin(115200);
  Serial.println("starting");
  pinMode(BUTTON_PIN, INPUT);

  zbGateway.onStatusNotification(sz_ias_zone_notification);
  zbGateway.setManufacturerAndModel("Espressif", "MyZigbeeGateway");
  zbGateway.allowMultipleBinding(true);
  Zigbee.addEndpoint(&zbGateway);
  Serial.println("added endpoint");
  // Open network for 180 seconds after boot
  Zigbee.setRebootOpenNetwork(180);
  Serial.println("set reboot");
  startTime = millis();
  printTime = millis();
  zbInit_delay = millis();
}

void loop()
{
  if (zbInit)
  {
    Serial.println("zbInit");

    //  esp_coex_wifi_i154_enable();

    // esp_zb_aps_data_indication_handler_register(zb_aps_data_indication_handler);
    // esp_zb_aps_data_confirm_handler_register(zb_aps_data_confirm_handler);

    // When all EPs are registered, start Zigbee with ZIGBEE_COORDINATOR mode
    if (!Zigbee.begin(ZIGBEE_COORDINATOR))
    {
      Serial.println("Zigbee failed to start!");
      Serial.println("Rebooting...");
      ESP.restart();
    }
    zbInit = false;
  }

  /*
    if (digitalRead(BUTTON_PIN) == LOW)
    { // Push button pressed
      // Key debounce handling
      delay(100);

      while (digitalRead(BUTTON_PIN) == LOW)
      {
        delay(50);
        if ((millis() - startTime) > 3000)
        {
          // If key pressed for more than 3secs, factory reset Zigbee and reboot
          Serial.printf("Resetting Zigbee to factory settings, reboot.\n");
          Zigbee.factoryReset();
        }
      }
      Zigbee.openNetwork(180);
    }
    delay(100);
  */
  if (zbGateway.isNewDeviceJoined())
  {

    zbGateway.clearNewDeviceJoined();
    zbGateway.printJoinedDevices();

    while (!zbGateway.getJoinedDevices().empty())
    {
      joined_device = zbGateway.getLastJoinedDevice();
      log_d("getJoinedDEvices %d %d %d before", zbGateway.getJoinedDevices().empty(), zbGateway.getJoinedDevices().size(), zbGateway.getJoinedDevices().max_size());
      // zbGateway.getJoinedDevices().pop_back();
      // zbGateway.getJoinedDevices().clear();
      log_d("getJoinedDEvices %d %d %d after", zbGateway.getJoinedDevices().empty(), zbGateway.getJoinedDevices().size(), zbGateway.getJoinedDevices().max_size());
      zbGateway.printJoinedDevices();

      log_i("manufacturer %s ", zbGateway.readManufacturer(joined_device->endpoint, joined_device->short_addr, joined_device->ieee_addr));
      strcpy(zbd_model_name, zbGateway.readModel(joined_device->endpoint, joined_device->short_addr, joined_device->ieee_addr));
      log_i("model %s ", zbd_model_name);
      if (strcmp(zbd_model_name, "ZigbeeTempSensor") == 0)
      {
        esp_zb_lock_acquire(portMAX_DELAY);
        zbGateway.bindDeviceCluster(joined_device, ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT);
        //addSuplaChannel(joined_device->ieee_addr, joined_device->short_addr, ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT);

        esp_zb_lock_release();
      }
      else if (strcmp(zbd_model_name, "TS0203") == 0)
      {
        esp_zb_lock_acquire(portMAX_DELAY);
        zbGateway.bindDeviceCluster(joined_device, ESP_ZB_ZCL_CLUSTER_ID_IAS_ZONE);
        esp_zb_lock_release();
      }
      else if (strcmp(zbd_model_name, "TS0201") == 0)
      {
        esp_zb_lock_acquire(portMAX_DELAY);
        zbGateway.bindDeviceCluster(joined_device, ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT);
        //addSuplaChannel(joined_device->ieee_addr, joined_device->short_addr, ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT);
        //addSuplaChannel(joined_device->ieee_addr, joined_device->short_addr, ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT);
        esp_zb_lock_release();
      }
       else if (strcmp(zbd_model_name, "TS0001") == 0)
      {
        esp_zb_lock_acquire(portMAX_DELAY);
        zbGateway.bindDeviceCluster(joined_device, ESP_ZB_ZCL_CLUSTER_ID_ON_OFF);
        //addSuplaChannel(joined_device->ieee_addr, joined_device->short_addr, ESP_ZB_ZCL_CLUSTER_ID_ON_OFF);
        esp_zb_lock_release();
      }
      else
        log_d("Unknown model %s, no binding is possible", zbd_model_name);
    }
  }
  if (millis() - printTime > 60000)
  {
    zbGateway.printGatewayDevices();
    zbGateway.printJoinedDevices();
    if (zbGateway.getGatewayDevices().size() > 0)
      zbGateway.setIASZReporting(10, 15);
    printTime = millis();
  }
}
