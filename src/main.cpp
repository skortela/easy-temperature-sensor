#include <Arduino.h>
#include "config.h"
#include "appSettings.h"
#include "Ticker.h"
//#include <DNSServer.h>
//#include <ESP8266WebServer.h>


#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

//#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESPAsyncWiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include "wifimanagerparamsext.h"

#include <ESP8266WiFi.h>
#define MQTT_MAX_PACKET_SIZE 700
#include <PubSubClient.h>

#include <ArduinoOTA.h>
//#include <ESP8266httpUpdate.h>

#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <EEPROM.h>

//#include <OneWire.h>
//#include <DallasTemperature.h>

#include <sensormap.h>
#include <eepromStream.h>
#include <utils.h>
#include "haintegration.h"
#include "sensons.h"
#include "config.h"
#include "jsonutils.h"

#define ST(A) #A
#define STR(A) ST(A)

static const char KINDEX_HTML[] PROGMEM = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"></head><body><h1>%s</h1></body></html>";

AsyncWebServer server(80);
DNSServer dns;

//WiFiManager wifiManager;
AppSettings m_settings;

//flag for saving data
bool m_shouldSaveConfig = false;
bool m_haRegisterationNeeded = true;

Ticker m_led_ticker;

WiFiClient m_espClient;
PubSubClient m_mqttClient(m_espClient);


Sensors m_sensors;

unsigned long m_lastStatusUpdate;
unsigned long m_lastInfoUpdate;


void blink() {
    //toggle state
    int state = digitalRead(LED_BUILTIN);  // get the current state of GPIO1 pin
    digitalWrite(LED_BUILTIN, !state);     // set pin to the opposite state
}
void sendState();

void configModeCallback(AsyncWiFiManager*) {
    Serial.println("Entering config mode");
    m_led_ticker.attach_ms(KTickerIntervalWifiConfig, blink);
}
//void configModeCallback(WiFiManager*) {
//    Serial.println("Entering config mode");
//    m_led_ticker.attach_ms(KTickerIntervalWifiConfig, blink);
//}
//callback notifying us of the need to save config
void saveConfigCallback () {
    Serial.println("Should save config");
    m_shouldSaveConfig = true;
}


bool loadConfig() {
    EepromStream stream;
   
    stream.setUnderlyingData( const_cast<uint8_t*>(EEPROM.getConstDataPtr()), 150);
    
    int32_t i = stream.readInt32();
    Serial.println("conf int:");
    Serial.println(i);
    if (i != KConfigValidationMagic)
    {
        Serial.println("no config");
        // no existing configuration
        return false;
    }
    Serial.println("loading config");
    m_settings.clear();
    m_settings.setDeviceHostname(stream.readString());
    m_settings.m_mqtt_server = stream.readStringDup();
    m_settings.m_mqtt_port = stream.readInt32();
    m_settings.m_mqtt_user = stream.readStringDup();
    m_settings.m_mqtt_passw = stream.readStringDup();
    m_settings.print();

    Sensormap* pSensors = m_sensors.sensormap();
    while (stream.readInt8() == 1) { // first byte tells is there more devices (0 = no, 1 = yes)
        uint8_t busIndex = stream.readInt8();
        uint64_t addr = stream.readInt64();
        const char* pName = stream.readString();
        float offset = stream.readFloat();
        pSensors->addSensor(busIndex, addr, pName, offset);
    }
    return true;
}

void saveConfig() {
    Serial.println("saveConfig");
    /*
    Serial.print("arg count: ");  Serial.println(wifiManager.server->args());
    for (int i=0; i< wifiManager.server->args(); i++)
    {
        Serial.print("key: "); Serial.println(wifiManager.server->argName(i)); 
        Serial.print("value: "); Serial.println(wifiManager.server->arg(i)); 
    }

    delay(5000);

    wifiManager.resetSettings();
    ESP.reset();
    delay(1000);

    return;
    */
    EepromStream stream;
    stream.setUnderlyingData( EEPROM.getDataPtr(), 150);
    
    stream.writeInt32(KConfigValidationMagic);

    stream.writeString(m_settings.m_deviceHostname);
    stream.writeString(m_settings.m_mqtt_server);
    stream.writeInt32(m_settings.m_mqtt_port);
    stream.writeString(m_settings.m_mqtt_user);
    stream.writeString(m_settings.m_mqtt_passw);

    Sensormap* pSensors = m_sensors.sensormap();
    pSensors->resetSearch();
    const node_t* pSensorNode = pSensors->getNext();
    while(pSensorNode) {
        stream.writeInt8(1); // has more
        stream.writeInt8(pSensorNode->busIndex);
        stream.writeInt64(pSensorNode->addr);
        stream.writeString(pSensorNode->pName);
        stream.writeFloat(pSensorNode->offset);
        
        pSensorNode = pSensors->getNext();
    }
    stream.writeInt8(0); // no more 

    EEPROM.commit();
}

