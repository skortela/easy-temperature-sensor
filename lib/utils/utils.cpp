#include <utils.h>

#ifdef ARDUINO
    #include <Arduino.h>
#else
    #include <stddef.h>
    #include <cstring>
#endif


char* addressToStr(const uint8_t* deviceAddress, char* pBuffer)
{
    sprintf(pBuffer, "0x");
    int pos = 2;
    for (uint8_t i = 0; i < 8; i++)
    {
        sprintf(pBuffer + pos, "%02x", deviceAddress[i]);
        pos += 2;
    }
    pBuffer[pos] = '\0';
    return pBuffer;
}
char* addressToStr(uint64_t deviceAddress, char* pBuffer)
{
    uint8_t val[8];
    uint64toUInt8Array(deviceAddress, val);
    return addressToStr(val, pBuffer);
}

void uint64toUInt8Array(uint64_t deviceAddress, uint8_t* pUint8Array)
{
    memcpy(pUint8Array, &deviceAddress, sizeof(deviceAddress));
    
}
uint64_t arrayToUint64(const uint8_t* pUint8Array)
{
    uint64_t deviceAddress;
    memcpy(&deviceAddress, pUint8Array, sizeof(deviceAddress));
    //deviceAddress = *((uint64_t*)pUint8Array);
    return deviceAddress;
}

// rounds a number to 2 decimal places
// example: round(3.14159) -> 3.14
double round2(double value) {
   return (int)(value * 100 + 0.5) / 100.0;
}
// rounds a number to 1 decimal places
// example: round(3.14159) -> 3.1
double round1(double value) {
   return (int)(value * 10 + 0.5) / 10.0;
}