#ifndef USE_SUPLA_WEB_SERVER

#include <ZigbeeGateway.h>

#include "z2s_web_gui.h"
#include "z2s_devices_database.h"
#include "z2s_devices_table.h"
#include "z2s_device_tuya_custom_cluster.h"
#include "z2s_device_virtual_relay.h"
#include "TuyaDatapoints.h"

#include "z2s_version_info.h"

#include <SuplaDevice.h>
#include <supla/storage/littlefs_config.h>
#include <supla/storage/storage.h>

#include <supla/device/register_device.h>

#include "web_gui_templates.h"

ESPAsyncHTTPUpdateServer updateServer;

extern ZigbeeGateway zbGateway;

extern uint8_t  _enable_gui_on_start;
extern uint8_t	_force_config_on_start;
extern uint32_t _gui_start_delay;

extern uint8_t _z2s_security_level;

#define MAX_ATTRIBUTE_ID_SELECTOR_OPTIONS 24
#define MAX_ATTRIBUTE_VALUE_SELECTOR_OPTIONS 6

//UI handles
uint16_t gateway_general_info;
uint16_t gateway_memory_info;
uint16_t enable_gui_switcher, force_config_switcher, gui_start_delay_number;

uint16_t wifi_ssid_text, wifi_pass_text, Supla_server, Supla_email, Supla_skip_certificate_switcher;
uint16_t save_button, save_label;

uint16_t pairing_mode_switcher;// pairing_mode_label;
uint16_t zigbee_tx_power_text, zigbee_get_tx_power_button, zigbee_set_tx_power_button;
uint16_t zigbee_primary_channel_text, zigbee_get_primary_channel_button, zigbee_set_primary_channel_button;
uint16_t zigbee_last_binding_result_label;
uint16_t factory_reset_switcher, factory_reset_button, factory_reset_label;

uint16_t device_selector;
uint16_t zb_device_info_label, device_status_label;
uint16_t device_name_text; 
uint16_t device_name_save_button;

uint16_t battery_voltage_min_number, battery_voltage_max_number;
uint16_t battery_voltage_min_save_button, battery_voltage_max_save_button;
uint16_t disable_battery_percentage_msg_switcher, disable_battery_voltage_msg_switcher;

uint16_t getswbuild_button, getrssi_button;
uint16_t swbuildlabel, rssilabel;

uint16_t device_endpoint_number;
uint16_t device_cluster_selector;
uint16_t device_attribute_id_text;
uint16_t device_read_attribute_label;	
uint16_t device_attribute_id_selector, device_attribute_type_selector, device_attribute_value_selector;
uint16_t device_attribute_size_number;
uint16_t device_config_min_number, device_config_max_number, device_config_delta_number; 
uint16_t device_attribute_value_text;
uint16_t device_async_switcher;
uint16_t device_read_attribute_button, device_write_attribute_button, device_read_config_button;
uint16_t device_write_config_button, device_custom_command_button;
uint16_t manufacturer_code_switcher, manufacturer_code_selector;
uint16_t device_Tuya_payload_label;
uint16_t remove_device_button, remove_device_and_channels_button, remove_all_devices_button;

uint16_t channel_selector;
uint16_t channel_status_label, zb_channel_info_label; // zb_channel_info_label_2;
uint16_t zb_channel_timings_label, zb_channel_flags_label;
uint16_t channel_name_text; //, channel_desc_number, channel_sub_id_number;
uint16_t channel_name_save_button;
uint16_t disable_channel_notifications_switcher, trv_auto_to_schedule_switcher, set_sorwns_on_start_switcher;
uint16_t keepalive_number, timeout_number, refresh_number;
uint16_t keepalive_save_button, timeout_save_button, refresh_save_button;
uint16_t remove_channel_button, remove_all_channels_button;

uint16_t advanced_device_selector, advanced_device_info_label;

uint16_t valve_program_selector, valve_cycles_number, valve_worktime_number, valve_volume_number, valve_pause_number;
uint16_t save_program_button, load_program_button, start_program_button, stop_program_button;
uint16_t valve_info_label;

uint16_t gas_alarm_ringtone_selector, gas_alarm_time_number;
uint16_t gas_alarm_ringtone_button, gas_alarm_time_button, gas_alarm_self_test_button, gas_alarm_silence_button;
uint16_t gas_alarm_info_label, gas_alarm_Tuya_payload_label;

uint16_t moes_alarm_melody_selector, moes_alarm_duration_number, moes_alarm_volume_number;
uint16_t moes_alarm_melody_button, moes_alarm_duration_button, moes_alarm_volume_button;
uint16_t moes_alarm_info_label, moes_alarm_Tuya_payload_label;

uint16_t Tuya_device_selector, Tuya_datapoint_id_number, Tuya_datapoint_type_selector, Tuya_datapoint_length_number;
uint16_t Tuya_datapoint_value_text, Tuya_device_info_label, Tuya_device_cmd_result_label, Tuya_device_payload_label;
uint16_t Tuya_device_data_request_button, Tuya_device_data_query_button;

uint16_t table_first_button, table_next_button, table_prev_button, table_last_button;
uint16_t actions_table_label;
uint16_t action_name_text, action_description_text;
uint16_t action_enabled_switcher;
uint16_t action_source_channel_selector, action_destination_channel_selector;
uint16_t action_event_selector, action_action_selector, action_condition_selector;
uint16_t action_condition_threshold_1_number, action_condition_threshold_2_number; 
uint16_t action_save_button, action_cancel_button, action_edit_button, action_new_button, action_remove_button;
uint16_t action_state_label;

uint8_t	 attribute_id_selector_options_count = 0;
uint16_t attribute_id_selector_options[MAX_ATTRIBUTE_ID_SELECTOR_OPTIONS];

uint8_t	 attribute_value_selector_options_count = 0;
uint16_t attribute_value_selector_options[MAX_ATTRIBUTE_VALUE_SELECTOR_OPTIONS];

volatile bool data_ready = false;
volatile bool device_controls_enabled = false;
volatile bool channel_controls_enabled = false;

bool isSonoffValvePresent 		= false;
bool isTuyaGasDetectorPresent = false;
bool isMoesAlarmPresent				= false;

#define GUI_UPDATE_CMD_NONE 								0x00
#define GUI_UPDATE_CMD_DEVICE_INFO_LABEL		0x10
#define GUI_UPDATE_CMD_CHANNEL_INFO_LABEL_1	0x20
#define GUI_UPDATE_CMD_CHANNEL_INFO_LABEL_2	0x21

volatile bool GUI_update_required = false;
volatile uint8_t GUI_update_cmd = GUI_UPDATE_CMD_NONE;

volatile uint16_t custom_cmd_tsn = 0;
volatile uint8_t 	Tuya_custom_cmd_dp = 0xFF;
volatile uint16_t current_Tuya_payload_label = 0;

volatile int16_t current_action_counter = -1;
volatile int16_t new_action_id = -1;
volatile int16_t current_action_id = -1;

volatile ActionGUIState current_action_gui_state = VIEW_ACTION;
volatile ActionGUIState previous_action_gui_state = VIEW_ACTION;

#define GUI_CB_ENABLE_GUI_FLAG							0x0100
#define GUI_CB_FORCE_CONFIG_FLAG						0x0101
#define GUI_CB_GUI_DELAY_FLAG								0x0102
#define GUI_CB_GUI_RESTART_FLAG							0x0103

#define GUI_CB_SAVE_FLAG										0x1000
#define GUI_CB_RESTART_FLAG									0x1001

#define GUI_CB_PAIRING_FLAG									0x2001
#define GUI_CB_FACTORY_FLAG									0x2002
#define GUI_CB_GET_TX_FLAG									0x2003
#define GUI_CB_SET_TX_FLAG									0x2004
#define GUI_CB_GET_PC_FLAG									0x2005
#define GUI_CB_SET_PC_FLAG									0x2006

#define GUI_CB_BATTERY_VOLTAGE_MIN_FLAG			0x3000
#define GUI_CB_BATTERY_VOLTAGE_MAX_FLAG			0x3001
#define GUI_CB_DISABLE_PERCENTAGE_MSG_FLAG	0x3002
#define GUI_CB_DISABLE_VOLTAGE_MSG_FLAG 		0x3003

#define GUI_CB_UPDATE_DEVICE_NAME_FLAG			0x3005

#define GUI_CB_SWBUILD_FLAG									0x3010
#define GUI_CB_RSSI_FLAG										0x3011

#define GUI_CB_READ_ATTR_FLAG								0x3020
#define GUI_CB_WRITE_ATTR_FLAG							0x3021
#define GUI_CB_CONFIG_REPORT_FLAG						0x3022
#define GUI_CB_READ_CONFIG_FLAG							0x3023
#define GUI_CB_CUSTOM_CMD_FLAG							0x3024

#define GUI_CB_SINGLE_FLAG									0x3030
#define GUI_CB_WITH_CHANNELS_FLAG						0x3031
#define GUI_CB_ALL_FLAG											0x3032

#define GUI_CB_UPDATE_CHANNEL_NAME_FLAG						0x4000
#define GUI_CB_UPDATE_CHANNEL_DESC_FLAG						0x4001
#define GUI_CB_UPDATE_CHANNEL_SUB_ID_FLAG					0x4002
#define GUI_CB_UPDATE_CHANNEL_FUNC_FLAG						0x4003
#define GUI_CB_DISABLE_CHANNEL_NOTIFICATIONS_FLAG	0x4005
#define GUI_CB_TRV_AUTO_TO_SCHEDULE_FLAG					0x4006
#define GUI_CB_SET_SORWNS_ON_START_FLAG						0x4007

#define GUI_CB_UPDATE_KEEPALIVE_FLAG				0x4010
#define GUI_CB_UPDATE_TIMEOUT_FLAG					0x4011
#define GUI_CB_UPDATE_REFRESH_FLAG					0x4012

#define GUI_CB_SAVE_PROGRAM_FLAG						0x5000
#define GUI_CB_LOAD_PROGRAM_FLAG						0x5001
#define GUI_CB_START_PROGRAM_FLAG						0x5002
#define GUI_CB_STOP_PROGRAM_FLAG						0x5003
#define GUI_CB_SEND_PROGRAM_FLAG						0x5004

#define GUI_CB_SELF_TEST_FLAG								0x6000
#define GUI_CB_SILENCE_FLAG									0x6001
#define GUI_CB_SEND_RINGTONE_FLAG						0x6002
#define GUI_CB_SEND_TIME_FLAG								0x6003

#define GUI_CB_SEND_VOLUME_FLAG							0x7000
#define GUI_CB_SEND_MELODY_FLAG							0x7001
#define GUI_CB_SEND_DURATION_FLAG						0x7002

#define GUI_CB_SEND_TUYA_REQUEST_FLAG				0x8000
#define GUI_CB_SEND_TUYA_QUERY_FLAG					0x8001

#define GUI_CB_ACTION_FIRST_FLAG						0x9000
#define GUI_CB_ACTION_NEXT_FLAG							0x9001
#define GUI_CB_ACTION_PREV_FLAG							0x9002
#define GUI_CB_ACTION_LAST_FLAG							0x9003

#define GUI_CB_ACTION_EDIT_FLAG							0x9010
#define GUI_CB_ACTION_NEW_FLAG							0x9011
#define GUI_CB_ACTION_SAVE_FLAG							0x9012
#define GUI_CB_ACTION_CANCEL_FLAG						0x9013
#define GUI_CB_ACTION_REMOVE_FLAG						0x9014



static constexpr char* three_dots_str PROGMEM = "...";
static constexpr char* empty_str PROGMEM = "";

static constexpr char* device_query_failed_str 								PROGMEM = "Device data query failed - try to wake it up first!";
static constexpr char* device_async_query_str 								PROGMEM = "Device data query sent asynchronously";
static constexpr char* device_query_attr_size_error_str 			PROGMEM = "Error - attribute size has to be in range 0..255";
static constexpr char* Tuya_device_payload_size_error_str 		PROGMEM = "Error - Tuya DP payload size has to be in range 0..58";
static constexpr char* device_query_attr_size_mismatch_str 		PROGMEM = "Error - attribute size and attribute value length mismatch";
static constexpr char* Tuya_device_payload_size_mismatch_str	PROGMEM = "Error - Tuya payload type length and payload value length mismatch";

static constexpr char* factory_reset_enabled_str PROGMEM = "Zigbee stack factory reset enabled";
static constexpr char* factory_reset_disabled_str PROGMEM = "Zigbee stack factory reset disabled";
static constexpr char* zigbee_tx_power_text_str PROGMEM = "Press Read to get current value or enter value between -24 and 20 and press Update";
static constexpr char* zigbee_primary_channel_text_str PROGMEM = "Press Read to get current value or enter value between 11 and 26 and press Update";

static char general_purpose_gui_buffer[1024] = {};

static constexpr char* disabledstyle PROGMEM = "background-color: #bbb; border-bottom: #999 3px solid;";
const String clearLabelStyle PROGMEM = "background-color: unset; width: 100%;";

//char zigbee_devices_labels[Z2S_ZB_DEVICES_MAX_NUMBER][11] = {};
//char zigbee_channels_labels[Z2S_CHANNELS_MAX_NUMBER][13] = {};

//const static String zigbee_devices_values[] PROGMEM {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9","10", "11", "12", "13", "14","15","16",
//																										 "17", "18", "19", "20", "21", "22", "23", "24", "25", "26","27", "28", "29", "30", "31"};	

const static String zero_str PROGMEM = "0";
const static String minus_one_str PROGMEM = "-1";
const static String max_int_str PROGMEM = "65535";

static String working_str;

bool isNumber(String& str);

const char* getSuplaChannelTypeName(int32_t channelType);
const char* getSuplaChannelFuncName(int32_t channelType, int32_t channelFunc);
const char* getZ2SDeviceDescName(uint32_t modelID);
const char* getZigbeeDataTypeName(uint8_t dataType);

void buildGatewayTabGUI();
void buildCredentialsGUI();
void buildZigbeeTabGUI();
void buildDevicesTabGUI();
void buildChannelsTabGUI();
void buildAdvancedDevicesTabGUI();
void buildTuyaCustomClusterTabGUI();

void updateChannelInfoLabel(uint8_t label_number);
void updateDeviceInfoLabel();

void gatewayCallback(Control *sender, int type, void *param);
void enterWifiDetailsCallback(Control *sender, int type, void *param);
void textCallback(Control *sender, int type);
void generalCallback(Control *sender, int type);
void onZigbeeTabCallback(Control *sender, int type);
void generalMinMaxCallback(Control *sender, int type, void *param);
void endpointCallback(Control *sender, int type);
void switchCallback(Control *sender, int type);
void deviceSelectorCallback(Control *sender, int type);
void advancedDeviceSelectorCallback(Control *sender, int type);
void TuyaDeviceSelectorCallback(Control *sender, int type);
void TuyaDatapointTypeSelectorCallback(Control *sender, int type);
void enableDeviceControls(bool enable);
void removeDeviceCallback(Control *sender, int type, void *param);
void channelSelectorCallback(Control *sender, int type);
void enableChannelControls(bool enable);
void editDeviceCallback(Control *sender, int type, void *param);
void editChannelCallback(Control *sender, int type, void *param);
void editChannelFlagsCallback(Control *sender, int type, void *param);
void batteryCallback(Control *sender, int type, void *param);
void batterySwitcherCallback(Control *sender, int type, void *param);
void removeChannelCallback(Control *sender, int type);
void removeAllChannelsCallback(Control *sender, int type);
void getZigbeeDeviceQueryCallback(Control *sender, int type, void *param);
void pairingSwitcherCallback(Control *sender, int type, void *param);
void generalZigbeeCallback(Control *sender, int type, void *param);
void datatypeCallback(Control *sender, int type);
void clusterCallback(Control *sender, int type);
void attributeCallback(Control *sender, int type);
void valueCallback(Control *sender, int type);
void valveCallback(Control *sender, int type, void *param);
void TuyaCustomCmdCallback(Control *sender, int type, void *param);
void actionsTableCallback(Control *sender, int type, void *param);

void enableControlStyle(uint16_t control_id, bool enable);

void fillGatewayGeneralnformation(char *buf);
void fillMemoryUptimeInformation(char *buf);

bool isNumber(String& str) {

    unsigned int stringLength = str.length();
 
    if (stringLength == 0) {
        return false;
    }
 
    //boolean seenDecimal = false;
 
    for(unsigned int i = 0; i < stringLength; ++i) {
        if (isDigit(str.charAt(i))) {
            continue;
        }
 
        /*if (str.charAt(i) == '.') {
            if (seenDecimal) {
                return false;
            }
            seenDecimal = true;
            continue;
        }*/
        return false;
    }
    return true;
}

const char* getSuplaChannelTypeName(int32_t channelType) {

	switch (channelType) {

		case SUPLA_CHANNELTYPE_BINARYSENSOR:
			return "Binary sensor";
		case SUPLA_CHANNELTYPE_RELAY:
			return "Relay";
		case SUPLA_CHANNELTYPE_THERMOMETER:
			return "Thermometer";
		case SUPLA_CHANNELTYPE_HUMIDITYSENSOR:
			return "Humidity sensor";
		case SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR:
			return "Humidity and temperature sensor";
		case SUPLA_CHANNELTYPE_PRESSURESENSOR:
			return "Pressure sensor";
		case SUPLA_CHANNELTYPE_DIMMER:
			return "Dimmer";
		case SUPLA_CHANNELTYPE_RGBLEDCONTROLLER:
			return "RGB controller";
		case SUPLA_CHANNELTYPE_ELECTRICITY_METER:
			return "Electricity meter";
		case SUPLA_CHANNELTYPE_THERMOSTAT:
			return "Thermostat";
		case SUPLA_CHANNELTYPE_HVAC:
			return "HVAC";
		case SUPLA_CHANNELTYPE_VALVE_OPENCLOSE:
			return "Valve open/close";
		case SUPLA_CHANNELTYPE_GENERAL_PURPOSE_MEASUREMENT:
			return "General Purpose Measurement";
		case SUPLA_CHANNELTYPE_ACTIONTRIGGER:
			return "Action Trigger";
		case -1:
			return "Undefined Supla channel type";
		default:
			return "Unknown Supla channel type";
	}
}

const char* getSuplaChannelFuncName(int32_t channelType, int32_t channelFunc) {

	switch (channelType) {

		case SUPLA_CHANNELTYPE_BINARYSENSOR:
			return Supla::getBinarySensorChannelName(channelFunc);
		case SUPLA_CHANNELTYPE_RELAY:
			return Supla::getRelayChannelName(channelFunc);
		default:
			return getSuplaChannelTypeName(channelType);
	}
//currently unused
/*switch (channelFunc) {

		case SUPLA_CHANNELFNC_THERMOMETER:
			return "Temperature measurement";
		case SUPLA_CHANNELFNC_HUMIDITY:
			return "Humidity measurement";
		case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
			return "Humidity and temperature measurement";
		case SUPLA_CHANNELFNC_DIMMER:
			return "Dimmer control";
		case SUPLA_CHANNELFNC_RGBLIGHTING:
			return "RGB control";
		case SUPLA_CHANNELFNC_ELECTRICITY_METER:
			return "Electricity measurement";
		case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
			return "HVAC/Thermostat control";
		case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
			return "Valve open/close control";
		case SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT:
			return "General puropose measurement";
		case SUPLA_CHANNELFNC_ACTIONTRIGGER:
			return "Action trigger control";
		default:
			return "Unknown Supla channel function";
	}*/
}

const char* getZ2SDeviceDescName(uint32_t modelID)  {

	switch (modelID) {

		case Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR:
		case Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR_1:
		case Z2S_DEVICE_DESC_TUYA_TEMPHUMIDITY_SENSOR:
			return "Temperature and humidity sensor(0x402, 0x405)";
		case Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR_POLL:
			return "Temperature and humidity sensor(0x20, 0x402, 0x405)";
		case Z2S_DEVICE_DESC_TUYA_TEMPHUMIDITY_EF00_SENSOR:
			return "Temperature and humidity sensor(Tuya 0xEF00)";
		case Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR_HUMIX10:
			return "Temperature and humidity(x10) sensor(0x402, 0x405)";
		case Z2S_DEVICE_DESC_TEMPHUMIPRESSURE_SENSOR:
			return "Temperature, pressure and humidity sensor(0x402, 0x403, 0x405)";
		case Z2S_DEVICE_DESC_TUYA_SOIL_TEMPHUMIDITY_SENSOR:
			return "Tuya soil temperature(/10) and humidity sensor(0xEF00)";
		case Z2S_DEVICE_DESC_TUYA_SOIL_TEMPHUMIDITY_SENSOR_1:
			return "Tuya soil temperature(/1) and humidity sensor(0xEF00)";
		case Z2S_DEVICE_DESC_TUYA_ILLUMINANCE_SENSOR:
			return "Tuya illuminance sensor (0x400)";
		case Z2S_DEVICE_DESC_TUYA_ILLUZONE_SENSOR:
			return "Illuminance and IAS Zone sensor (0x400, 0x500)";
		case Z2S_DEVICE_DESC_TUYA_RAIN_SENSOR:
			return "Tuya rain and illuminance sensor (0xEF00)";
		case Z2S_DEVICE_DESC_IAS_ZONE_SENSOR:
			return "IAS Zone sensor: ALARM1(0x500)";
		case Z2S_DEVICE_DESC_IAS_ZONE_SENSOR_1_2_T:
			return "IAS Zone sensor: ALARM1, ALARM2, TAMPER(0x500)";
		case Z2S_DEVICE_DESC_IAS_ZONE_SENSOR_1_T_B:
			return "IAS Zone sensor: ALARM1, TAMPER, LOW BATTERY(0x500)";
		case Z2S_DEVICE_DESC_IAS_ZONE_SENSOR_1_B:
			return "IAS Zone sensor: ALARM1, LOW BATTERY(0x500)";
		case Z2S_DEVICE_DESC_IKEA_IAS_ZONE_SENSOR:
			return "IKEA IAS Zone sensor(0x6, 0x500)";
		case Z2S_DEVICE_DESC_IKEA_VALLHORN_1:
			return "IKEA VALLHORN Wireless Motion Sensor(0x6, 0x406)";
		case Z2S_DEVICE_DESC_LUMI_MAGNET_SENSOR:
			return "LUMI magnet sensor(0x6)";
		case Z2S_DEVICE_DESC_LUMI_MOTION_SENSOR:
			return "LUMI motion sensor(0x400, 0x406)";
		case Z2S_DEVICE_DESC_IAS_ZONE_SENSOR_1_SONOFF_T_B: 
			return "Sonoff IAS Zone sensor: ALARM1, TAMPER, LOW BATTERY(0x500, 0xFC11)";
		case Z2S_DEVICE_DESC_TUYA_VIBRATION_SENSOR:          
			return "Tuya vibration sensor(0xEF00)";
		case Z2S_DEVICE_DESC_TUYA_SMOKE_DETECTOR:       
			return "Tuya smoke sensor: BATTERY LEVEL(0xEF00)";
		case Z2S_DEVICE_DESC_TUYA_SMOKE_DETECTOR_1:
			return "Tuya vibration sensor: BATTERY STATE(0xEF00)";         
		case Z2S_DEVICE_DESC_TUYA_CO_DETECTOR:                
			return "Tuya CO sensor(0xEF00)";         
		case Z2S_DEVICE_DESC_TUYA_PRESENCE_SENSOR:
			return "Tuya presence and motion state sensor (0xEF00)";          
		case Z2S_DEVICE_DESC_TUYA_PRESENCE_SENSOR_5:     
			return "Tuya presence sensor (0xEF00)";
		case Z2S_DEVICE_DESC_ADEO_SMART_PIRTH_SENSOR: 
			return "ADEO smart PIRTH sensor";        
		case Z2S_DEVICE_DESC_ADEO_CONTACT_VIBRATION_SENSOR: 
			return "ADEO contact and vibration sensor"; 
		case Z2S_DEVICE_DESC_SONOFF_PIR_SENSOR:
			return "Sonoff PIR sensor";               
		default:
			return "Unknown Zigbee model";
	}
}

const char* getZigbeeDataTypeName(uint8_t dataType)  {

	uint32_t zigbee_datatypes_count = sizeof(zigbee_datatypes)/sizeof(zigbee_datatype_t);

	for (uint8_t datatypes_counter = 0; datatypes_counter < zigbee_datatypes_count; datatypes_counter++) {
  
		if (zigbee_datatypes[datatypes_counter].zigbee_datatype_id == dataType)
			return zigbee_datatypes[datatypes_counter].zigbee_datatype_name;
	}
	return PSTR("Unknown data type");
}

void fillGatewayGeneralnformation(char *buf) {

	if (buf) {
		
		char guid_buf[128] = {};

		generateHexString(Supla::RegisterDevice::getGUID(), guid_buf, SUPLA_GUID_SIZE);

		snprintf_P(buf, 1024, PSTR("<b><i>Supla firmware:</i></b> %s<br><br><b><i>Supla GUID:</i></b> %s<br><br><b><i>Z2S Gateway version:</i></b> %s<br><br>"), 
						Supla::RegisterDevice::getSoftVer(), guid_buf, Z2S_VERSION);
	
		log_i("Device information %s", buf);

	}
}

