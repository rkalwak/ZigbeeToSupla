/* Class of Zigbee Temperature sensor endpoint inherited from common EP class */

#pragma once

#include "soc/soc_caps.h"
#include "sdkconfig.h"
#if SOC_IEEE802154_SUPPORTED 
//&& CONFIG_ZB_ENABLED

#include "ZigbeeEP.h"
#include "ha/esp_zigbee_ha_standard.h"

//define the thermostat configuration to avoid narrowing conversion issue in zigbee-sdk
#define ZB_DEFAULT_GATEWAY_CONFIG()                                                               \
  {                                                                                                  \
    .basic_cfg =                                                                                     \
      {                                                                                              \
        .zcl_version = ESP_ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE,                                   \
        .power_source = ESP_ZB_ZCL_BASIC_POWER_SOURCE_DEFAULT_VALUE,                                 \
      },                                                                                             \
    .identify_cfg =                                                                                  \
      {                                                                                              \
        .identify_time = ESP_ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE,                            \
      }                                                                                        \
  }

#define TUYA_PRIVATE_CLUSTER_0    0xE000
#define TUYA_PRIVATE_CLUSTER_1    0xE001
#define TUYA_PRIVATE_CLUSTER_EF00 0xEF00

#define READ_ATTR_TSN_UNKNOWN 0x00
#define READ_ATTR_TSN_SYNC    0x01
#define READ_ATTR_TSN_ASYNC   0x02


#define ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID 0x0021

typedef struct findcb_userdata_s {
  uint8_t   _endpoint;
  uint16_t  _cluster_id;
} findcb_userdata_t;

typedef struct query_basic_cluster_data_s {
  char zcl_manufacturer_name[32];
  uint8_t zcl_version_id;
  uint8_t zcl_application_version_id;
  uint8_t zcl_power_source_id;
  char zcl_model_name[32];
} query_basic_cluster_data_t;

typedef struct zbg_device_params_s {
  uint32_t model_id;
  bool rejoined;
  bool ZC_binding;
  esp_zb_ieee_addr_t ieee_addr;
  uint8_t endpoint;
  uint16_t cluster_id;
  uint16_t short_addr;
  uint32_t user_data;
} zbg_device_params_t;

class ZigbeeGateway : public ZigbeeEP {
public:
  ZigbeeGateway(uint8_t endpoint);
  ~ZigbeeGateway();

  std::list<zbg_device_params_t *> getGatewayDevices() const {
    return _gateway_devices;
  }
  void addGatewayDevice(zbg_device_params_t * device){ 
    if (device) _gateway_devices.push_back(device);
  }
  std::list<zbg_device_params_t *> getJoinedDevices() const {
    return _joined_devices;
  }
  zbg_device_params_t * getLastJoinedDevice() { 
    zbg_device_params_t *last_joined_device = NULL;
    if (!_joined_devices.empty()) {
      last_joined_device = _joined_devices.back();
      _joined_devices.pop_back(); 
    }
    return last_joined_device;
  }
  void printGatewayDevices();
  void printJoinedDevices();
  static bool isNewDeviceJoined() {
    return _new_device_joined;
  }
  static void clearNewDeviceJoined() {
    _new_device_joined = false;
  }
  static void setEndpoints2Bind(uint8_t endpoints_count) {
    _endpoints_2_bind = endpoints_count;
  }
  static void setClusters2Bind(uint16_t clusters_count) {
    _clusters_2_bind = clusters_count;
  }
  static query_basic_cluster_data_t * getQueryBasicClusterData() {
    return &_last_device_query;
  }
    static esp_zb_zcl_attribute_t * getReadAttrLastResult() {
    return &_read_attr_last_result;
  }

  void zbPrintDeviceDiscovery (zbg_device_params_t * device);
  static void bindDeviceCluster(zbg_device_params_t *,int16_t cluster_id);

  bool zbQueryDeviceBasicCluster(zbg_device_params_t * device);
  //void zbReadBasicCluster(const esp_zb_zcl_attribute_t *attribute) override;
  void zbReadBasicCluster(esp_zb_zcl_addr_t src_address, uint16_t src_endpoint, uint16_t cluster_id, esp_zb_zcl_attribute_t *attribute) override;
  void setClusterReporting(zbg_device_params_t * device, uint16_t cluster_id, uint16_t attribute_id, uint8_t attribute_type,
                                        uint16_t min_interval, uint16_t max_interval, uint16_t delta, bool ack);
  void readClusterReportCmd(zbg_device_params_t * device, uint16_t cluster_id, uint16_t attribute_id, bool ack);
  void readClusterReportCfgCmd(zbg_device_params_t * device, uint16_t cluster_id, uint16_t attribute_id, bool ack);

