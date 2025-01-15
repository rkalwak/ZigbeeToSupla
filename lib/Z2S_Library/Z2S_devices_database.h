#ifndef Z2S_DEVICES_DATABASE_H_
#define Z2S_DEVICES_DATABASE_H_

// Z2S_DEVICE_DESC_ID - used for selection of clusters to bind and matching Supla channels

typedef struct z2s_device_entity_s {
char manufacturer_name[32];
char model_name[32];
uint32_t z2s_device_desc_id;
} z2s_device_entity_t;

#define MAX_BOUND_CLUSTERS 16

typedef struct z2s_device_desc_s {
uint32_t z2s_device_desc_id;
uint8_t z2s_device_clusters_count;
uint16_t z2s_device_clusters[MAX_BOUND_CLUSTERS];
} z2s_device_desc_t;


#define Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR         0x1000
#define Z2S_DEVICE_DESC_IAS_ZONE_SENSOR             0x2000
#define Z2S_DEVICE_DESC_RELAY                       0x4000
#define Z2S_DEVICE_DESC_RELAY_ELECTRICITY_METER     0x4100

static z2s_device_desc_t Z2S_DEVICES_DESC[] PROGMEM = {
  {.z2s_device_desc_id = Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR, .z2s_device_clusters_count = 4, .z2s_device_clusters =
    {ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY, 
    ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG, 
    ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,
    ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT}},
  {.z2s_device_desc_id = Z2S_DEVICE_DESC_IAS_ZONE_SENSOR, .z2s_device_clusters_count = 4, .z2s_device_clusters =
    {ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY, 
    ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG, 
    ESP_ZB_ZCL_CLUSTER_ID_IAS_ZONE,
    ESP_ZB_ZCL_CLUSTER_ID_ON_OFF}},
  {.z2s_device_desc_id = Z2S_DEVICE_DESC_RELAY, .z2s_device_clusters_count = 3, .z2s_device_clusters =
    {ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY, 
    ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG,
    ESP_ZB_ZCL_CLUSTER_ID_ON_OFF}},
  {.z2s_device_desc_id = Z2S_DEVICE_DESC_RELAY_ELECTRICITY_METER, .z2s_device_clusters_count = 5, .z2s_device_clusters =
    {ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY, 
    ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG, 
    ESP_ZB_ZCL_CLUSTER_ID_ON_OFF,
    ESP_ZB_ZCL_CLUSTER_ID_METERING,
    ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT}}
};
        

static z2s_device_entity_t Z2S_DEVICES_LIST[] PROGMEM = { 
  {.manufacturer_name = "_TZ3000_akqdg6g7", .model_name = "TS0201", .z2s_device_desc_id = Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR},
  {.manufacturer_name = "_TZ3000_rdhukkmi", .model_name = "TS0201", .z2s_device_desc_id = Z2S_DEVICE_DESC_TEMPHUMIDITY_SENSOR},
  {.manufacturer_name = "TEST1", .model_name = "TS0202", .z2s_device_desc_id = Z2S_DEVICE_DESC_IAS_ZONE_SENSOR},
  {.manufacturer_name = "_TZ3000_26fmupbb", .model_name = "TS0203",.z2s_device_desc_id = Z2S_DEVICE_DESC_IAS_ZONE_SENSOR},
  {.manufacturer_name = "_TZ3000_rcuyhwe3", .model_name = "TS0203",.z2s_device_desc_id = Z2S_DEVICE_DESC_IAS_ZONE_SENSOR},
  {.manufacturer_name = "TEST1", .model_name = "TS0205", .z2s_device_desc_id = Z2S_DEVICE_DESC_IAS_ZONE_SENSOR},
  {.manufacturer_name = "_TZ3000_cehuw1lw", .model_name = "TS011F", .z2s_device_desc_id = Z2S_DEVICE_DESC_RELAY_ELECTRICITY_METER}
};

//
#endif