void fillMemoryUptimeInformation(char *buf) {

	if (buf) {
		time_t local_time_info;
		time(&local_time_info);

		snprintf_P(buf, 1024, PSTR("<b><i>Flash chip real size:</b></i> %u B <b>| <i>Free Sketch Space:</b></i> %u B<br>"
						"<b><i>HeapSize:</b></i> %u B <b>| <i>FreeHeap:</b></i> %u B <b>| <i>"
						"MinimalFreeHeap:</b></i> %u B <b>| <i>MaxAllocHeap:</b></i> %u B<br>"
						"<b><i>uxTaskGetStackHighWaterMark:</b></i> %u B <br>"
						"<b><i>Total PSRAM:</b></i> %u B <b>| <i>Free PSRAM:</b></i> %u B<br><br>"
						"<b><i>Local time:</i></b> %s<b><i>Supla uptime:</i></b> %lu s"), 
						ESP.getFlashChipSize(), ESP.getFreeSketchSpace(), 
						ESP.getHeapSize(), ESP.getFreeHeap(), ESP.getMinFreeHeap(), ESP.getMaxAllocHeap(), 
						uxTaskGetStackHighWaterMark(NULL),
						ESP.getPsramSize(), ESP.getFreePsram(), 
						ctime(&local_time_info),  SuplaDevice.uptime.getUptime());

		log_i("Memory & uptime information %s", buf);
	}
}

void enableControlStyle(uint16_t control_id, bool enable) {

	ESPUI.setEnabled(control_id, enable);
	if (enable) {

		working_str = PSTR(";");
		ESPUI.setPanelStyle(control_id, working_str);
	}
	else {

		working_str = disabledstyle;
		ESPUI.setPanelStyle(control_id, working_str);	
	}
}

void updateLabel_P(Control::ControlId_t id, const char* value) {

	working_str = value;
	ESPUI.updateLabel(id, working_str);
}

void buildGatewayTabGUI() {

	//char buf[1024] = {};
	working_str = PSTR("Gateway");
	auto gatewaytab = ESPUI.addControl(Control::Type::Tab, PSTR(empty_str), working_str, 
																		 Control::Color::Emerald, Control::noParent, 
																		 generalCallback);
	
	fillGatewayGeneralnformation(general_purpose_gui_buffer);

	working_str = PSTR(empty_str);
	ESPUI.addControl(Control::Type::Separator, PSTR("General information"), working_str, 
									 Control::Color::None, gatewaytab);

	working_str = general_purpose_gui_buffer;
	gateway_general_info = ESPUI.addControl(Control::Type::Label, PSTR("Device information"), working_str, 
																					Control::Color::Emerald, gatewaytab);
	ESPUI.setElementStyle(gateway_general_info, "color:black;text-align: justify; font-family:tahoma;"
												" font-size: 4 px; font-style: normal; font-weight: normal;");
	ESPUI.setPanelWide(gateway_general_info, true);

	fillMemoryUptimeInformation(general_purpose_gui_buffer);
	
	working_str = PSTR(empty_str);
	ESPUI.addControl(Control::Type::Separator, PSTR("Status"), working_str, Control::Color::None, gatewaytab);
	working_str = general_purpose_gui_buffer;
	gateway_memory_info = ESPUI.addControl(Control::Type::Label, PSTR("Memory & Uptime"), working_str, 
																				 Control::Color::Emerald, gatewaytab);
	//ESPUI.setElementStyle(gateway_memory_info, "text-align: justify; font-size: 4 px; font-style: normal; font-weight: normal;");
	ESPUI.setElementStyle(gateway_memory_info, "color:black;text-align: justify; font-family:tahoma;"
												" font-size: 4 px; font-style: normal; font-weight: normal;");
	ESPUI.setPanelWide(gateway_memory_info, true);

	enable_gui_switcher = ESPUI.addControl(Control::Type::Switcher, PSTR("Enable GUI on start"), zero_str, 
																				 Control::Color::Emerald, gatewaytab, gatewayCallback,(void*)GUI_CB_ENABLE_GUI_FLAG);

	working_str = PSTR("When GUI is disabled on start use 5x BOOT to enable it.");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, 
																				 Control::Color::None, enable_gui_switcher), PSTR(clearLabelStyle));

	force_config_switcher = ESPUI.addControl(Control::Type::Switcher, PSTR("Force config mode on next startup"), zero_str, 
																				 Control::Color::Emerald, gatewaytab, gatewayCallback,(void*)GUI_CB_FORCE_CONFIG_FLAG);

	working_str = PSTR("On next startup gateway will enter config mode.");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, 
																				 Control::Color::None, force_config_switcher), PSTR(clearLabelStyle));

	gui_start_delay_number = ESPUI.addControl(Control::Type::Number, PSTR("GUI start delay (s)"), zero_str, 
																						Control::Color::Emerald, gatewaytab, generalMinMaxCallback, (void*)3600);

	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, gui_start_delay_number);
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), max_int_str, Control::Color::None, gui_start_delay_number);
	working_str = PSTR("Save");
	save_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, 
																 Control::Color::Emerald, gui_start_delay_number, gatewayCallback, (void*)GUI_CB_GUI_DELAY_FLAG);

	working_str = PSTR("Restart gateway");
	save_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, 
																 Control::Color::Emerald, gatewaytab, gatewayCallback, (void*)GUI_CB_GUI_RESTART_FLAG);
	ESPUI.updateNumber(enable_gui_switcher, _enable_gui_on_start);
	ESPUI.updateNumber(gui_start_delay_number, _gui_start_delay);
	ESPUI.updateNumber(force_config_switcher, _force_config_on_start);
}

void buildCredentialsGUI() {

	//char buf[512] = {};
	working_str = PSTR("WiFi & Supla credentials");
	auto wifitab = ESPUI.addControl(Control::Type::Tab, PSTR(empty_str), working_str);

	working_str = PSTR(empty_str);
	wifi_ssid_text = ESPUI.addControl(Control::Type::Text, PSTR("SSID"), working_str, Control::Color::Emerald, wifitab, textCallback);
	working_str = PSTR("32");
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), working_str, Control::Color::None, wifi_ssid_text);

	working_str = PSTR(empty_str);
	wifi_pass_text = ESPUI.addControl(Control::Type::Text, PSTR("Password"), working_str, Control::Color::Emerald, wifitab, textCallback);
	working_str = PSTR("64");
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), working_str, Control::Color::None, wifi_pass_text);
	working_str = PSTR("password");
	ESPUI.setInputType(wifi_pass_text, working_str);

	working_str = PSTR(empty_str);
	Supla_server = ESPUI.addControl(Control::Type::Text, PSTR("Supla server"), working_str, Control::Color::Emerald, wifitab, textCallback);
	working_str = PSTR("64");
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), working_str, Control::Color::None, Supla_server);

	working_str = PSTR(empty_str);
	Supla_email = ESPUI.addControl(Control::Type::Text, PSTR("Supla email"), working_str, Control::Color::Emerald, wifitab, textCallback);
	working_str = PSTR("64");	
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), working_str, Control::Color::None, Supla_email);

	Supla_skip_certificate_switcher = ESPUI.addControl(Control::Type::Switcher, PSTR("Skip CA certificate check"), zero_str, 
																										 Control::Color::Emerald, wifitab, generalCallback);

	working_str = PSTR("Save");
	save_button = ESPUI.addControl(Control::Type::Button, PSTR("Save"), working_str, 
																 Control::Color::Emerald, wifitab, enterWifiDetailsCallback,(void*)GUI_CB_SAVE_FLAG);
	working_str = PSTR("Save & Restart");
	auto save_n_restart_button = ESPUI.addControl(Control::Type::Button, PSTR("Save & Restart"), working_str, 
																								Control::Color::Emerald, save_button, enterWifiDetailsCallback, (void*)GUI_CB_RESTART_FLAG);
	working_str = PSTR("Missing data...");
	save_label = ESPUI.addControl(Control::Type::Label, PSTR("Status"), working_str, Control::Color::Wetasphalt, save_button);

	auto cfg = Supla::Storage::ConfigInstance();
  
	if (cfg) {

  	memset(general_purpose_gui_buffer, 0, sizeof(general_purpose_gui_buffer));
  	if (cfg->getWiFiSSID(general_purpose_gui_buffer) && strlen(general_purpose_gui_buffer) > 0)
			ESPUI.updateText(wifi_ssid_text, general_purpose_gui_buffer);
		memset(general_purpose_gui_buffer, 0, sizeof(general_purpose_gui_buffer));
		if (cfg->getSuplaServer(general_purpose_gui_buffer) && strlen(general_purpose_gui_buffer) > 0)
			ESPUI.updateText(Supla_server, general_purpose_gui_buffer);
		memset(general_purpose_gui_buffer, 0, sizeof(general_purpose_gui_buffer));
		if (cfg->getEmail(general_purpose_gui_buffer) && strlen(general_purpose_gui_buffer) > 0)
			ESPUI.updateText(Supla_email, general_purpose_gui_buffer);
		ESPUI.updateNumber(Supla_skip_certificate_switcher, _z2s_security_level == 2 ? 1 : 0);
	}			
}

void buildZigbeeTabGUI() {

	working_str = PSTR("Zigbee settings");
	auto zigbeetab = ESPUI.addControl(Control::Type::Tab, 
																		PSTR(empty_str), 
																		working_str, 
																		Control::Color::Emerald, 
																		Control::noParent, 
																		onZigbeeTabCallback);

	working_str = PSTR("Pairing mode");
	pairing_mode_switcher = ESPUI.addControl(Control::Type::Switcher, 
																							PSTR("Pairing mode"), 
																							working_str, 
																							Control::Color::Emerald, 
																							zigbeetab, 
																							pairingSwitcherCallback,(void*)GUI_CB_PAIRING_FLAG);

	working_str = zigbee_tx_power_text_str;
	zigbee_tx_power_text = ESPUI.addControl(Control::Type::Text, PSTR("Zigbee TX power"), working_str, Control::Color::Emerald, zigbeetab, generalCallback);

	working_str = zigbee_primary_channel_text_str;
	zigbee_primary_channel_text = ESPUI.addControl(Control::Type::Text, PSTR("Zigbee primary channel"), working_str, Control::Color::Emerald, zigbeetab, generalCallback);
	
	working_str = PSTR("Read");
	zigbee_get_tx_power_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, Control::Color::Emerald, zigbee_tx_power_text, 
																								generalZigbeeCallback,(void*)GUI_CB_GET_TX_FLAG); //&get_tx_flag);
	zigbee_get_primary_channel_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, Control::Color::Emerald, zigbee_primary_channel_text, 
																											 generalZigbeeCallback,(void*)GUI_CB_GET_PC_FLAG);//&get_pc_flag);
	working_str = PSTR("Update");
	zigbee_set_tx_power_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, Control::Color::Emerald, zigbee_tx_power_text, 
																								generalZigbeeCallback,(void*)GUI_CB_SET_TX_FLAG); //&set_tx_flag);
	zigbee_set_primary_channel_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, Control::Color::Emerald, zigbee_primary_channel_text, 
																											 generalZigbeeCallback,(void*)GUI_CB_SET_PC_FLAG); //&set_pc_flag);
	
	zigbee_last_binding_result_label = ESPUI.addControl(Control::Type::Label, PSTR("Last binding result"), three_dots_str, Control::Color::Emerald, zigbeetab);

	working_str = PSTR(empty_str);
	ESPUI.addControl(Control::Type::Separator, PSTR("Zigbee stack factory reset"), working_str, Control::Color::None, zigbeetab);
	factory_reset_switcher = ESPUI.addControl(Control::Type::Switcher, PSTR("Enable Zigbee stack factory reset"), zero_str, 
																						Control::Color::Alizarin, zigbeetab, switchCallback);
	working_str = factory_reset_disabled_str;
	factory_reset_label = ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, Control::Color::Wetasphalt, factory_reset_switcher);
	working_str = PSTR("FACTORY RESET!");
	factory_reset_button = ESPUI.addControl(Control::Type::Button, PSTR("FACTORY RESET!"), working_str, Control::Color::Alizarin, zigbeetab, 
																					generalZigbeeCallback,(void*)GUI_CB_FACTORY_FLAG); //&factory_flag);
}

void buildDevicesTabGUI() {

	working_str = PSTR("Zigbee devices");
	auto devicestab = ESPUI.addControl(Control::Type::Tab, PSTR(empty_str), working_str);
	
	working_str = -1;
	device_selector = ESPUI.addControl(Control::Type::Select, PSTR("Devices"), working_str, 
																		 Control::Color::Emerald, devicestab, deviceSelectorCallback);
	ESPUI.addControl(Control::Type::Option, PSTR("Select Zigbee device..."), working_str, 
									 Control::Color::None, device_selector);

	for (uint8_t devices_counter = 0; devices_counter < Z2S_ZB_DEVICES_MAX_NUMBER; devices_counter++) 
    if (z2s_zb_devices_table[devices_counter].record_id > 0) {

			//sprintf_P(zigbee_devices_labels[devices_counter], PSTR("Device #%02d"), devices_counter);
			working_str = devices_counter;
			z2s_zb_devices_table[devices_counter].device_gui_id = 
				ESPUI.addControl(Control::Type::Option, 
												 z2s_zb_devices_table[devices_counter].device_local_name, 
												 working_str, 
												 Control::Color::None, 
												 device_selector);
		}

	ESPUI.setPanelWide(device_selector, true);

	zb_device_info_label = ESPUI.addControl(Control::Type::Label, PSTR("Device info"), three_dots_str, Control::Color::Emerald, devicestab); 
	ESPUI.setElementStyle(zb_device_info_label, "color:black;text-align: justify; font-family:tahoma; font-size: 4 px; font-style: normal; font-weight: normal;");
	
	ESPUI.setPanelWide(zb_device_info_label, true);

	working_str = PSTR(empty_str);
	device_name_text = ESPUI.addControl(Control::Type::Text, PSTR("Device panel"), working_str, 
																			 Control::Color::Emerald, devicestab, generalCallback); 
	working_str = PSTR("&#10023; device name (local) &#10023;");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, 
																				 Control::Color::None, device_name_text), 
																				 PSTR(clearLabelStyle));
	working_str = PSTR("Save");
	device_name_save_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, 
																						 Control::Color::Emerald, device_name_text, 
																				  	 editDeviceCallback, (void*)GUI_CB_UPDATE_DEVICE_NAME_FLAG);

	working_str = PSTR("Read Software Build ID");
	getswbuild_button = ESPUI.addControl(Control::Type::Button, PSTR("Software Build ID / RSSI"), working_str, 
																			 Control::Color::Emerald, devicestab, 
																			 getZigbeeDeviceQueryCallback, (void*)GUI_CB_SWBUILD_FLAG);
	//swbuildlabel = ESPUI.addControl(Control::Type::Label, PSTR(empty_str), three_dots_str, Control::Color::Emerald, getswbuild_button);

	working_str = PSTR("Read device RSSI");
	getrssi_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str,
																		Control::Color::Emerald, getswbuild_button, 
																		getZigbeeDeviceQueryCallback, (void*)GUI_CB_RSSI_FLAG);
	rssilabel = ESPUI.addControl(Control::Type::Label, PSTR(empty_str), three_dots_str, 
															 Control::Color::Emerald, getswbuild_button);
	
	battery_voltage_min_number = ESPUI.addControl(Control::Type::Number, PSTR("Battery panel"), zero_str, 
																								Control::Color::Emerald, devicestab, 
																								generalMinMaxCallback, (void*)255);
	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, battery_voltage_min_number);
	working_str = 255;
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), working_str, Control::Color::None, battery_voltage_min_number);

	working_str = PSTR("Save");
	battery_voltage_min_save_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, Control::Color::Emerald, 
																										battery_voltage_min_number, batteryCallback, (void*)GUI_CB_BATTERY_VOLTAGE_MIN_FLAG);
	working_str = PSTR("&#10023; V(min) x10, ie. 28 = 2,8V &#10023;");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str,
																				 Control::Color::None, battery_voltage_min_number), PSTR(clearLabelStyle));

	battery_voltage_max_number = ESPUI.addControl(Control::Type::Number, PSTR(empty_str), zero_str, 
																								Control::Color::Emerald, battery_voltage_min_number, generalMinMaxCallback, (void*)255);
	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, battery_voltage_max_number);
	working_str = 255;
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), working_str, Control::Color::None, battery_voltage_max_number);

	working_str = PSTR("Save");
	battery_voltage_max_save_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, Control::Color::Emerald, 
																										 battery_voltage_min_number, batteryCallback, (void*)GUI_CB_BATTERY_VOLTAGE_MAX_FLAG);
	working_str = PSTR("&#10023; V(max) x10, ie. 33 = 3,3V &#10023;");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str,
																				 Control::Color::None, battery_voltage_min_number), PSTR(clearLabelStyle));
	
	disable_battery_percentage_msg_switcher = ESPUI.addControl(Control::Type::Switcher, PSTR(empty_str), zero_str, 
																														 Control::Color::Emerald, 
																													   battery_voltage_min_number, batterySwitcherCallback, 
																													   (void*)GUI_CB_DISABLE_PERCENTAGE_MSG_FLAG);
	working_str = PSTR("&#10023; <i>DISABLE BATTERY PERCENTAGE MESSAGES</i> &#10023;");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str,
																				 Control::Color::None, battery_voltage_min_number), 
																				 PSTR(clearLabelStyle));							

	disable_battery_voltage_msg_switcher = ESPUI.addControl(Control::Type::Switcher, PSTR(empty_str), zero_str, 
																													Control::Color::Emerald, 
																											    battery_voltage_min_number, batterySwitcherCallback, 
																												  (void*)GUI_CB_DISABLE_VOLTAGE_MSG_FLAG);
	working_str = PSTR("&#10023; <i>DISABLE BATTERY VOLTAGE MESSAGES</i> &#10023;");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str,
																				 Control::Color::None, battery_voltage_min_number), 
																				 PSTR(clearLabelStyle));							


	ESPUI.addControl(Control::Type::Separator, PSTR(empty_str), working_str, Control::Color::None, devicestab);

	working_str = 1;
	device_endpoint_number = ESPUI.addControl(Control::Type::Number, PSTR("Clusters&Attributes"), working_str, 
																									 Control::Color::Emerald, devicestab, endpointCallback);
	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), working_str, Control::Color::None, device_endpoint_number);
	working_str = 255;
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), working_str, Control::Color::None, device_endpoint_number);
	working_str = PSTR("Endpoint id");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, 
																				 Control::Color::None, device_endpoint_number), PSTR(clearLabelStyle));

	device_cluster_selector = ESPUI.addControl(Control::Type::Select, PSTR(empty_str), minus_one_str, 
																						 Control::Color::Emerald, device_endpoint_number, clusterCallback);
	working_str = PSTR("Cluster id");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, 
																				 Control::Color::None, device_endpoint_number), PSTR(clearLabelStyle));

	device_attribute_id_selector = ESPUI.addControl(Control::Type::Select, PSTR(empty_str), minus_one_str, 
																    						 		Control::Color::Emerald, device_endpoint_number, attributeCallback);
	//ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, "", "Attribute ID/Command ID", 
	//																			 Control::Color::None, device_endpoint_number), PSTR(clearLabelStyle));

	device_attribute_id_text = ESPUI.addControl(Control::Type::Text, PSTR(empty_str), zero_str, 
																							Control::Color::Emerald, device_endpoint_number, generalCallback);
	working_str = PSTR("Select or enter attribute id/custom command id<br>use 0x for hexadecimal values)");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, 
																				 Control::Color::None, device_endpoint_number), PSTR(clearLabelStyle));

	device_attribute_type_selector = ESPUI.addControl(Control::Type::Select, PSTR(empty_str), minus_one_str, 
																    						 		Control::Color::Emerald, device_endpoint_number, datatypeCallback);
	//ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, "", "Attribute type/Command data type", 
	//																			 Control::Color::None, device_endpoint_number), PSTR(clearLabelStyle));

	device_attribute_size_number = ESPUI.addControl(Control::Type::Number, PSTR(empty_str), zero_str, 
																						 Control::Color::Emerald, device_endpoint_number, generalMinMaxCallback, (void*)255);

	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, device_attribute_size_number);
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), max_int_str, Control::Color::None, device_attribute_size_number);

	working_str = PSTR("Select or enter attribute/custom command payload size<br>(use 0x for hexadecimal values)");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, 
																				 Control::Color::None, device_endpoint_number), PSTR(clearLabelStyle));
	
	device_attribute_value_selector = ESPUI.addControl(Control::Type::Select, PSTR(empty_str), minus_one_str, 
																    						 		Control::Color::Emerald, device_endpoint_number, valueCallback);
	
	device_attribute_value_text = ESPUI.addControl(Control::Type::Text, PSTR("Value"), zero_str, 
																									 Control::Color::Emerald, device_endpoint_number, generalCallback);

	working_str = PSTR("Select or enter attribute value<br>or custom command payload<br>"
																				 "use 0x for hexadecimal values or hexstring without 0x");															 
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), 
																				 working_str, 
																				 Control::Color::None, device_endpoint_number), PSTR(clearLabelStyle));
	
	device_config_min_number =	ESPUI.addControl(Control::Type::Number, PSTR(empty_str), zero_str, 
																									 Control::Color::Emerald, device_endpoint_number, generalMinMaxCallback, (void*)65535);
	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, device_config_min_number);
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), max_int_str, Control::Color::None, device_config_min_number);
	working_str = PSTR("Min interval");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, 
																				 Control::Color::None, device_endpoint_number), PSTR(clearLabelStyle));
	
	device_config_max_number =	ESPUI.addControl(Control::Type::Number, PSTR(empty_str), zero_str, 
																									 Control::Color::Emerald, device_endpoint_number, generalMinMaxCallback, (void*)65535);
	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, device_config_max_number);
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), max_int_str, Control::Color::None, device_config_max_number);
	working_str = PSTR("Max interval");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, 
																				 Control::Color::None, device_endpoint_number), PSTR(clearLabelStyle));

	device_config_delta_number =	ESPUI.addControl(Control::Type::Number, PSTR(empty_str), zero_str, 
																									 Control::Color::Emerald, device_endpoint_number, generalMinMaxCallback, (void*)65535);
	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, device_config_delta_number);
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), max_int_str, Control::Color::None, device_config_delta_number);
	working_str = PSTR("Delta");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, 
																				 Control::Color::None, device_endpoint_number), PSTR(clearLabelStyle));
	

	ESPUI.addControl(Control::Type::Option, PSTR("Select Zigbee cluster..."), minus_one_str, Control::Color::None, device_cluster_selector);
	ESPUI.addControl(Control::Type::Option, PSTR("Select attribute id..."), minus_one_str, Control::Color::None, device_attribute_id_selector);
	ESPUI.addControl(Control::Type::Option, PSTR("Select attribute type..."), minus_one_str, Control::Color::None, device_attribute_type_selector);
	ESPUI.addControl(Control::Type::Option, PSTR("Select attribute value..."), minus_one_str, Control::Color::None, device_attribute_value_selector);

	uint32_t zigbee_clusters_count = sizeof(zigbee_clusters)/sizeof(zigbee_cluster_t);

	for (uint8_t clusters_counter = 0; clusters_counter < zigbee_clusters_count; clusters_counter++) {
  
		working_str = zigbee_clusters[clusters_counter].zigbee_cluster_id;
		ESPUI.addControl(Control::Type::Option, zigbee_clusters[clusters_counter].zigbee_cluster_name, 
		working_str, Control::Color::None, device_cluster_selector);
	}

	uint32_t zigbee_datatypes_count = sizeof(zigbee_datatypes)/sizeof(zigbee_datatype_t);

	for (uint8_t datatypes_counter = 0; datatypes_counter < zigbee_datatypes_count; datatypes_counter++) {
  
		working_str = datatypes_counter; 
		ESPUI.addControl(Control::Type::Option, zigbee_datatypes[datatypes_counter].zigbee_datatype_name, 
		working_str, Control::Color::None, device_attribute_type_selector);
	}

	working_str = PSTR("Read attribute");
	device_read_attribute_button = ESPUI.addControl(Control::Type::Button, PSTR("Attribute commands"), working_str, 
																									Control::Color::Emerald, devicestab, 
																									getZigbeeDeviceQueryCallback, (void*)GUI_CB_READ_ATTR_FLAG);
	working_str = PSTR("Read reporting settings");
	device_read_config_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, 
																									Control::Color::Emerald, device_read_attribute_button, 
																									getZigbeeDeviceQueryCallback, (void*)GUI_CB_READ_CONFIG_FLAG);
	working_str = PSTR("Write attribute");
	device_write_attribute_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, 
																									Control::Color::Emerald, device_read_attribute_button, 
																									getZigbeeDeviceQueryCallback, (void*)GUI_CB_WRITE_ATTR_FLAG);
	working_str = PSTR("Configure reporting");
	device_write_config_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, 
																									Control::Color::Emerald, device_read_attribute_button, 
																									getZigbeeDeviceQueryCallback, (void*)GUI_CB_CONFIG_REPORT_FLAG); 
	working_str = PSTR("Send custom command");
	device_custom_command_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, 
																									Control::Color::Emerald, device_read_attribute_button, 
																									getZigbeeDeviceQueryCallback, (void*)GUI_CB_CUSTOM_CMD_FLAG);

	manufacturer_code_switcher = ESPUI.addControl(Control::Type::Switcher, PSTR("Manufacturer code"), zero_str, Control::Color::Emerald, 
										  													devicestab, generalCallback);
	working_str = PSTR("&#10023; <i>Use manufacturer code</i> &#10023;");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str,
																				 Control::Color::None, manufacturer_code_switcher), PSTR(clearLabelStyle));

	manufacturer_code_selector = ESPUI.addControl(Control::Type::Select, PSTR(empty_str), minus_one_str, 
										 													  Control::Color::Emerald, manufacturer_code_switcher, generalCallback);
	working_str = PSTR("Manufacturer code");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, 
																				 Control::Color::None, manufacturer_code_switcher), PSTR(clearLabelStyle));
	ESPUI.addControl(Control::Type::Option, PSTR("Select manufacturer code..."), minus_one_str, 
									 Control::Color::None, manufacturer_code_selector);

	uint32_t zigbee_manufacturer_codes_count = sizeof(zigbee_manufacturer_codes)/sizeof(zigbee_manufacturer_code_t);

	for (uint8_t manufacturer_codes_counter = 0; 
			 manufacturer_codes_counter < zigbee_manufacturer_codes_count; 
			 manufacturer_codes_counter++) {
  
		working_str = zigbee_manufacturer_codes[manufacturer_codes_counter].manufacturer_code;
		ESPUI.addControl(Control::Type::Option, zigbee_manufacturer_codes[manufacturer_codes_counter].manufacturer_name, 
		working_str, Control::Color::None, manufacturer_code_selector);
	}

	device_async_switcher = ESPUI.addControl(Control::Type::Switcher, PSTR("Asynchronous command"), zero_str, 
																				 Control::Color::Emerald, devicestab, generalCallback);

	device_read_attribute_label = ESPUI.addControl(Control::Type::Label, PSTR("Command result"), three_dots_str, 
																								 Control::Color::Emerald, devicestab);

	device_Tuya_payload_label = ESPUI.addControl(Control::Type::Label, PSTR("Tuya payload"), three_dots_str, 
																							 Control::Color::Emerald, devicestab);

	working_str = PSTR(empty_str);
	ESPUI.addControl(Control::Type::Separator, PSTR(empty_str), working_str, Control::Color::None, devicestab);

	working_str = PSTR("Remove device!");
	remove_device_button = ESPUI.addControl(Control::Type::Button, PSTR("Remove Zigbee device(s)"), working_str, Control::Color::Alizarin, 
																					devicestab, removeDeviceCallback, (void*)GUI_CB_SINGLE_FLAG);
	working_str = PSTR("Remove device with channels!");
	remove_device_and_channels_button = 
		ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, Control::Color::Alizarin, remove_device_button, 
										 removeDeviceCallback, (void*)GUI_CB_WITH_CHANNELS_FLAG); 
	working_str = PSTR("Remove all devices!");
	remove_all_devices_button = 
		ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, Control::Color::Alizarin, remove_device_button, 
										 removeDeviceCallback, (void*)GUI_CB_ALL_FLAG); 
	device_status_label = ESPUI.addControl(Control::Type::Label, PSTR("Status"), three_dots_str, Control::Color::Alizarin, remove_device_button);
}

