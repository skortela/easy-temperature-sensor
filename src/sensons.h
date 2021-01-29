#include "sensormap.h"
#include "OneWire.h"
#include "DallasTemperature.h"

class Sensors
{
    public:
        Sensors();
        ~Sensors();

        void begin();

        void loop();

        // first readings ready?
        bool isReady() const;

        void searchNewSensors(bool& newSensorsFound);

        Sensormap* sensormap();

    private:
        unsigned long m_lastTemperatureRequestTime;
        bool m_hasTemperaturesRead;
        bool m_ready;
        Sensormap m_sensorNameMap;

        OneWire m_oneWire;
        DallasTemperature m_dallas;
};