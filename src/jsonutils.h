
#ifndef JSONUTILS_H
#define JSONUTILS_H

#include <ArduinoJson.h>
class Sensormap;

// caller is responsible to free the returned pointer
char* createSensorsArr(Sensormap* pSensors);

void addSensorsToArray(Sensormap* pSensors, JsonArray& jsonArr);

void addSensorsObj(Sensormap* pSensors, JsonObject& root);

#endif // JSONUTILS_H