  bool sendAttributeRead(zbg_device_params_t * device, int16_t cluster_id, uint16_t attribute_id, bool ack = false);
  void sendAttributeWrite( zbg_device_params_t * device, int16_t cluster_id, uint16_t attribute_id,
                                        esp_zb_zcl_attr_type_t attribute_type, uint16_t attribute_size, void *attribute_value);
  void sendIASzoneEnrollResponseCmd(zbg_device_params_t *device, uint8_t enroll_rsp_code, uint8_t zone_id);

  void sendOnOffCmd(zbg_device_params_t *device, bool value); 
  void sendLevelMoveToLevelCmd(zbg_device_params_t *device, uint8_t level, uint16_t transition_time);
  void sendColorMoveToHueCmd(zbg_device_params_t *device, uint8_t hue, uint8_t directon, uint16_t transition_time);
  void sendColorMoveToSaturationCmd(zbg_device_params_t *device, uint8_t saturation, uint16_t transition_time);
  void sendColorMoveToHueAndSaturationCmd(zbg_device_params_t *device, uint8_t hue, uint8_t saturation, uint16_t transition_time);
  void sendColorEnhancedMoveToHueAndSaturationCmd(zbg_device_params_t *device, uint16_t enhanced_hue, uint8_t saturation, uint16_t transition_time);
  void sendColorMoveToColorCmd(zbg_device_params_t *device, uint16_t color_x, uint16_t color_y, uint16_t transition_time);
  void sendColorMoveToColorTemperatureCmd(zbg_device_params_t *device, uint16_t color_temperature, uint16_t transition_time);

  void sendDeviceFactoryReset(zbg_device_params_t *device, bool isTuya = false);
  void sendCustomClusterCmd(zbg_device_params_t * device, int16_t custom_cluster_id, uint16_t custom_command_id, esp_zb_zcl_attr_type_t data_type, 
                            uint16_t custom_data_size, uint8_t *custom_data, bool ack = false, uint8_t direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV);

  
  void onIASzoneStatusChangeNotification (void (*callback)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, int, signed char rssi)) {
    _on_IAS_zone_status_change_notification = callback;
  }
  void onTemperatureReceive(void (*callback)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, float, signed char rssi)) {
    _on_temperature_receive = callback; 
  }
  void onHumidityReceive(void (*callback)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, float, signed char rssi)) {
    _on_humidity_receive = callback;
  }
  void onOnOffReceive(void (*callback)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, bool, signed char rssi)) {
    _on_on_off_receive = callback;
  }
  void onRMSVoltageReceive(void (*callback)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint16_t, signed char rssi)) {
    _on_rms_voltage_receive = callback;
  }
  void onRMSCurrentReceive(void (*callback)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint16_t, signed char rssi)) {
    _on_rms_current_receive = callback;
  }
  void onRMSActivePowerReceive(void (*callback)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint16_t, signed char rssi)) {
    _on_rms_active_power_receive = callback;
  }
  void onCurrentSummationReceive(void (*callback)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint64_t, signed char rssi)) {
    _on_current_summation_receive = callback;
  }
  void onBatteryPercentageReceive(void (*callback)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint8_t)) {
    _on_battery_percentage_receive = callback;
   }
   void onCurrentLevelReceive(void (*callback)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint8_t, signed char rssi)) {
    _on_current_level_receive = callback;
   }
   void onColorHueReceive(void (*callback)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint8_t, signed char rssi)) {
    _on_color_hue_receive = callback;
   }
   void onColorSaturationReceive(void (*callback)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint8_t, signed char rssi)) {
    _on_color_saturation_receive = callback;
   }
  void onOnOffCustomCmdReceive(void (*callback)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint8_t, uint8_t, signed char rssi)) {
    _on_on_off_custom_cmd_receive = callback;
   }
   void onCustomCmdReceive(bool (*callback)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint8_t, uint8_t, uint8_t *, signed char  rssi)) {
    _on_custom_cmd_receive = callback;
   }
   void onCmdCustomClusterReceive(void (*callback)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint8_t, uint16_t, uint8_t *, signed char rssi)) {
    _on_cmd_custom_cluster_receive = callback;
   }
  void onBoundDevice(void (*callback)(zbg_device_params_t *, bool)) {
    _on_bound_device = callback;
  }
  void onBTCBoundDevice(void (*callback)(zbg_device_params_t *)) {
    _on_btc_bound_device = callback;
  }

