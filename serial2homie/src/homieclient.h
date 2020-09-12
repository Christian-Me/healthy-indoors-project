#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <string>
#include "../../include/utils.h"

#define MAX_PARAMETERS 10
#define MAX_DEVICES 10
#define MAX_NODES 5

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
uint8_t resendFrom = 0; // index to retry a publish if last publish failed;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe(HOMIE_SUB, 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
}

void onMqttPublish(uint16_t packetId) {
    #ifdef SERIAL_TRACE
        Serial.print(F(" Publish acknowledged. Id:"));
        Serial.println(packetId);
    #endif
}

struct homieProperiesType {
    String propertyId;
//    String datatype;
//    String format;
//    String unit;
};

struct homieNodeType {
    String nodeId;
    struct homieProperiesType homieProperties[MAX_PARAMETERS];
};

struct homieDeviceType {
    String deviceId;
    uint16_t uptime;
    struct homieNodeType homieNodes[MAX_NODES];
};

class homieClient {
    struct homieDeviceType homieDevices[MAX_DEVICES];
    homieDeviceType* getHomieDevice(char [18]);
    homieNodeType* getHomieNode(char [18], uint8_t, uint8_t);
    uint8_t setHomieProperty(homieDeviceType*, homieNodeType*,const char*,const char*,const char*,const char*, float, uint8_t);
    void setWill(const char* , const char*, const char*); // publish LWT
    void updateUptime(void);
    uint16_t publishHomie(const char* , const char*, const char*); // publish to homie Device
    uint16_t publishHomie(const char* , const char*, const char*, const char*); // publish to homie Node
    uint16_t publishHomie(const char* , const char*, const char*, const char*, const char*); // publish to homie Propertie
    String tempString = "";
    void cleanMemory();
    homieDeviceType* resendDevice;
    homieNodeType* resendNode;
  public:
    homieClient(void);
    bool init(void);
    void loop(void);
    bool rts(void); // returns true if server is Ready To Send
    bool update(char*, dataPacket*);
};

float getRssI() { 
    float RssI = WiFi.RSSI();
    RssI = isnan(RssI) ? -100.0 : RssI;
    RssI = min(max(2 * (RssI + 100.0), 0.0), 100.0);
    return RssI;
};

bool homieClient::rts(void){
    return (resendFrom==0);
}

// update the uptime property of all connected devices
void homieClient::updateUptime(void) {
    
}

void homieClient::cleanMemory() {
    Serial.print(F("Clean Memory "));
    for (uint8_t device=0; device < MAX_DEVICES; device++) {
        homieDevices[device].deviceId = "";
        for (uint8_t node=0; node < MAX_NODES; node++) {
            homieDevices[device].homieNodes[node].nodeId = "";
            for (uint8_t parameter=0; parameter < MAX_PARAMETERS; parameter++) {
                Serial.print(F("."));
                homieDevices[device].homieNodes[node].homieProperties[parameter].propertyId = "";
            }
        }
    }
    Serial.println(F(" ok"));
}
uint8_t homieClient::setHomieProperty(homieDeviceType* homieDevice, homieNodeType* homieNode,const char* propertyId,const char* datatype,const char* unit,const char* format, float value, uint8_t resendFrom = 0) {
    uint16_t result = 0;
    uint8_t i = 0;
    Serial.print(F(" Property: "));
    Serial.print(propertyId);
    Serial.print(F(" value: "));
    Serial.println(value,2);
    for (i = 0; i < MAX_PARAMETERS; i++) {
        if (homieNode -> homieProperties[i].propertyId.startsWith(propertyId)) {
            break;
        }
        if (homieNode -> homieProperties[i].propertyId.length()==0) {
            Serial.print(F(" New propertyId: "));
            Serial.print(propertyId);
            Serial.print(F(" on Node: "));
            Serial.println(homieNode -> nodeId);

            // rebuild $properties enum string
            String properties = "";
            uint8_t j = 0;
            while (homieNode -> homieProperties[j].propertyId.length() > 0) {
                properties+=homieNode -> homieProperties[j].propertyId;
                properties+=",";
                j++;
            }
            properties+= propertyId;
            #ifdef SERIAL_TRACE
                Serial.print(F(" $properties="));
                Serial.println(properties);
            #endif
            // return on failure -> will be updated next time a package arrives
            result = publishHomie(homieDevice->deviceId.c_str(), homieNode -> nodeId.c_str(), "$properties", properties.c_str());
            if (result==0) return resendFrom;
            result = publishHomie(homieDevice->deviceId.c_str(), homieNode -> nodeId.c_str(), propertyId, "$name", propertyId);
            if (result==0) return resendFrom;
            result = publishHomie(homieDevice->deviceId.c_str(), homieNode -> nodeId.c_str(), propertyId, "$datatype", datatype);
            if (result==0) return resendFrom;
            result = publishHomie(homieDevice->deviceId.c_str(), homieNode -> nodeId.c_str(), propertyId, "$unit", unit);
            if (result==0) return resendFrom;
            result = publishHomie(homieDevice->deviceId.c_str(), homieNode -> nodeId.c_str(), propertyId, "$format", format);
            if (result==0) return resendFrom;
            // when all successfull store to known properties array
            homieNode -> homieProperties[i].propertyId = propertyId;
            break;
        }
    }
    // publishHomie(deviceId, "$state", "ready");
    // prepare enum property
    if (String(datatype).startsWith("enum")) {
        Serial.print(F("   value="));
        Serial.print(value);
        String resultString = enumGetIndex(String(format),value);
        Serial.print(F(" result="));
        Serial.print(resultString);
        result= publishHomie(homieDevice->deviceId.c_str(), homieNode -> nodeId.c_str(), propertyId, resultString.c_str());
    } else {
        result= publishHomie(homieDevice->deviceId.c_str(), homieNode -> nodeId.c_str(), propertyId, toString(value,2).c_str());
    }
    if (result!=0) {
        resendFrom++; // increment success counter by one
    }
    return resendFrom;
};

