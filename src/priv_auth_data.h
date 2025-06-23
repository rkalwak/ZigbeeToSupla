#ifndef priv_auth_data_
#define priv_auth_data_
#include <supla-common/proto.h>
static char SUPLA_WIFI_SSID[20]="Dom44A";
static char SUPLA_WIFI_PASS[50]="najciemniejpodlatarnia1234";
static char SUPLA_EMAIL[50]="rafalekkalwak@wp.pl";
static char SUPLA_SVR[50]="svr85.supla.org";

static char GUID[SUPLA_GUID_SIZE] = {0xA4, 0x45, 0xDF, 0xAF, 0x7F, 0xB1, 0x16, 0x39, 0xB1, 0xBB, 0xB8, 0x48, 0x18, 0x65, 0x81, 0xF8};
static char AUTHKEY[SUPLA_AUTHKEY_SIZE] = {0x09, 0xB6, 0xF1, 0x47, 0x59, 0xD7, 0x73, 0x5E, 0xCE, 0x43, 0x9C, 0x96, 0xF1, 0x43, 0xF2, 0xEC};
static int ZG_OPEN_NETWORK=30;
static int ZG_SCENE_SWITCH_DEBOUNCE_TIME_MS=1500;
#endif // priv_auth_data_