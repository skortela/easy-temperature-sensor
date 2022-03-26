#ifndef SENSORMAP_H
#define SENSORMAP_H

#include <inttypes.h>
#include "averageTemperature.h"

typedef struct node {
    uint64_t addr;
    char* pName;
    float offset; // temperature offset
    AverageTemperature averageTemp;
    uint8_t busIndex;
    struct node * next;
} node_t;


class Sensormap {

    public:
        Sensormap();
        ~Sensormap();
        
        void addSensor(uint8_t busIndex, uint64_t addr, const char* name, float offset);
        bool rename(uint64_t addr, const char* name);
        bool rename(const char* oldName, const char* newName);
        // get sensorname of addr, or NULL if not found
        const char*  getSensorName(uint64_t addr) const;
        node_t* getSensorNode(uint64_t addr) const;
        float getSensorOffset(uint64_t addr) const;
        //bool setSensorOffset(uint64_t addr, float offset);
        bool setSensorOffset(const char* pSensorName, float offset);
        bool removeSensor(uint64_t addr);
        int count() const;

        // set search pointer to begin
        void resetSearch();
        // return next node, or null
        const node_t* getNext();
    
        node_t* begin() const;
    private:
        node_t* m_pBegin;
        node_t* m_pSearchPos;
};

#endif