private:
  // save instance of the class in order to use it in static functions
  static ZigbeeGateway *_instance;

  static bool GatewayReady;

  static findcb_userdata_t findcb_userdata;
  static bool _last_bind_success;
  static bool _in_binding;
  static bool _new_device_joined;

  static uint8_t _binding_error_retries;

  static uint16_t _clusters_2_discover;
  static uint16_t _attributes_2_discover;

  static uint16_t _endpoints_2_bind;
  static uint16_t _clusters_2_bind;

  static query_basic_cluster_data_t _last_device_query;

  static uint8_t _read_attr_last_tsn;
  static uint8_t _read_attr_tsn_list[256];
  //static bool _read_attr_async;
  static esp_zb_zcl_attribute_t _read_attr_last_result;

  void (*_on_IAS_zone_status_change_notification)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, int, signed char rssi);
  void (*_on_temperature_receive)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, float, signed char rssi);
  void (*_on_humidity_receive)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, float, signed char rssi);
  void (*_on_on_off_receive)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, bool, signed char rssi);
  void (*_on_rms_voltage_receive)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint16_t, signed char rssi );
  void (*_on_rms_current_receive)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint16_t, signed char rssi);
  void (*_on_rms_active_power_receive)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint16_t, signed char rssi);
  void (*_on_current_summation_receive)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint64_t, signed char rssi);
  void (*_on_battery_percentage_receive)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint8_t);
  void (*_on_current_level_receive)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint8_t, signed char rssi);
  void (*_on_color_hue_receive)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint8_t, signed char rssi);
  void (*_on_color_saturation_receive)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint8_t, signed char rssi);

  void (*_on_on_off_custom_cmd_receive)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint8_t, uint8_t, signed char rssi);
  bool (*_on_custom_cmd_receive)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint8_t, uint8_t, uint8_t *, signed char );

  void (*_on_cmd_custom_cluster_receive)(esp_zb_ieee_addr_t ieee_addr, uint16_t, uint16_t, uint8_t, uint16_t, uint8_t *, signed char rssi);

  void (*_on_bound_device)(zbg_device_params_t *, bool);
  void (*_on_btc_bound_device)(zbg_device_params_t *);


  void findEndpoint(esp_zb_zdo_match_desc_req_param_t *cmd_req);

  bool zbRawCmdHandler( esp_zb_zcl_addr_t source, uint8_t src_endpoint, uint8_t dst_endpoint, uint16_t cluster_id, uint8_t cmd_id, 
                                bool is_common_command, bool disable_default_response, bool is_manuf_specific, uint16_t manuf_specific,
                                uint8_t buffer_size, uint8_t *buffer, signed char  rssi);

  static void bindCb(esp_zb_zdp_status_t zdo_status, void *user_ctx);
  static void find_Cb(esp_zb_zdp_status_t zdo_status, uint16_t addr, uint8_t endpoint, void *user_ctx);

  static void Z2S_active_ep_req_cb(esp_zb_zdp_status_t zdo_status, uint8_t ep_count, uint8_t *ep_id_list, void *user_ctx);
  static void Z2S_simple_desc_req_cb(esp_zb_zdp_status_t zdo_status, esp_zb_af_simple_desc_1_1_t *simple_desc, void *user_ctx);

  void zbAttributeReporting(esp_zb_zcl_addr_t src_address, uint16_t src_endpoint, uint16_t cluster_id, const esp_zb_zcl_attribute_t *attribute, signed char  rssi) override;
  void zbReadAttrResponse(uint8_t tsn, esp_zb_zcl_addr_t src_address, uint16_t src_endpoint, uint16_t cluster_id, const esp_zb_zcl_attribute_t *attribute, signed char  rssi) override;
  void zbIASZoneStatusChangeNotification(const esp_zb_zcl_ias_zone_status_change_notification_message_t *message) override;
  void zbCmdDiscAttrResponse(esp_zb_zcl_addr_t src_address, uint16_t src_endpoint, uint16_t cluster_id, 
                            const esp_zb_zcl_disc_attr_variable_t *variable) override;
  void zbCmdCustomClusterReq(esp_zb_zcl_addr_t src_address, uint16_t src_endpoint, uint16_t cluster_id,uint8_t command_id, uint16_t payload_size, uint8_t *payload) override;
  void zbConfigReportResponse(esp_zb_zcl_addr_t src_address, uint16_t src_endpoint, uint16_t cluster_id, esp_zb_zcl_status_t status, uint8_t direction, 
                             uint16_t attribute_id) override;
  void zbCmdDefaultResponse( esp_zb_zcl_addr_t src_address, uint16_t src_endpoint, uint16_t cluster_id, uint8_t resp_to_cmd, esp_zb_zcl_status_t status_code) override;

  void zbDeviceAnnce(uint16_t short_addr, esp_zb_ieee_addr_t ieee_addr) override;

  void addBoundDevice(zb_device_params_t *device, uint16_t cluster_id) override;
  bool isDeviceBound(uint16_t short_addr, esp_zb_ieee_addr_t ieee_addr) override;

protected:

  static SemaphoreHandle_t gt_lock;

  std::list<zbg_device_params_t *> _joined_devices;
  std::list<zbg_device_params_t *> _gateway_devices;
};

#endif  //SOC_IEEE802154_SUPPORTED && CONFIG_ZB_ENABLED
