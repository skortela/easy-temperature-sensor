#ifndef UTILS_H
#define UTILS_H

#include <inttypes.h>

char* addressToStr(const uint8_t* deviceAddress, char* pBuffer);
char* addressToStr(uint64_t deviceAddress, char* pBuffer);

void uint64toUInt8Array(uint64_t deviceAddress, uint8_t* pUint8Array);
uint64_t arrayToUint64(const uint8_t* pUint8Array);

// rounds a number to 2 decimal places
// example: round(3.14159) -> 3.14
double round2(double value);
// rounds a number to 1 decimal places
// example: round(3.14159) -> 3.1
double round1(double value);

#endif