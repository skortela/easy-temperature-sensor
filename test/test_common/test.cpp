/*
 Copyright (c) 2014-present PlatformIO <contact@platformio.org>

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
**/
#include <Arduino.h>
#include <sensormap.h>
#include <eepromStream.h>
#include <unity.h>

Sensormap sensormap;


// void setUp(void) {
// // set stuff up here
// }

// void tearDown(void) {
// // clean stuff up here
// }

void test_function_sensormap_add(void) {

    uint64_t addr1 = 0x0001000006000000ULL;
    uint64_t addr2 = 0x20010000060000ffULL;

    const char* KSensorname_1 = "testsensor_1";
    const char* KSensorname_2 = "testsensor_2";

    sensormap.addSensor(addr1, KSensorname_1);
    sensormap.addSensor(addr2, KSensorname_2);

    const char* pName = sensormap.getSensorName(addr1);

    TEST_ASSERT_EQUAL_STRING(KSensorname_1, pName);

    pName = sensormap.getSensorName(addr2);
    TEST_ASSERT_EQUAL_STRING(KSensorname_2, pName);

    uint64_t addr3  = addr2+1;

    pName = sensormap.getSensorName(addr3);
    TEST_ASSERT_EQUAL(NULL, pName);

    // TEST_ASSERT_EACH_EQUAL_CHAR_MESSAGE

}

void test_function_sensormap_delete() {

    const int KSensorCount = 10;
    for (int i= 0; i< KSensorCount; i++) {
        sensormap.addSensor(i, "test" + i);
    }

    for (int i= 0; i< KSensorCount; i++) {
        const char* pName = sensormap.getSensorName(i);
        TEST_ASSERT(pName != NULL);
    }

    // remove first
    TEST_ASSERT(sensormap.removeSensor(0) == true);
    for (int i= 0; i< KSensorCount; i++) {
        const char* pName = sensormap.getSensorName(i);
        if ( i== 0) {
            TEST_ASSERT(pName == NULL);
        }
        else {
            TEST_ASSERT(pName != NULL);
        }
    }

    // remove last
    TEST_ASSERT(sensormap.removeSensor(KSensorCount-1));
    for (int i= 0; i< KSensorCount; i++) {
        const char* pName = sensormap.getSensorName(i);
        if ( i== 0 || i==KSensorCount-1) {
            TEST_ASSERT(pName == NULL);
        }
        else {
            TEST_ASSERT(pName != NULL);
        }
    }

    // remove mid
    const int KSensorDel = 5;
    TEST_ASSERT(sensormap.removeSensor(KSensorDel));
    for (int i= 0; i< KSensorCount; i++) {
        const char* pName = sensormap.getSensorName(i);
        if ( i== 0 || i==KSensorCount-1 || i==KSensorDel) {
            TEST_ASSERT(pName == NULL);
        }
        else {
            TEST_ASSERT(pName != NULL);
        }
    }

    // remove again, expect to fail
    TEST_ASSERT(sensormap.removeSensor(KSensorDel) == false);

}

void test_function_sensormap_rename() {
    TEST_ASSERT(sensormap.rename(0, "test") == false);

    sensormap.addSensor(1, "test_1");
    TEST_ASSERT(sensormap.rename(1, "newname"))

    const char* pName = sensormap.getSensorName(1);
    TEST_ASSERT_EQUAL_STRING(pName, "newname");
}

void test_eeprom()
{
    void* pData = malloc(500);

    EepromStream writer;
    EepromStream reader;
    writer.setUnderlyingData(pData, 500);
    reader.setUnderlyingData(pData, 500);

    writer.writeInt8(0x01);
    writer.writeInt8(0x02);
    writer.writeInt8(0x03);
    writer.writeInt16(0x0405);


    writer.writeString("Jaakko Teppoo");
    writer.writeInt8(0x12);
    writer.writeString("Teppo");
    writer.writeInt32(0x03040506);
    writer.writeInt8(0x05);
    writer.writeInt8(0x00);
    writer.writeInt16(0x0102);

    writer.writeString("Tapio");
    writer.writeString("jep");
    
    //writer.writeInt64(0x0708090A0B0C0E0F);

/*
    UnityPrintNumberHex(reader.readInt8(), 2);
    UnityPrintNumberHex(reader.readInt8(), 2);
    UnityPrintNumberHex(reader.readInt8(), 2);
    UnityPrintNumberHex(reader.readInt8(), 2);
    UnityPrintNumberHex(reader.readInt8(), 2);
    UnityPrintNumberHex(reader.readInt8(), 2);
    */
    TEST_ASSERT_EQUAL_HEX8(0x01, reader.readInt8());
    TEST_ASSERT_EQUAL_HEX8(0x02, reader.readInt8());
    TEST_ASSERT_EQUAL_HEX8(0x03, reader.readInt8());
    TEST_ASSERT_EQUAL_HEX16(0x0405, reader.readInt16());


   TEST_ASSERT_EQUAL_STRING("Jaakko Teppoo", reader.readString());
   TEST_ASSERT_EQUAL_HEX8(0x12, reader.readInt8());
    TEST_ASSERT_EQUAL_STRING("Teppo", reader.readString());
    TEST_ASSERT_EQUAL_HEX8(0x03040506, reader.readInt32());

    TEST_ASSERT_EQUAL_HEX8(0x05, reader.readInt8());
    TEST_ASSERT_EQUAL_HEX8(0x00, reader.readInt8());
    TEST_ASSERT_EQUAL_HEX16(0x0102, reader.readInt16());
    
    //TEST_ASSERT_EQUAL_HEX64(0x0708090A0B0C0E0F, reader.readInt64());


    

    TEST_ASSERT_EQUAL_STRING("Tapio", reader.readString());
    TEST_ASSERT_EQUAL_STRING("jep", reader.readString());

}



void process()
{
    UNITY_BEGIN();
    RUN_TEST(test_function_sensormap_add);
    RUN_TEST(test_function_sensormap_delete);
    RUN_TEST(test_function_sensormap_rename);
    RUN_TEST(test_eeprom);
    UNITY_END();
}

#ifdef ARDUINO

#include <Arduino.h>
void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);
    process();
}

void loop() {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(500);
}

#else

int main(int argc, char **argv) {
    process();
    return 0;
}

#endif