#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "Arduino.h"


class AppSettings {
    public:
        AppSettings();
        ~AppSettings();

        void clear();

        void print();

        void setDeviceHostname(const char* pHostname);

        String topic(const char* pPostFix = NULL) const;

    public:
        char* m_deviceHostname;
        char* m_mqtt_server;
        int m_mqtt_port;
        char* m_mqtt_user;
        char* m_mqtt_passw;

        char* m_availabilityTopic;
        char* m_commandListen;
        char* m_stateTopic;
        char* m_infoTopic;
        //int m_oneWirePin;

    private:
};

#endif