void buildChannelsTabGUI() {

	working_str = PSTR("Zigbee channels");
	auto channelstab = ESPUI.addControl(Control::Type::Tab, PSTR(empty_str), working_str);
  
	channel_selector = ESPUI.addControl(Control::Type::Select, 
																		  PSTR("Channels"), 
																			minus_one_str, 
																			Control::Color::Emerald, 
																			channelstab, 
																			channelSelectorCallback);
	ESPUI.addControl(Control::Type::Option, 
									 PSTR("Select Zigbee channel..."), 
									 minus_one_str, 
									 Control::Color::None, 
									 channel_selector);
	
	ESPUI.setPanelWide(channel_selector, true);

	for (uint8_t channels_counter = 0; channels_counter < Z2S_CHANNELS_MAX_NUMBER; channels_counter++) {
    if (z2s_channels_table[channels_counter].valid_record) {
      
			working_str = channels_counter;
			z2s_channels_table[channels_counter].gui_control_data.gui_control_id  = 
				ESPUI.addControl(Control::Type::Option, 
												 z2s_channels_table[channels_counter].Supla_channel_name, 
												 working_str, 
												 Control::Color::None, 
												 channel_selector);
		}
	}
	zb_channel_info_label = ESPUI.addControl(Control::Type::Label, 
																					 PSTR("Channel info"), 
																					 three_dots_str, 
																					 Control::Color::Emerald, 
																					 channelstab);
	
	ESPUI.setElementStyle(zb_channel_info_label, "color:black;text-align: justify;"
												" font-family:tahoma; font-size: 4 px; font-style: normal; font-weight: normal;");
	ESPUI.setPanelWide(zb_channel_info_label, true);

	working_str = PSTR(empty_str);
	channel_name_text = ESPUI.addControl(Control::Type::Text, 
																			 PSTR("Channel description panel"), 
																			 working_str, 
																			 Control::Color::Emerald, 
																			 channelstab, 
																			 generalCallback); 
	working_str = PSTR("&#10023; channel name (local) &#10023;");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, 
																				 PSTR(empty_str), 
																				working_str, 
																				Control::Color::None, 
																				channel_name_text), 
												PSTR(clearLabelStyle));
	working_str = PSTR("Save");
	channel_name_save_button = ESPUI.addControl(Control::Type::Button, 
																							PSTR(empty_str), working_str, 
																							Control::Color::Emerald, 
																							channel_name_text, 
																				  		editChannelCallback, 
																							(void*)GUI_CB_UPDATE_CHANNEL_NAME_FLAG);

	
	zb_channel_flags_label = ESPUI.addControl(Control::Type::Label, 
																						PSTR("Channel flags panel"), 
																					 	PSTR("Here you can set different channel flags<br>(changes are saved immediately)"), 
																					 	Control::Color::Emerald, 
																					 	channelstab);
	working_str = PSTR(empty_str);
	disable_channel_notifications_switcher = ESPUI.addControl(Control::Type::Switcher, 
																														PSTR(empty_str), 
																														working_str, 
																														Control::Color::Emerald, 
																													  zb_channel_flags_label, 
																														editChannelFlagsCallback, 
																													  (void*)GUI_CB_DISABLE_CHANNEL_NOTIFICATIONS_FLAG);
	working_str = PSTR("&#10023; <i>DISABLE SINGLE CHANNEL NOTIFICATIONS</i> &#10023;");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, 
																				 PSTR(empty_str), 
																				 working_str,
																				 Control::Color::None, 
																				 zb_channel_flags_label), 
												PSTR(clearLabelStyle));

	trv_auto_to_schedule_switcher = ESPUI.addControl(Control::Type::Switcher, 
																									 PSTR(empty_str), 
																									 zero_str, 
																									 Control::Color::Emerald, 
																									 zb_channel_flags_label, 
																									 editChannelFlagsCallback, 
																									 (void*)GUI_CB_TRV_AUTO_TO_SCHEDULE_FLAG);
	working_str = PSTR("&#10023; <i>TRV AUTO MODE SWITCHES TO SCHEDULE</i> &#10023;");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, 
																				 PSTR(empty_str), 
																				 working_str,
																				 Control::Color::None, 
																				 zb_channel_flags_label), 
												PSTR(clearLabelStyle));
	
	set_sorwns_on_start_switcher = ESPUI.addControl(Control::Type::Switcher, 
																									PSTR(empty_str), 
																									zero_str, 
																									Control::Color::Emerald, 
																									zb_channel_flags_label, 
																									editChannelFlagsCallback, 
																									(void*)GUI_CB_SET_SORWNS_ON_START_FLAG);
	working_str = PSTR("&#10023; <i>BINARY SENSOR STARTS IN OFFLINE STATE<br>(NO REMOTE WAKEUP SUPPORTED)</i> &#10023;");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, 
																				 PSTR(empty_str), working_str,
																				 Control::Color::None, 
																				 zb_channel_flags_label), 
												PSTR(clearLabelStyle));

	zb_channel_timings_label = ESPUI.addControl(Control::Type::Label, 
																							PSTR("Channel timings panel"), 
																						 	PSTR("Input new value and press Save"), 
																						 	Control::Color::Emerald, 
																					 		channelstab);

	keepalive_number = ESPUI.addControl(Control::Type::Number, 
																			PSTR(empty_str), 
																			zero_str, 
																			Control::Color::Emerald, 
																			zb_channel_timings_label, 
																			generalCallback);
	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, keepalive_number);
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), max_int_str, Control::Color::None, keepalive_number);
	working_str = PSTR("Save");
	keepalive_save_button = ESPUI.addControl(Control::Type::Button, 
																					 PSTR(empty_str),
																					 working_str, 
																					 Control::Color::Emerald, 
																					 zb_channel_timings_label, 
																					 editChannelCallback, 
																					 (void*)GUI_CB_UPDATE_KEEPALIVE_FLAG);
	working_str = PSTR("&#10023; keepalive (s) &#10023;");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, 
																				 PSTR(empty_str), 
																				 working_str, 
																				 Control::Color::None, 
																				 zb_channel_timings_label), 
												PSTR(clearLabelStyle));
	
	timeout_number = ESPUI.addControl(Control::Type::Number, 
																		PSTR(empty_str), 
																		zero_str, 
																		Control::Color::Emerald, 
																		zb_channel_timings_label, 
																		generalCallback);
	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, timeout_number);
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), max_int_str, Control::Color::None, timeout_number);
	working_str = PSTR("Save");
	timeout_save_button = ESPUI.addControl(Control::Type::Button, 
																				 PSTR(empty_str), 
																				 working_str, 
																				 Control::Color::Emerald, 
																				 zb_channel_timings_label, 
																				 editChannelCallback, 
																				 (void*)GUI_CB_UPDATE_TIMEOUT_FLAG);
	working_str = PSTR("&#10023; timeout (s) &#10023;");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, 
																				 PSTR(empty_str), working_str, 
																				 Control::Color::None, 
																				 zb_channel_timings_label), 
												PSTR(clearLabelStyle));
	
	refresh_number = ESPUI.addControl(Control::Type::Number, 
																		PSTR(empty_str), 
																		zero_str, 
																		Control::Color::Emerald, 
																		zb_channel_timings_label, 
																		generalCallback);
	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, refresh_number);
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), max_int_str, Control::Color::None, refresh_number);
	working_str = PSTR("Save");
	refresh_save_button = ESPUI.addControl(Control::Type::Button, 
																				 PSTR(empty_str), 
																				 working_str, 
																				 Control::Color::Emerald, 
																				 zb_channel_timings_label, 
																				 editChannelCallback, 
																				 (void*)GUI_CB_UPDATE_REFRESH_FLAG); 
	working_str = PSTR("&#10023; refresh(s) [ElectricityMeter] &#10023; autoset(s) [VirtualBinary] &#10023;"
										 "<br>&#10023; debounce(ms) [VirtualSceneSwitch] &#10023;");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, 
																				 PSTR(empty_str), 
																				 working_str, 
																				 Control::Color::None, 
																				 zb_channel_timings_label), 
												PSTR(clearLabelStyle));

	ESPUI.setPanelWide(zb_channel_timings_label, false);
	
	working_str = PSTR(empty_str);
	ESPUI.addControl(Control::Type::Separator, 
									 PSTR(empty_str), 
									 working_str, 
									 Control::Color::None, 
									 channelstab);

	working_str = PSTR("Remove channel");
	remove_channel_button = ESPUI.addControl(Control::Type::Button, 
																					 PSTR("Remove Supla channel(s)"), 
																					 working_str, 
																					 Control::Color::Alizarin, 
																					 channelstab, 
																					removeChannelCallback);
	working_str = PSTR("Remove all channels!");
	remove_all_channels_button = ESPUI.addControl(Control::Type::Button, 
																								PSTR(empty_str), 
																								working_str, 
																								Control::Color::Alizarin, 
																								remove_channel_button, 
																								removeAllChannelsCallback);
	channel_status_label = ESPUI.addControl(Control::Type::Label, 
																					PSTR("Status"), 
																					three_dots_str, 
																					Control::Color::Alizarin, 
																					remove_channel_button);
}

void buildSonoffValveGUI(uint16_t advanced_devices_tab) {

	valve_program_selector = ESPUI.addControl(Control::Type::Select, "SONOFF SWV VALVE", minus_one_str, 
																						Control::Color::Emerald, advanced_devices_tab, generalCallback);
	ESPUI.addControl(Control::Type::Option, PSTR("Select program..."), minus_one_str, Control::Color::None, valve_program_selector);
	working_str = 1;
	ESPUI.addControl(Control::Type::Option, PSTR("Time program..."), working_str, Control::Color::None, valve_program_selector);
	working_str = 2;
	ESPUI.addControl(Control::Type::Option, PSTR("Volume program..."), working_str, Control::Color::None, valve_program_selector);

	valve_cycles_number =	ESPUI.addControl(Control::Type::Number, PSTR(empty_str), zero_str, 
																									 Control::Color::Emerald, valve_program_selector, generalMinMaxCallback, (void*)100);
	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, valve_cycles_number);
	working_str = 100;
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), working_str, Control::Color::None, valve_cycles_number);
	working_str = PSTR("Valve cycles count (0-100)");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, 
																				 Control::Color::None, valve_program_selector), PSTR(clearLabelStyle));

	valve_worktime_number =	ESPUI.addControl(Control::Type::Number, PSTR(empty_str), zero_str, 
																									 Control::Color::Emerald, valve_program_selector, generalMinMaxCallback, (void*)86400);
	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, valve_worktime_number);
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), "86400", Control::Color::None, valve_worktime_number);
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), "Valve cycle worktime (0 s - 86400 s)", 
																				 Control::Color::None, valve_program_selector), PSTR(clearLabelStyle));

	valve_volume_number =	ESPUI.addControl(Control::Type::Number, PSTR(empty_str), zero_str, 
																									 Control::Color::Emerald, valve_program_selector, generalMinMaxCallback, (void*)6500);
	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, valve_volume_number);
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), "6500", Control::Color::None, valve_volume_number);
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), "Valve cycle volume (0 L - 6500 L)", 
																				 Control::Color::None, valve_program_selector), PSTR(clearLabelStyle));

	valve_pause_number =	ESPUI.addControl(Control::Type::Number, PSTR(empty_str), zero_str, 
																									 Control::Color::Emerald, valve_program_selector, generalMinMaxCallback, (void*)86400);
	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, valve_pause_number);
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), "86400", Control::Color::None, valve_pause_number);
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), "Valve cycle pause (0 s - 86400 s)", 
																				 Control::Color::None, valve_program_selector), PSTR(clearLabelStyle));

	save_program_button = ESPUI.addControl(Control::Type::Button, "Valve programs", "Save program", 
																			 Control::Color::Emerald, valve_program_selector, valveCallback, (void*)GUI_CB_SAVE_PROGRAM_FLAG);

	load_program_button = ESPUI.addControl(Control::Type::Button, "Valve programs", "Load program", 
																			 Control::Color::Emerald, valve_program_selector, valveCallback, (void*)GUI_CB_LOAD_PROGRAM_FLAG);

	start_program_button  = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), "Start program", 
																				 Control::Color::Emerald, valve_program_selector, valveCallback, (void*)GUI_CB_START_PROGRAM_FLAG);

	stop_program_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), "Stop program", 
																				 Control::Color::Emerald, valve_program_selector, valveCallback, (void*)GUI_CB_STOP_PROGRAM_FLAG);

	stop_program_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), "Save program in Supla channel", 
																				 Control::Color::Emerald, valve_program_selector, valveCallback, (void*)GUI_CB_SEND_PROGRAM_FLAG);
																			 
	valve_info_label =  ESPUI.addControl(Control::Type::Label, PSTR(empty_str), three_dots_str,	Control::Color::Emerald, valve_program_selector);
}

void enableSonoffValveGUI(bool enable) {

	ESPUI.updateSelect(valve_program_selector, minus_one_str);
	ESPUI.updateNumber(valve_cycles_number, 0);
	ESPUI.updateNumber(valve_worktime_number, 0);
	ESPUI.updateNumber(valve_volume_number, 0);
	ESPUI.updateNumber(valve_pause_number, 0);
	
	enableControlStyle(valve_program_selector, enable);
	enableControlStyle(valve_cycles_number, enable);
	enableControlStyle(valve_worktime_number, enable);
	enableControlStyle(valve_volume_number, enable);
	enableControlStyle(valve_pause_number, enable);
	enableControlStyle(save_program_button, enable);
	enableControlStyle(load_program_button, enable);
	enableControlStyle(start_program_button, enable);
	enableControlStyle(stop_program_button, enable);
	enableControlStyle(valve_info_label, enable);	
}

void enableTuyaGasDetectorGUI(bool enable) {

	ESPUI.updateSelect(gas_alarm_ringtone_selector, minus_one_str);
	ESPUI.updateNumber(gas_alarm_time_number, 0);
	updateLabel_P(gas_alarm_info_label, three_dots_str);
	updateLabel_P(gas_alarm_Tuya_payload_label, three_dots_str);

	enableControlStyle(gas_alarm_ringtone_selector, enable);
	enableControlStyle(gas_alarm_time_number, enable);
	enableControlStyle(gas_alarm_ringtone_button, enable);
	enableControlStyle(gas_alarm_time_button, enable);
	enableControlStyle(gas_alarm_self_test_button, enable);
	enableControlStyle(gas_alarm_silence_button, enable);
}

void buildTuyaGasDetectorGUI(uint16_t advanced_devices_tab) {

	static const char ringtones[][9] PROGMEM = {"MELODY 1", "MELODY 2", "MELODY 3", "MELODY 4", "MELODY 5"};

	gas_alarm_ringtone_selector = ESPUI.addControl(Control::Type::Select, "GAS DETECTOR", minus_one_str, Control::Color::Emerald, advanced_devices_tab, generalCallback);
	ESPUI.addControl(Control::Type::Option, PSTR("Select alarm ringtone..."), minus_one_str, Control::Color::None, gas_alarm_ringtone_selector);
	for (uint8_t r = 0; r < 5; r++) {

		working_str = r;
		ESPUI.addControl(Control::Type::Option, ringtones[r], working_str, Control::Color::None, gas_alarm_ringtone_selector);
	}
	
	ESPUI.addControl(Control::Type::Separator,PSTR(empty_str),"", Control::Color::None, gas_alarm_ringtone_selector);
	gas_alarm_time_number =	ESPUI.addControl(Control::Type::Number, PSTR(empty_str), zero_str, 
																					 Control::Color::Emerald, gas_alarm_ringtone_selector, generalMinMaxCallback, (void*)180);
	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, gas_alarm_time_number);
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), "180", Control::Color::None, gas_alarm_time_number);
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), "Alarm duration (0-180 s)", 
																				 Control::Color::None, gas_alarm_ringtone_selector), PSTR(clearLabelStyle));

	gas_alarm_ringtone_button  = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), "Send alarm ringtone", 
																				 				Control::Color::Emerald, gas_alarm_ringtone_selector, valveCallback, (void*)GUI_CB_SEND_RINGTONE_FLAG); //&send_ringtone_flag);
	
	gas_alarm_time_button  = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), "Send alarm duration", 
																				 				Control::Color::Emerald, gas_alarm_ringtone_selector, valveCallback, (void*)GUI_CB_SEND_TIME_FLAG); //&send_time_flag);

	gas_alarm_self_test_button  = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), "Self test", 
																				 				Control::Color::Emerald, gas_alarm_ringtone_selector, valveCallback, (void*)GUI_CB_SELF_TEST_FLAG); //&self_test_flag);

	gas_alarm_silence_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), "Silence alarm", 
																						 Control::Color::Emerald, gas_alarm_ringtone_selector, valveCallback, (void*)GUI_CB_SILENCE_FLAG); //&silence_flag);
																			 
	gas_alarm_info_label =  ESPUI.addControl(Control::Type::Label, PSTR(empty_str), three_dots_str,	Control::Color::Emerald, gas_alarm_ringtone_selector);

	gas_alarm_Tuya_payload_label = ESPUI.addControl(Control::Type::Label, PSTR(empty_str), three_dots_str, Control::Color::Emerald, gas_alarm_ringtone_selector);
}

void enableMoesAlarmGUI(bool enable) {

	ESPUI.updateSelect(moes_alarm_melody_selector, minus_one_str);
	ESPUI.updateNumber(moes_alarm_duration_number, 0);
	ESPUI.updateNumber(moes_alarm_volume_number, 0);
	updateLabel_P(moes_alarm_info_label, three_dots_str);
	updateLabel_P(moes_alarm_Tuya_payload_label, three_dots_str);

	enableControlStyle(moes_alarm_melody_selector, enable);
	enableControlStyle(moes_alarm_duration_number, enable);
	enableControlStyle(moes_alarm_volume_number, enable);
	enableControlStyle(moes_alarm_melody_button, enable);
	enableControlStyle(moes_alarm_duration_button, enable);
	enableControlStyle(moes_alarm_volume_button, enable);
}

void buildMoesAlarmGUI(uint16_t advanced_devices_tab) {

	static const char melodies[][10] PROGMEM = {"MELODY 1", "MELODY 2", "MELODY 3", "MELODY 4", "MELODY 5", "MELODY 6", "MELODY 7", "MELODY 8", "MELODY 9", "MELODY 10",
																							  "MELODY 11", "MELODY 12", "MELODY 13", "MELODY 14", "MELODY 15", "MELODY 16", "MELODY 17", "MELODY 18"};

	moes_alarm_melody_selector = ESPUI.addControl(Control::Type::Select, PSTR("SIREN ALARM"), minus_one_str, Control::Color::Emerald, 
																								advanced_devices_tab, generalCallback);
	ESPUI.addControl(Control::Type::Option, PSTR("Select alarm melody..."), minus_one_str, Control::Color::None, moes_alarm_melody_selector);
	for (uint8_t m = 0; m < 18; m++) {

		working_str = m;
		ESPUI.addControl(Control::Type::Option, melodies[m], working_str, Control::Color::None, moes_alarm_melody_selector);
	}
	
	ESPUI.addControl(Control::Type::Separator,PSTR(empty_str),"", Control::Color::None, moes_alarm_melody_selector);

	moes_alarm_duration_number =	ESPUI.addControl(Control::Type::Number, PSTR(empty_str), zero_str, 
																								 Control::Color::Emerald, moes_alarm_melody_selector, generalMinMaxCallback, (void*)1800);
	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, moes_alarm_duration_number);
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), "1800", Control::Color::None, moes_alarm_duration_number);
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), "Alarm duration (0 - 1800 s)", 
																				 Control::Color::None, moes_alarm_melody_selector), PSTR(clearLabelStyle));
	
	moes_alarm_volume_number =	ESPUI.addControl(Control::Type::Number, PSTR(empty_str), zero_str, 
																								 Control::Color::Emerald, moes_alarm_melody_selector, generalMinMaxCallback, (void*)2);
	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, moes_alarm_volume_number);
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), "2", Control::Color::None, moes_alarm_volume_number);
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), "Alarm volume (0 - 2)", 
																				 Control::Color::None, moes_alarm_melody_selector), PSTR(clearLabelStyle));

	moes_alarm_melody_button  = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), "Send alarm melody", 
																				 			Control::Color::Emerald, moes_alarm_melody_selector, valveCallback, (void*)GUI_CB_SEND_MELODY_FLAG); 
	
	moes_alarm_duration_button  = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), "Send alarm duration", 
																				 				Control::Color::Emerald, moes_alarm_melody_selector, valveCallback, (void*)GUI_CB_SEND_DURATION_FLAG);

	moes_alarm_volume_button  = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), "Send alarm volume", 
																				 				Control::Color::Emerald, moes_alarm_melody_selector, valveCallback, (void*)GUI_CB_SEND_VOLUME_FLAG); 

																			 
	moes_alarm_info_label =  ESPUI.addControl(Control::Type::Label, PSTR(empty_str), three_dots_str,	Control::Color::Emerald, moes_alarm_melody_selector);

	moes_alarm_Tuya_payload_label = ESPUI.addControl(Control::Type::Label, PSTR(empty_str), three_dots_str, Control::Color::Emerald, moes_alarm_melody_selector);
}

