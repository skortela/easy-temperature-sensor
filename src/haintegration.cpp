#include <Arduino.h>
#include "haintegration.h"
#include "config.h"
#include "appSettings.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "sensormap.h"
#include "utils.h"

String deviceUid()
{
    char buffer[19] = { 0 };
    uint8_t mac[6];
    WiFi.macAddress(mac);
    sprintf(buffer, "esp_0x%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buffer);
}

bool sendSensorRegisterationConfig(PubSubClient* pMqttClient, const AppSettings* pSettings, const Sensormap* pSensors)
{
    String strDeviceUid = deviceUid();

    bool ok(true);
    char addrStr[19];
    int i=0;
    const node_t* pSensorNode = pSensors->begin();
    while (pSensorNode && ok) {
        
        addressToStr(pSensorNode->addr, addrStr);
        ok = sendSensorRegisterationConfig(pMqttClient, strDeviceUid.c_str(), addrStr, pSettings, pSensorNode->pName);

        pSensorNode = pSensorNode->next;
        i++;
        //if (i==2) break;
    }
    return ok;
}
bool sendSensorRegisterationConfig(PubSubClient* pMqttClient,const char* nodeId, const char* objectId, const AppSettings* pSettings, const char* pSensorName)
{
    int len = strlen(KHADiscoveryPrefix) + 8 + 7;
    if (nodeId)
        len += strlen(nodeId) + 1;
    if (objectId)
        len += strlen(objectId) + 1;

    char* topic = (char*)malloc(len+1);

    strcpy(topic, KHADiscoveryPrefix);
    strcat(topic, "/sensor/");

    if (nodeId) {
        strcat(topic, nodeId);
        strcat(topic, "/");
    }
    if (objectId) {
        strcat(topic, objectId);
        strcat(topic, "/");
    }
    strcat(topic, "config");

    String strDeviceUid = deviceUid();

    StaticJsonDocument<800> doc;
    JsonObject root = doc.to<JsonObject>();

    root["device_class"] = "temperature";
    String name = pSettings->m_deviceHostname;
    name += String("_") + String(pSensorName);
    root["name"] = name;
    root["availability_topic"] = pSettings->m_availabilityTopic;
    root["state_topic"] = pSettings->m_stateTopic;
    root["unit_of_measurement"] = "°C";
    root["value_template"] = String("{{ value_json.") + String(pSensorName) + String(" }}");
    root["platform"] = "mqtt";
    root["unique_id"] = strDeviceUid + String("_") + String(pSensorName);

    JsonObject device = root.createNestedObject("device");
    device["name"] = pSettings->m_deviceHostname;
    device["model"] = "ESP temperature sensor";
    JsonArray identifiers = device.createNestedArray("identifiers");
    identifiers.add(strDeviceUid);

    char* jsonBuffer = (char*) malloc(800);
    serializeJson(root, jsonBuffer, 800);
    Serial.println("sending config, topic:");
    Serial.println(topic);
    Serial.println("payload:");
    Serial.println(jsonBuffer);

    Serial.print("payload size: ");
    Serial.println(strlen(jsonBuffer));

    Serial.print("mqtt buffer size: ");
    Serial.println(pMqttClient->getBufferSize());

    bool ok = pMqttClient->publish(topic, jsonBuffer, true);
    free(jsonBuffer);
    if (!ok) {
        Serial.println("failed to publish!");
    }
    
    return ok;
}