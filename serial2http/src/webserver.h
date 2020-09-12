/*
  Healthy Indoors Project
  Webserver

  The webserver part was initially based on:

  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp-now-wi-fi-web-server/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include "ESPAsyncWebServer.h"
#include <Arduino_JSON.h>
#include "html.h"

// ToDo: integrate webserver classes into the class webserver (help needed)
AsyncWebServer webServer(80);
AsyncEventSource events("/events");
JSONVar board;

class webserver {
  public:
    webserver(void);
    bool init(void);
    void loop(void);
    bool update(char*, dataPacket*);
};

bool webserver::update(char* id, dataPacket* packet) {
    board["id"] = id;
    board["type"] = sensors[packet -> sensorType];
    board["temperature"] = packet -> temperature;
    board["humidity"] = packet -> humidity;
    board["pressure"] = packet -> pressure;
    board["gasResistance"] = packet -> gasResistance;
    board["iaq"] = packet -> iaq;
    board["staticIaq"] = packet -> staticIaq;
    board["co2Equivalent"] = packet -> co2Equivalent;
    board["breathVocEquivalent"] = packet -> breathVocEquivalent;

    String jsonString = JSON.stringify(board);
    events.send(jsonString.c_str(), "new_readings", millis());
    return true;
};

webserver::webserver(void) {
};

bool webserver::init() {
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
   
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  webServer.addHandler(&events);
  webServer.begin();
  return true;
};

void webserver::loop() {
  static unsigned long lastEventTime = millis();
  static const unsigned long EVENT_INTERVAL_MS = 5000;
  if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
    events.send("ping",NULL,millis());
    lastEventTime = millis();
  }
};