void buildTuyaCustomClusterTabGUI() {

	working_str = "Tuya custom cluster (0xEF00) devices";
	auto Tuya_custom_cluster_tab = ESPUI.addControl(Control::Type::Tab, PSTR(empty_str), working_str);
	
	Tuya_device_selector = ESPUI.addControl(Control::Type::Select, PSTR("Tuya devices"), minus_one_str, Control::Color::Emerald, 
																				 Tuya_custom_cluster_tab, TuyaDeviceSelectorCallback);
	ESPUI.addControl(Control::Type::Option, PSTR("Select Tuya device..."), minus_one_str, Control::Color::None, Tuya_device_selector);

	for (uint8_t devices_counter = 0; devices_counter < Z2S_ZB_DEVICES_MAX_NUMBER; devices_counter++) {
    if ((z2s_zb_devices_table[devices_counter].record_id > 0) && hasTuyaCustomCluster(z2s_zb_devices_table[devices_counter].desc_id)) {

			//sprintf_P(zigbee_devices_labels[devices_counter], PSTR("Device #%02d"), devices_counter);
			working_str = devices_counter;
			ESPUI.addControl(Control::Type::Option, z2s_zb_devices_table[devices_counter].device_local_name, working_str, Control::Color::None, Tuya_device_selector);
		}
	}

	Tuya_device_info_label =  ESPUI.addControl(Control::Type::Label, PSTR("Tuya device"), three_dots_str,	
																						 Control::Color::Emerald, Tuya_custom_cluster_tab);

	Tuya_datapoint_id_number =	ESPUI.addControl(Control::Type::Number, PSTR(empty_str), zero_str, 
																								 Control::Color::Emerald, Tuya_custom_cluster_tab, generalMinMaxCallback, (void*)255);
	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, Tuya_datapoint_id_number);
	working_str = 255;
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), working_str, Control::Color::None, Tuya_datapoint_id_number);
	working_str = "Datapoint id (1 - 255)";
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, 
																				 Control::Color::None, Tuya_datapoint_id_number), PSTR(clearLabelStyle));
	
	Tuya_datapoint_type_selector = ESPUI.addControl(Control::Type::Select, PSTR("Datapoint type"), minus_one_str, 
																									Control::Color::Emerald, Tuya_datapoint_id_number, TuyaDatapointTypeSelectorCallback);
	ESPUI.addControl(Control::Type::Option, PSTR("Select datapoint type..."), minus_one_str, Control::Color::None, Tuya_datapoint_type_selector);
	
	for (uint8_t t = 0; t < 6; t++) {

		working_str = Tuya_datapoint_types[t].Tuya_datapoint_type_id;
		ESPUI.addControl(Control::Type::Option, Tuya_datapoint_types[t].Tuya_datapoint_type_name, working_str, 
										 Control::Color::None, Tuya_datapoint_type_selector);
	}

	Tuya_datapoint_length_number =	ESPUI.addControl(Control::Type::Number, PSTR(empty_str), zero_str, 
																							  	 Control::Color::Emerald, Tuya_datapoint_id_number, generalMinMaxCallback, (void*)58);

	ESPUI.addControl(Control::Type::Min, PSTR(empty_str), zero_str, Control::Color::None, Tuya_datapoint_length_number);
	working_str = 58;
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), working_str, Control::Color::None, Tuya_datapoint_length_number);
	working_str = "Datapoint type length (1-58)";
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, 
																				 Control::Color::None, Tuya_datapoint_id_number), PSTR(clearLabelStyle));
	
	Tuya_datapoint_value_text = ESPUI.addControl(Control::Type::Text, PSTR("Datapoint payload"), zero_str, 
																										 Control::Color::Emerald, Tuya_datapoint_id_number, generalCallback);

	working_str = PSTR("Enter custom command payload - "
										 "use hexstring without 0x <br> ie. 00120B020004090706FF");															 
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), 
																				 working_str, 
																				 Control::Color::None, Tuya_datapoint_id_number), PSTR(clearLabelStyle));

	Tuya_device_data_request_button  = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), "Data request", 
																				 			Control::Color::Emerald, Tuya_custom_cluster_tab, TuyaCustomCmdCallback, 
																							(void*)GUI_CB_SEND_TUYA_REQUEST_FLAG); 
	
	Tuya_device_data_query_button  = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), "Data query", 
																				 				Control::Color::Emerald, Tuya_device_data_request_button, TuyaCustomCmdCallback, 
																								(void*)GUI_CB_SEND_TUYA_QUERY_FLAG);

	Tuya_device_cmd_result_label =  ESPUI.addControl(Control::Type::Label, PSTR("Command Result"), three_dots_str,	
																							  Control::Color::Emerald, Tuya_custom_cluster_tab);

	Tuya_device_payload_label =  ESPUI.addControl(Control::Type::Label, PSTR("Command Response payload"), three_dots_str,	
																							  Control::Color::Emerald, Tuya_custom_cluster_tab);

}

void buildAdvancedDevicesTabGUI() {

	auto advanced_devices_tab = ESPUI.addControl(Control::Type::Tab, PSTR(empty_str), "Advanced devices");
	
	advanced_device_selector = ESPUI.addControl(Control::Type::Select, PSTR("Advanced devices"), minus_one_str, 
																							Control::Color::Emerald, advanced_devices_tab, advancedDeviceSelectorCallback);
	ESPUI.addControl(Control::Type::Option, PSTR("Select device..."), minus_one_str, Control::Color::None, advanced_device_selector);

	for (uint8_t devices_counter = 0; devices_counter < Z2S_ZB_DEVICES_MAX_NUMBER; devices_counter++) 
    if ((z2s_zb_devices_table[devices_counter].record_id > 0) && 
		((z2s_zb_devices_table[devices_counter].desc_id == Z2S_DEVICE_DESC_SONOFF_SMART_VALVE) ||
		  (z2s_zb_devices_table[devices_counter].desc_id == Z2S_DEVICE_DESC_TUYA_GAS_DETECTOR) ||
			(z2s_zb_devices_table[devices_counter].desc_id == Z2S_DEVICE_DESC_MOES_ALARM))) {
			
			switch (z2s_zb_devices_table[devices_counter].desc_id) {
				
				case Z2S_DEVICE_DESC_SONOFF_SMART_VALVE:	isSonoffValvePresent = true; break;
				case Z2S_DEVICE_DESC_TUYA_GAS_DETECTOR:		isTuyaGasDetectorPresent = true; break;
				case Z2S_DEVICE_DESC_MOES_ALARM:					isMoesAlarmPresent = true; break;
			}
			working_str = devices_counter;
			ESPUI.addControl(Control::Type::Option, z2s_zb_devices_table[devices_counter].device_local_name, working_str, Control::Color::None, advanced_device_selector);
		}
	advanced_device_info_label =  ESPUI.addControl(Control::Type::Label, PSTR("Advanced Device"), three_dots_str,	
																								 Control::Color::Emerald, advanced_devices_tab);

//SONOFF VALVE SWV
	if (Z2S_hasZBDevice(Z2S_DEVICE_DESC_SONOFF_SMART_VALVE)) {
		buildSonoffValveGUI(advanced_devices_tab);
		enableSonoffValveGUI(false);
	}

	//GAS DETECTOR
	if (Z2S_hasZBDevice(Z2S_DEVICE_DESC_TUYA_GAS_DETECTOR)) {
		buildTuyaGasDetectorGUI(advanced_devices_tab);
		enableTuyaGasDetectorGUI(false);
	}
	//MOES ALARM
	if (Z2S_hasZBDevice(Z2S_DEVICE_DESC_MOES_ALARM)) 
	{
		buildMoesAlarmGUI(advanced_devices_tab);
		enableMoesAlarmGUI(false);
	}
}

const char* getSuplaActionName(Supla::Action action_id) {

	uint16_t actions_number = sizeof(Supla_actions)/sizeof(Supla_action_type_t);

	for (uint16_t i = 0; i < actions_number; i++)
		if (Supla_actions[i].Supla_action_id == action_id)
			return Supla_actions[i].Supla_action_name;
	return "Unknown";
}

const char* getSuplaEventName(Supla::Event event_id) {

	uint16_t events_number = sizeof(Supla_events)/sizeof(Supla_event_type_t);

	for (uint16_t i = 0; i < events_number; i++)
		if (Supla_events[i].Supla_event_id == event_id)
			return Supla_events[i].Supla_event_name;
	return "Unknown";	
}

const char* getSuplaConditionName(Supla::Conditions condition_id) {

	uint16_t conditions_number = sizeof(Supla_conditions)/sizeof(Supla_condition_type_t);

	for (uint16_t i = 0; i < conditions_number; i++)
		if (Supla_conditions[i].Supla_condition_id == condition_id)
			return Supla_conditions[i].Supla_condition_name;
	return "Unknown";	
}

void sprintfAction(z2s_channel_action_t &action) {

	/*sprintf(general_purpose_gui_buffer, "<style>table {width: 100%;} th, td {border: 2px solid black;text-align: left; padding: 8px;}"
								"tr:nth-child(even) {background-color: LightSlateGray;}</style>"
								"<table><tr><th>Action Name</th><th>Source channel</th><th>Destination channel</th>"
								"<th>Supla event</th><th>Supla action</th><th>Condition</th></tr>"
								"<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr></table>",
								action.action_name, 
								z2s_channels_table[Z2S_findTableSlotByChannelNumber(action.src_Supla_channel)].Supla_channel_name, 
								z2s_channels_table[Z2S_findTableSlotByChannelNumber(action.dst_Supla_channel)].Supla_channel_name,
								getSuplaEventName(action.src_Supla_event), 
								getSuplaActionName(action.dst_Supla_action), 
								action.is_condition ? "YES" : "NO");*/

	sprintf(general_purpose_gui_buffer, "<b>Action#:</b> <i>%d</i> <b>of</b> <i>%d</i><br><br>"
																			"<b>Action name:</b> <i>%s</i><br><br>"
																			"<b>Event:</b> <i>{%s}</i> <b>from source channel:</b> <i>[%s]</i><br><br>"
																			"<b>Action:</b> <i>{%s}</i> <b>on destination channel:</b> <i>[%s]</i>",
								current_action_counter, Z2S_getActionsNumber(),
								action.action_name, 
								getSuplaEventName(action.src_Supla_event),
								z2s_channels_table[Z2S_findTableSlotByChannelNumber(action.src_Supla_channel)].Supla_channel_name, 
								getSuplaActionName(action.dst_Supla_action),
								z2s_channels_table[Z2S_findTableSlotByChannelNumber(action.dst_Supla_channel)].Supla_channel_name);

	working_str = general_purpose_gui_buffer;
}

void enableActionDetails(bool enable) {

	if (!enable) { 

		ESPUI.updateSwitcher(action_enabled_switcher, false);
		working_str = "";
		ESPUI.updateText(action_name_text, working_str);
		ESPUI.updateText(action_description_text, working_str);
		working_str = minus_one_str;
		ESPUI.updateSelect(action_source_channel_selector, working_str);
		ESPUI.updateSelect(action_event_selector, working_str);
		ESPUI.updateSelect(action_destination_channel_selector, working_str);
		ESPUI.updateSelect(action_action_selector, working_str);
	}

	enableControlStyle(action_enabled_switcher, enable);
	enableControlStyle(action_name_text, enable);
	enableControlStyle(action_description_text, enable);
	enableControlStyle(action_source_channel_selector, enable);
	enableControlStyle(action_event_selector, enable);
	enableControlStyle(action_destination_channel_selector, enable);
	enableControlStyle(action_action_selector, enable);
}

void enableActionControls(bool enable) {

	enableControlStyle(table_first_button, enable);
	enableControlStyle(table_next_button, enable);
	enableControlStyle(table_prev_button, enable);
	enableControlStyle(table_last_button, enable);
}

void updateActionButtons() {

	bool action_save_button_enable, action_new_button_enable, action_edit_button_enable, action_cancel_button_enable, action_remove_button_enable;
	switch (current_action_gui_state) {

		case VIEW_ACTION: {

			action_save_button_enable = false;
			action_cancel_button_enable = false;

			if (current_action_id == -1) {

				action_edit_button_enable  = false;
				action_remove_button_enable = false;
			} else {

				action_edit_button_enable  = true;
				action_remove_button_enable = true;
			}

			action_new_button_enable  = true;
			
		} break;

		case NEW_ACTION: {

			action_save_button_enable = true;
			action_new_button_enable  = false;
			action_edit_button_enable  = false;
			action_cancel_button_enable = true;
			action_remove_button_enable = false;
		} break;

		case EDIT_ACTION: {

			action_save_button_enable = true;
			action_new_button_enable  = false;
			action_edit_button_enable  = false;
			action_cancel_button_enable = true;
			action_remove_button_enable = false;
		} break;
	}

	enableControlStyle(action_save_button, action_save_button_enable);
	enableControlStyle(action_new_button, action_new_button_enable);
	enableControlStyle(action_edit_button, action_edit_button_enable);
	enableControlStyle(action_cancel_button, action_cancel_button_enable);
	enableControlStyle(action_remove_button, action_remove_button_enable);
}

void updateActionDetails(z2s_channel_action_t &action, bool empty_action = false) {

	if (current_action_gui_state == VIEW_ACTION) {

		if (previous_action_gui_state != VIEW_ACTION)
			enableActionDetails(false);
		return;
	}

	if (current_action_gui_state != VIEW_ACTION)
		enableActionControls(false);

	ESPUI.updateSwitcher(action_enabled_switcher, empty_action ? true : action.is_enabled);
	working_str = empty_action ? empty_str : action.action_name;
	ESPUI.updateText(action_name_text, working_str);
	working_str = empty_action ? empty_str : action.action_description;
	ESPUI.updateText(action_description_text, working_str);
	working_str = empty_action ? -1 : action.src_Supla_channel;
	ESPUI.updateSelect(action_source_channel_selector, working_str);
	working_str = empty_action ? -1 : action.src_Supla_event;
	ESPUI.updateSelect(action_event_selector, working_str);
	working_str = empty_action ? -1 : action.dst_Supla_channel;
	ESPUI.updateSelect(action_destination_channel_selector, working_str);
	working_str = empty_action ? -1 : action.dst_Supla_action;
	ESPUI.updateSelect(action_action_selector, working_str);
}

bool fillActionDetails(z2s_channel_action_t &action) {

	action.is_enabled = ESPUI.getControl(action_enabled_switcher)->value.toInt();

	const char *action_name_str = ESPUI.getControl(action_name_text)->value.c_str();
	if (strlen(action_name_str) >= 0)
		strncpy(action.action_name, action_name_str, 32);
	else 
		return false;

	strncpy(action.action_description, ESPUI.getControl(action_description_text)->value.c_str(), 127);

	int16_t selector_value = ESPUI.getControl(action_source_channel_selector)->value.toInt();
	if ( selector_value >= 0)
		action.src_Supla_channel = selector_value;
	else
		return false;

	selector_value = ESPUI.getControl(action_event_selector)->value.toInt();
	if ( selector_value >= 0)
		action.src_Supla_event = (Supla::Event)selector_value;
	else
		return false;
	
	selector_value = ESPUI.getControl(action_destination_channel_selector)->value.toInt();
	if ( selector_value >= 0)
		action.dst_Supla_channel = selector_value;
	else
		return false;

	selector_value = ESPUI.getControl(action_action_selector)->value.toInt();
	if ( selector_value >= 0)
		action.dst_Supla_action = (Supla::Action)selector_value;
	else
		return false;

	action.is_condition = false;

	return true;
}

void buildActionsTabGUI() {

	auto actions_tab = ESPUI.addControl(Control::Type::Tab, PSTR(empty_str), "Local actions");
	
	z2s_channel_action_t new_action = {};

	//working_str = Z2S_getActionsNumber();
	//auto actions_number_label = ESPUI.addControl(Control::Type::Label, PSTR("Actions number"), working_str, 
	//																		 Control::Color::Emerald, test_tab);

	working_str = "FIRST";							
	table_first_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, 
																					  	Control::Color::Emerald, actions_tab, actionsTableCallback, (void*)GUI_CB_ACTION_FIRST_FLAG);
	working_str = "NEXT";							
	table_next_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, 
																		  			Control::Color::Emerald, table_first_button, actionsTableCallback, (void*)GUI_CB_ACTION_NEXT_FLAG);
	working_str = "PREV";							
	table_prev_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, 
																				  	Control::Color::Emerald, table_first_button, actionsTableCallback, (void*)GUI_CB_ACTION_PREV_FLAG);
	working_str = "LAST";							
	 table_last_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, 
																			 			 Control::Color::Emerald, table_first_button, actionsTableCallback, (void*)GUI_CB_ACTION_LAST_FLAG);

	working_str = "No actions";
	
	int16_t first_action = Z2S_findNextActionPosition(0);
  if (first_action >= 0) {

			current_action_id = first_action;
			current_action_counter = Z2S_getActionCounter(first_action);

      Z2S_loadAction(first_action, new_action);
			sprintfAction(new_action);
		}
	
	log_i("table html source length: %u", strlen(general_purpose_gui_buffer));
								
	actions_table_label = ESPUI.addControl(Control::Type::Label, PSTR("Table label"), working_str, 
									  		 							   Control::Color::Emerald, actions_tab);
	ESPUI.setElementStyle(actions_table_label, "text-align: left; font-family:tahoma; font-size: 4 px; font-style: normal; font-weight: normal;");
	ESPUI.setPanelWide(actions_table_label, false);

	action_enabled_switcher = ESPUI.addControl(Control::Type::Switcher, PSTR("Action details"), zero_str, 
																						 Control::Color::Emerald, actions_tab, generalCallback);
	working_str = PSTR("&#10023; action enabled &#10023;");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, 
																				 Control::Color::None, action_enabled_switcher), PSTR(clearLabelStyle));										 
	working_str = PSTR(empty_str);
	action_name_text = ESPUI.addControl(Control::Type::Text, PSTR(empty_str), working_str, 
									  		 							   Control::Color::Emerald, action_enabled_switcher, generalCallback);
	working_str = PSTR("&#10023; action name (max 32 characters) &#10023;");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, 
																				 Control::Color::None, action_enabled_switcher), PSTR(clearLabelStyle));
	working_str = PSTR(empty_str);
	action_description_text = ESPUI.addControl(Control::Type::Text, PSTR(empty_str), working_str, 
									  		 							  		 Control::Color::Emerald, action_enabled_switcher, generalCallback);
	working_str = PSTR("&#10023; action short description (max 127 characters) &#10023;");
	ESPUI.setElementStyle(ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, 
																				 Control::Color::None, action_enabled_switcher), PSTR(clearLabelStyle));
	
	action_source_channel_selector = ESPUI.addControl(Control::Type::Select, PSTR(empty_str), minus_one_str, 
																											Control::Color::Emerald, action_enabled_switcher, generalCallback);
	action_event_selector = ESPUI.addControl(Control::Type::Select, PSTR(empty_str), minus_one_str, 
																					 Control::Color::Emerald, action_enabled_switcher, generalCallback);
	action_destination_channel_selector = ESPUI.addControl(Control::Type::Select, PSTR(empty_str), minus_one_str, 
																													Control::Color::Emerald, action_enabled_switcher, generalCallback);
	action_action_selector = ESPUI.addControl(Control::Type::Select, PSTR(empty_str), minus_one_str, 
																					 	Control::Color::Emerald, action_enabled_switcher, generalCallback);


	ESPUI.addControl(Control::Type::Option, PSTR("Select source channel..."), minus_one_str, 
									 Control::Color::None, action_source_channel_selector);
	ESPUI.addControl(Control::Type::Option, PSTR("Select source event..."), minus_one_str, 
									 Control::Color::None, action_event_selector);	
	ESPUI.addControl(Control::Type::Option, PSTR("Select destination channel..."), minus_one_str, 
									 Control::Color::None, action_destination_channel_selector);
	ESPUI.addControl(Control::Type::Option, PSTR("Select destination action..."), minus_one_str, 
									 Control::Color::None, action_action_selector);

	for (uint8_t channels_counter = 0; channels_counter < Z2S_CHANNELS_MAX_NUMBER; channels_counter++) {
    if (z2s_channels_table[channels_counter].valid_record) {
      
			working_str = channels_counter;
			ESPUI.addControl(Control::Type::Option, z2s_channels_table[channels_counter].Supla_channel_name, working_str, 
											 Control::Color::None, action_source_channel_selector);
			ESPUI.addControl(Control::Type::Option, z2s_channels_table[channels_counter].Supla_channel_name, working_str, 
											 Control::Color::None, action_destination_channel_selector);
		}
	}

	uint16_t actions_number = sizeof(Supla_actions)/sizeof(Supla_action_type_t);

	for (uint16_t actions_counter = 0; actions_counter < actions_number; actions_counter++) {

		working_str = Supla_actions[actions_counter].Supla_action_id;
		ESPUI.addControl(Control::Type::Option, Supla_actions[actions_counter].Supla_action_name, working_str, 
										 Control::Color::None, action_action_selector);
	}

	uint16_t events_number = sizeof(Supla_events)/sizeof(Supla_event_type_t);

	for (uint16_t events_counter = 0; events_counter < events_number; events_counter++) {

		working_str = Supla_events[events_counter].Supla_event_id;
		ESPUI.addControl(Control::Type::Option, Supla_events[events_counter].Supla_event_name, working_str, 
										 Control::Color::None, action_event_selector);
	}

	working_str = "EDIT ACTION";							
	action_edit_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, 
																					   Control::Color::Emerald, actions_tab, actionsTableCallback, (void*)GUI_CB_ACTION_EDIT_FLAG);
	working_str = "NEW ACTION";							
	action_new_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, 
																		  			Control::Color::Emerald, action_edit_button, actionsTableCallback, (void*)GUI_CB_ACTION_NEW_FLAG);
	working_str = "SAVE ACTION";							
	action_save_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, 
																				  	 Control::Color::Emerald, action_edit_button, actionsTableCallback, (void*)GUI_CB_ACTION_SAVE_FLAG);
	working_str = "CANCEL CHANGES";							
	action_cancel_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, 
																			 					 Control::Color::Emerald, action_edit_button, actionsTableCallback, (void*)GUI_CB_ACTION_CANCEL_FLAG);
	working_str = "DELETE ACTION";							
	action_remove_button = ESPUI.addControl(Control::Type::Button, PSTR(empty_str), working_str, 
																			 			 		Control::Color::Emerald, action_edit_button, actionsTableCallback, (void*)GUI_CB_ACTION_REMOVE_FLAG);
	working_str = three_dots_str;
	action_state_label = ESPUI.addControl(Control::Type::Label, PSTR(empty_str), working_str, 
																			 			 		Control::Color::Emerald, action_edit_button);
	
	enableActionDetails(false);
}

void Z2S_buildWebGUI() {
 
	ESPUI.sliderContinuous = true;
	//ESPUI.setVerbosity(Verbosity::VerboseJSON);

	working_str.reserve(1056);

	buildGatewayTabGUI();
	buildCredentialsGUI();
	buildZigbeeTabGUI();
	buildDevicesTabGUI();
	buildChannelsTabGUI();
	buildActionsTabGUI();
	buildAdvancedDevicesTabGUI();
	buildTuyaCustomClusterTabGUI();
	enableDeviceControls(false);
	enableChannelControls(false);
}

void Z2S_startWebGUIConfig() {

	//char buf[1024] = {};
	
	fillGatewayGeneralnformation(general_purpose_gui_buffer);

	ESPUI.addControl(Control::Type::Separator, PSTR("General information"), "", Control::Color::None);
	gateway_general_info = ESPUI.addControl(Control::Type::Label, "Device information", general_purpose_gui_buffer, Control::Color::Emerald);

	fillMemoryUptimeInformation(general_purpose_gui_buffer);
	
	ESPUI.addControl(Control::Type::Separator, PSTR("Status"), "", Control::Color::None);
	gateway_memory_info = ESPUI.addControl(Control::Type::Label, "Memory & Uptime", general_purpose_gui_buffer, Control::Color::Emerald);
	//ESPUI.setElementStyle(gateway_memory_info, "text-align: left; font-size: 6 px; font-style: normal; font-weight: normal;");

	ESPUI.addControl(Control::Type::Separator, PSTR("Status"), PSTR(empty_str), Control::Color::None);
  ESPUI.addControl(Control::Type::Separator, PSTR("WiFi & Supla credentials"), "", Control::Color::None);
	wifi_ssid_text = ESPUI.addControl(Control::Type::Text, PSTR("SSID"), "", Control::Color::Emerald, Control::noParent, textCallback);
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), "32", Control::Color::None, wifi_ssid_text);
	wifi_pass_text = ESPUI.addControl(Control::Type::Text, "Password", "", Control::Color::Emerald, Control::noParent, textCallback);
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), "64", Control::Color::None, wifi_pass_text);
	ESPUI.setInputType(wifi_pass_text, "password");
	Supla_server = ESPUI.addControl(Control::Type::Text, PSTR("Supla server"), "", Control::Color::Emerald, Control::noParent, textCallback);
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), "64", Control::Color::None, Supla_server);
	Supla_email = ESPUI.addControl(Control::Type::Text, "Supla email", "", Control::Color::Emerald, Control::noParent, textCallback);
	ESPUI.addControl(Control::Type::Max, PSTR(empty_str), "64", Control::Color::None, Supla_email);
	Supla_skip_certificate_switcher = ESPUI.addControl(Control::Type::Switcher, PSTR("Skip CA certificate check"), zero_str, 
																									 Control::Color::Emerald, Control::noParent, generalCallback);

	save_button = ESPUI.addControl(Control::Type::Button, PSTR("Save"), "Save", Control::Color::Emerald, 
																 Control::noParent, enterWifiDetailsCallback,(void*) GUI_CB_SAVE_FLAG);

	auto save_n_restart_button = ESPUI.addControl(Control::Type::Button, PSTR("Save & Restart"), "Save & Restart", 
																								Control::Color::Emerald, save_button, enterWifiDetailsCallback, (void*) GUI_CB_RESTART_FLAG);

	save_label = ESPUI.addControl(Control::Type::Label, PSTR("Status"), "Missing data...", Control::Color::Emerald, save_button);

	auto cfg = Supla::Storage::ConfigInstance();
  
	if (cfg) {

  	memset(general_purpose_gui_buffer, 0, sizeof(general_purpose_gui_buffer));
  	if (cfg->getWiFiSSID(general_purpose_gui_buffer) && strlen(general_purpose_gui_buffer) > 0)
			ESPUI.updateText(wifi_ssid_text, general_purpose_gui_buffer);
		memset(general_purpose_gui_buffer, 0, sizeof(general_purpose_gui_buffer));
		if (cfg->getSuplaServer(general_purpose_gui_buffer) && strlen(general_purpose_gui_buffer) > 0)
			ESPUI.updateText(Supla_server, general_purpose_gui_buffer);
		memset(general_purpose_gui_buffer, 0, sizeof(general_purpose_gui_buffer));
		if (cfg->getEmail(general_purpose_gui_buffer) && strlen(general_purpose_gui_buffer) > 0)
			ESPUI.updateText(Supla_email, general_purpose_gui_buffer);
		ESPUI.updateNumber(Supla_skip_certificate_switcher, _z2s_security_level);
	}

	ESPUI.begin("ZIGBEE <=> SUPLA CONFIG PAGE");
}