void resetConfig() {
    // clear first 100 bytes. it's enought to clear first int32 val, but erasing user sensitive data is more secure aproach.
    for (int i=0; i< 100; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
}

void checkResetButton()
{
    if (digitalRead(KResetPin) == LOW)
    {
        Serial.println("reset pressed");
        delay(50);
        unsigned long starttime = millis();
        while (digitalRead(KResetPin) == LOW)
        {
            delay(50);
            if ((unsigned long)(millis() - starttime) > 5000)
            {
                // 5 sec pressed, do reset settings and reboot
                Serial.println("Reset settings");
                Serial.flush();
                //turn off led
                digitalWrite(LED_BUILTIN, HIGH); // active LOW
                delay(2000);
                // turn on led
                digitalWrite(LED_BUILTIN, LOW);
                delay(100);
                
                resetConfig();

                //WiFiManager wifiManager;
                //wifiManager.resetSettings();
                WiFi.disconnect(true);
                
                ESP.reset();
                delay(1000);

            }
        }
        Serial.println("reset aborted");
    }
}


void initSensors() {
    bool newSensorsFound;
    m_sensors.searchNewSensors(newSensorsFound);
    if (newSensorsFound)
        saveConfig();
}

#ifdef ENABLE_OTA
void initOTA() {
    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    // ArduinoOTA.setHostname("myesp8266");

    Serial.print("FreeSketchSpace: ");
    Serial.println(ESP.getFreeSketchSpace());
    
#ifdef OTA_PASSWD
    // Require password
    ArduinoOTA.setPassword(STR(OTA_PASSWD));
#else
    #pragma message "Warning: no OTA password"
    Serial.println("Warning! no OTA password defined");
#endif

    ArduinoOTA.onStart([]() {
        m_mqttClient.disconnect();
        Serial.println("OTA updating");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("Update finished, rebooting");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        int prog = progress / (total / 100);
        Serial.printf("Progress: %u%%\r", prog);
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Update error [%u]: ", error);
        if (error == OTA_AUTH_ERROR) {Serial.println("Auth Failed");}
        else if (error == OTA_BEGIN_ERROR) {Serial.println("Begin Failed");}
        else if (error == OTA_CONNECT_ERROR) {Serial.println("Connect Failed");}
        else if (error == OTA_RECEIVE_ERROR) {Serial.println("Receive Failed");}
        else if (error == OTA_END_ERROR) {Serial.println("End Failed");}
        else {Serial.println("Unknown error");}
    });
    ArduinoOTA.begin();
}
#endif

void sendSensorsInfo() {
    Serial.println("sending info");
    char* jsonBuffer = createSensorsArr(m_sensors.sensormap());
    bool ok = m_mqttClient.publish(m_settings.topic("sensors").c_str(), jsonBuffer);
    free (jsonBuffer);
    if (!ok) {
        Serial.println("failed to publish!");
    }
}

void wifiSetup()
{
    char strPort[10];
    sprintf(strPort, "%d", m_settings.m_mqtt_port);

    // The extra parameters to be configured (can be either global or just in the setup)
    // After connecting, parameter.getValue() will get you the configured value
    // id/name placeholder/prompt default length

    AsyncWiFiManagerParameter custom_deviceHostname("clientname", "clientname", m_settings.m_deviceHostname, 40);

    AsyncWiFiManagerParameter custom_mqtt_server("server", "mqtt server", m_settings.m_mqtt_server, 40);
    AsyncWiFiManagerParameter custom_mqtt_port("port", "mqtt port", strPort, 6);
    //WiFiManagerParamInt custom_mqtt_port("port", "mqtt port", m_settings.m_mqtt_port, 6);
    AsyncWiFiManagerParameter custom_mqtt_user("username", "username", m_settings.m_mqtt_user, 40);
    AsyncWiFiManagerParameter custom_mqtt_passw("password", "password", m_settings.m_mqtt_passw, 40);


//const char* custom_radio_str = "<br/><label for='customfieldid'>Custom Field Label</label><input type='radio' name='customfieldid' value='1' checked> One<br><input type='radio' name='customfieldid' value='2'> Two<br><input type='radio' name='customfieldid' value='3'> Three";
 /*   const char* cc = "<select name='cars' id='cars'>\
  <option value='volvo'>Volvo</option>\
  <option value='saab'>Saab</option>\
  <option value='mercedes'>Mercedes</option>\
  <option value='audi'>Audi</option>\
</select>";
*/
    //WiFiManagerParamInt custom_onewire_pin("sensors_pin", "sensors pin", m_settings.m_oneWirePin, 4, cc);
    //WiFiManagerParameter custom_onewire_pin(cc);
    //m_led_ticker.interval(1000);
    //m_led_ticker.start();

    AsyncWiFiManager wifiManager(&server,&dns);

    wifiManager.setCustomHeadElement("<meta charset=\"UTF-8\">");

    //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    wifiManager.setAPCallback(configModeCallback);
    //set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    if (WiFi.SSID() != "")
    {
        Serial.println("Saved SSID found, set timeout for config portal");
        wifiManager.setConfigPortalTimeout(300); // 5 minutes timeout
    }

    //add all your parameters here
    
    wifiManager.addParameter(&custom_deviceHostname);
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_user);
    wifiManager.addParameter(&custom_mqtt_passw);

    //wifiManager.addParameter(&custom_onewire_pin);
   


    if(!wifiManager.autoConnect(m_settings.m_deviceHostname)) {
        Serial.println("failed to connect and hit timeout");
        //reset and try again, or maybe put it to deep sleep
        ESP.reset();
        delay(1000);
    }

    Serial.println("autoconnect continue");

     //wifiManager.addParameter(&custom_onewire_pin);
    /*if(wifiManager.server->hasArg("cars")) {
        String value = wifiManager.server->arg("cars");
        Serial.println("value: " + value);
    }*/

    


    

     //read updated parameters
     /*
    strcpy(m_mqtt_server, custom_mqtt_server.getValue());
    m_mqtt_port = atoi(custom_mqtt_port.getValue());
    strcpy(m_mqtt_user, custom_mqtt_user.getValue());
    strcpy(m_mqtt_passw, custom_mqtt_passw.getValue());
    */
    m_settings.clear();
    m_settings.setDeviceHostname(custom_deviceHostname.getValue());
    m_settings.m_mqtt_server = strdup(custom_mqtt_server.getValue());
    m_settings.m_mqtt_port = atoi(custom_mqtt_port.getValue());
    //m_settings.m_mqtt_port = custom_mqtt_port.getValue();
    m_settings.m_mqtt_user = strdup(custom_mqtt_user.getValue());
    m_settings.m_mqtt_passw = strdup(custom_mqtt_passw.getValue());

    //m_settings.m_oneWirePin = custom_onewire_pin.getValue();

    m_settings.print();


    if (m_shouldSaveConfig) {
        // Save custom params
        saveConfig();
        Serial.println("config saved");
    }

    bool ok = WiFi.hostname(m_settings.m_deviceHostname);
    if (!ok) {
        Serial.println("Failed to se hostname!");
    }


    Serial.println("Connected");
    Serial.println("local ip");
    Serial.println(WiFi.localIP());

    m_led_ticker.detach();
}

