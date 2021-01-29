
#include <sensormap.h>

#ifdef ARDUINO
    #include <Arduino.h>
#else
    #include <stddef.h>
    #include <cstring>
#endif

Sensormap::Sensormap()
{
    m_pBegin = NULL;
    resetSearch();
}
Sensormap::~Sensormap()
{
    while (m_pBegin) {
        node_t* p = m_pBegin;
        m_pBegin = m_pBegin->next;
        delete p;
    }
}

void Sensormap::addSensor(uint64_t addr, const char* name, float offset)
{
    node_t* pNew = new node_t;
    pNew->addr = addr;
    pNew->pName = strdup(name);
    pNew->next = NULL;
    pNew->offset = offset;

    if (!m_pBegin)
        m_pBegin = pNew;
    else {
        node_t* p = m_pBegin;
        while (p->next)
            p = p->next;
        p->next = pNew;
    }
}
bool Sensormap::rename(uint64_t addr, const char* name)
{
    if (m_pBegin) {
        node_t* p = m_pBegin;
        while (p) {
            if (p->addr == addr) {
                delete p->pName;
                p->pName = strdup(name);
                return true;
            }
            else {
                p = p->next;
            }
        }
    }
    return false;
}
bool Sensormap::rename(const char* oldName, const char* newName)
{
    if (m_pBegin) {
        node_t* p = m_pBegin;
        while (p) {
            if (strcmp(p->pName, oldName) == 0) {
                delete p->pName;
                p->pName = strdup(newName);
                return true;
            }
            else {
                p = p->next;
            }
        }
    }
    return false;
}
const char* Sensormap::getSensorName(uint64_t addr) const
{
    if (m_pBegin) {
        node_t* p = m_pBegin;
        while (p) {
            if (p->addr == addr)
                return p->pName;
            else {
                p = p->next;
            }
        }
    }
    return NULL;
}
float Sensormap::getSensorOffset(uint64_t addr) const
{
    if (m_pBegin) {
        node_t* p = m_pBegin;
        while (p) {
            if (p->addr == addr)
                return p->offset;
            else {
                p = p->next;
            }
        }
    }
    return 0;
}
/*bool Sensormap::setSensorOffset(uint64_t addr, float offset)
{
    if (m_pBegin) {
        node_t* p = m_pBegin;
        while (p) {
            if (p->addr == addr) {
                p->offset = offset;
                return true;
            }
            else {
                p = p->next;
            }
        }
    }
    return false;
}*/
bool Sensormap::setSensorOffset(const char* pSensorName, float offset)
{
    if (m_pBegin) {
        node_t* p = m_pBegin;
        while (p) {
            if (strcmp(p->pName, pSensorName) == 0) {
                p->offset = offset;
                return true;
            }
            else {
                p = p->next;
            }
        }
    }
    return false;
}
bool Sensormap::removeSensor(uint64_t addr)
{
    if (m_pBegin) {
        node_t* prev = NULL;
        node_t* p = m_pBegin;
        while (p) {
            if (p->addr == addr) {
                node_t* pDel = p;
                if (!prev)
                    m_pBegin = m_pBegin->next;
                else
                    prev->next = p->next;

                delete pDel;
                return true;
            }

            prev = p;
            p = p->next;
        }
    }
    return false;
}
int Sensormap::count() const
{
    return 0;
}

// set search pointer to begin
void Sensormap::resetSearch()
{
    m_pSearchPos = m_pBegin;
}
// return next node, or null
const node_t* Sensormap::getNext()
{
    node_t* p = m_pSearchPos;
    if (p)
        m_pSearchPos = m_pSearchPos->next;
    
    return p;
}

node_t* Sensormap::begin() const
{
    return m_pBegin;
}