void Z2S_startWebGUI() {

  ESPUI.begin("ZIGBEE <=> SUPLA CONTROL PANEL");

}

void Z2S_stopWebGUI() {
	
	if (ESPUI.WebServer())
		ESPUI.WebServer()->end();
	current_Tuya_payload_label = 0;
}

void onUpdateBegin(const UpdateType type, int &result) {

	rgbLedWrite(RGB_BUILTIN, 255, 0, 255);  
}

void onUpdateEnd(const UpdateType type, int &result) {

	rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
}

void Z2S_startUpdateServer() {

	if (ESPUI.WebServer()) {
  	updateServer.setup(ESPUI.WebServer());
		updateServer.onUpdateBegin = onUpdateBegin;
		updateServer.onUpdateEnd = onUpdateEnd;
	}
	/*if (ESPUI.WebServer())
  	ESPUI.WebServer()->serveStatic("/files", LittleFS, "/");
	if (ESPUI.WebServer())
  	ESPUI.WebServer()->serveStatic("/supla", LittleFS, "/supla");*/
}

void Z2S_updateWebGUI() {

	//char buf[1024] = {};

	fillMemoryUptimeInformation(general_purpose_gui_buffer);
	//working_str = general_purpose_gui_buffer;
	updateLabel_P(gateway_memory_info, general_purpose_gui_buffer);
}

void Z2S_loopWebGUI() {

}

void enterWifiDetailsCallback(Control *sender, int type, void *param) {

	if ((type == B_UP) && data_ready) {
		Serial.println("Saving credentials to Supla Config...");
		Serial.println(ESPUI.getControl(wifi_ssid_text)->value);
		Serial.println(ESPUI.getControl(wifi_pass_text)->value);
		Serial.println(ESPUI.getControl(Supla_server)->value);
		Serial.println(ESPUI.getControl(Supla_email)->value);
		
    auto cfg = Supla::Storage::ConfigInstance();

		if (cfg) {
			cfg->setWiFiSSID(ESPUI.getControl(wifi_ssid_text)->value.c_str());
  		cfg->setWiFiPassword(ESPUI.getControl(wifi_pass_text)->value.c_str());
			cfg->setSuplaServer(ESPUI.getControl(Supla_server)->value.c_str());
		  cfg->setEmail(ESPUI.getControl(Supla_email)->value.c_str());
			cfg->setUInt8(PSTR("security_level"), ESPUI.getControl(Supla_skip_certificate_switcher)->value.toInt() > 0 ? 2 :0);

			cfg->commit();
			if ((uint32_t)param == GUI_CB_RESTART_FLAG) SuplaDevice.softRestart();
		}
	}
}

void textCallback(Control *sender, int type) {
	Serial.println(type);
	//Serial.println(ESPUI.getControl(wifi_ssid_text)->value);
//		Serial.println(ESPUI.getControl(wifi_pass_text)->value);
	//	Serial.println(ESPUI.getControl(Supla_server)->value);
//		Serial.println(ESPUI.getControl(Supla_email)->value);
		
		if ((ESPUI.getControl(wifi_ssid_text)->value.length() > 0) &&
				(ESPUI.getControl(wifi_pass_text)->value.length() > 0) &&
				(ESPUI.getControl(Supla_server)->value.length() > 0) &&
				(ESPUI.getControl(Supla_email)->value.length() > 0)) {
			updateLabel_P(save_label, PSTR("Data complete. Press Save"));
			data_ready = true;
		} else {
			updateLabel_P(save_label, PSTR("Data incomplete!"));
			data_ready = false;
		}
}

void generalCallback(Control *sender, int type) {
	Serial.print("CB: id(");
	Serial.print(sender->GetId());
	Serial.print(") Type(");
	Serial.print(type);
	Serial.print(") '");
	Serial.print(sender->label);
	Serial.print("' = ");
	Serial.println(sender->value);
}

void onZigbeeTabCallback(Control *sender, int type) {
	Serial.print("CB: id(");
	Serial.print(sender->GetId());
	Serial.print(") Type(");
	Serial.print(type);
	Serial.print(") '");
	Serial.print(sender->label);
	Serial.print("' = ");
	Serial.println(sender->value);
	ESPUI.updateNumber(pairing_mode_switcher, Zigbee.isNetworkOpen() ? 1 : 0);	
}

void generalMinMaxCallback(Control *sender, int type, void* param) {
	Serial.print("CB: id(");
	Serial.print(sender->GetId());
	if (sender->value.toInt() < 0)
		ESPUI.updateNumber(sender->GetId(), 0);
	uint32_t max_value = (uint32_t)param;
	if (sender->value.toInt() > max_value)
		ESPUI.updateNumber(sender->GetId(), max_value);
}

void endpointCallback(Control *sender, int type) {
	Serial.print("CB: id(");
	Serial.print(sender->GetId());
	if (sender->value.toInt() < 1)
		ESPUI.updateNumber(sender->GetId(), 1);
	//uint8_t device_endpoints = z2s_zb_devices_table[ESPUI.getControl(device_selector)->value.toInt()].endpoints_count;
	if (sender->value.toInt() > 255) //device_endpoints)
		ESPUI.updateNumber(sender->GetId(), 255); //device_endpoints);

}

void enableDeviceControls(bool enable) {

	updateLabel_P(zb_device_info_label, three_dots_str);
	enableControlStyle(zb_device_info_label, enable);

	working_str = PSTR(empty_str);
	ESPUI.updateText(device_name_text, working_str);

	ESPUI.updateNumber(battery_voltage_min_number, 0);
	ESPUI.updateNumber(battery_voltage_max_number, 0);
	
	enableControlStyle(device_name_text, enable);
	enableControlStyle(device_name_save_button, enable);

	enableControlStyle(battery_voltage_min_number, enable);
	enableControlStyle(battery_voltage_min_save_button, enable);
	enableControlStyle(battery_voltage_max_number, enable);
	enableControlStyle(battery_voltage_max_save_button, enable);
		
	//updateLabel_P(swbuildlabel, three_dots_str);
	updateLabel_P(rssilabel, three_dots_str);
	updateLabel_P(device_read_attribute_label, three_dots_str);
	enableControlStyle(device_read_attribute_label, enable);
	updateLabel_P(device_Tuya_payload_label, three_dots_str);
	enableControlStyle(device_Tuya_payload_label, enable);

	ESPUI.updateSelect(device_cluster_selector, minus_one_str);
	ESPUI.updateSelect(device_attribute_id_selector, minus_one_str);
	ESPUI.updateSelect(device_attribute_value_selector, minus_one_str);
	ESPUI.updateSelect(device_attribute_type_selector, minus_one_str);

	ESPUI.updateNumber(device_endpoint_number, 1);
	ESPUI.updateNumber(device_attribute_size_number, 0);
	ESPUI.updateNumber(device_config_min_number, 0);
	ESPUI.updateNumber(device_config_max_number, 0);
	ESPUI.updateNumber(device_config_delta_number, 0);
	
	ESPUI.updateText(device_attribute_id_text, zero_str);
	enableControlStyle(getswbuild_button, enable);
	enableControlStyle(getrssi_button, enable);

	enableControlStyle(device_endpoint_number, enable);
	enableControlStyle(device_cluster_selector, enable);
	enableControlStyle(device_attribute_id_text, enable);
	enableControlStyle(device_attribute_id_selector, enable);
	enableControlStyle(device_attribute_type_selector, enable);
	enableControlStyle(device_attribute_value_selector, enable);
	enableControlStyle(device_attribute_size_number, enable);
	enableControlStyle(device_config_min_number, enable);
	enableControlStyle(device_config_max_number, enable);
	enableControlStyle(device_config_delta_number, enable);
	enableControlStyle(device_attribute_value_text, enable);
	enableControlStyle(device_async_switcher, enable);
	enableControlStyle(device_read_attribute_button, enable);
	enableControlStyle(device_write_attribute_button, enable);
	enableControlStyle(device_read_config_button, enable);
	enableControlStyle(device_write_config_button, enable);
	enableControlStyle(device_custom_command_button, enable);

	enableControlStyle(remove_device_button, enable);
	enableControlStyle(remove_device_and_channels_button, enable);
	enableControlStyle(remove_all_devices_button, enable);

	device_controls_enabled = enable;
}

void updateDeviceInfoLabel() {

	char ieee_addr_str[24] 		= {};

	uint8_t device_slot = ESPUI.getControl(device_selector)->value.toInt();

  sprintf_P(ieee_addr_str, PSTR("%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X"), 
					z2s_zb_devices_table[device_slot].ieee_addr[7],
					z2s_zb_devices_table[device_slot].ieee_addr[6], 
					z2s_zb_devices_table[device_slot].ieee_addr[5], 
					z2s_zb_devices_table[device_slot].ieee_addr[4], 
          z2s_zb_devices_table[device_slot].ieee_addr[3],
					z2s_zb_devices_table[device_slot].ieee_addr[2],
					z2s_zb_devices_table[device_slot].ieee_addr[1], 
					z2s_zb_devices_table[device_slot].ieee_addr[0]);

	uint8_t battery_percentage = z2s_zb_devices_table[device_slot].battery_percentage >= 0x80 ? 
												  		 z2s_zb_devices_table[device_slot].battery_percentage - 0x80 : 0xFF;

	sprintf_P(general_purpose_gui_buffer,PSTR("<b><i><style=color:black>Slot#</b></i> %02u <b>| <i>Manufacturer name<style=;></i></b> %s "
					"<b>| <i>model ID</b></i> %s <b>| <i>Z2S model</b></i> %s [0x%04X]<br>"
					"<b><i>IEEE address</b></i> %s <b>| <i>Short address</b></i> 0x%04X <b>| <i>Power source</b></i> 0x%02X<br>"
					"<b><i>Battery percentage</b></i> %u <b>| <i>Last seen (ms)</b></i> %lu "
					"<b>| <i>Gateway unit last seen (ms)</b></i>  %lu <b>| <i>Gateway unit last RSSI</b></i> %d<br>"
					"<b><i>Device flags</b></i> 0x%08X <b>| <i>ud(1)</b></i> 0x%08X <b>| <i>ud(2)</b></i> 0x%08X<br>"), 
					device_slot,
					Z2S_getZBDeviceManufacturerName(device_slot),
					Z2S_getZBDeviceModelName(device_slot),
					getZ2SDeviceDescName(z2s_zb_devices_table[device_slot].desc_id), z2s_zb_devices_table[device_slot].desc_id,
					ieee_addr_str,
					z2s_zb_devices_table[device_slot].short_addr,
					z2s_zb_devices_table[device_slot].power_source,
					battery_percentage,
					z2s_zb_devices_table[device_slot].last_seen_ms,
					zbGateway.getZbgDeviceUnitLastSeenMs(z2s_zb_devices_table[device_slot].short_addr),
					zbGateway.getZbgDeviceUnitLastRssi(z2s_zb_devices_table[device_slot].short_addr),
					z2s_zb_devices_table[device_slot].user_data_flags,
					z2s_zb_devices_table[device_slot].user_data_1,
					z2s_zb_devices_table[device_slot].user_data_2);

	log_i("value = %s, length = %u", 
				general_purpose_gui_buffer, 
				strlen(general_purpose_gui_buffer));

	updateLabel_P(zb_device_info_label, general_purpose_gui_buffer);

	working_str = z2s_zb_devices_table[device_slot].device_local_name;
	ESPUI.updateText(device_name_text, working_str);

	ESPUI.updateNumber(battery_voltage_min_number, 
										 z2s_zb_devices_table[device_slot].battery_voltage_min);
	ESPUI.updateNumber(battery_voltage_max_number, 
										 z2s_zb_devices_table[device_slot].battery_voltage_max);
	
	ESPUI.updateNumber(disable_battery_percentage_msg_switcher, 
										 (z2s_zb_devices_table[device_slot].user_data_flags & 
										 	ZBD_USER_DATA_FLAG_DISABLE_BATTERY_PERCENTAGE_MSG) ? 1 : 0);
	ESPUI.updateNumber(disable_battery_voltage_msg_switcher, 
										 (z2s_zb_devices_table[device_slot].user_data_flags & 
										 ZBD_USER_DATA_FLAG_DISABLE_BATTERY_VOLTAGE_MSG) ? 1 : 0);
	
	
}

void deviceSelectorCallback(Control *sender, int type) {

	if ((!isNumber(sender->value)) || 
			(sender->value.toInt() < 0) || 
			(sender->value.toInt() >= Z2S_ZB_DEVICES_MAX_NUMBER)) {

		if (device_controls_enabled)
			enableDeviceControls(false);
		return;
	}
	
	if (!device_controls_enabled)
		enableDeviceControls(true);

	updateDeviceInfoLabel();
}

void enableChannelControls(bool enable) {

	
	updateLabel_P(zb_channel_info_label, three_dots_str);
	working_str = PSTR(empty_str);
	ESPUI.updateText(channel_name_text, working_str);

	ESPUI.updateNumber(keepalive_number, 0);
	ESPUI.updateNumber(timeout_number, 0);
	ESPUI.updateNumber(refresh_number, 0);
	ESPUI.updateNumber(disable_channel_notifications_switcher, 0);
	ESPUI.updateNumber(trv_auto_to_schedule_switcher, 0);
	ESPUI.updateNumber(set_sorwns_on_start_switcher, 0);
	
	enableControlStyle(channel_name_text, enable);
	enableControlStyle(channel_name_save_button, enable);
	enableControlStyle(disable_channel_notifications_switcher,enable);
	enableControlStyle(trv_auto_to_schedule_switcher, enable);
	enableControlStyle(set_sorwns_on_start_switcher, enable);
	enableControlStyle(keepalive_number, enable);
	enableControlStyle(keepalive_save_button, enable);
	enableControlStyle(timeout_number, enable);
	enableControlStyle(timeout_save_button, enable);
	enableControlStyle(refresh_number, enable);
	enableControlStyle(refresh_save_button, enable);
	enableControlStyle(remove_channel_button, enable);
	enableControlStyle(remove_all_channels_button, enable);
	enableControlStyle(zb_channel_timings_label, enable);
	enableControlStyle(zb_channel_flags_label, enable);
	
	channel_controls_enabled = enable;
}

void enableChannelTimings(uint8_t timings_mask) {

	if (timings_mask & 1)
		enableControlStyle(keepalive_number, true);
	else {
		ESPUI.updateNumber(keepalive_number, 0);
		enableControlStyle(keepalive_number, false);
	}

	if (timings_mask & 2)
		enableControlStyle(timeout_number, true);
	else {
		ESPUI.updateNumber(timeout_number, 0);
		enableControlStyle(timeout_number, false);
	}

	if (timings_mask & 4)
		enableControlStyle(refresh_number, true);
	else {
		ESPUI.updateNumber(refresh_number, 0);
		enableControlStyle(refresh_number, false);
	}

	if (timings_mask == 0)
		enableControlStyle(zb_channel_timings_label, false);
	else
		enableControlStyle(zb_channel_timings_label, true);
}

void enableChannelFlags(uint8_t flags_mask) {

	if (flags_mask & 1)
		enableControlStyle(disable_channel_notifications_switcher, true);
	else {
		ESPUI.updateNumber(disable_channel_notifications_switcher, 0);
		enableControlStyle(disable_channel_notifications_switcher, false);
	}

	if (flags_mask & 2)
		enableControlStyle(trv_auto_to_schedule_switcher, true);
	else {
		ESPUI.updateNumber(trv_auto_to_schedule_switcher, 0);
		enableControlStyle(trv_auto_to_schedule_switcher, false);
	}

	if (flags_mask & 4)
		enableControlStyle(set_sorwns_on_start_switcher, true);
	else {
		ESPUI.updateNumber(set_sorwns_on_start_switcher, 0);
		enableControlStyle(set_sorwns_on_start_switcher, false);
	}

	if (flags_mask == 0)
		enableControlStyle(zb_channel_flags_label, false);
	else
		enableControlStyle(zb_channel_flags_label, true);
}


void updateChannelInfoLabel(uint8_t label_number) {

	char ieee_addr_str[24] 		= {};

	uint8_t channel_slot = ESPUI.getControl(channel_selector)->value.toInt();
	
  sprintf_P(ieee_addr_str, PSTR("%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X"), 
					z2s_channels_table[channel_slot].ieee_addr[7],
					z2s_channels_table[channel_slot].ieee_addr[6], 
					z2s_channels_table[channel_slot].ieee_addr[5], 
					z2s_channels_table[channel_slot].ieee_addr[4], 
          z2s_channels_table[channel_slot].ieee_addr[3],
					z2s_channels_table[channel_slot].ieee_addr[2],
					z2s_channels_table[channel_slot].ieee_addr[1], 
					z2s_channels_table[channel_slot].ieee_addr[0]);
	
	sprintf_P(general_purpose_gui_buffer,
					//PSTR("<meta charset=\"UTF-8\"><b><i>Channel name:</i></b> %s<br>"
					PSTR("<meta charset=\"UTF-8\">"
					"<b><i>IEEE address</i></b> %s <b>| <i>Short address</i></b> 0x%04X <b>| <i>endpoint</i></b> 0x%02X <b>| <i>cluster</i></b> 0x%04X<br>"
					"<b><i>Model id</i></b> %s [0x%04X] <b>| <i>channel</i></b> #%u <b>| <i>secondary channel</i></b> #%u<br>"
					"<b><i>Type</b></i> %s <b>| <i>Function</b></i> %s <b>| <i>Sub id</b></i> %d<br>"
					"<b><i>Channel flags</b></i> 0x%08X <b>| <i>ud(1)</b></i> 0x%08X <b>| <i>ud(2)</b></i> 0x%08X<br>"
					"<b><i>ZB device</b></i> %s (%s::%s)"),
					//strlen(z2s_channels_table[channel_slot].Supla_channel_name) > 0 ? z2s_channels_table[channel_slot].Supla_channel_name : "---",
					ieee_addr_str,
					z2s_channels_table[channel_slot].short_addr,
					z2s_channels_table[channel_slot].endpoint,
        	z2s_channels_table[channel_slot].cluster_id,
					getZ2SDeviceDescName(z2s_channels_table[channel_slot].model_id),
        	z2s_channels_table[channel_slot].model_id,
        	z2s_channels_table[channel_slot].Supla_channel,
        	z2s_channels_table[channel_slot].Supla_secondary_channel,
					getSuplaChannelTypeName(z2s_channels_table[channel_slot].Supla_channel_type),
        	z2s_channels_table[channel_slot].Supla_channel_func > 0 ? 
						getSuplaChannelFuncName(z2s_channels_table[channel_slot].Supla_channel_type, 
																		z2s_channels_table[channel_slot].Supla_channel_func) : "none",
        	z2s_channels_table[channel_slot].sub_id,
					z2s_channels_table[channel_slot].user_data_flags,
					z2s_channels_table[channel_slot].user_data_1,
					z2s_channels_table[channel_slot].user_data_2,
					Z2S_getZBDeviceLocalName(z2s_channels_table[channel_slot].ZB_device_id),
					Z2S_getZBDeviceManufacturerName(z2s_channels_table[channel_slot].ZB_device_id),
					Z2S_getZBDeviceModelName(z2s_channels_table[channel_slot].ZB_device_id));
	
	log_i("Up2HERE!, value = %s, length = %u", general_purpose_gui_buffer, strlen(general_purpose_gui_buffer));
	//if (label_number == 1)
	updateLabel_P(zb_channel_info_label, general_purpose_gui_buffer);
	//delay(200);
	/*sprintf(channel_info_str_2, 
				"<meta charset=\"UTF-8\"><b><i>Data flags</b></i> %lu <b>| <i>user data(1)</b></i> %lu <b>| <i>user data(2)</b></i> %lu <b>| <i>user data(3)</b></i> %lu" 
					"<b>| <i>user data(4)</b></i> %lu <b>| <i>data counter</b></i> %llu<br>"
					"<b><i>ZB device</b></i> %s::%s",
        	z2s_channels_table[channel_slot].user_data_flags,
        	z2s_channels_table[channel_slot].user_data_1,
        	z2s_channels_table[channel_slot].user_data_2,
        	z2s_channels_table[channel_slot].user_data_3,
        	z2s_channels_table[channel_slot].user_data_4,
					z2s_channels_table[channel_slot].data_counter,
					z2s_zb_devices_table[z2s_channels_table[channel_slot].ZB_device_id].manufacturer_name,
					z2s_zb_devices_table[z2s_channels_table[channel_slot].ZB_device_id].model_name);

	log_i("Up2HERE!, value = %s, length = %u", channel_info_str_2, strlen(channel_info_str_2));
	//updateLabel_P(zb_channel_info_label, channel_info_str);
	if (label_number == 2)
		updateLabel_P(zb_channel_info_label_2, channel_info_str_2);*/
	working_str = z2s_channels_table[channel_slot].Supla_channel_name;
	ESPUI.updateText(channel_name_text, working_str);
	switch (z2s_channels_table[channel_slot].Supla_channel_type) {


		case SUPLA_CHANNELTYPE_BINARYSENSOR: {

			enableChannelTimings(6); //timeout+debounce
			ESPUI.updateNumber(timeout_number, z2s_channels_table[channel_slot].timeout_secs);
			ESPUI.updateNumber(refresh_number, z2s_channels_table[channel_slot].refresh_secs);
	
			enableChannelFlags(5);
			ESPUI.updateNumber(disable_channel_notifications_switcher, 
										 (z2s_channels_table[channel_slot].user_data_flags & 
											USER_DATA_FLAG_DISABLE_NOTIFICATIONS) ? 1 : 0);
			ESPUI.updateNumber(set_sorwns_on_start_switcher, 
										 (z2s_channels_table[channel_slot].user_data_flags & 
										 USER_DATA_FLAG_SET_SORWNS_ON_START) ? 1 : 0);
			
		} break;


		case SUPLA_CHANNELTYPE_ACTIONTRIGGER: {

			enableChannelTimings(6); //timeout + debounce 
			ESPUI.updateNumber(timeout_number, z2s_channels_table[channel_slot].timeout_secs);
			ESPUI.updateNumber(refresh_number, z2s_channels_table[channel_slot].refresh_secs);
	
			enableChannelFlags(0); 
		} break;


		case SUPLA_CHANNELTYPE_THERMOMETER:
		case SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR:
		case SUPLA_CHANNELTYPE_PRESSURESENSOR: {

			enableChannelTimings(2); //timeout only
			ESPUI.updateNumber(timeout_number, z2s_channels_table[channel_slot].timeout_secs);
	
			enableChannelFlags(4);
		} break;


		case SUPLA_CHANNELTYPE_GENERAL_PURPOSE_MEASUREMENT: {

			enableChannelTimings(0);
			enableChannelFlags(0);
		} break;
		
		
		case SUPLA_CHANNELTYPE_HVAC: {
			
			enableChannelTimings(2); //timeout only
			ESPUI.updateNumber(timeout_number, z2s_channels_table[channel_slot].timeout_secs);
	
			enableChannelFlags(2);
			ESPUI.updateNumber(trv_auto_to_schedule_switcher, 
										 (z2s_channels_table[channel_slot].user_data_flags &
										 USER_DATA_FLAG_TRV_AUTO_TO_SCHEDULE) ? 1 : 0);
		} break;


		case SUPLA_CHANNELTYPE_RELAY:
		case SUPLA_CHANNELTYPE_VALVE_OPENCLOSE:
		case SUPLA_CHANNELTYPE_DIMMER:
		case SUPLA_CHANNELTYPE_RGBLEDCONTROLLER: {

			enableChannelTimings(3); //timeout + keepalive
			ESPUI.updateNumber(keepalive_number, z2s_channels_table[channel_slot].keep_alive_secs);
			ESPUI.updateNumber(timeout_number, z2s_channels_table[channel_slot].timeout_secs);

			enableChannelFlags(0);
		} break;

		case SUPLA_CHANNELTYPE_ELECTRICITY_METER: {

			enableChannelTimings(7); //timeout + keepalive + refresh
			ESPUI.updateNumber(keepalive_number, z2s_channels_table[channel_slot].keep_alive_secs);
			ESPUI.updateNumber(timeout_number, z2s_channels_table[channel_slot].timeout_secs);
			ESPUI.updateNumber(refresh_number, z2s_channels_table[channel_slot].refresh_secs);

			enableChannelFlags(0);
		} break;


		default: {

			enableChannelTimings(0);
			enableChannelFlags(0);
		} break;
	}
}

