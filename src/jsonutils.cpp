#include "jsonutils.h"
#include "sensormap.h"
#include "DallasTemperature.h"
#include "utils.h"
#include <ArduinoJson.h>
#include <AsyncJson.h>

// caller is responsible to free the returned pointer
char* createSensorsArr(Sensormap* pSensors)
{
    StaticJsonDocument<800> doc;
    JsonArray arr = doc.to<JsonArray>();

    addSensorsToArray(pSensors, arr);

    char* pJsonBuffer = (char*) malloc(800);

    serializeJson(arr, pJsonBuffer, 800);
    return pJsonBuffer;
}

void addSensorsToArray(Sensormap* pSensors, JsonArray& jsonArr)
{
    DeviceAddress addr;
    char addrStr[19];

    const node_t* pSensor = pSensors->begin();
    while (pSensor) {

        JsonObject sensorObj = jsonArr.createNestedObject();

        uint64toUInt8Array(pSensor->addr, addr);
        addressToStr(addr, addrStr);

        char sensorName[30];
        strncpy(sensorName, pSensor->pName, sizeof(sensorName));
        sensorObj["name"] = sensorName;
        sensorObj["address"] = addrStr;
        if (pSensor->averageTemp.isValid())
        {
            sensorObj["temperature"] = round2(pSensor->averageTemp.value());
            sensorObj["available"].set(true);
        }
        else {
            sensorObj["temperature"] = nullptr;
            sensorObj["available"].set(false);
        }
        sensorObj["offset"] = pSensor->offset;

        pSensor = pSensor->next;
    }
}

void addSensorsObj(Sensormap* pSensors, JsonObject& root)
{
    JsonArray sensorsArr = root.createNestedArray("sensors");
    addSensorsToArray(pSensors, sensorsArr);
}