void mqtt_message_received(char* topic, byte* payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    // set null termination as it may be missing
    payload[length] = 0;
    const char* strPayload = (const char*) payload;
    Serial.println(strPayload);

    size_t len = strlen(m_settings.m_commandListen) -1; // remove last '#' char
    if (strlen(topic) <= len)
        return;


    const char* topicCmd = topic + len;
    //char* pos = strrchr(topic, '/');
    //if (!pos)
    //    return;

    //pos++;

    char* commandEndPos = strrchr(topicCmd, '/');
    int commandLen;
    if (!commandEndPos)
        commandLen = strlen(topicCmd);
    else
        commandLen = commandEndPos-topicCmd-1;

    
    Serial.println(topicCmd);

    Serial.println(commandLen);



    if (strncmp(topicCmd, "sensors", commandLen) == 0) {
        sendSensorsInfo();
    }
    else if (strncmp(topicCmd, "rename", commandLen) == 0) {
        char* pos = strrchr(topic, '/');
        if (!pos)
            return;
        const char* oldName = pos+1;
        Serial.print("old name: "); Serial.print(oldName);
        Serial.print("new name: "); Serial.print(strPayload);
        
        Sensormap* pSensormap = m_sensors.sensormap();
        bool ok = pSensormap->rename(oldName, strPayload);
        if (ok) {
            Serial.print("sensor renamed [");
            Serial.print(oldName);
            Serial.print("] -> ["); 
            Serial.print(strPayload);
            Serial.println("]");
            saveConfig();
            sendSensorRegisterationConfig(&m_mqttClient, &m_settings, pSensormap);
        }
        else {
            Serial.print("rename failed, sensor not found [");
            Serial.print(oldName);
            Serial.println("]");
        }
        m_lastStatusUpdate = 0;
        
    }
    else if (strncmp(topicCmd, "set_offset", commandLen) == 0) {
        char* pos = strrchr(topic, '/');
        if (!pos)
            return;
        const char* pSensorname = pos+1;
        float offset = atof(strPayload);
        bool ok = m_sensors.sensormap()->setSensorOffset(pSensorname, offset);
        if (ok) {
            Serial.print("sensor offset changed [");
            Serial.print(pSensorname);
            Serial.print("] -> ["); 
            Serial.print(offset);
            Serial.println("]");
            saveConfig();
            sendSensorsInfo();
            m_lastStatusUpdate = 0;
        }
    }
    else if (strncmp(topicCmd, "restart", commandLen) == 0) {
        Serial.println("Restart...");
        ESP.restart();
    }
    else
        m_lastStatusUpdate = 0;
}
/*
char* addressToStr(DeviceAddress deviceAddress, char* pBuffer)
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
}*/
void printAddress(DeviceAddress deviceAddress)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (deviceAddress[i] < 16) Serial.print("0");
        Serial.print(deviceAddress[i], HEX);
    }
}