void channelSelectorCallback(Control *sender, int type) {
	
	if (GUI_update_required) return;

	if ((!isNumber(sender->value)) || 
			(sender->value.toInt() < 0) || 
			(sender->value.toInt() >= Z2S_CHANNELS_MAX_NUMBER)) {

		if (channel_controls_enabled)
			enableChannelControls(false);
		return;
	}
	
	if (!channel_controls_enabled)
		enableChannelControls(true);

	updateChannelInfoLabel(1);
}

void getZigbeeDeviceQueryCallback(Control *sender, int type, void *param) {

	if ((type == B_UP) && (ESPUI.getControl(device_selector)->value.toInt() >= 0)) {

		zbg_device_params_t device;

		log_i("device_selector value %u", ESPUI.getControl(device_selector)->value.toInt());
    device.endpoint = ESPUI.getControl(device_endpoint_number)->value.toInt();
    device.cluster_id = 0; 
    memcpy(&device.ieee_addr, z2s_zb_devices_table[ESPUI.getControl(device_selector)->value.toInt()].ieee_addr,8);
    device.short_addr = z2s_zb_devices_table[ESPUI.getControl(device_selector)->value.toInt()].short_addr;

		uint16_t cluster_id = ESPUI.getControl(device_cluster_selector)->value.toInt();
		uint16_t attribute_id = strtoul(ESPUI.getControl(device_attribute_id_text)->value.c_str(), nullptr,0);
		uint8_t attribute_type_idx = ESPUI.getControl(device_attribute_type_selector)->value.toInt();
		uint8_t attribute_type = attribute_type_idx < 0xFF ? zigbee_datatypes[attribute_type_idx].zigbee_datatype_id : 0xFF;
		bool sync_cmd = ESPUI.getControl(device_async_switcher)->value.toInt() == 0;
		uint8_t manuf_specific = (ESPUI.getControl(manufacturer_code_switcher)->value.toInt() == 0) ? 0 : 1;
		uint16_t manuf_code = (ESPUI.getControl(manufacturer_code_selector)->value.toInt() > 0) ? 
			ESPUI.getControl(manufacturer_code_selector)->value.toInt() : 0;

		switch ((uint32_t)param) {
			case GUI_CB_SWBUILD_FLAG: { //software build ID

				if (zbGateway.zbQueryDeviceBasicCluster(&device, true, ESP_ZB_ZCL_ATTR_BASIC_SW_BUILD_ID)) {
					if (strlen(zbGateway.getQueryBasicClusterData()->software_build_ID) > 0) 
						updateLabel_P(rssilabel, zbGateway.getQueryBasicClusterData()->software_build_ID);
				} else
						updateLabel_P(rssilabel, device_query_failed_str);
			} break;
			case GUI_CB_RSSI_FLAG: { //RSSI

				if (zbGateway.sendCustomClusterCmd(&device, 0x0003, 0x0000, ESP_ZB_ZCL_ATTR_TYPE_NULL, 0, nullptr, true)) {
					
					sprintf_P(general_purpose_gui_buffer, PSTR("%d"), zbGateway.getZbgDeviceUnitLastRssi(device.short_addr));
					updateLabel_P(rssilabel, general_purpose_gui_buffer);
				}
				else
					updateLabel_P(rssilabel, device_query_failed_str);
			} break;

			case GUI_CB_READ_ATTR_FLAG : { //read attribute
					//if (ESPUI.getControl(device_cluster_selector)->value.toInt() < 0) 
					//	break;
					
					bool result = zbGateway.sendAttributeRead(&device, cluster_id, attribute_id, sync_cmd,
																										ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV, 1, manuf_specific, manuf_code);
					if (result) {
						if (*zbGateway.getReadAttrStatusLastResult() == ESP_ZB_ZCL_STATUS_SUCCESS) {

							uint64_t readAttrValue;
							esp_zb_uint48_t readAttrValue48;
							esp_zb_uint24_t readAttrValue24;

							switch (zbGateway.getReadAttrLastResult()->data.size) {
								case 1: readAttrValue = *(uint8_t *)zbGateway.getReadAttrLastResult()->data.value; break;
								case 2: readAttrValue = *(uint16_t *)zbGateway.getReadAttrLastResult()->data.value; break;
								case 3: {
									readAttrValue24 = *(esp_zb_uint24_t *)zbGateway.getReadAttrLastResult()->data.value; 
									readAttrValue = (((uint64_t)readAttrValue24.high) << 16) + readAttrValue24.low;
								} break;
								case 4: readAttrValue = *(uint32_t *)zbGateway.getReadAttrLastResult()->data.value; break;
								case 6: {
									readAttrValue48 = *(esp_zb_uint48_t *)zbGateway.getReadAttrLastResult()->data.value; 
									readAttrValue = (((uint64_t)readAttrValue48.high) << 32) + readAttrValue48.low;
								} break;
								case 8: readAttrValue = *(uint64_t *)zbGateway.getReadAttrLastResult()->data.value; break;
							}
						
							sprintf_P(general_purpose_gui_buffer, PSTR("Reading attribute successful!<br>Data value is %llu(0x%llX)<br>Data type is %s(0x%X)<br>Data size is 0x%X"), 
                      readAttrValue, readAttrValue, getZigbeeDataTypeName(zbGateway.getReadAttrLastResult()->data.type), zbGateway.getReadAttrLastResult()->data.type,
							 		  	zbGateway.getReadAttrLastResult()->data.size);
							updateLabel_P(device_read_attribute_label, general_purpose_gui_buffer);
						} else {

							sprintf_P(general_purpose_gui_buffer, PSTR("Reading attribute failed!<br>"
											"Status = %s(0x%02X)<br>"
											"Attribute id = 0x%04X"),
											esp_zb_zcl_status_to_name(*zbGateway.getReadAttrStatusLastResult()),
											*zbGateway.getReadAttrStatusLastResult(),
											zbGateway.getReadAttrLastResult()->id);
							updateLabel_P(device_read_attribute_label, general_purpose_gui_buffer);
						}
					} else {
						if (sync_cmd)
							updateLabel_P(device_read_attribute_label, device_query_failed_str);
						else
							updateLabel_P(device_read_attribute_label, device_async_query_str);
					} 
			} break;

			case GUI_CB_READ_CONFIG_FLAG : { //read attribute config report
					bool result = zbGateway.readClusterReportCfgCmd(&device, cluster_id, attribute_id, sync_cmd);
					if (result) {
						if (zbGateway.getReportConfigRespVariableLastResult()->status == ESP_ZB_ZCL_STATUS_SUCCESS) {

							sprintf_P(general_purpose_gui_buffer, PSTR("Reading attribute config report successful! <br>"
											"Attribute id is 0x%X<br>Data type is %s(0x%X)<br>"
											"Min interval is %u(0x%x)<br>Max interval is %u(0x%X)<br>"
											"Delta is %u(0x%X)"), 
                  	  zbGateway.getReportConfigRespVariableLastResult()->attribute_id,
											getZigbeeDataTypeName(zbGateway.getReportConfigRespVariableLastResult()->client.attr_type),
											zbGateway.getReportConfigRespVariableLastResult()->client.attr_type,
											zbGateway.getReportConfigRespVariableLastResult()->client.min_interval,
											zbGateway.getReportConfigRespVariableLastResult()->client.min_interval,
											zbGateway.getReportConfigRespVariableLastResult()->client.max_interval,
											zbGateway.getReportConfigRespVariableLastResult()->client.max_interval,
											zbGateway.getReportConfigRespVariableLastResult()->client.delta[0],
											zbGateway.getReportConfigRespVariableLastResult()->client.delta[0]);
							updateLabel_P(device_read_attribute_label, general_purpose_gui_buffer);
						} else {
							sprintf_P(general_purpose_gui_buffer, 
											PSTR("Reading attribute config report failed!<br>"
											"Status = %s(0x%02X)<br>"
											"Attribute id = 0x%04X<br>"),
											esp_zb_zcl_status_to_name(zbGateway.getReportConfigRespVariableLastResult()->status),
											zbGateway.getReportConfigRespVariableLastResult()->status,
											zbGateway.getReportConfigRespVariableLastResult()->status,
      		       		  zbGateway.getReportConfigRespVariableLastResult()->attribute_id);
							updateLabel_P(device_read_attribute_label, general_purpose_gui_buffer);
						}
					} else {
						if (sync_cmd)
							updateLabel_P(device_read_attribute_label, device_query_failed_str);
						else
							updateLabel_P(device_read_attribute_label, device_async_query_str);
					}
			} break;

			case GUI_CB_CONFIG_REPORT_FLAG : {	//configure reporting
					
				uint16_t min_interval = ESPUI.getControl(device_config_min_number)->value.toInt();
				uint16_t max_interval = ESPUI.getControl(device_config_max_number)->value.toInt();
				uint16_t delta = ESPUI.getControl(device_config_delta_number)->value.toInt();

				bool result = zbGateway.setClusterReporting(&device, cluster_id, attribute_id, attribute_type, min_interval, max_interval, delta, sync_cmd);
				if (result) {
					if (*zbGateway.getConfigReportStatusLastResult() == ESP_ZB_ZCL_STATUS_SUCCESS) {

						sprintf_P(general_purpose_gui_buffer, PSTR("Configure reporting successful! <br>"
										"Attribute id is 0x%X, data type is %s(0x%X)<br>"
										"Min interval is %u(0x%x), max interval is %u(0x%X)<br>"
										"Delta is %u(0x%X)"), 
          	        attribute_id,
										getZigbeeDataTypeName(attribute_type),
										attribute_type,
										min_interval,
										min_interval,
										max_interval,
										max_interval,
										delta,
										delta);
						updateLabel_P(device_read_attribute_label, general_purpose_gui_buffer);
					} else {
						sprintf_P(general_purpose_gui_buffer, 
										PSTR("Configure reporting failed! <br>"
										"Status = %s(0x%02X)<br>"
										"Attribute id = 0x%04X<br>"),
										esp_zb_zcl_status_to_name(*zbGateway.getConfigReportStatusLastResult()),
										*zbGateway.getConfigReportStatusLastResult(),
										*zbGateway.getConfigReportStatusLastResult(),
      		          attribute_id);
						updateLabel_P(device_read_attribute_label, general_purpose_gui_buffer);
					}
				} else {
					if (sync_cmd)
						updateLabel_P(device_read_attribute_label, device_query_failed_str);
					else
						updateLabel_P(device_read_attribute_label, device_async_query_str);
				}
			} break;

			case GUI_CB_WRITE_ATTR_FLAG : {	//write attribute
					
				void *value = nullptr;
				
				uint8_t write_mask;
				uint16_t write_mask_16;
				uint32_t write_mask_32;

				uint8_t *write_attribute_payload = nullptr;

				uint16_t attribute_size = ESPUI.getControl(device_attribute_size_number)->value.toInt();
				
				if (attribute_size > 255) {
					updateLabel_P(device_read_attribute_label, device_query_attr_size_error_str);
					return;
				}

				const char* attribute_value = ESPUI.getControl(device_attribute_value_text)->value.c_str();

				log_i("attribute_value = %s, length = %u", attribute_value, strlen(attribute_value));

      	if ((attribute_type >= ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING) && (attribute_type <= ESP_ZB_ZCL_ATTR_TYPE_BAG))  {
        
					if ((strlen(attribute_value) / 2) != attribute_size) {
						updateLabel_P(device_read_attribute_label, device_query_attr_size_mismatch_str);
						return;
					}

        	char byte_str[3];
        	byte_str[2] = '\0';
					write_attribute_payload = (uint8_t*)malloc(attribute_size); //2 by

					if (write_attribute_payload == nullptr) {
						updateLabel_P(device_read_attribute_label, PSTR("Error allocating attribute write buffer!"));
						return;
					}

					memset(write_attribute_payload, 0, attribute_size);
					
        	for (int i = 0; i < attribute_size; i++) {
          memcpy(byte_str, attribute_value + (i * 2), 2);
          *(write_attribute_payload + i) = strtoul(byte_str, nullptr, 16); //here hex base must be explicit
					log_i("write_attribute_payload[%u] = %u(0x%02X)", i, *(write_attribute_payload + i), *(write_attribute_payload + i));
        }
        value = write_attribute_payload;
      	} else {
        	switch (attribute_size) {
          	case 1: {
            	write_mask = strtoul(attribute_value, nullptr, 0);
            	value = &write_mask; 
          	} break;
          	case 2: {
            	write_mask_16 = strtoul(attribute_value, nullptr, 0);
            	value = &write_mask_16; 
          	} break;
          	case 4: {
            	write_mask_32 = strtoul(attribute_value, nullptr, 0);
            	value = &write_mask_32; 
          	} break;
        	}
      	}
      	if (value) {
					
					bool result = zbGateway.sendAttributeWrite(&device, cluster_id, attribute_id, (esp_zb_zcl_attr_type_t)attribute_type, 
																										 attribute_size, value, sync_cmd, manuf_specific, manuf_code);
					
					if (result) {
						if (*zbGateway.getWriteAttrStatusLastResult() == ESP_ZB_ZCL_STATUS_SUCCESS) {

						
							sprintf_P(general_purpose_gui_buffer, PSTR("Write attribute successful! <br>"
											"Attribute id   = 0x%04X<br>"
											"Attribute type = %s(0x%02X)<br>"
											"Attribute size = %u(0x%04X)"),
      		            attribute_id,
											getZigbeeDataTypeName(attribute_type),
											attribute_type,
											attribute_size,
											attribute_size);
							updateLabel_P(device_read_attribute_label, general_purpose_gui_buffer);
						} else {
							sprintf_P(general_purpose_gui_buffer, 
											PSTR("Write attribute failed! <br>"
											"Status = %s(0x%02X)<br>"
											"Attribute id = 0x%04X<br>"
											"Attribute type = %s(0x%04X)<br>"
											"Attribute size = %u(0x%04X)"),
											esp_zb_zcl_status_to_name(*zbGateway.getWriteAttrStatusLastResult()),
											*zbGateway.getWriteAttrStatusLastResult(),
											*zbGateway.getWriteAttrStatusLastResult(),
      		            *zbGateway.getWriteAttrAttributeIdLastResult(),
											getZigbeeDataTypeName(attribute_type),
											attribute_type,
											attribute_size,
											attribute_size);
							updateLabel_P(device_read_attribute_label, general_purpose_gui_buffer);
						}
					} else {
						if (sync_cmd)
							updateLabel_P(device_read_attribute_label, device_query_failed_str);
						else
							updateLabel_P(device_read_attribute_label, device_async_query_str);
					}
				}
				if (write_attribute_payload) 
					free(write_attribute_payload);
			} break;

			case GUI_CB_CUSTOM_CMD_FLAG : {	//custom command
					
				uint8_t *custom_cmd_payload = nullptr;

				uint16_t payload_size = ESPUI.getControl(device_attribute_size_number)->value.toInt();
				
				if (payload_size > 255) {
					updateLabel_P(device_read_attribute_label, device_query_attr_size_error_str);
					return;
				}

				const char* payload_value = ESPUI.getControl(device_attribute_value_text)->value.c_str();

				log_i("payload_value = %s, length = %u", payload_value, strlen(payload_value));

      	//if ((attribute_type >= ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING) && (attribute_type <= ESP_ZB_ZCL_ATTR_TYPE_BAG))  {
				if (true) {	
					if (payload_size > 0) {
						
						if ((strlen(payload_value) / 2) != payload_size) {
							updateLabel_P(device_read_attribute_label, device_query_attr_size_mismatch_str);
							return;
						}

        		char byte_str[3];
        		byte_str[2] = '\0';
						custom_cmd_payload = (uint8_t*)malloc(payload_size); //2 by

						if (custom_cmd_payload == nullptr) {
							updateLabel_P(device_read_attribute_label, PSTR("Error allocating custom command payload buffer!"));
							return;
						}

						memset(custom_cmd_payload, 0, payload_size);
					
        		for (int i = 0; i < payload_size; i++) {
          		memcpy(byte_str, payload_value + (i * 2), 2);
          		*(custom_cmd_payload + i) = strtoul(byte_str, nullptr, 16); //here hex base must be explicit
							log_i("custom_cmd_payload[%u] = %u(0x%02X)", i, *(custom_cmd_payload + i), *(custom_cmd_payload + i));
        		}
					} else
						log_i("Payload size equals 0"); 
      	} else {
        	updateLabel_P(device_read_attribute_label, PSTR("Invalid data type for custom command!"));
					return;
      	}
      	if (true) {
					
					if (cluster_id == 0xEF00) {

						custom_cmd_tsn = (((uint16_t)(*custom_cmd_payload)) << 16) + (*(custom_cmd_payload + 1));
						Tuya_custom_cmd_dp = *(custom_cmd_payload + 2);
						current_Tuya_payload_label = device_Tuya_payload_label;
					}

					bool result = zbGateway.sendCustomClusterCmd(&device, cluster_id, attribute_id, 
																											 (esp_zb_zcl_attr_type_t)attribute_type, 
																											 payload_size, custom_cmd_payload, sync_cmd,
																											 ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV, 1, 
																											 manuf_specific, manuf_code);
					
					if (result) {
						if (*zbGateway.getCustomCmdStatusLastResult() == ESP_ZB_ZCL_STATUS_SUCCESS) {

						
							sprintf_P(general_purpose_gui_buffer, PSTR("Custom command successful! <br>"
											"Command id   = 0x%04X<br>"
											"Data type = %s(0x%02X)<br>"
											"Payload size = %u(0x%04X)"),
      		            attribute_id,
											getZigbeeDataTypeName(attribute_type),
											attribute_type,
											payload_size,
											payload_size);
							updateLabel_P(device_read_attribute_label, general_purpose_gui_buffer);
						} else {
							sprintf_P(general_purpose_gui_buffer, 
											PSTR("Custom command failed! <br>"
											"Status = %s(0x%02X)"
											"Command id = 0x%04X<br>"
											"Data type = %s(0x%04X)<br>"
											"Payload size = %u(0x#04X)"),
											esp_zb_zcl_status_to_name(*zbGateway.getCustomCmdStatusLastResult()),
											*zbGateway.getCustomCmdStatusLastResult(),
											*zbGateway.getCustomCmdStatusLastResult(),
      		            *zbGateway.getCustomCmdRespToCmdLastResult(),
											getZigbeeDataTypeName(attribute_type),
											attribute_type,
											payload_size,
											payload_size);
							updateLabel_P(device_read_attribute_label, general_purpose_gui_buffer);
						}
					} else {
						if (sync_cmd)
							updateLabel_P(device_read_attribute_label, device_query_failed_str);
						else
							updateLabel_P(device_read_attribute_label, device_async_query_str);
					}
				}
				if (custom_cmd_payload) 
					free(custom_cmd_payload);
			} break;
		}
	}
}

void removeDeviceCallback(Control *sender, int type, void *param) {

	if ((type == B_UP) && (ESPUI.getControl(device_selector)->value.toInt() >= 0)) {

		uint8_t device_slot = ESPUI.getControl(device_selector)->value.toInt();

		//char status_line[128];
		bool restart_required = false;

		switch ((uint32_t)param) {

			case GUI_CB_SINGLE_FLAG : {		
				
				z2s_zb_devices_table[device_slot].record_id = 0;
				if (Z2S_saveZBDevicesTable()) {
					sprintf_P(general_purpose_gui_buffer, PSTR("Device #%02u removed! Restarting..."), device_slot);
					restart_required = true;
				} else
					sprintf_P(general_purpose_gui_buffer, PSTR("Device #%02u removal failed! Error saving ZB devices table!"), device_slot);
			} break;

			case GUI_CB_WITH_CHANNELS_FLAG : {	

				if (Z2S_removeZBDeviceWithAllChannels(device_slot)) {

					sprintf_P(general_purpose_gui_buffer, PSTR("Device #%02u and it's all channels removed! Restarting..."), device_slot);
					restart_required = true;
				} else
					sprintf_P(general_purpose_gui_buffer, PSTR("Device #%02u or some of it's channels removal failed! Error saving one of devices table!"), device_slot);
			} break;

			case GUI_CB_ALL_FLAG: {

				if (Z2S_clearZBDevicesTable()) {
					sprintf_P(general_purpose_gui_buffer, PSTR("All devices removed! Restarting..."));
					restart_required = true;
				} else
					sprintf_P(general_purpose_gui_buffer, PSTR("Devices removal failed! Error saving ZB devices table!"));
			} break;
		}
    updateLabel_P(device_status_label, general_purpose_gui_buffer);
    if (restart_required) 
			SuplaDevice.scheduleSoftRestart(1000);
	}
}

void removeChannelCallback(Control *sender, int type) {

	if ((type == B_UP) && (ESPUI.getControl(channel_selector)->value.toInt() >= 0)) {

		uint8_t channel_slot = ESPUI.getControl(channel_selector)->value.toInt();

		z2s_channels_table[channel_slot].valid_record = false;
      
		if (Z2S_saveChannelsTable()) {
			Z2S_removeChannelActions(z2s_channels_table[channel_slot].Supla_channel, false);
			sprintf_P(general_purpose_gui_buffer, PSTR("Channel # %02u with all actions removed. Restarting..."), channel_slot);
      updateLabel_P(channel_status_label, general_purpose_gui_buffer);
      SuplaDevice.scheduleSoftRestart(1000);
		}
	}
}

void removeAllChannelsCallback(Control *sender, int type) {

	if (type == B_UP) {
		
		if (Z2S_clearChannelsTable()) {
			Z2S_removeChannelActions(0, true);
      updateLabel_P(channel_status_label, PSTR("All channels and actions removed!. Restarting..."));
      SuplaDevice.scheduleSoftRestart(1000);
		}
	}
}

void pairingSwitcherCallback(Control *sender, int type, void *param){


	switch ((uint32_t)param) {

		case GUI_CB_PAIRING_FLAG : {		//open network

			if (type == S_ACTIVE) 
				Zigbee.openNetwork(180); 
			else {
    if (Zigbee.isNetworkOpen())
			Zigbee.openNetwork(0);//ESPUI.updateNumber(pairing_mode_switcher, 1);
			}
		} break;
	}
}

void generalZigbeeCallback(Control *sender, int type, void *param){

	if (type == B_UP) {

		switch ((uint32_t)param) {

			case GUI_CB_PAIRING_FLAG : {		//open network
				Zigbee.openNetwork(180); 
			} break;

			case GUI_CB_FACTORY_FLAG : {	//factory reset
				if (ESPUI.getControl(factory_reset_switcher)->value.toInt() > 0)
					Zigbee.factoryReset(); 
			} break;

			case GUI_CB_GET_TX_FLAG : { //read TX power
				int8_t zb_tx_power;
    		esp_zb_get_tx_power(&zb_tx_power);
				log_i("get tx power %d", zb_tx_power);
				working_str = zb_tx_power;
				ESPUI.updateText(zigbee_tx_power_text, working_str);
			} break;

			case GUI_CB_SET_TX_FLAG: {//update TX power
				if (isNumber(ESPUI.getControl(zigbee_tx_power_text)->value) &&
						(ESPUI.getControl(zigbee_tx_power_text)->value.toInt() >= -24) &&
						(ESPUI.getControl(zigbee_tx_power_text)->value.toInt() <= 20))
							esp_zb_set_tx_power(ESPUI.getControl(zigbee_tx_power_text)->value.toInt());
				else
					ESPUI.updateText(zigbee_tx_power_text, zigbee_tx_power_text_str);

			} break;

			case GUI_CB_GET_PC_FLAG : { //read primary channel
				uint32_t zb_primary_channel = esp_zb_get_primary_network_channel_set();
    		for (uint8_t i = 11; i <= 26; i++) {
      		if (zb_primary_channel & (1 << i)) {

						working_str = i;
						ESPUI.updateText(zigbee_primary_channel_text, working_str);
					}
    		}		
			} break;

			case GUI_CB_SET_PC_FLAG : { //write primary channel
					
					if (isNumber(ESPUI.getControl(zigbee_primary_channel_text)->value) &&
						(ESPUI.getControl(zigbee_primary_channel_text)->value.toInt() >= 11) &&
						(ESPUI.getControl(zigbee_primary_channel_text)->value.toInt() <= 26)) {
							uint32_t zb_primary_channel = ESPUI.getControl(zigbee_primary_channel_text)->value.toInt();

							if (Supla::Storage::ConfigInstance()->setUInt32(Z2S_ZIGBEE_PRIMARY_CHANNEL, (1 << zb_primary_channel))) {
        				ESPUI.updateText(zigbee_primary_channel_text, "New Zigbee primary channel write success! Restarting...");
   				     	Supla::Storage::ConfigInstance()->commit();
        				SuplaDevice.scheduleSoftRestart(1000);
							}
						}
				else
					ESPUI.updateText(zigbee_primary_channel_text, zigbee_primary_channel_text_str);
			} break;
		}
	}
}

