#ifndef CONFIG_H
#define CONFIG_H

#include <inttypes.h>

#define KDeviceDefaultHostname "TemperatureMqtt"
#define KDefaultAvailabilityTopic "/availability"
#define KDefaultCommandTopic "/cmd/#"
#define KDefaultStateTopic "/state"
#define KDefaultInfoTopic "/info"
/*
extern const int KStatusUpdateInterval;  // 60 sec
//const char* KAvailabilityTopic = "vallox/availability";
extern const char* KOnline;
extern const char* KOffline;
//const char* KMqttTopicCommandListen = "vallox/cmd/#";
//const char* KStateTopic = "vallox/state"; // for status updates

extern const int KTickerIntervalWifiConfig;
extern const int KTickerIntervalMQTTConnect;

// Magic number to check if eeprom config is valid
extern const int32_t KConfigValidationMagic;
*/

#define ONE_WIRE_PIN D7 // pannuhuone
//#define ONE_WIRE_PIN D6 // other
#define KResetPin D5

#define KDefaultMqttPort 1883

#define KOffline "offline"
#define KOnline  "online"

#define KStatusUpdateInterval 60000
#define KInfoUpdateInterval 30*60000
//const char* KOnline = "online";
//const char* KOffline = "offline";
#define KTickerIntervalWifiConfig 200
#define KTickerIntervalMQTTConnect 1000

// Magic number to check if eeprom config is valid
#define KConfigValidationMagic 0x09278263

#define KHADiscoveryPrefix "homeassistant"

#endif