char* infoJson()
{
    StaticJsonDocument<800> doc;
    JsonObject root = doc.to<JsonObject>();

    root["ip"] = WiFi.localIP().toString();
    root["mac"] = WiFi.macAddress();
    root["hostname"] = WiFi.hostname();

    addSensorsObj(m_sensors.sensormap(), root);

    char* jsonBuffer = (char*)malloc(800);
    serializeJson(root, jsonBuffer, 800);
    return jsonBuffer;
}

void sendInfo() {
    char* pJsonBuffer = infoJson();
    Serial.println("sending info");
    //Serial.println(pJsonBuffer);
    m_mqttClient.publish(m_settings.m_infoTopic, pJsonBuffer);
    free(pJsonBuffer);
}

char* getConfig() {
    StaticJsonDocument<300> doc;
    JsonObject root = doc.to<JsonObject>();

    root["hostname"] = m_settings.m_deviceHostname;
    root["mqtt_server"] = m_settings.m_mqtt_server;
    root["mqtt_port"] = m_settings.m_mqtt_port;
    root["mqtt_user"] = m_settings.m_mqtt_user;
    root["availability_topic"] = m_settings.m_availabilityTopic;

    char* pJsonBuffer = (char*) malloc(300);
    serializeJson(root, pJsonBuffer, 300);
    return pJsonBuffer;
}