void switchCallback(Control *sender, int type) {

	if (sender->value.toInt() == 1)
		updateLabel_P(factory_reset_label, factory_reset_enabled_str);
	else
		updateLabel_P(factory_reset_label, factory_reset_disabled_str);
}

void editDeviceCallback(Control *sender, int type, void *param) {

	if ((type == B_UP) && (ESPUI.getControl(device_selector)->value.toInt() >= 0)) {

		uint8_t device_slot = ESPUI.getControl(device_selector)->value.toInt();

		switch ((uint32_t)param) {

			case GUI_CB_UPDATE_DEVICE_NAME_FLAG : {

				strncpy(z2s_zb_devices_table[device_slot].device_local_name, ESPUI.getControl(device_name_text)->value.c_str(), 32);
				z2s_zb_devices_table[device_slot].device_local_name[32] = '\0';
				if (Z2S_saveZBDevicesTable()) {
					log_i("%d, %s", z2s_zb_devices_table[device_slot].device_gui_id, 
								z2s_zb_devices_table[device_slot].device_local_name);
					ESPUI.updateControlLabel(z2s_zb_devices_table[device_slot].device_gui_id, 
																	 z2s_zb_devices_table[device_slot].device_local_name);
					ESPUI.jsonReload();
				}
			} break;
		}
	}
}



void editChannelCallback(Control *sender, int type, void *param) {

	if ((type == B_UP) && (ESPUI.getControl(channel_selector)->value.toInt() >= 0)) {

		uint8_t channel_slot = ESPUI.getControl(channel_selector)->value.toInt();

		switch ((uint32_t)param) {

			case GUI_CB_UPDATE_CHANNEL_NAME_FLAG : {	

				strncpy(z2s_channels_table[channel_slot].Supla_channel_name, 
								ESPUI.getControl(channel_name_text)->value.c_str(), 32);
				z2s_channels_table[channel_slot].Supla_channel_name[32] = '\0';

				if (Z2S_saveChannelsTable()) {

					log_i("%d, %s", z2s_channels_table[channel_slot].gui_control_data.gui_control_id, 
								z2s_channels_table[channel_slot].Supla_channel_name);
					ESPUI.updateControlLabel(z2s_channels_table[channel_slot].gui_control_data.gui_control_id, 
																	 z2s_channels_table[channel_slot].Supla_channel_name);
					ESPUI.jsonReload();
				}
			} break;

			case GUI_CB_UPDATE_KEEPALIVE_FLAG : {	

				updateTimeout(channel_slot, 0, 1, ESPUI.getControl(keepalive_number)->value.toInt());
			} break;


			case GUI_CB_UPDATE_TIMEOUT_FLAG : {		
				updateTimeout(channel_slot, 0, 2, ESPUI.getControl(timeout_number)->value.toInt());
			} break;


			case GUI_CB_UPDATE_REFRESH_FLAG : {		

				updateTimeout(channel_slot,0, 4, ESPUI.getControl(refresh_number)->value.toInt());
			} break;	
		}
	}
}

void editChannelFlagsCallback(Control *sender, int type, void *param) {

	if (ESPUI.getControl(channel_selector)->value.toInt() >= 0) {

		uint8_t channel_slot = ESPUI.getControl(channel_selector)->value.toInt();

		switch ((uint32_t)param) {

				case GUI_CB_DISABLE_CHANNEL_NOTIFICATIONS_FLAG: {

						if (type == S_ACTIVE)
							Z2S_setChannelFlags(channel_slot, USER_DATA_FLAG_DISABLE_NOTIFICATIONS);
						else
							Z2S_clearChannelFlags(channel_slot, USER_DATA_FLAG_DISABLE_NOTIFICATIONS);

				} break;

				case GUI_CB_TRV_AUTO_TO_SCHEDULE_FLAG: {

					if (type == S_ACTIVE)
							Z2S_setChannelFlags(channel_slot, USER_DATA_FLAG_TRV_AUTO_TO_SCHEDULE);
						else
							Z2S_clearChannelFlags(channel_slot, USER_DATA_FLAG_TRV_AUTO_TO_SCHEDULE);

				} break;

				case GUI_CB_SET_SORWNS_ON_START_FLAG: {

					if (type == S_ACTIVE)
							Z2S_setChannelFlags(channel_slot, USER_DATA_FLAG_SET_SORWNS_ON_START);
						else
							Z2S_clearChannelFlags(channel_slot, USER_DATA_FLAG_SET_SORWNS_ON_START);
				} break;	
		}
	}
}

void batteryCallback(Control *sender, int type, void *param) {

	if ((type == B_UP) && (ESPUI.getControl(device_selector)->value.toInt() >= 0)) {

		uint8_t channel_slot = ESPUI.getControl(device_selector)->value.toInt();

		switch ((uint32_t)param) {

			case GUI_CB_BATTERY_VOLTAGE_MIN_FLAG : {		
				z2s_zb_devices_table[channel_slot].battery_voltage_min = ESPUI.getControl(battery_voltage_min_number)->value.toInt();
				Z2S_saveZBDevicesTable();
			} break;

			case GUI_CB_BATTERY_VOLTAGE_MAX_FLAG : {		
				z2s_zb_devices_table[channel_slot].battery_voltage_max = ESPUI.getControl(battery_voltage_max_number)->value.toInt();
				Z2S_saveZBDevicesTable();
			} break;	
		}
	}
}

void batterySwitcherCallback(Control *sender, int type, void *param) {

	if (ESPUI.getControl(device_selector)->value.toInt() >= 0) {

		uint8_t device_slot = ESPUI.getControl(device_selector)->value.toInt();

		switch ((uint32_t)param) {

			case GUI_CB_DISABLE_VOLTAGE_MSG_FLAG : {		
				if (type == S_ACTIVE)
					Z2S_setZBDeviceFlags(device_slot, ZBD_USER_DATA_FLAG_DISABLE_BATTERY_VOLTAGE_MSG);
				else
					Z2S_clearZBDeviceFlags(device_slot, ZBD_USER_DATA_FLAG_DISABLE_BATTERY_VOLTAGE_MSG);
			} break;

			case GUI_CB_DISABLE_PERCENTAGE_MSG_FLAG : {	
				if (type == S_ACTIVE)
					Z2S_setZBDeviceFlags(device_slot, ZBD_USER_DATA_FLAG_DISABLE_BATTERY_PERCENTAGE_MSG);
				else
					Z2S_clearZBDeviceFlags(device_slot, ZBD_USER_DATA_FLAG_DISABLE_BATTERY_PERCENTAGE_MSG);
			} break;
		}
	}
}

void datatypeCallback(Control *sender, int type) {

	int8_t device_attribute_type_selector_value =  ESPUI.getControl(device_attribute_type_selector)->value.toInt();

	if (device_attribute_type_selector_value >= 0)
		ESPUI.updateNumber(device_attribute_size_number, 
											 zigbee_datatypes[device_attribute_type_selector_value].zigbee_datatype_size);
}

void TuyaDatapointTypeSelectorCallback(Control *sender, int type) {

	int8_t Tuya_datapoint_type_selector_value = ESPUI.getControl(Tuya_datapoint_type_selector)->value.toInt();

	if (Tuya_datapoint_type_selector_value >= 0)
		ESPUI.updateNumber(Tuya_datapoint_length_number, 
											 Tuya_datapoint_types[Tuya_datapoint_type_selector_value].Tuya_datapoint_type_length);
}

void clearAttributeIdSelect() {

	if (attribute_id_selector_options_count > 0) {
		for (uint8_t i = 0; i < attribute_id_selector_options_count; i++) {

			ESPUI.removeControl(attribute_id_selector_options[i], false);		
					//ESPUI.jsonReload();
		}
		attribute_id_selector_options_count = 0;
	}

	ESPUI.updateControlValue(device_attribute_id_selector, minus_one_str);
	ESPUI.updateControlValue(device_attribute_type_selector, minus_one_str);
}

void clearAttributeValueSelect() {

	log_i("attribute_value_selector_options_count = %u", attribute_value_selector_options_count);
	if (attribute_value_selector_options_count > 0) {
		for (uint8_t i = 0; i < attribute_value_selector_options_count; i++) {

			ESPUI.removeControl(attribute_value_selector_options[i], false);		
			//ESPUI.jsonReload();
		}
		attribute_value_selector_options_count = 0;
	}
	
	ESPUI.updateControlValue(device_attribute_value_selector, minus_one_str);
	ESPUI.updateNumber(device_attribute_value_text, 0);
}

void attributeCallback (Control *sender, int type) {

	if (ESPUI.getControl(device_attribute_id_selector)->value.toInt() >= 0) {

		uint16_t zigbee_attributes_idx = ESPUI.getControl(device_attribute_id_selector)->value.toInt();

		uint32_t zigbee_datatypes_count = sizeof(zigbee_datatypes)/sizeof(zigbee_datatype_t);

		for (uint8_t datatypes_counter = 0; datatypes_counter < zigbee_datatypes_count; datatypes_counter++) {
  
			if (zigbee_datatypes[datatypes_counter].zigbee_datatype_id == zigbee_attributes[zigbee_attributes_idx].zigbee_attribute_datatype_id) {

				working_str = datatypes_counter;
				ESPUI.updateSelect(device_attribute_type_selector, working_str);
				ESPUI.updateNumber(device_attribute_size_number, zigbee_datatypes[datatypes_counter].zigbee_datatype_size);
				break;
			}
		}
		ESPUI.updateNumber(device_attribute_id_text, zigbee_attributes[zigbee_attributes_idx].zigbee_attribute_id);

		clearAttributeValueSelect();

		//attribute_value_selector_options_count = 0;
		uint32_t zigbee_attribute_values_count = sizeof(zigbee_attribute_values)/sizeof(zigbee_attribute_value_t);
		uint16_t cluster_id = ESPUI.getControl(device_cluster_selector)->value.toInt();

		//log_i("cluster_id = %u, zigbee_attributes_count = %u", cluster_id, zigbee_attributes_count);
		for (uint32_t i = 0; i < zigbee_attribute_values_count; i++) {
			//log_i("i = %u, cluster_id = %u, cluster_id = %u", i, zigbee_attributes[i].zigbee_attribute_cluster_id, cluster_id);
			if ((zigbee_attribute_values[i].zigbee_cluster_id == cluster_id) && 
					(zigbee_attribute_values[i].zigbee_attribute_id == zigbee_attributes[zigbee_attributes_idx].zigbee_attribute_id)) {

				working_str = i;
				attribute_value_selector_options[attribute_value_selector_options_count] = 
					ESPUI.addControl(Control::Type::Option, zigbee_attribute_values[i].zigbee_attribute_value_name, working_str, 
													Control::Color::None, device_attribute_value_selector);
				attribute_value_selector_options_count++;
			}
		}
	}
}
	
void clusterCallback (Control *sender, int type) {

	if (sender->value.toInt() >= 0) {
		
		clearAttributeIdSelect();
		clearAttributeValueSelect();

		uint16_t cluster_id = sender->value.toInt();
		//attribute_id_selector_options_count = 0;

		ESPUI.updateNumber(device_attribute_size_number, 0);
		ESPUI.updateNumber(device_attribute_id_text, 0);
		
		uint32_t zigbee_attributes_count = sizeof(zigbee_attributes)/sizeof(zigbee_attribute_t);
		
		//log_i("cluster_id = %u, zigbee_attributes_count = %u", cluster_id, zigbee_attributes_count);
		for (uint32_t i = 0; i < zigbee_attributes_count; i++) {
			//log_i("i = %u, cluster_id = %u, cluster_id = %u", i, zigbee_attributes[i].zigbee_attribute_cluster_id, cluster_id);
			if (zigbee_attributes[i].zigbee_attribute_cluster_id == cluster_id) {

				working_str = i;
				attribute_id_selector_options[attribute_id_selector_options_count] = 
					ESPUI.addControl(Control::Type::Option, zigbee_attributes[i].zigbee_attribute_name, working_str, 
													 Control::Color::None, device_attribute_id_selector);

				attribute_id_selector_options_count++;
			}
		}
	}
}

void valueCallback(Control *sender, int type) {
	
	int8_t sender_value = sender->value.toInt(); //max 128 attribute values

	if (sender_value >= 0) {
	
		ESPUI.updateNumber(device_attribute_value_text, zigbee_attribute_values[sender_value].zigbee_attribute_value);
	}
}

void advancedDeviceSelectorCallback(Control *sender, int type) {

	log_i("value = %s(%u)", sender->value.c_str(), sender->value.toInt());
	if ((!isNumber(sender->value)) || (sender->value.toInt() < 0) || (sender->value.toInt() >= Z2S_ZB_DEVICES_MAX_NUMBER)) {

		updateLabel_P(advanced_device_info_label, three_dots_str);

		if (isSonoffValvePresent)
			enableSonoffValveGUI(false);
		if (isTuyaGasDetectorPresent)
			enableTuyaGasDetectorGUI(false);
		if (isMoesAlarmPresent)
			enableMoesAlarmGUI(false);
		return;
	}
	
	uint8_t device_slot = sender->value.toInt();

	if (z2s_zb_devices_table[device_slot].desc_id == Z2S_DEVICE_DESC_SONOFF_SMART_VALVE)
		enableSonoffValveGUI(true);
	else {
		if (isSonoffValvePresent)
			enableSonoffValveGUI(false);
	}
	if (z2s_zb_devices_table[device_slot].desc_id == Z2S_DEVICE_DESC_TUYA_GAS_DETECTOR)
		enableTuyaGasDetectorGUI(true);
	else {
		if (isSonoffValvePresent)
			enableTuyaGasDetectorGUI(false);
	}
	if (z2s_zb_devices_table[device_slot].desc_id == Z2S_DEVICE_DESC_MOES_ALARM)
		enableMoesAlarmGUI(true);
	else {
		if (isMoesAlarmPresent)
			enableMoesAlarmGUI(false);
	}

	sprintf_P(general_purpose_gui_buffer,PSTR("<b><i>Manufacturer name</i></b> %s "
					"<b>| <i>model ID</b></i> %s"), 
					Z2S_getZBDeviceManufacturerName(device_slot),
					Z2S_getZBDeviceModelName(device_slot));

	updateLabel_P(advanced_device_info_label, general_purpose_gui_buffer);
}

void gatewayCallback(Control *sender, int type, void *param) {

	switch ((uint32_t)param) {
			
		case GUI_CB_ENABLE_GUI_FLAG: { 

			if (Supla::Storage::ConfigInstance()->setUInt8(Z2S_ENABLE_GUI_ON_START, ESPUI.getControl(enable_gui_switcher)->value.toInt()))
      	Supla::Storage::ConfigInstance()->commit();
		} break;

		case GUI_CB_FORCE_CONFIG_FLAG: { 

			if (Supla::Storage::ConfigInstance()->setUInt8(Z2S_FORCE_CONFIG_ON_START, ESPUI.getControl(force_config_switcher)->value.toInt()))
      	Supla::Storage::ConfigInstance()->commit();
		} break;

		case GUI_CB_GUI_DELAY_FLAG: {

			if (Supla::Storage::ConfigInstance()->setUInt32(Z2S_GUI_ON_START_DELAY, ESPUI.getControl(gui_start_delay_number)->value.toInt()))
      	Supla::Storage::ConfigInstance()->commit();
		} break;

		case GUI_CB_GUI_RESTART_FLAG: {

			SuplaDevice.softRestart();
		} break;
	}
}

void TuyaCustomCmdCallback(Control *sender, int type, void *param) {

	if ((type == B_UP) && (ESPUI.getControl(Tuya_device_selector)->value.toInt() >= 0)) {

		uint8_t Tuya_device_selector_value = ESPUI.getControl(Tuya_device_selector)->value.toInt();

		zbg_device_params_t device;
		log_i("device_selector value %u, param id %d", Tuya_device_selector_value, (uint32_t)param);
    
		device.endpoint = 1;
    device.cluster_id = TUYA_PRIVATE_CLUSTER_EF00;

		uint8_t cmd_id;

		uint16_t tsn_id = random(0x0000, 0xFFFF);;

    memcpy(&device.ieee_addr, z2s_zb_devices_table[Tuya_device_selector_value].ieee_addr,8);
    device.short_addr = z2s_zb_devices_table[Tuya_device_selector_value].short_addr;

		switch ((uint32_t)param) {
			
			case GUI_CB_SEND_TUYA_REQUEST_FLAG: {

				uint8_t cmd_dp_id = ESPUI.getControl(Tuya_datapoint_id_number)->value.toInt();

				if (cmd_dp_id == 0) {

					updateLabel_P(Tuya_device_cmd_result_label, PSTR("Tuya datapoint id must be in range [1...255]!"));
					return;
				}

				int8_t cmd_dp_type = ESPUI.getControl(Tuya_datapoint_type_selector)->value.toInt();

				if (cmd_dp_type < 0) {

					updateLabel_P(Tuya_device_cmd_result_label, PSTR("Tuya datapoint type not selected!"));
					return;
				}

				uint8_t cmd_dp_length = ESPUI.getControl(Tuya_datapoint_length_number)->value.toInt();

				if (cmd_dp_type > 58) {

					updateLabel_P(Tuya_device_cmd_result_label, PSTR("Tuya datapoint length must be in range [1..58]!"));
					return;
				}

				cmd_id = 0x00;
				
				uint8_t *custom_cmd_payload = nullptr;

				uint8_t payload_size = 6 /*tsn(2)+id(1)+type(1)+size(2)*/ + cmd_dp_length;
				
				if (payload_size > 64) {
					updateLabel_P(Tuya_device_cmd_result_label, Tuya_device_payload_size_error_str);
					return;
				}

				const char* payload_value = ESPUI.getControl(Tuya_datapoint_value_text)->value.c_str();

				log_i("payload_value = %s, length = %u", payload_value, strlen(payload_value));

      	if (cmd_dp_length > 0) {
						
					if ((strlen(payload_value) / 2) != cmd_dp_length) {
						
						updateLabel_P(Tuya_device_cmd_result_label, Tuya_device_payload_size_mismatch_str);
						return;
					}

        	char byte_str[3];
        	byte_str[2] = '\0';
					
					custom_cmd_payload = (uint8_t*)malloc(payload_size);

					if (custom_cmd_payload == nullptr) {
						
						updateLabel_P(Tuya_device_cmd_result_label, PSTR("Error allocating custom command payload buffer!"));
						return;
					}

					memset(custom_cmd_payload, 0, payload_size);
					
					*(custom_cmd_payload + 0) = (tsn_id & 0xFF00);
      		*(custom_cmd_payload + 1) = (tsn_id & 0x00FF);

					*(custom_cmd_payload + 2) = cmd_dp_id;
					*(custom_cmd_payload + 3) = Tuya_datapoint_types[cmd_dp_type].Tuya_datapoint_type_id;
					*(custom_cmd_payload + 5) = cmd_dp_length;

        	for (int i = 0; i < cmd_dp_length; i++) {
          		
						memcpy(byte_str, payload_value + (i * 2), 2);
          	*(custom_cmd_payload + 6 + i) = strtoul(byte_str, nullptr, 16); //here hex base must be explicit
						log_i("custom_cmd_payload[%u] = %u(0x%02X)", 6 + i, *(custom_cmd_payload + 6 + i), *(custom_cmd_payload + 6 + i));
        	}
				} else
					log_i("Tuya datapoint payload size equals 0"); 
      	
				custom_cmd_tsn = (((uint16_t)(*custom_cmd_payload)) << 16) + (*(custom_cmd_payload + 1));
				
				Tuya_custom_cmd_dp = cmd_dp_id;
				current_Tuya_payload_label = Tuya_device_payload_label;
					

					bool result = zbGateway.sendCustomClusterCmd(&device, device.cluster_id, cmd_id, ESP_ZB_ZCL_ATTR_TYPE_SET, 
																											 payload_size, custom_cmd_payload, true);
					
					if (result) {
						if (*zbGateway.getCustomCmdStatusLastResult() == ESP_ZB_ZCL_STATUS_SUCCESS) {
						
							sprintf_P(general_purpose_gui_buffer, PSTR("Tuya custom cluster data request sent successfully! <br>"
											"Datapoint id   = %u(0x%02X)<br>"
											"Datapoint type = %s(0x%02X)<br>"
											"Payload size 	= %u(0x%02X)"),
      		            cmd_dp_id,
											cmd_dp_id,
											Tuya_datapoint_types[cmd_dp_type].Tuya_datapoint_type_name,
											Tuya_datapoint_types[cmd_dp_type].Tuya_datapoint_type_id,
											payload_size,
											payload_size);
							updateLabel_P(Tuya_device_cmd_result_label, general_purpose_gui_buffer);
						} else {
							sprintf_P(general_purpose_gui_buffer, 
											PSTR("Tuya custom cluster data request failed! <br>"
											"Status         = %u(0x%02X)<br>"
											"Datapoint id   = %u(0x%02X)<br>"
											"Datapoint type = %s(0x%02X)<br>"
											"Payload size 	= %u(0x%02X)"),
											*zbGateway.getCustomCmdStatusLastResult(),
											*zbGateway.getCustomCmdStatusLastResult(),
      		            cmd_dp_id,
											cmd_dp_id,
											Tuya_datapoint_types[cmd_dp_type].Tuya_datapoint_type_name,
											Tuya_datapoint_types[cmd_dp_type].Tuya_datapoint_type_id,
											payload_size,
											payload_size);
							updateLabel_P(Tuya_device_cmd_result_label, general_purpose_gui_buffer);
						}
					} else {

							updateLabel_P(Tuya_device_cmd_result_label, device_query_failed_str);
					}
				if (custom_cmd_payload) 
					free(custom_cmd_payload);
			} break;

			case GUI_CB_SEND_TUYA_QUERY_FLAG: {
			
			} break; 
		}
	}
}

