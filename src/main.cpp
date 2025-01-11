#include <Arduino.h>
#include "ZigbeeGateway.h"
#include "esp_coexist.h"
#include <map>
#include "supla/sensor/general_purpose_measurement.h"
#include "supla/control/virtual_relay.h"
#include "SuplaDevice.h"
#include <string>
#define GATEWAY_ENDPOINT_NUMBER 1

#define BUTTON_PIN 9 // Boot button for C6/
ZigbeeGateway zbGateway = ZigbeeGateway(GATEWAY_ENDPOINT_NUMBER);
// SuplaGateway suplaGateway = SuplaGateway();
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
std::map<std::string, Supla::Sensor::GeneralPurposeMeasurement *> suplaChannels{};

std::string longAddressAsString(const esp_zb_ieee_addr_t ieee_addr_64)
{
  // should be A4:C1:38:97:D0:A9:3C:8D
  char buffer[24];
  sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", ieee_addr_64[7], ieee_addr_64[6], ieee_addr_64[5], ieee_addr_64[4],
          ieee_addr_64[3], ieee_addr_64[2], ieee_addr_64[1], ieee_addr_64[0]);
  std::string longAddressString(buffer);
  log_i("parsed address: %s", longAddressString.c_str());
  Serial.print(buffer);
  return longAddressString;
}

void displaySuplaChannels()
{
  log_i("displaySuplaChannels");
  log_i("available channels: %d", suplaChannels.size());
  std::map<std::string, Supla::Sensor::GeneralPurposeMeasurement *>::iterator it;
  for (it = suplaChannels.begin(); it != suplaChannels.end(); it++)
  {
    log_i("Key: %s Value: %s", it->first.c_str(), it->second);
  }
  log_i("displayedSuplaChannels");
}

void onNewValueReceive(uint16_t cluster_id, const esp_zb_ieee_addr_t long_address, const esp_zb_zcl_attribute_t *attribute)
{
  log_i("onNewValueReceive");
  std::string longAddressString = longAddressAsString(long_address);

  if (cluster_id == ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT)
  {
    if (attribute->id == ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID && attribute->data.type == ESP_ZB_ZCL_ATTR_TYPE_S16)
    {
      int16_t value = attribute->data.value ? *(int16_t *)attribute->data.value : 0;
      displaySuplaChannels();
      Supla::Sensor::GeneralPurposeMeasurement *sensor = (Supla::Sensor::GeneralPurposeMeasurement *)suplaChannels[longAddressString];
      if (sensor != nullptr)
      {

        log_i("zbAttributeRead temperature measurement %f", ((float)value) / 100);
        sensor->setValue(((float)value) / 100);
      }
      else
      {
        log_i("sensor not found");
      }
    }
  }
  /*
  if (cluster_id == ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT)
  {
    if (attribute->id == ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID && attribute->data.type == ESP_ZB_ZCL_ATTR_TYPE_U16)
    {
      int16_t value = attribute->data.value ? *(int16_t *)attribute->data.value : 0;
      Supla::Sensor::GeneralPurposeMeasurement *sensor = (Supla::Sensor::GeneralPurposeMeasurement *)suplaChannels[longAddressString];
      sensor->setValue(((double)value) / 100);
      log_i("zbAttributeRead humidity measurement %f", ((float)value) / 100);
    }
  }
  */
}

