#ifndef WIFIMANAGERPARAMSEXT_H
#define WIFIMANAGERPARAMSEXT_H

#ifdef nodef

#include <WiFiManager.h>
#include <Arduino.h>

class WiFiManagerParamInt : public WiFiManagerParameter {
public:
    WiFiManagerParamInt(const char *id, const char *placeholder, int value, const uint8_t length = 10, const char* custom = "")
        : WiFiManagerParameter("") {
        char strValue[10];
        sprintf(strValue, "%d", value);
        init(id, placeholder, strValue, length, custom, WFM_LABEL_BEFORE);
    }

    int getValue() {
        //return String(WiFiManagerParameter::getValue()).toInt();
        return atoi(WiFiManagerParameter::getValue());
    }
};
//#ifdef nodef
class WiFiManagerParamComboPin : public WiFiManagerParameter {
public:
    WiFiManagerParamComboPin(const char *id, const char *placeholder, int value, const uint8_t length = 10)
        : WiFiManagerParameter("") {
        char strValue[10];
        sprintf(strValue, "%d", value);

        init(id, placeholder, strValue, length, "", WFM_LABEL_BEFORE);
    }

    int getValue() {
        //return String(WiFiManagerParameter::getValue()).toInt();
        return atoi(WiFiManagerParameter::getValue());
    }
};
#endif

#endif