void actionsTableCallback(Control *sender, int type, void *param) {

	if (type == B_UP) {

		z2s_channel_action_t new_action = {};
		
		working_str = "No actions";

		switch ((uint32_t)param) {

			case GUI_CB_ACTION_FIRST_FLAG: {

				int16_t first_action = Z2S_findNextActionPosition(0);
  			if (first_action >= 0) {

					current_action_id = first_action;
					current_action_counter = Z2S_getActionCounter(first_action);

      		Z2S_loadAction(first_action, new_action);
					sprintfAction(new_action);
					ESPUI.updateLabel(actions_table_label, working_str);
					updateActionDetails(new_action);
				}
			} break;

			case GUI_CB_ACTION_NEXT_FLAG: {

				if (current_action_id < 0)
					return;

				int16_t next_action = Z2S_findNextActionPosition(current_action_id + 1);
  			if (next_action >= 0) {

					current_action_id = next_action;
					current_action_counter = Z2S_getActionCounter(next_action);

      		Z2S_loadAction(next_action, new_action);
					sprintfAction(new_action);
					ESPUI.updateLabel(actions_table_label, working_str);
					updateActionDetails(new_action);
				}				
			} break;

			case GUI_CB_ACTION_PREV_FLAG: {

				if (current_action_id <= 0)
					return;

				int16_t prev_action = Z2S_findPrevActionPosition(current_action_id - 1);
  			if (prev_action >= 0) {

					current_action_id = prev_action;
					current_action_counter = Z2S_getActionCounter(prev_action);
				
      		Z2S_loadAction(prev_action, new_action);
					sprintfAction(new_action);
					ESPUI.updateLabel(actions_table_label, working_str);
					updateActionDetails(new_action);
				}
			} break;

			case GUI_CB_ACTION_LAST_FLAG: {

				if (current_action_id < 0)
					return;

				int16_t last_action = Z2S_findPrevActionPosition(Z2S_ACTIONS_MAX_NUMBER);
  			if (last_action >= 0) {

					current_action_id = last_action;
					current_action_counter = Z2S_getActionCounter(last_action);

      		Z2S_loadAction(last_action, new_action);
					sprintfAction(new_action);
					ESPUI.updateLabel(actions_table_label, working_str);
					updateActionDetails(new_action);
				}
			} break;

			case GUI_CB_ACTION_EDIT_FLAG: {

				if ((current_action_gui_state != VIEW_ACTION) || (current_action_id == -1))
					return;

				previous_action_gui_state = current_action_gui_state;
				current_action_gui_state = EDIT_ACTION;
				Z2S_loadAction(current_action_id, new_action);
				enableActionDetails(true);
				updateActionDetails(new_action);
				updateActionButtons();
			} break;

			case GUI_CB_ACTION_NEW_FLAG: {

				if (current_action_gui_state != VIEW_ACTION)
					return;
				
				new_action_id = Z2S_findFreeActionIndex();
				if (new_action_id == -1) {
					
					updateLabel_P(action_state_label, "Actions table is full - can't add new Z2S Action!");
					return;
				} else {
					previous_action_gui_state = current_action_gui_state;
					current_action_gui_state = NEW_ACTION;
					enableActionDetails(true);
					updateActionButtons();
					updateActionDetails(new_action, true);
				}
			} break;

			case GUI_CB_ACTION_SAVE_FLAG: {

				if ((current_action_gui_state != NEW_ACTION) && (current_action_gui_state != EDIT_ACTION))
					return;
				
				if (!fillActionDetails(new_action)) {
					updateLabel_P(action_state_label, "Required action data is missing!");
					return;
				}

				previous_action_gui_state = SAVE_ACTION;
				
				if (current_action_gui_state == NEW_ACTION) {
					
					current_action_id = new_action_id;
				}

				bool save_result = Z2S_saveAction(current_action_id, new_action);

				if (current_action_gui_state == NEW_ACTION) {
					
					current_action_counter = Z2S_getActionCounter(current_action_id);
				}

				current_action_gui_state = VIEW_ACTION;

				enableActionDetails(false);
				enableActionControls(true);
				sprintfAction(new_action);
				ESPUI.updateLabel(actions_table_label, working_str);
				updateActionButtons();

				if (save_result)
					updateLabel_P(action_state_label, "Saving Z2S Action: SUCCESS!<br>"
																						"Restart gateway to activate action!");
				else
					updateLabel_P(action_state_label, "Saving Z2S Action: FAILED!");
			} break;

			case GUI_CB_ACTION_CANCEL_FLAG: {

				if ((current_action_gui_state != NEW_ACTION) && (current_action_gui_state != EDIT_ACTION))
					return;

				previous_action_gui_state = CANCEL_ACTION;
				current_action_gui_state = VIEW_ACTION;
				enableActionDetails(false);				
				enableActionControls(true);
				updateActionButtons();
				updateLabel_P(action_state_label, "Changes cancelled!");
			} break;

			case GUI_CB_ACTION_REMOVE_FLAG: {

				if ((current_action_gui_state != VIEW_ACTION) || (current_action_id == -1))
					return;

				previous_action_gui_state = REMOVE_ACTION;

				bool remove_result = Z2S_removeAction(current_action_id);
				current_action_id = Z2S_findNextActionPosition(0);

				if (current_action_id >= 0) {

					current_action_counter = Z2S_getActionCounter(current_action_id);
					Z2S_loadAction(current_action_id, new_action);
					sprintfAction(new_action);
					ESPUI.updateLabel(actions_table_label, working_str);
				}
				else {

					current_action_counter = -1;
					working_str = "Actions table is empty.";
					ESPUI.updateLabel(actions_table_label, working_str);
				}
				current_action_gui_state = VIEW_ACTION;
				enableActionDetails(false);
				enableActionControls(true);
				updateActionButtons();

				if (remove_result)
					updateLabel_P(action_state_label, "Removing Z2S Action: SUCCESS! <br>"
																						"Action still active in Supla Device - restart gateway to deactivate it!");
				else
					updateLabel_P(action_state_label, "Removing Z2S Action: FAILED!");
			} break;
		}
	}
}

void valveCallback(Control *sender, int type, void *param) {

	if ((type == B_UP) && (ESPUI.getControl(advanced_device_selector)->value.toInt() >= 0)) {

		uint8_t device_slot = ESPUI.getControl(advanced_device_selector)->value.toInt();

		zbg_device_params_t device;
		log_i("device_selector value %u, param id %d", device_slot, (uint32_t)param);
    device.endpoint = 1;//ESPUI.getControl(device_endpoint_number)->value.toInt();//1; //z2s_channels_table[channel_number_slot].endpoint;
    device.cluster_id = SONOFF_CUSTOM_CLUSTER; //z2s_channels_table[channel_number_slot].cluster_id;
    memcpy(&device.ieee_addr, z2s_zb_devices_table[device_slot].ieee_addr,8);
    device.short_addr = z2s_zb_devices_table[device_slot].short_addr;

		uint16_t attribute_id;

		uint32_t value_32;

		switch ((uint32_t)param) {
			
			case GUI_CB_SAVE_PROGRAM_FLAG: { //save valve program

				if ((ESPUI.getControl(valve_program_selector)->value.toInt() > 0) &&
						(ESPUI.getControl(valve_cycles_number)->value.toInt() > 0)) {

					uint8_t program_mode = ESPUI.getControl(valve_program_selector)->value.toInt();
					uint8_t valve_cycles = ESPUI.getControl(valve_cycles_number)->value.toInt();

					if (program_mode == 1) 
						value_32 = ESPUI.getControl(valve_worktime_number)->value.toInt();
						
					else {
						value_32 = ESPUI.getControl(valve_volume_number)->value.toInt();
						//valve_cycles = valve_cycles + 0x80;
					}
					
					z2s_zb_devices_table[device_slot].smart_valve_data.program = program_mode;
					z2s_zb_devices_table[device_slot].smart_valve_data.cycles = valve_cycles;
					z2s_zb_devices_table[device_slot].smart_valve_data.value = value_32;
					z2s_zb_devices_table[device_slot].smart_valve_data.pause_time = ESPUI.getControl(valve_pause_number)->value.toInt();
					//z2s_zb_devices_table[device_slot].user_data_1 = (valve_cycles << 24) + value_32;

					//value_32 = ESPUI.getControl(valve_pause_number)->value.toInt();

					//z2s_zb_devices_table[device_slot].user_data_2 = value_32;

					if (Z2S_saveZBDevicesTable()) 
        		updateLabel_P(valve_info_label, PSTR("Valve program saved successfully."));
					else
						updateLabel_P(valve_info_label, PSTR("Valve program save failed."));
				} else
				updateLabel_P(valve_info_label, PSTR("No valve program to save"));
			} break;

			case GUI_CB_LOAD_PROGRAM_FLAG: { //load valve program

				uint8_t valve_cycles = z2s_zb_devices_table[device_slot].smart_valve_data.cycles; 
				//z2s_zb_devices_table[device_slot].user_data_1 >> 24;
				
				if (valve_cycles > 0) {

					uint8_t program_mode = z2s_zb_devices_table[device_slot].smart_valve_data.program; //(valve_cycles & 0x80) + 1;

					ESPUI.updateNumber(valve_program_selector, program_mode);
					ESPUI.updateNumber(valve_cycles_number, valve_cycles); // & 0x7F);

					value_32 = z2s_zb_devices_table[device_slot].smart_valve_data.value; 
					//z2s_zb_devices_table[device_slot].user_data_1 & 0x00FFFFFF;
					
					if (program_mode == 1) {
						ESPUI.updateNumber(valve_worktime_number, value_32);
						ESPUI.updateNumber(valve_volume_number, 0);
					}
					else {
						ESPUI.updateNumber(valve_volume_number, value_32);
						ESPUI.updateNumber(valve_worktime_number, 0);
					}

					value_32 = z2s_zb_devices_table[device_slot].smart_valve_data.pause_time; 
					//z2s_zb_devices_table[device_slot].user_data_2 & 0x00FFFFFF;

					ESPUI.updateNumber(valve_pause_number, value_32);

        	updateLabel_P(valve_info_label, PSTR("Valve program loaded successfully."));	
				} else
					updateLabel_P(valve_info_label, PSTR("No Valve program found."));
			} break;
			
			case GUI_CB_START_PROGRAM_FLAG: { //start valve program

				switch (ESPUI.getControl(valve_program_selector)->value.toInt()) {

					case 1: attribute_id = SONOFF_CUSTOM_CLUSTER_TIME_IRRIGATION_CYCLE_ID; break;

					case 2: attribute_id = SONOFF_CUSTOM_CLUSTER_VOLUME_IRRIGATION_CYCLE_ID; break;

					default: return;
				}
		
				uint8_t valve_cmd_payload[11] = {0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

				valve_cmd_payload[2] = ESPUI.getControl(valve_cycles_number)->value.toInt();
		
				if (attribute_id == 0x5008) 
					value_32 = ESPUI.getControl(valve_worktime_number)->value.toInt();
				else 
					value_32 = ESPUI.getControl(valve_volume_number)->value.toInt();

				valve_cmd_payload[6] = value_32 & 0xFF;
				valve_cmd_payload[5] = (value_32 & 0xFF00) >> 8;
				valve_cmd_payload[4] = (value_32 & 0xFF0000) >> 16;
		
				value_32 = ESPUI.getControl(valve_pause_number)->value.toInt();
				valve_cmd_payload[10] = value_32 & 0xFF;
				valve_cmd_payload[9] = (value_32 & 0xFF00) >> 8;
				valve_cmd_payload[8] = (value_32 & 0xFF0000) >> 16;
		
				for (int i = 0; i < 11; i++)
					log_i("valve payload [%u] = 0x%02x", i,valve_cmd_payload[i]);

				if (zbGateway.sendAttributeWrite(&device, SONOFF_CUSTOM_CLUSTER, attribute_id, 
																				 ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING, 11, &valve_cmd_payload, true))
					if (*zbGateway.getWriteAttrStatusLastResult() == ESP_ZB_ZCL_STATUS_SUCCESS)
						updateLabel_P(valve_info_label, PSTR("Valve program start success!"));
					else
						updateLabel_P(valve_info_label, PSTR("Valve program start error!"));
				else
					updateLabel_P(valve_info_label, device_query_failed_str);
			} break;

			case GUI_CB_STOP_PROGRAM_FLAG: { //stop valve program

				zbGateway.sendOnOffCmd(&device, false);
			} break;

			case GUI_CB_SEND_PROGRAM_FLAG: {

				int16_t channel_number_slot = Z2S_findChannelNumberSlot(z2s_zb_devices_table[device_slot].ieee_addr, -1, SONOFF_CUSTOM_CLUSTER, 
																																SUPLA_CHANNELTYPE_RELAY, SONOFF_SMART_VALVE_RUN_PROGRAM_SID);
  
  			if (channel_number_slot < 0) {
					
					log_i("no Supla channel for Sonoff program run");
					return;
				}

				switch (ESPUI.getControl(valve_program_selector)->value.toInt()) {

					case 1: {
						
						z2s_channels_table[channel_number_slot].smart_valve_data.program = 1;
						z2s_channels_table[channel_number_slot].smart_valve_data.value = ESPUI.getControl(valve_worktime_number)->value.toInt();
						msgZ2SDeviceVirtualRelayValue(channel_number_slot, VRV_S8_ID, 1);
						msgZ2SDeviceVirtualRelayValue(channel_number_slot, VRV_S32_ID,
																					ESPUI.getControl(valve_worktime_number)->value.toInt());
					} break;

					case 2: {
						
						z2s_channels_table[channel_number_slot].smart_valve_data.program = 2;
						z2s_channels_table[channel_number_slot].smart_valve_data.value = ESPUI.getControl(valve_volume_number)->value.toInt();
						msgZ2SDeviceVirtualRelayValue(channel_number_slot, VRV_S8_ID, 2);
						msgZ2SDeviceVirtualRelayValue(channel_number_slot, VRV_S32_ID,
																					ESPUI.getControl(valve_volume_number)->value.toInt());
					} break;

					default: {
						z2s_channels_table[channel_number_slot].smart_valve_data.program = 0;
						z2s_channels_table[channel_number_slot].smart_valve_data.cycles = 0;
						z2s_channels_table[channel_number_slot].smart_valve_data.value = 0;
						msgZ2SDeviceVirtualRelayValue(channel_number_slot, VRV_S8_ID, 0);
						Z2S_saveChannelsTable();
						return;
					} break;
				}
				z2s_channels_table[channel_number_slot].smart_valve_data.cycles = ESPUI.getControl(valve_cycles_number)->value.toInt();
				z2s_channels_table[channel_number_slot].smart_valve_data.pause_time = ESPUI.getControl(valve_pause_number)->value.toInt();
				Z2S_saveChannelsTable();
				msgZ2SDeviceVirtualRelayValue(channel_number_slot, VRV_U8_ID,
																			ESPUI.getControl(valve_cycles_number)->value.toInt());
																			
				msgZ2SDeviceVirtualRelayValue(channel_number_slot, VRV_U32_ID, 
																			ESPUI.getControl(valve_pause_number)->value.toInt());
			} break;

			case GUI_CB_SEND_RINGTONE_FLAG: { //write gas detector alarm ringtone
 
				if (ESPUI.getControl(gas_alarm_ringtone_selector)->value.toInt() >= 0) {

					uint8_t gas_detector_payload[10];
					uint16_t _tsn_number = random(0x0000, 0xFFFF);
					
        	gas_detector_payload[0] = (_tsn_number & 0xFF00) >> 8;
        	gas_detector_payload[1] = (_tsn_number & 0x00FF);
					gas_detector_payload[2] = TUYA_GAS_DETECTOR_RINGTONE_DP;
					gas_detector_payload[3] = 0x04; //ENUM
					gas_detector_payload[4] = 0x00;
					gas_detector_payload[5] = 0x01;
					gas_detector_payload[6] = ESPUI.getControl(gas_alarm_ringtone_selector)->value.toInt();

					Tuya_custom_cmd_dp = TUYA_GAS_DETECTOR_RINGTONE_DP;
					current_Tuya_payload_label = gas_alarm_Tuya_payload_label;

					if (zbGateway.sendCustomClusterCmd(&device, TUYA_PRIVATE_CLUSTER_EF00, 0x0000, ESP_ZB_ZCL_ATTR_TYPE_SET, 7, gas_detector_payload, true))
					updateLabel_P(gas_alarm_info_label, PSTR("New ringtone sent"));
				else
					updateLabel_P(gas_alarm_info_label, device_query_failed_str);
				} else
					updateLabel_P(gas_alarm_info_label, PSTR("Select ringtone to send."));
			} break;

			case GUI_CB_SEND_TIME_FLAG: { //write gas detector alarm duration

				if (ESPUI.getControl(gas_alarm_time_number)->value.toInt() >= 0) {

					uint8_t gas_detector_payload[10];
					uint16_t _tsn_number = random(0x0000, 0xFFFF);

        	gas_detector_payload[0] = (_tsn_number & 0xFF00) >> 8;
        	gas_detector_payload[1] = (_tsn_number & 0x00FF);
					gas_detector_payload[2] = TUYA_GAS_DETECTOR_ALARM_TIME_DP;
					gas_detector_payload[3] = 0x02; //VALUE
					gas_detector_payload[4] = 0x00;
					gas_detector_payload[5] = 0x04;
					gas_detector_payload[6] = 0x00;
					gas_detector_payload[7] = 0x00;
					gas_detector_payload[8] = 0x00;
					gas_detector_payload[9] = ESPUI.getControl(gas_alarm_time_number)->value.toInt();

					Tuya_custom_cmd_dp = TUYA_GAS_DETECTOR_ALARM_TIME_DP;
					current_Tuya_payload_label = gas_alarm_Tuya_payload_label;

					if (zbGateway.sendCustomClusterCmd(&device, TUYA_PRIVATE_CLUSTER_EF00, 0x0000, ESP_ZB_ZCL_ATTR_TYPE_SET, 10, gas_detector_payload, true))
					updateLabel_P(gas_alarm_info_label, PSTR("New alarm time sent"));
				else
					updateLabel_P(gas_alarm_info_label, device_query_failed_str);
				} else
					updateLabel_P(gas_alarm_info_label, PSTR("Enter alarm duration to send."));
			} break;

			case GUI_CB_SELF_TEST_FLAG: { //gas detector self test cmd

				uint8_t gas_detector_payload[10];

				uint16_t _tsn_number = random(0x0000, 0xFFFF); 

        gas_detector_payload[0] = (_tsn_number & 0xFF00) >> 8;
        gas_detector_payload[1] = (_tsn_number & 0x00FF);
				gas_detector_payload[2] = TUYA_GAS_DETECTOR_SELF_TEST_DP;
				gas_detector_payload[3] = 0x01; //BOOL
				gas_detector_payload[4] = 0x00;
				gas_detector_payload[5] = 0x01;
				gas_detector_payload[6] = 0x01; //ON

				Tuya_custom_cmd_dp = TUYA_GAS_DETECTOR_SELF_TEST_RESULT_DP;
				current_Tuya_payload_label = gas_alarm_Tuya_payload_label;

				if (zbGateway.sendCustomClusterCmd(&device, TUYA_PRIVATE_CLUSTER_EF00, 0x0000, ESP_ZB_ZCL_ATTR_TYPE_SET, 7, gas_detector_payload, true))
					updateLabel_P(gas_alarm_info_label, PSTR("Self-test command sent"));
				else
					updateLabel_P(gas_alarm_info_label, device_query_failed_str);

			} break;

			case GUI_CB_SILENCE_FLAG: { //gas detector silence cmd

				uint8_t gas_detector_payload[10];

				uint16_t _tsn_number = random(0x0000, 0xFFFF); 

        gas_detector_payload[0] = (_tsn_number & 0xFF00) >> 8;
        gas_detector_payload[1] = (_tsn_number & 0x00FF);
				gas_detector_payload[2] = TUYA_GAS_DETECTOR_SILENCE_DP;
				gas_detector_payload[3] = 0x01; //BOOL
				gas_detector_payload[4] = 0x00;
				gas_detector_payload[5] = 0x01;
				gas_detector_payload[6] = 0x01; //ON

				Tuya_custom_cmd_dp = TUYA_GAS_DETECTOR_SILENCE_DP;
				current_Tuya_payload_label = gas_alarm_Tuya_payload_label;

				if (zbGateway.sendCustomClusterCmd(&device, TUYA_PRIVATE_CLUSTER_EF00, 0x0000, ESP_ZB_ZCL_ATTR_TYPE_SET, 7, gas_detector_payload, true))
					updateLabel_P(gas_alarm_info_label, PSTR("Silence command sent"));
				else
					updateLabel_P(gas_alarm_info_label, device_query_failed_str);

			} break;

			case GUI_CB_SEND_MELODY_FLAG: { //send moes alarm melody
 
				if (ESPUI.getControl(moes_alarm_melody_selector)->value.toInt() >= 0) {

					uint8_t moes_alarm_payload[10];
					uint16_t _tsn_number = random(0x0000, 0xFFFF);
					
        	moes_alarm_payload[0] = (_tsn_number & 0xFF00) >> 8;
        	moes_alarm_payload[1] = (_tsn_number & 0x00FF);
					moes_alarm_payload[2] = MOES_ALARM_MELODY_DP;
					moes_alarm_payload[3] = 0x04; //ENUM
					moes_alarm_payload[4] = 0x00;
					moes_alarm_payload[5] = 0x01;
					moes_alarm_payload[6] = ESPUI.getControl(moes_alarm_melody_selector)->value.toInt();

					Tuya_custom_cmd_dp = MOES_ALARM_MELODY_DP;
					current_Tuya_payload_label = moes_alarm_Tuya_payload_label;

					if (zbGateway.sendCustomClusterCmd(&device, TUYA_PRIVATE_CLUSTER_EF00, 0x0000, ESP_ZB_ZCL_ATTR_TYPE_SET, 7, moes_alarm_payload, true))
					updateLabel_P(moes_alarm_info_label, PSTR("New melody sent"));
				else
					updateLabel_P(moes_alarm_info_label, device_query_failed_str);
				} else
					updateLabel_P(moes_alarm_info_label, PSTR("Select melody to send."));
			} break;

			case GUI_CB_SEND_VOLUME_FLAG: { //send moes alarm melody
 
				if (ESPUI.getControl(moes_alarm_volume_number)->value.toInt() >= 0) {

					uint8_t moes_alarm_payload[10];
					uint16_t _tsn_number = random(0x0000, 0xFFFF);
					
        	moes_alarm_payload[0] = (_tsn_number & 0xFF00) >> 8;
        	moes_alarm_payload[1] = (_tsn_number & 0x00FF);
					moes_alarm_payload[2] = MOES_ALARM_VOLUME_DP;
					moes_alarm_payload[3] = 0x04; //ENUM
					moes_alarm_payload[4] = 0x00;
					moes_alarm_payload[5] = 0x01;
					moes_alarm_payload[6] = ESPUI.getControl(moes_alarm_volume_number)->value.toInt();

					Tuya_custom_cmd_dp = MOES_ALARM_VOLUME_DP;
					current_Tuya_payload_label = moes_alarm_Tuya_payload_label;

					if (zbGateway.sendCustomClusterCmd(&device, TUYA_PRIVATE_CLUSTER_EF00, 0x0000, ESP_ZB_ZCL_ATTR_TYPE_SET, 7, moes_alarm_payload, true))
					updateLabel_P(moes_alarm_info_label, PSTR("New volume sent"));
				else
					updateLabel_P(moes_alarm_info_label, device_query_failed_str);
				} else
					updateLabel_P(moes_alarm_info_label, PSTR("Enter volume to send."));
			} break;

			case GUI_CB_SEND_DURATION_FLAG: { //send moes alarm duration

				if (ESPUI.getControl(moes_alarm_duration_number)->value.toInt() >= 0) {

					uint8_t moes_alarm_payload[10];
					uint16_t _tsn_number = random(0x0000, 0xFFFF);

					uint16_t alarm_duration = ESPUI.getControl(moes_alarm_duration_number)->value.toInt();

        	moes_alarm_payload[0] = (_tsn_number & 0xFF00) >> 8;
        	moes_alarm_payload[1] = (_tsn_number & 0x00FF);
					moes_alarm_payload[2] = MOES_ALARM_DURATION_DP;
					moes_alarm_payload[3] = 0x02; //VALUE
					moes_alarm_payload[4] = 0x00;
					moes_alarm_payload[5] = 0x04;
					moes_alarm_payload[6] = 0x00;
					moes_alarm_payload[7] = 0x00;
					moes_alarm_payload[8] = (alarm_duration & 0xFF00) >> 8;
					moes_alarm_payload[9] = (alarm_duration & 0x00FF);

					Tuya_custom_cmd_dp = MOES_ALARM_DURATION_DP;
					current_Tuya_payload_label = moes_alarm_Tuya_payload_label;

					if (zbGateway.sendCustomClusterCmd(&device, TUYA_PRIVATE_CLUSTER_EF00, 0x0000, ESP_ZB_ZCL_ATTR_TYPE_SET, 10, moes_alarm_payload, true))
					updateLabel_P(moes_alarm_info_label, PSTR("New alarm duration sent"));
				else
					updateLabel_P(moes_alarm_info_label, device_query_failed_str);
				} else
					updateLabel_P(moes_alarm_info_label, PSTR("Enter alarm duration to send."));
			} break; 
		}
	}
}


void TuyaDeviceSelectorCallback(Control *sender, int type) {

	int16_t device_slot = sender->value.toInt();

	if (device_slot >=0) {

		sprintf_P(general_purpose_gui_buffer,PSTR("<b><i>Manufacturer name</i></b> %s "
						"<b>| <i>model ID</b></i> %s"), 
						Z2S_getZBDeviceManufacturerName(device_slot),
						Z2S_getZBDeviceModelName(device_slot));

		updateLabel_P(Tuya_device_info_label, general_purpose_gui_buffer);
	} else
		updateLabel_P(Tuya_device_info_label, three_dots_str);
}

void GUI_onTuyaCustomClusterReceive(uint8_t command_id, uint16_t payload_size, uint8_t * payload_data){

	if (current_Tuya_payload_label == 0)
		return;
	
	char payload_buffer[255] = {};

	payload_buffer[0] = '{';
	
	char hex_chunk[3];
	
	hex_chunk[3] = 0x00;

	uint16_t	tsn_id = (((uint16_t)(*payload_data)) << 16) + (*(payload_data + 1));

	uint8_t dp_id = *(payload_data + 2);

	if (dp_id != Tuya_custom_cmd_dp)
		return;

	for (uint16_t payload_idx = 0; payload_idx < payload_size; payload_idx++) {

		sprintf(hex_chunk, "%02X", *(payload_data + payload_idx));
		memcpy(payload_buffer + 1 + (payload_idx * 3), hex_chunk, 2);
		payload_buffer[3 + payload_idx * 3] = ',';
	}
	payload_buffer[payload_size * 3] = '}';
	payload_buffer[(payload_size * 3) + 1] = 0x00;
	updateLabel_P(current_Tuya_payload_label, payload_buffer);	
}

void GUI_onLastBindingFailure(bool binding_failed) {
	
	sprintf_P(general_purpose_gui_buffer, PSTR("Unknown model %s::%s, no binding is possible"), 
						zbGateway.getQueryBasicClusterData()->zcl_manufacturer_name,
            zbGateway.getQueryBasicClusterData()->zcl_model_name);

	updateLabel_P(zigbee_last_binding_result_label, general_purpose_gui_buffer);
}

void GUI_onZigbeeOpenNetwork(bool is_network_open) {

	ESPUI.updateNumber(pairing_mode_switcher, is_network_open ? 1 : 0);
}
#endif //USE_SUPLA_WEB_SERVER
