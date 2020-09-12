#include <string>

// currently measurements are stored in fixed arrays
// perhaps someone could come up with a flexible solution

#define MAX_DEVICES 20          // maximum amount of devices (CPUs) 
#define MAX_SENSORS 5           // maximum amount of sensors per device
#define MAX_PROPERTIES 10       // maximum amount of properties per sensor

typedef void (*onValueCB) (float,int);

struct propertyType {
    String propertyId;
    float value;
};

struct sensorType {
    uint8_t sensorId;
    uint8_t sensorIndex;
    struct propertyType properties[MAX_PROPERTIES];
    propertyType setProperty(String propertyId, float value) {
        uint8_t i = 0;
        for (i = 0; i < MAX_PROPERTIES; i++) {
            if (properties[i].propertyId.startsWith(propertyId)) {
                break;
            }
            if (properties[i].propertyId.length()==0) {
                properties[i].propertyId=propertyId;
                break;
            }
        }
        properties[i].value = value;
        #ifdef SERIAL_TRACE
            Serial.print(" ");
            Serial.print(propertyId);
            Serial.print(": ");
            Serial.println(value);
        #endif
        return properties[i]; //ToDo what happens if i == MAX_PROPERIES
    };
    float getValue(String propertyId) {
        for (uint8_t i = 0; i < MAX_PROPERTIES; i++) {
            if (properties[i].propertyId.startsWith(propertyId)) {
                return properties[i].value;
            }
        }
        return -1;
    };
};

struct deviceType {
    char deviceId[18];
    unsigned long lastUpdate;
    struct sensorType sensors[MAX_SENSORS];
};

class dataCollection {
    public:
        bool init(void);
        void loop(void);
        uint8_t getDeviceIndex(char [18]); // get device based on deviceId (MAC Address)
        uint8_t getMaxDevices(void); // get maximum of known devices
        deviceType* getDevice(char [18]);
        propertyType* getProperty(char [18], uint8_t, uint8_t, String);
        sensorType* getSensor(char [18], uint8_t, uint8_t);
        propertyType* getProperty(char [18], uint8_t, uint8_t);
        void forEachProperty(String, onValueCB);
        bool getMinMaxAvg(String, float*, float*, float*);
        uint8_t update(char*, dataPacket*);
        dataCollection(void);
        struct deviceType devices[MAX_DEVICES];
    private:
        uint8_t maxDevices = 0;
        uint8_t samplePropertyIndex = 0;
        String tempString = "";
        void cleanMemory();
};

// get amount of known devices
uint8_t dataCollection::getMaxDevices(void){
    return maxDevices+1;
};

// get device index by deviceId (MAC Address)
uint8_t dataCollection::getDeviceIndex(char* deviceId) {
    uint8_t i = 0;
    for (i = 0; i < MAX_DEVICES; i++) {
        if (strcmp(devices[i].deviceId,deviceId) == 0) {
            break;
        }
    }
    return i;
};
// get device based on deviceId (MAC Address)
deviceType* dataCollection::getDevice(char* deviceId) {
    uint8_t i = 0;
    for (i = 0; i < MAX_DEVICES; i++) {
        if (strcmp(devices[i].deviceId,deviceId) == 0) {
            break;
        }
        if (devices[i].deviceId[0]=='\0') {
            maxDevices = i;
            Serial.print(F(" New device: "));
            Serial.println(deviceId);
            strcpy(devices[i].deviceId,deviceId);
            break;
        }
    }
    return &devices[i];
};

// get sensor based on deviceId and sensorId and sensorIndex
sensorType* dataCollection::getSensor(char* deviceId, uint8_t sensorId, uint8_t sensorIndex) {
    deviceType* device = getDevice(deviceId);
    uint8_t i = 0;
    for (i = 0; i < MAX_SENSORS; i++) {
        if (device->sensors[i].sensorId==sensorId &&
            device->sensors[i].sensorIndex==sensorIndex) {
            break;
        }
        if (device->sensors[i].sensorId==0) {
            Serial.print(F(" New sensor: "));
            Serial.print(sensors[sensorId]);
            Serial.print(":");
            Serial.println(sensorIndex);

            device->sensors[i].sensorId=sensorId;
            device->sensors[i].sensorIndex=sensorIndex;
            break;
        }
    }
    return &device->sensors[i];
};

