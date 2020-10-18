#ifndef C_ESPNOW_H__
#define C_ESPNOW_H__

#ifdef ESP32
    #include <WiFi.h>
    #include <esp_wifi.h>
    #include <esp_now.h>
    typedef void (*c_espnow_onReceiveCallback) (const uint8_t*, const uint8_t*, int); // mac address, data, length
    typedef void (*c_espnow_onSendCallback) (const uint8_t*, const uint8_t); // mac address, transimssion status
#endif
#ifdef ESP8266
    #include <ESP8266WiFi.h>
    #include <espnow.h>
    typedef void (*c_espnow_onReceiveCallback) (uint8_t*, uint8_t*, uint8_t);
    typedef void (*c_espnow_onSendCallback) (const uint8_t*, const uint8_t*); // mac address, transimssion status
#endif
#include "../../include/credentials.h"

typedef void (*receiveCallback) (const uint8_t * mac_addr, const uint8_t *data, int dataLength);
typedef void (*sendCallback) (const uint8_t * mac_addr, const uint8_t status);

// handle esp-now specific functionality
// c++ version
// callback functions are defined globally! Only one instance of c_espnow is possible.

c_espnow_onReceiveCallback c_espnow_receiveCallback = nullptr;
c_espnow_onSendCallback c_espnow_sendCallback = nullptr;

uint8_t receiverAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};     // please update this with the MAC address of the receiver if you do not prefere bradcasts (i.e. mutiple rooms)
uint8_t broadcastAddress[] = {0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF};

class c_espnow {
  private:
    bool paused = false;
    static void dataReceived(const uint8_t * mac_addr, const uint8_t *data, int dataLength);
    static void dataSend(const uint8_t * mac_addr, esp_now_send_status_t status);
    int32_t getWiFiChannel(const char *ssid);
  public:
    void pauseReceive(void);
    void resumeReceive(void);
    void registerReceiveCallback(receiveCallback newFunction);
    void registerSendCallback(sendCallback newFunction);
    void send(dataPacket* data, size_t len);
    void send(dataPacket* data, size_t len, const uint8_t *peer_addr);
    bool init(const char* nodeName);
    void loop(void);
};

int32_t c_espnow::getWiFiChannel(const char *ssid) {
    Serial.println(F("Scanning WiFi APs"));
    int32_t channelFound = 0;
    long lastRSSI = -999;
    if (int32_t n = WiFi.scanNetworks()) {
        for (uint8_t i=0; i<n; i++) {
            Serial.print(" ");
            Serial.print(WiFi.SSID(i).c_str());
            Serial.print(F(" Channel: "));
            Serial.print(WiFi.channel(i));
            Serial.print(F(" Signal: "));
            Serial.print(WiFi.RSSI(i));
            if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
                if (lastRSSI<WiFi.RSSI(i)) {
                    Serial.print(" *");
                    channelFound=WiFi.channel(i);
                    lastRSSI=WiFi.RSSI(i);
                }
            }
            Serial.println();
        }
    }
    Serial.print(F(" using channel: "));
    Serial.println(channelFound);
    return channelFound;
}
void c_espnow::pauseReceive(void) {
    if (paused) return;
    esp_now_unregister_recv_cb(); // stop esp-now callback during e-paper update because new packages interfeer the update process
    paused=true;
}

void c_espnow::resumeReceive(void) {
    if (!paused) return;
    esp_now_register_recv_cb(c_espnow::dataReceived);  // re register esp-now callback
    paused=false;
}

void c_espnow::registerReceiveCallback(c_espnow_onReceiveCallback newFunction){
    c_espnow_receiveCallback = newFunction;
    Serial.println(F(" receive callback registered"));
};

void c_espnow::registerSendCallback(c_espnow_onSendCallback newFunction){
    c_espnow_sendCallback = newFunction;
    Serial.println(F(" send callback registered"));
};

