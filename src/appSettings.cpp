#include "appSettings.h"

#ifdef ARDUINO
    #include <Arduino.h>
#else
    #include <stddef.h>
    #include <cstring>
#endif

#include "config.h"



AppSettings::AppSettings() :  m_mqtt_server(NULL), m_mqtt_port(KDefaultMqttPort), m_mqtt_user(NULL), m_mqtt_passw(NULL), m_availabilityTopic(NULL), m_commandListen(NULL), m_stateTopic(NULL), m_infoTopic(NULL)
{
    setDeviceHostname(KDeviceDefaultHostname);
}
AppSettings::~AppSettings()
{
    clear();
}

void AppSettings::clear()
{
    delete m_deviceHostname;
    m_deviceHostname = NULL;
    delete m_mqtt_server;
    m_mqtt_server = NULL;
    m_mqtt_port = KDefaultMqttPort;
    delete m_mqtt_user;
    m_mqtt_user = NULL;
    delete m_mqtt_passw;
    m_mqtt_passw = NULL;

    if (m_availabilityTopic) {
        free(m_availabilityTopic);
        m_availabilityTopic = NULL;
    }
    if (m_commandListen) {
        free(m_commandListen);
        m_commandListen = NULL;
    }
    if (m_stateTopic) {
        free(m_stateTopic);
        m_stateTopic = NULL;
    }
    if (m_infoTopic) {
        free(m_infoTopic);
        m_infoTopic = NULL;
    }
    
    //m_oneWirePin = 0;
}

void AppSettings::print()
{
    Serial.println("AppSettings:");
    
    Serial.print("m_deviceHostname: ");    Serial.println(m_deviceHostname);
    Serial.print("m_mqtt_server: ");    Serial.println(m_mqtt_server);
    Serial.print("m_mqtt_port: ");      Serial.println(m_mqtt_port);
    Serial.print("m_mqtt_user: ");      Serial.println(m_mqtt_user);
    //Serial.print("m_mqtt_passw: ");     Serial.println(m_mqtt_passw);
    //Serial.print("m_oneWirePin: ");     Serial.println(m_oneWirePin);
    Serial.print("m_availabilityTopic: "); Serial.println(m_availabilityTopic);
    Serial.print("m_commandListen: ");     Serial.println(m_commandListen);
    Serial.print("m_stateTopic: ");     Serial.println(m_stateTopic);
    Serial.print("m_infoTopic: ");      Serial.println(m_infoTopic);
}

void AppSettings::setDeviceHostname(const char* pHostname)
{
    delete m_deviceHostname;
    m_deviceHostname = strdup(pHostname);

    if (m_availabilityTopic)
        free(m_availabilityTopic);
    m_availabilityTopic = (char*)malloc(strlen(m_deviceHostname)+strlen(KDefaultAvailabilityTopic)+2);
    strcpy(m_availabilityTopic, m_deviceHostname);
    strcat(m_availabilityTopic, KDefaultAvailabilityTopic);

    if (m_commandListen)
        free(m_commandListen);
    m_commandListen = (char*)malloc(strlen(m_deviceHostname)+strlen(KDefaultCommandTopic)+2);
    strcpy(m_commandListen, m_deviceHostname);
    strcat(m_commandListen, KDefaultCommandTopic);

    if (m_stateTopic)
        free(m_stateTopic);
    m_stateTopic = (char*)malloc(strlen(m_deviceHostname)+strlen(KDefaultStateTopic)+2);
    strcpy(m_stateTopic, m_deviceHostname);
    strcat(m_stateTopic, KDefaultStateTopic);

    if (m_infoTopic)
        free(m_infoTopic);
    m_infoTopic = (char*)malloc(strlen(m_deviceHostname)+strlen(KDefaultInfoTopic)+2);
    strcpy(m_infoTopic, m_deviceHostname);
    strcat(m_infoTopic, KDefaultInfoTopic);
}

String AppSettings::topic(const char* pPostFix) const
{
    String ret(m_deviceHostname);
    if (pPostFix)
        ret += String("/") + String(pPostFix);
    return ret;
}