// get property based on deviceId and sensorId &sensorIndex and propertyId
propertyType* dataCollection::getProperty(char* deviceId, uint8_t sensorId, uint8_t sensorIndex, String propertyId) {
    sensorType* sensor = getSensor(deviceId, sensorId, sensorIndex);
    uint8_t i = 0;
    for (i = 0; i < MAX_PROPERTIES; i++) {
        if (sensor[i].properties->propertyId.startsWith(propertyId)) {
            break;
        }
        if (sensor[i].properties->propertyId.length()==0) {
            sensor[i].properties->propertyId=propertyId;
            sensor[i].properties->value=0;
            break;
        }
    }
    return sensor[i].properties;
}

void dataCollection::cleanMemory() {
    Serial.print(F("Clean Memory "));
    for (uint8_t device=0; device < MAX_DEVICES; device++) {
        Serial.print(F("."));
        devices[device].deviceId[0]='\0';
        for (uint8_t sensor=0; sensor < MAX_SENSORS; sensor++) {
            devices[device].sensors[sensor].sensorId = 0;
            devices[device].sensors[sensor].sensorIndex = 0;
            for (uint8_t property=0; property < MAX_PROPERTIES; property++) {
                devices[device].sensors[sensor].properties[property].propertyId = "";
            }
        }
    }
    Serial.println(F(" ok"));
};

// loop through all properties and call callback function for specific property
bool dataCollection::getMinMaxAvg(String propertyId, float* min, float* max, float* avg) {
    int index=0;
    float value=0;
    bool result = false;
    *min=99999;
    *max=0;
    *avg=0;
    for (uint8_t device=0; device < MAX_DEVICES; device++) {
        for (uint8_t sensor=0; sensor < MAX_SENSORS; sensor++) {
            for (uint8_t property=0; property < MAX_PROPERTIES; property++) {
                if (devices[device].sensors[sensor].properties[property].propertyId == propertyId) {
                    value= devices[device].sensors[sensor].properties[property].value;
                    if (value>*max) *max = value;
                    if (value<*min) *min = value;
                    *avg+=value;
                    index++;
                    result = true;
                };
            }
        }
    }
    if (index>0) {
        *avg=*avg/index;
    } else {
        *avg=0;
    }
    return result;
}

// loop through all properties and call callback function for specific property
void dataCollection::forEachProperty(String propertyId, onValueCB callback) {
    int index=0;
    for (uint8_t device=0; device < MAX_DEVICES; device++) {
        for (uint8_t sensor=0; sensor < MAX_SENSORS; sensor++) {
            for (uint8_t property=0; property < MAX_PROPERTIES; property++) {
                if (devices[device].sensors[sensor].properties[property].propertyId == propertyId) {
                    callback(devices[device].sensors[sensor].properties[property].value,index);
                    index++;
                };
            }
        }
    }
}

// update properties with new data package
uint8_t dataCollection::update(char* deviceId = nullptr, dataPacket* data = nullptr) {
    Serial.print(F("Collection update "));
    Serial.print(sensors[data->sensorType]);
    Serial.print(":");
    Serial.println(data->sensorIndex);
    deviceType* device = getDevice(deviceId);
    device->lastUpdate = millis();
    sensorType* sensor = getSensor(deviceId, data->sensorType, data->sensorIndex);
    sensor->setProperty("temperature",data->temperature);
    sensor->setProperty("humidity",data->humidity);
    sensor->setProperty("pressure",data->pressure);
    sensor->setProperty("iaq",data->iaq);
    sensor->setProperty("staticIaq",data->staticIaq);
    sensor->setProperty("co2Equivalent",data->co2Equivalent);
    sensor->setProperty("breathVocEquivalent",data->breathVocEquivalent);
    sensor->setProperty("accuracy",data->accuracy);
    return getDeviceIndex(deviceId);
};

// constructor
dataCollection::dataCollection(void) {
    cleanMemory();
};

// individual init
bool dataCollection::init(void) {
    cleanMemory();
    return true;
};

// loop
void dataCollection::loop(void) {

};