#ifdef ESP32
    void c_espnow::dataReceived(const uint8_t * mac_addr, const uint8_t *data, int dataLength) {
#endif
#ifdef ESP8266
    void c_espnow::dataReceived(uint8_t *mac_addr, uint8_t *data, uint8_t dataLength) {
#endif
    if (c_espnow_receiveCallback == nullptr) {
        Serial.println(F("ERR: Please register receive callback first!"));
        return;
    }
    digitalWrite(LED_BUILTIN, LOW);
    Serial.printf("espnow received: %d bytes from %02x:%02x:%02x:%02x:%02x:%02x ",
        dataLength,
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]
    );
    Serial.println();
    if (c_espnow_receiveCallback!=nullptr) c_espnow_receiveCallback(mac_addr, data, dataLength);
};

void c_espnow::dataSend(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if(status == 0) {
    Serial.println(" esp-now data sent successfully");
  } else {
    Serial.print(" esp-now error code: ");
    Serial.println(status);
  }
  if (c_espnow_sendCallback!=nullptr) c_espnow_sendCallback(mac_addr, status);
}

void c_espnow::send(dataPacket* data, size_t len = 0){
    if (len==0) len = sizeof(*data);
    Serial.print(F(" Sending "));
    Serial.print(len);
    Serial.print(F(" bytes uptime="));
    Serial.println(data->uptime);
    printDataPackage((uint8_t *) data, len);
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) data, len);
    if (result != ESP_OK) {
        Serial.print(F(" esp-now send failed: "));
        switch (result) {
            case ESP_ERR_ESPNOW_NOT_INIT: Serial.print(F("ESPNOW is not initialized")); break;
            case ESP_ERR_ESPNOW_ARG : Serial.print(F("invalid argument")); break;
            case ESP_ERR_ESPNOW_INTERNAL : Serial.print(F("internal error")); break;
            case ESP_ERR_ESPNOW_NO_MEM : Serial.print(F("out of memory")); break;
            case ESP_ERR_ESPNOW_NOT_FOUND : Serial.print(F("peer is not found")); break;
            case ESP_ERR_ESPNOW_IF : Serial.print(F("current WiFi interface doesn't match that of peer")); break;
        }
        Serial.println("!");
    }
}

void c_espnow::send(dataPacket* data, size_t len, const uint8_t *peerAddress){
    esp_now_send(peerAddress, (uint8_t *) &data, len);
}

bool c_espnow::init(const char* nodeName) {
    Serial.println();
    Serial.println(F("Initializing esp-now ..."));
    Serial.print(F(" Node Name:"));
    Serial.println(nodeName);
    Serial.print(F(" MAC address: "));
    Serial.println(WiFi.macAddress());


    WiFi.mode(WIFI_STA);
    // WiFi.mode(WIFI_AP_STA);
    Serial.print(F(" SoftAP MAC address: "));
    Serial.println( WiFi.softAPmacAddress() );
    WiFi.disconnect();
    if(esp_now_init() != 0) {
        Serial.println(" ESP-NOW initialization failed!");
        return false;
    };
    esp_now_peer_info_t peerInfo;
    int32_t wifiCannel = getWiFiChannel(WIFI_SSID);
    if (wifiCannel == 0) wifiCannel = ESP_NOW_CANNEL; // use default channel
    Serial.print(F(" WiFi channel: "));
    Serial.print( wifiCannel );
    Serial.println((wifiCannel == ESP_NOW_CANNEL) ? " (default)" : " (auto)");
    memcpy(peerInfo.peer_addr, broadcastAddress, wifiCannel);
    peerInfo.channel = wifiCannel;  // configure in credentials.h
    peerInfo.encrypt = false;
    peerInfo.ifidx = ESP_IF_WIFI_STA;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println(F(" Failed to add peer"));
    } else {
        Serial.print(F(" added broadcast to peer list on channel: "));
        Serial.println(peerInfo.channel);
    }

    esp_now_register_send_cb(c_espnow::dataSend);       // this function will get called once all data is sent
    esp_now_register_recv_cb(c_espnow::dataReceived);   // this function will get called once data is received

    Serial.println("ESP-NOW initialization OK.");
    return true;
}

void c_espnow::loop(void){
    resumeReceive();
};
#endif