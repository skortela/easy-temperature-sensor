#ifndef CONFIG_H
#define CONFIG_H

#include <inttypes.h>
#include <pins_arduino.h>

#define KDeviceDefaultHostname "easy-temperature-sensor"
#define KDefaultAvailabilityTopic "/availability"
#define KDefaultCommandTopic "/cmd/#"
#define KDefaultStateTopic "/state"
#define KDefaultInfoTopic "/info"

#define KResetPin D5
// Define one wire bus count
#define ONE_WIRE_BUS_COUNT 2
// Define one wire bus pins
static const int8_t KOneWirePin[] = {D4, D2};

#define KDefaultMqttPort 1883

#define KOffline "offline"
#define KOnline  "online"

#define KStatusUpdateInterval 60000
#define KInfoUpdateInterval 30*60000
#define KTickerIntervalWifiConfig 200
#define KTickerIntervalMQTTConnect 1000

// Magic number to check if eeprom config is valid
const int32_t KConfigValidationMagic = 0x09278269;

#define KHADiscoveryPrefix "homeassistant"

#define USE_ROUND

#endif