void addSuplaChannel(esp_zb_ieee_addr_t longAddress, uint16_t shortAddress, esp_zb_zcl_cluster_id_t clusterId)
{
  std::string longAddressString = longAddressAsString(longAddress);
  switch (clusterId)
  {
  case ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT:
  {
    auto sensor = new Supla::Sensor::GeneralPurposeMeasurement();
    sensor->setDefaultUnitAfterValue("%");
    sensor->setInitialCaption("Humidity");
    // sensor->setValue();
    if (suplaChannels.count(longAddressString) ==0)
      {
        suplaChannels.insert({longAddressString, sensor});
      }
    break;
  }

    /*
      case ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT:
      {
        auto sensor = new Supla::Sensor::GeneralPurposeMeasurement();
        sensor->setDefaultUnitAfterValue("C");
        sensor->setInitialCaption("Temperature");
        // sensor->setValue();
        suplaChannels.insert({longAddressString, sensor});
        break;
      }
    */
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

static void Z2S_simple_desc_req(esp_zb_zdp_status_t zdo_status, esp_zb_af_simple_desc_1_1_t *simple_desc, void *user_ctx)
{
  if (zdo_status == ESP_ZB_ZDP_STATUS_SUCCESS)
  {

    uint16_t short_addr = *((uint16_t *)(user_ctx));

    log_i("Z2S simple_desc_req: device address %d, endpoint # %d", short_addr, simple_desc->endpoint);
    log_i("Z2S simple_desc_req: in clusters # %d, out clusters # %d", simple_desc->app_input_cluster_count, simple_desc->app_output_cluster_count);
    for (int i = 0; i < simple_desc->app_input_cluster_count; i++)
    {

      log_i("In cluster # %d, id %d ", i + 1, *(simple_desc->app_cluster_list + i));
    }
    for (int i = 0; i < simple_desc->app_output_cluster_count; i++)
    {
      log_i("Out cluster # %d, id %d ", i + 1, *(simple_desc->app_cluster_list + simple_desc->app_input_cluster_count + i));
    }
  }
  else
    log_i("Z2S simple desc failed");
}
static void Z2S_active_ep_req(esp_zb_zdp_status_t zdo_status, uint8_t ep_count, uint8_t *ep_id_list, void *user_ctx)
{
  if (zdo_status == ESP_ZB_ZDP_STATUS_SUCCESS)
  {

    uint16_t short_addr = *((uint16_t *)(user_ctx));
    esp_zb_zdo_simple_desc_req_param_t cl_cmd_req;
    log_i("Z2S active_ep_req: device address %d, endpoints count %d", short_addr, ep_count);

    for (int i = 0; i < ep_count; i++)
    {
      log_i("Endpont # %d, id %d ", i + 1, *(ep_id_list + i));
      cl_cmd_req.addr_of_interest = short_addr;
      cl_cmd_req.endpoint = *(ep_id_list + i);
      esp_zb_zdo_simple_desc_req(&cl_cmd_req, Z2S_simple_desc_req, &short_addr);
    }
  }
  else
    log_i("Z2S active_ep_req failed");
}

uint16_t short_addr_req;

void setup()
{
  Serial.begin(115200);
  Serial.println("starting");
  pinMode(BUTTON_PIN, INPUT);

  zbGateway.onStatusNotification(sz_ias_zone_notification);
  zbGateway.setManufacturerAndModel("Espressif", "MyZigbeeGateway");
  zbGateway.allowMultipleBinding(true);
  zbGateway.onNewValueReceive(onNewValueReceive);
  Zigbee.addEndpoint(&zbGateway);
  Serial.println("added endpoint");
  // Open network for 180 seconds after boot
  Zigbee.setRebootOpenNetwork(30);
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

    esp_coex_wifi_i154_enable();

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
        // addSuplaChannel(joined_device->ieee_addr, joined_device->short_addr, ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT);

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
        addSuplaChannel(joined_device->ieee_addr, joined_device->short_addr, ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT);
        addSuplaChannel(joined_device->ieee_addr, joined_device->short_addr, ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT);
        esp_zb_lock_release();
      }
      else if (strcmp(zbd_model_name, "TS0001") == 0)
      {
        esp_zb_lock_acquire(portMAX_DELAY);
        zbGateway.bindDeviceCluster(joined_device, ESP_ZB_ZCL_CLUSTER_ID_ON_OFF);
        // addSuplaChannel(joined_device->ieee_addr, joined_device->short_addr, ESP_ZB_ZCL_CLUSTER_ID_ON_OFF);
        esp_zb_lock_release();
      }
      else if (strcmp(zbd_model_name, "TS0601") == 0)
      {
        esp_zb_lock_acquire(portMAX_DELAY);
        zbGateway.bindDeviceCluster(joined_device, 61184);
        // zbGateway.bindDeviceCluster(joined_device, ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT);
        // zbGateway.bindDeviceCluster(joined_device, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT);
        // zbGateway.bindDeviceCluster(joined_device, ESP_ZB_ZCL_CLUSTER_ID_METERING);
        // zbGateway.bindDeviceCluster(joined_device, ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT);

        // addSuplaChannel(joined_device->ieee_addr, joined_device->short_addr, ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT);
        // addSuplaChannel(joined_device->ieee_addr, joined_device->short_addr, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT);
        // addSuplaChannel(joined_device->ieee_addr, joined_device->short_addr, ESP_ZB_ZCL_CLUSTER_ID_METERING);
        // addSuplaChannel(joined_device->ieee_addr, joined_device->short_addr, ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT);
        esp_zb_lock_release();
      }
      else
        log_d("Unknown model %s, no binding is possible", zbd_model_name);
    }
  }
  if (millis() - printTime > 10000)
  {
    zbGateway.printGatewayDevices();
    zbGateway.printJoinedDevices();
    // if (zbGateway.getGatewayDevices().size() > 0) zbGateway.setIASZReporting(0, 0);
    // if (zbGateway.getGatewayDevices().size() > 0) {
    for ([[maybe_unused]]
         const auto &device : zbGateway.getGatewayDevices())
    {

      esp_zb_zdo_active_ep_req_param_t ep_cmd_req;

      esp_zb_zcl_disc_attr_cmd_t disc_attr_cmd_req;
      short_addr_req = device->short_addr; // zbGateway.getGatewayDevices().front()->short_addr;
      // log_i("address before zdo_active_ep_req %d", short_addr_req);
      ep_cmd_req.addr_of_interest = short_addr_req;

      /*disc_attr_cmd_req.zcl_basic_cmd.dst_endpoint = 1;
      disc_attr_cmd_req.zcl_basic_cmd.dst_addr_u.addr_short = short_addr_req;
      disc_attr_cmd_req.zcl_basic_cmd.src_endpoint = 1;
      disc_attr_cmd_req.address_mode = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
      disc_attr_cmd_req.cluster_id = 61184;
      disc_attr_cmd_req.direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV;
      disc_attr_cmd_req.start_attr_id = 0;
      disc_attr_cmd_req.max_attr_number = 99;*/
      esp_zb_lock_acquire(portMAX_DELAY);
      esp_zb_zdo_active_ep_req(&ep_cmd_req, Z2S_active_ep_req, &short_addr_req);
      // esp_zb_zdo_simple_desc_req(&cl_cmd_req, Z2S_simple_desc_req, &short_addr_req);
      // esp_zb_zcl_disc_attr_cmd_req(&disc_attr_cmd_req);
      esp_zb_lock_release();
    }
    printTime = millis();
  }
}