homieNodeType* homieClient::getHomieNode(char* deviceId, uint8_t nodeType, uint8_t nodeIndex) {
    uint8_t i = 0;
    uint16_t result = 0;
    homieDeviceType *homieDevice = getHomieDevice(deviceId);
    String nodeId = sensors[nodeType];
    String nodes = "";
    nodeId += F("-");
    nodeId += nodeIndex;
    for (i = 0; i < MAX_NODES; i++) {
        if (homieDevice -> homieNodes[i].nodeId.startsWith(nodeId)) {
            break;
        }
        if (homieDevice -> homieNodes[i].nodeId.length()==0) {
            Serial.print(F(" New nodeId: "));
            Serial.println(nodeId);
            homieDevice -> homieNodes[i].nodeId = nodeId;
            result= publishHomie(deviceId, nodeId.c_str(), "$name", sensors[nodeType]);
            if (result==0) Serial.println(F("publish node/$name failed!"));
            result= publishHomie(deviceId, nodeId.c_str(), "$type", "enviornment sensor");
            if (result==0) Serial.println(F("publish node/$type failed!"));

            nodes+=nodeId;
            result= publishHomie(deviceId, "$nodes", nodes.c_str());
            if (result==0) Serial.println(F("publish node/$nodes failed!"));
            break;
        }
        nodes+=homieDevice -> homieNodes[i].nodeId;
        nodes+=",";
    }
    return &homieDevice -> homieNodes[i];
};

homieDeviceType* homieClient::getHomieDevice(char* deviceId) {
    uint8_t i = 0;
    for (i = 0; i < MAX_DEVICES; i++) {
        if (homieDevices[i].deviceId.startsWith(deviceId)) {
            return &homieDevices[i];
        }
        if (homieDevices[i].deviceId.length()==0) {
            Serial.print(F(" New deviceId: "));
            Serial.println(deviceId);
            homieDevices[i].deviceId = deviceId;
            publishHomie(deviceId, "$state", "init");
            setWill(deviceId, "$state", "lost");
            publishHomie(deviceId, "$homie", "4.0.0");
            tempString = "sensor-node-";
            tempString += deviceId;
            publishHomie(deviceId, "$name", tempString.c_str());
            publishHomie(deviceId, "$extensions", "org.homie.legacy-stats:0.1.1:[4.x],org.homie.legacy-firmware:0.1.1:[4.x]");
            publishHomie(deviceId, "$localip", WiFi.localIP().toString().c_str());
            publishHomie(deviceId, "$mac", deviceId);
            #ifdef ESP32
                publishHomie(deviceId, "$implementation", "ESP32");
            #else
                publishHomie(deviceId, "$implementation", "ESP8266");
            #endif
            publishHomie(deviceId, "$fw/version", VERSION);
            publishHomie(deviceId, "$fw/name", "healthy-indoors-homie-bridge");
            publishHomie(deviceId, "$stats/interval", toString(HOMIE_INTERVAL,0).c_str());
            publishHomie(deviceId, "$stats/uptime", toString(homieDevices[i].uptime,0).c_str());
            publishHomie(deviceId, "$stats/signal", toString(getRssI(),1).c_str());
            publishHomie(deviceId, "$nodes", "");
            
            return &homieDevices[i];
        }
    }
    return &homieDevices[i];
}

void homieClient::setWill(const char*  device, const char* property, const char* payload) {
    String topic = HOMIE_BASE;
    topic+=F("/");
    topic+=device;
    topic+=F("/");
    topic+=property;
    #ifdef SERIAL_TRACE
        Serial.print(F("publish LWT "));
        Serial.print(topic);
        Serial.print("=");
        Serial.print(payload);
        Serial.print(" ");
    #endif
    mqttClient.setWill(topic.c_str(), 2, true, payload);
    #ifdef SERIAL_TRACE
        Serial.println(result);
    #endif
}

uint16_t homieClient::publishHomie(const char*  device, const char* property, const char* payload) {
    String topic = HOMIE_BASE;
    topic+=F("/");
    topic+=device;
    topic+=F("/");
    topic+=property;
    #ifdef SERIAL_TRACE
        Serial.print(F("Publish "));
        Serial.print(topic);
        Serial.print("=");
        Serial.print(payload);
        Serial.print(" ");
    #endif
    uint16_t result = mqttClient.publish(topic.c_str(), 2, true, payload);
    #ifdef SERIAL_TRACE
        Serial.println(result);
    #endif
    return result;
}

