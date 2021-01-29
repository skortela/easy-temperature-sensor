#ifndef HAINTEGRATION_H
#define HAINTEGRATION_H

class PubSubClient;
class AppSettings;
class Sensormap;

bool sendSensorRegisterationConfig(PubSubClient* pMqttClient, const AppSettings* pSettings, const Sensormap* pSensors);
bool sendSensorRegisterationConfig(PubSubClient* pMqttClient, const char* nodeId, const char* objectId, const AppSettings* pSettings, const char* pSensorName);

#endif // HAINTEGRATION_H