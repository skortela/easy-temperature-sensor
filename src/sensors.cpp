
#include "sensons.h"
#include "config.h"
#include "utils.h"

Sensors::Sensors()
{
    m_lastTemperatureRequestTime = 0;
    m_ready = false;
}
Sensors::~Sensors()
{

}

void Sensors::begin()
{
    m_oneWire.begin(ONE_WIRE_PIN);
    m_dallas.setOneWire(&m_oneWire);
    m_dallas.setResolution(12);

    m_dallas.requestTemperatures();
    m_hasTemperaturesRead = false;
    m_lastTemperatureRequestTime = millis();
}

void Sensors::loop()
{
    if (m_lastTemperatureRequestTime == 0 || (unsigned long)(millis() - m_lastTemperatureRequestTime) > 5000) {
        m_dallas.requestTemperatures();
        m_hasTemperaturesRead = false;
        m_lastTemperatureRequestTime = millis();
    }

    if (!m_hasTemperaturesRead && (unsigned long)(millis() - m_lastTemperatureRequestTime) > 750) {
        m_hasTemperaturesRead = true;
        DeviceAddress addr;
        node_t* pSensor = m_sensorNameMap.begin();
        while (pSensor) {
            uint64toUInt8Array(pSensor->addr, addr);
            pSensor->averageTemp.addValue(m_dallas.getTempC(addr));
            pSensor = pSensor->next;
        }
        if (!m_ready)
            m_ready = true;
    }
}

bool Sensors::isReady() const {
    return m_ready;
}

void Sensors::searchNewSensors(bool& newSensorsFound)
{
    Serial.println("initSensors");
    newSensorsFound = false;
    DeviceAddress addr;

    char buffer[10];
    char* pBuffer;

    m_oneWire.reset_search();
	while ( m_oneWire.search(addr)) {
		if (m_dallas.validAddress(addr)) {
            Serial.println("found sensor");
            uint64_t uint64Addr = arrayToUint64(addr);
            //Serial.print("uint64Addr: ");
            //    Serial.println(uint64Addr);

            const char* sensorName = m_sensorNameMap.getSensorName(uint64Addr);
            if (!sensorName)
            {
                Serial.println(" no name");
                //addressToStr(addr, addrStr);
                //sensorName = addrStr;
                //Serial.print("no sensorName: ");
                //Serial.println(sensorName);
                //Serial.print("addrStr: ");
                //Serial.println(addrStr);

                pBuffer = &buffer[0];

                sprintf(pBuffer, "sensor_%d", m_sensorNameMap.count()+1);

                Serial.print("new: ");
                Serial.println(pBuffer);
                // Add as new sensor
                m_sensorNameMap.addSensor(uint64Addr, pBuffer, 0);
                newSensorsFound = true;
            }
            else {
                Serial.println("got name");
            }
        }
	}
}

Sensormap* Sensors::sensormap()
{
    return &m_sensorNameMap;
}