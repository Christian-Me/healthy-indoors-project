#ifdef ESP32
  #include <WiFi.h>
#endif
#ifdef ESP8266
  #include <ESP8266WiFi.h>
#endif

#include <espnow.h>
#include "../../include/utils.h"

// handle esp-now specific functionality
// plain c version
//
// a c++ version is in .backup but still not solved the common pointer to member function challenge

typedef void (*c_onReceiveCallback) (uint8_t*, uint8_t*, uint8_t);
c_onReceiveCallback receiveCallback = nullptr;

void c_espnow_registerReceiveCB(c_onReceiveCallback newFunction){
    receiveCallback = newFunction;
    Serial.println(F(" receive callback registered"));
};
void c_espnow_dataReceived(uint8_t *mac_addr, uint8_t *data, uint8_t dataLength) {
    if (receiveCallback == nullptr) {
        Serial.println(F("ERR: Please register receive callback first!"));
        return;
    }
    digitalWrite(LED_BUILTIN, LOW);
    Serial.printf("espnow received: %d bytes from %02x:%02x:%02x:%02x:%02x:%02x ",
        dataLength,
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]
    );
    Serial.println();
    receiveCallback(mac_addr, data, dataLength);
};

bool c_espnow_init(const char* nodeName) {
    Serial.println();
    Serial.println(F("Initializing esp-now ..."));
    Serial.print(F(" Node Name:"));
    Serial.println(nodeName);
    Serial.print(F(" MAC address: "));
    Serial.println(WiFi.macAddress());


    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    if(esp_now_init() != 0) {
        Serial.println("ESP-NOW initialization failed!");
        return false;
    };

    esp_now_register_recv_cb(c_espnow_dataReceived);   // this function will get called once all data is sent
    Serial.println("ESP-NOW initialization OK.");
    return true;
}

void c_espnow_loop(void){

};