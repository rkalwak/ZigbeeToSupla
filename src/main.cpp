#include <Arduino.h>
#include "ZigbeeGateway.h"

#include "esp_coexist.h"
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
void setup()
{
  Serial.begin(115200);
    Serial.println("starting");
  pinMode(BUTTON_PIN, INPUT);

  zbGateway.onStatusNotification(sz_ias_zone_notification);
  zbGateway.setManufacturerAndModel("Espressif", "MyZigbeeGateway");
  zbGateway.allowMultipleBinding(true);
  Zigbee.addEndpoint(&zbGateway);

  // Open network for 180 seconds after boot
  Zigbee.setRebootOpenNetwork(180);

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
        esp_zb_lock_release();
      }
      else if (strcmp(zbd_model_name, "TS0203") == 0)
      {
        esp_zb_lock_acquire(portMAX_DELAY);
        zbGateway.bindDeviceCluster(joined_device, ESP_ZB_ZCL_CLUSTER_ID_IAS_ZONE);
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