char* getStatusJson() {
    StaticJsonDocument<500> doc;
    JsonObject root = doc.to<JsonObject>();

    const node_t* pSensor = m_sensors.sensormap()->begin();
    while(pSensor) {
        if (pSensor->averageTemp.isValid()) {
            root[pSensor->pName] = pSensor->averageTemp.value() + pSensor->offset;
        }
        else {
            root[pSensor->pName] = nullptr;
        }

        pSensor = pSensor->next;
    }
#ifdef nodef
    oneWire.reset_search();
	while ( oneWire.search(addr)) {
		if (sensors.validAddress(addr)) {


            uint64_t uint64Addr = arrayToUint64(addr);//  *((uint64_t*)&addr[0]);
            //Serial.print("uint64Addr: ");
            //    Serial.println(uint64Addr);
            const char* sensorName = m_sensorNameMap.getSensorName(uint64Addr);
            if (!sensorName)
            {
                addressToStr(addr, addrStr);
                sensorName = addrStr;
                Serial.print("no sensorName: ");
                Serial.println(sensorName);
                Serial.print("addrStr: ");
                Serial.println(addrStr);
                // Add as new sensor
                m_sensorNameMap.addSensor(uint64Addr, sensorName, 0);
            }
            else {
                addressToStr(addr, addrStr);
                Serial.print("sensorName: ");
                Serial.println(sensorName);
                Serial.print("addrStr: ");
                Serial.println(addrStr);
                
            }

            char keyname[30];
            strncpy(keyname, sensorName, sizeof(keyname));

            float temperature = sensors.getTempC(addr);
            if (temperature != DEVICE_DISCONNECTED_C) {
                float offset = m_sensorNameMap.getSensorOffset(uint64Addr);
                //root[keyname] = round2(temperature + offset);
                root[keyname] = temperature + offset;
            }
            else {
                root[keyname] = nullptr;
            }
        }
	}
    #endif
    char* pJsonBuffer = (char*) malloc(500);
    serializeJson(root, pJsonBuffer, 500);
    return pJsonBuffer;
}

void sendState() {
    char* pJsonBuffer = getStatusJson();
    
    Serial.println("sending state");
    Serial.println(pJsonBuffer);

    m_mqttClient.publish(m_settings.m_stateTopic, pJsonBuffer);
    free(pJsonBuffer);
}

