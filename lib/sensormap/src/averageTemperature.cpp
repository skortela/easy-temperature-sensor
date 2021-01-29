

#include "averageTemperature.h"
#include <DallasTemperature.h>

AverageTemperature::AverageTemperature() : m_errorCount(0), m_indexpos(0), m_isPopulated(false)
{}
AverageTemperature::~AverageTemperature()
{}

bool AverageTemperature::isValid() const
{
    return (m_isPopulated || m_indexpos > 0);
}

void AverageTemperature::addValue(float value)
{
    if (value == DEVICE_DISCONNECTED_C) {
        m_errorCount++;
        if (m_errorCount >= 5) {
            // too many errors
            m_indexpos = 0;
            m_isPopulated = false;
        }
        return;
    }
    else
        m_errorCount = 0;
    
    m_values[m_indexpos] = value;
    m_indexpos++;
    if (m_indexpos == AVG_COUNT) {
        m_indexpos = 0;
        m_isPopulated = true;
    }
}

float AverageTemperature::value() const
{
    if (!isValid())
        return DEVICE_DISCONNECTED_C;
    
    float sum(0);
    int valueCount;
    if (m_isPopulated)
        valueCount = AVG_COUNT;
    else
        valueCount = m_indexpos;
    
    for (int i=0; i<valueCount; i++)
        sum += m_values[i];
    return sum / valueCount;
}