uint16_t homieClient::publishHomie(const char*  device, const char* node, const char* property, const char* payload) {
    String topic = HOMIE_BASE;
    topic+=F("/");
    topic+=device;
    topic+=F("/");
    topic+=node;
    topic+=F("/");
    topic+=property;
    #ifdef SERIAL_TRACE
        Serial.print(F("Publish "));
        Serial.print(topic);
        Serial.print("=");
        Serial.print(payload);
        Serial.print(" ");
    #endif
    uint16_t result = mqttClient.publish(topic.c_str(), 2, true, payload);
    #ifdef SERIAL_TRACE
        Serial.println(result);
    #endif
    return result;
}

uint16_t homieClient::publishHomie(const char*  device, const char* node, const char* property, const char* parameter, const char* payload) {
    String topic = HOMIE_BASE;
    topic+=F("/");
    topic+=device;
    topic+=F("/");
    topic+=node;
    topic+=F("/");
    topic+=property;
    topic+=F("/");
    topic+=parameter;
    #ifdef SERIAL_TRACE
        Serial.print(F("Publish "));
        Serial.print(topic);
        Serial.print("=");
        Serial.print(payload);
        Serial.print(" ");
    #endif
    uint16_t result = mqttClient.publish(topic.c_str(), 2, true, payload);
    #ifdef SERIAL_TRACE
        Serial.println(result);
    #endif
    return result;
}

// keep last node and dataPacket global for resend attempts
homieNodeType* homieNode;
dataPacket* currentPacket;

bool homieClient::update(char* deviceId = nullptr, dataPacket* newPacket = nullptr) {
    homieDeviceType* homieDevice = nullptr;
    if (resendFrom==0) { // ready to send new package 
        homieDevice = getHomieDevice(deviceId);
        Serial.print(F(" Device: "));
        Serial.println(homieDevice->deviceId);
        currentPacket = newPacket;
        homieNode = getHomieNode(deviceId, currentPacket -> sensorType, currentPacket -> sensorIndex);
        resendDevice = homieDevice;
        resendNode = homieNode;
        Serial.print(F(" Node: "));
        Serial.println(homieNode->nodeId);
        resendFrom=1;
    } else { // resend in progress
        Serial.print(F("Try to resend messages from: "));
        Serial.println(resendFrom);
        homieDevice = resendDevice;
        homieNode = resendNode;

    }

    // sending data to mqtt
    if (resendFrom==1) resendFrom = setHomieProperty(homieDevice, homieNode,"temperature","float","°C","-20:40", currentPacket -> temperature, resendFrom);
    if (resendFrom==2) resendFrom = setHomieProperty(homieDevice, homieNode,"humidity","float","%","0:10", currentPacket -> humidity, resendFrom);
    if (resendFrom==3) resendFrom = setHomieProperty(homieDevice, homieNode,"pressure","float","hPa","300:1100", currentPacket -> pressure, resendFrom);
    if (resendFrom==4) resendFrom = setHomieProperty(homieDevice, homieNode,"gasResistance","float","Ohm","", currentPacket -> gasResistance, resendFrom);
    if (resendFrom==5) resendFrom = setHomieProperty(homieDevice, homieNode,"iaq","float","","0:500", currentPacket -> iaq, resendFrom);
    if (resendFrom==6) resendFrom = setHomieProperty(homieDevice, homieNode,"staticIaq","float","","0:500", currentPacket -> staticIaq, resendFrom);
    if (resendFrom==7) resendFrom = setHomieProperty(homieDevice, homieNode,"co2Equivalent","float","ppm","0:5000", currentPacket -> co2Equivalent, resendFrom);
    if (resendFrom==8) resendFrom = setHomieProperty(homieDevice, homieNode,"breathVocEquivalent","float","ppm","0:1", currentPacket -> breathVocEquivalent, resendFrom);
    if (resendFrom==9) resendFrom = setHomieProperty(homieDevice, homieNode,"accuracy","enum","","started,uncertain,calibrating,calibrated", currentPacket -> accuracy, resendFrom);
    if (resendFrom==10) {
        publishHomie(homieDevice->deviceId.c_str(), "$state", "ready");
        resendFrom = 0; // back to idle;
    }
    return true;
};

// initial constructor
homieClient::homieClient(void) {
};

// initialize object as needed
bool homieClient::init() {
    Serial.println();
    cleanMemory();
    wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onSubscribe(onMqttSubscribe);
    mqttClient.onUnsubscribe(onMqttUnsubscribe);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.onPublish(onMqttPublish);
    mqttClient.setCredentials(MQTT_USER, MQTT_PASSWORD);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);

    connectToWifi();
    resendFrom  =0; // ready to send;
    return true;
};

void homieClient::loop() {
    // try to send missing messages
    if (resendFrom!=0) {
        Serial.println(F("Update not finished ... "));
        update();
    }
    
};