void mqtt_reconnect() {
    m_led_ticker.attach_ms(KTickerIntervalMQTTConnect, blink);
    // Loop until we're reconnected
    while (!m_mqttClient.connected()) {

        if (!m_settings.isValid()) {
            //Serial.println("Settings are not valid!");
            return;
        }
        m_mqttClient.setServer(m_settings.m_mqtt_server, m_settings.m_mqtt_port);

        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "temperaturemqtt-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect

/*
        Serial.println(clientId.c_str());
        Serial.println(m_settings.m_deviceHostname);
        Serial.println(m_settings.m_mqtt_server);
        Serial.println(m_settings.m_mqtt_port);
        Serial.println(m_settings.m_mqtt_user);
        Serial.println(m_settings.m_mqtt_passw);
        Serial.println(m_settings.m_availabilityTopic);
*/

        if (m_mqttClient.connect(clientId.c_str(), m_settings.m_mqtt_user, m_settings.m_mqtt_passw, m_settings.m_availabilityTopic, MQTTQOS0, true, KOffline)) {
            Serial.println("connected");
            // Once connected, publish an announcement...
            m_mqttClient.publish(m_settings.m_availabilityTopic, KOnline, true);
            // ... and resubscribe
            m_mqttClient.subscribe(m_settings.m_commandListen);

            if (m_haRegisterationNeeded)
            {
                sendSensorRegisterationConfig(&m_mqttClient, &m_settings, m_sensors.sensormap());
            }
        } else {
            Serial.print("failed, rc=");
            Serial.print(m_mqttClient.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying

            // 50 x 100ms = 5 sec
            for (int i=0; i<50; i++) {
                // Make sure user is able to reset
                checkResetButton();
                delay(100);
            }
        }
    }
    m_led_ticker.detach();
}

void setup() {
    Serial.begin(115200);
    /*for (int i=0; i< 20;i++) {
        Serial.println("starting..");
        delay(100);
        yield();
    }*/
    Serial.println("\n\nStarted");

    EEPROM.begin(300);
    
    //resetConfig();

    //WiFiManager wifiManager;
    //wifiManager.resetSettings();

    m_lastStatusUpdate = 0;
    m_lastInfoUpdate = 0;

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(KResetPin, INPUT_PULLUP);

    checkResetButton();

    m_sensors.begin();

    bool hasConfig = loadConfig();
    
    if (!hasConfig && WiFi.SSID() != "") {
        // does not have valid config, but has saved SSID?
        // Misconfigured, reset settings

        Serial.print("Config error, reset!");
        resetConfig();

        AsyncWiFiManager wifiManager(&server,&dns);
        wifiManager.resetSettings();
        
        ESP.reset();
        delay(1000);
    }
    initSensors();

    //delay(2000);

    Serial.print("last used SSID: ");
    Serial.println(WiFi.SSID());

#ifdef WIFI_SSID

    wl_status_t st = WiFi.begin(STR(WIFI_SSID),STR(WIFI_PW));
    Serial.print("wl_status_t: ");
    Serial.println(st);
    Serial.print("Connecting to: ");
    Serial.println(STR(WIFI_SSID));
    // Keep checking the connection status until it is connected
    while (WiFi.status() != WL_CONNECTED) {
        delay(10);
        yield();
    }
    Serial.print("Connected, local ip: ");
    Serial.println(WiFi.localIP());
    WiFi.hostname(m_settings.m_deviceHostname);
#else
    wifiSetup();
#endif

    m_mqttClient.setBufferSize(800);
    m_mqttClient.setCallback(mqtt_message_received);

#ifdef ENABLE_OTA
    initOTA();
#endif

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        //char* pBuffer = new char[300];
        //strcpy(pBuffer, KINDEX_HTML);
        
        //request->send_P(200, "text/html", KINDEX_HTML);

        //delete pBuffer;
        //Serial.println("done sending");
        //Serial.flush();

        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->printf_P(KINDEX_HTML, m_settings.m_deviceHostname);
        //send the response last
        request->send(response);
    });
    server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request){
        char* pJsonBuffer = infoJson();
        request->send(200, "application/json", pJsonBuffer);
        free(pJsonBuffer);
    });
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
        char* pJsonBuffer = getStatusJson();
        request->send(200, "application/json", pJsonBuffer);
        free(pJsonBuffer);
    });
    server.on("/sensors", HTTP_GET, [](AsyncWebServerRequest *request){
        char* pJsonBuffer = createSensorsArr(m_sensors.sensormap());
        Serial.println("sending sensors");
        //Serial.println(pJsonBuffer);
        request->send(200, "application/json", pJsonBuffer);
        free(pJsonBuffer);
    });
    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
        char* pJsonBuffer = getConfig();
        request->send(200, "application/json", pJsonBuffer);
        free(pJsonBuffer);
    });

    /*AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/set", [](AsyncWebServerRequest *request, JsonVariant &json) {
        const JsonObject& jsonObj = json.as<JsonObject>();
        if (jsonObj.containsKey("main_power")) {
            setMainPower(jsonObj["main_power"]);
        }
        if (jsonObj.containsKey("water_wanted")) {
            m_settings.m_temperature_mode_normal = jsonObj["water_wanted"];
            saveConfig();
        }

        char* jsonBuffer = new char[300];
        getStatus(jsonBuffer);
        request->send(200, "application/json", jsonBuffer);
        delete jsonBuffer;
    });
    server.addHandler(handler);*/

    server.onNotFound( [](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    });
    server.begin();

    delay(1000);
    Serial.println("Started!");
    digitalWrite(D4, HIGH);
}

void loop() {
    m_sensors.loop();
#ifdef ENABLE_OTA
    ArduinoOTA.handle();
#endif
    checkResetButton();

    if (!m_mqttClient.connected()) {
        mqtt_reconnect();
        digitalWrite(LED_BUILTIN, LOW); // Turn led on
    }
    m_mqttClient.loop();

    unsigned long now = millis();
    
    if (m_lastInfoUpdate == 0 || (unsigned long) (now - m_lastInfoUpdate) > KInfoUpdateInterval) {
        // send info update
        sendInfo();
        m_lastInfoUpdate = now;
    }

    if (m_sensors.isReady() && (m_lastStatusUpdate == 0 || (unsigned long) (now - m_lastStatusUpdate) > KStatusUpdateInterval)) {
        // send status update
        sendState();
        m_lastStatusUpdate = now;
    }
}
