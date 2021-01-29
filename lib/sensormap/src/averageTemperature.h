#ifndef AVERAGETEMPERATURE_H
#define AVERAGETEMPERATURE_H

#include <inttypes.h>

#define AVG_COUNT 6

class AverageTemperature {
    public:
        AverageTemperature();
        ~AverageTemperature();

        bool isValid() const;

        void addValue(float value);
        float value() const;

    private:
        int m_errorCount;
        float m_values[AVG_COUNT];
        int m_indexpos;
        bool m_isPopulated;
};

#endif