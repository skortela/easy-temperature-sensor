
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
    for (int i=0; i<ONE_WIRE_BUS_COUNT; i++) {
        m_oneWire[i].begin(KOneWirePin[i]);
        m_dallas[i].setOneWire(&m_oneWire[i]);
        m_dallas[i].setResolution(12);
        m_dallas[i].setWaitForConversion(false);
        m_dallas[i].requestTemperatures();
    }
    m_hasTemperaturesRead = false;
    m_lastTemperatureRequestTime = millis();
}

void Sensors::loop()
{
    if (m_lastTemperatureRequestTime == 0 || (unsigned long)(millis() - m_lastTemperatureRequestTime) > 5000) {
        for (int i=0; i<ONE_WIRE_BUS_COUNT; i++) {
            m_dallas[i].requestTemperatures();
        }
        m_hasTemperaturesRead = false;
        m_lastTemperatureRequestTime = millis();
    }

    if (!m_hasTemperaturesRead && (unsigned long)(millis() - m_lastTemperatureRequestTime) > 750) {
        m_hasTemperaturesRead = true;
        DeviceAddress addr;
        node_t* pSensor = m_sensorNameMap.begin();
        while (pSensor) {
            uint64toUInt8Array(pSensor->addr, addr);
            float temp(DEVICE_DISCONNECTED_C);
            if (pSensor->busIndex < ONE_WIRE_BUS_COUNT) {
                temp = m_dallas[pSensor->busIndex].getTempC(addr);
            }

            pSensor->averageTemp.addValue(temp);
            
            pSensor = pSensor->next;
        }
        if (!m_ready)
            m_ready = true;
    }
}

bool Sensors::isReady() const {
    return m_ready;
}

void searchSensors(OneWire* pOneWire, DallasTemperature* pDallas, uint8_t busIndex, Sensormap* pSensorMap, bool& newSensorsFound)
{
    DeviceAddress addr;

    char buffer[19];
    char* pBuffer;
    int foundCount = 0;

    pOneWire->reset_search();
	while ( pOneWire->search(addr)) {
		if (pDallas->validAddress(addr)) {
            foundCount++;
            uint64_t uint64Addr = arrayToUint64(addr);
            Serial.print("found sensor: ");
            Serial.println(uint64Addr, 16);
            
            node_t* pSensorNode = pSensorMap->getSensorNode(uint64Addr);
            if (!pSensorNode)
            {
                Serial.println(" no name");
                //addressToStr(addr, addrStr);
                //sensorName = addrStr;
                //Serial.print("no sensorName: ");
                //Serial.println(sensorName);
                //Serial.print("addrStr: ");
                //Serial.println(addrStr);

                pBuffer = &buffer[0];

                sprintf(pBuffer, "sensor_%d", pSensorMap->count()+1);

                Serial.print("new: ");
                Serial.println(pBuffer);
                // Add as new sensor
                pSensorMap->addSensor(busIndex, uint64Addr, pBuffer, 0);
                newSensorsFound = true;
            }
            else {

                Serial.print("name: ");
                Serial.println(pSensorNode->pName);
                // make sure busIndex is correct (user may have changed sensor to another bus)
                pSensorNode->busIndex = busIndex;
            }
        }
    }

    if (!foundCount) {
        Serial.print("Warning! Sensors not found from bus index: ");
        Serial.println(busIndex);
    }
}

void Sensors::searchNewSensors(bool& newSensorsFound)
{
    Serial.println("initSensors");
    newSensorsFound = false;

    for (int i=0; i<ONE_WIRE_BUS_COUNT; i++) {
        searchSensors(&m_oneWire[i], &m_dallas[i], i, &m_sensorNameMap, newSensorsFound);
    }
}

Sensormap* Sensors::sensormap()
{
    return &m_sensorNameMap;
}