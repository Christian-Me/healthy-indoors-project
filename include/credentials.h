// Replace with your network credentials of your router and mqtt broker
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// mqtt broker
#define MQTT_HOST "broker.hivemq.com"
// #define MQTT_HOST IPAddress(192, 168, 2, 14)     // this is my private LAN broker
// #define MQTT_HOST IPAddress(192, 168, 2, 227)    // and another one
#define MQTT_USER ""
#define MQTT_PASSWORD ""
#define MQTT_PORT 1883

// homie convention
#define HOMIE_BASE "healthy-indoors-project"
#define HOMIE_SUB "healthy-indoors-project/+/+/+/set"
#define HOMIE_INTERVAL 60 // device data update rate (nodes can update more often)
