#ifndef main_h
#define main_h
#include "params.h"
#include <Arduino.h>
#include "UdawaLogger.h"
#include "UdawaSerialLogger.h"
#include "Udawa.h"
#include "UdawaConfig.h"
#include <time.h>
#include <PZEM004Tv30.h>
#include "PCF8575.h"

#ifdef USE_IOT
#include <WiFiClientSecure.h>
#include <Arduino_MQTT_Client.h>
#include <Provision.h>
#include <ThingsBoard.h>
#endif

#ifdef USE_IOT

#ifdef USE_IOT_SECURE
    WiFiClientSecure espClient;
#else
    WiFiClient espClient;
#endif

Arduino_MQTT_Client mqttClient(espClient);
Provision<> prov;
const std::array<IAPI_Implementation*, 1U> apis = {
    &prov
};


/* 2. Instantiate the ThingsBoardSized object with your custom logger and parameters */
ThingsBoardSized<UdawaThingsboardLogger> tb(
    mqttClient,                      // Your MQTT client instance
    IOT_MAX_MESSAGE_RECEIVE_SIZE,    // Max receive buffer size
    IOT_MAX_MESSAGE_SEND_SIZE,       // Max send buffer size
    IOT_DEFAULT_MAX_STACK_SIZE,      // Max stack allocation before using heap
    IOT_BUFFERING_SIZE,              // Buffering size for StreamUtils
    IOT_DEFAULT_MAX_RESPONSE_SIZE,   // Max size for deserializing responses
    apis.cbegin(), // Pass the begin iterator
    apis.cend()    // Pass the end iterator
);

#endif

struct Config {
    uint8_t s1tx = s1tx; //Neo 26, V3.1 33, V3 32
    uint8_t s1rx = s1rx; //Neo 25, V3.1 32, V3 4
    unsigned long intvWeb = intvWeb;
    unsigned long intvAttr = intvAttr;
    unsigned long intvTele = intvTele;
    int maxWatt = maxWatt;
    bool relayON = relayON;
};
Config config;

struct TimerConfig {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    unsigned long duration; // Duration in seconds

    TimerConfig() : hour(0), minute(0), second(0), duration(0) {}
    TimerConfig(uint8_t h, uint8_t m, uint8_t s, uint16_t d) : hour(h), minute(m), second(s), duration(d) {}
};

struct Relay {
    uint8_t pin;
    uint8_t mode;
    uint16_t wattage;
    unsigned long lastActive;
    unsigned long lastChanged;

    uint8_t dutyCycle;
    unsigned long dutyRange;
    unsigned long autoOff;
    bool state;

    String label;
    uint16_t overrunInSec;

    TimerConfig timers[maxTimers];

    unsigned long datetime;
    unsigned long duration;

    Relay(uint8_t pin, uint8_t mode, uint16_t wattage,  unsigned long lastActive, unsigned long lastChanged, 
        uint8_t dutyCycle, unsigned long dutyRange, unsigned long autoOff, bool state, String label, 
        uint16_t overrunInSec, unsigned long datetime, unsigned long duration) : pin(pin), 
        mode(mode), wattage(wattage), lastActive(lastActive), lastChanged(lastChanged), dutyCycle(dutyCycle), 
        dutyRange(dutyRange), autoOff(autoOff), state(state), label(label), overrunInSec(overrunInSec),
        datetime(datetime), duration(duration) {
            for (uint8_t i = 0; i < maxTimers; i++) {
                timers[i] = TimerConfig();
            }
        }
};
Relay relays[4] = {
    Relay(0, 0, 0, 0, 0, 0, 0, 0, false, PSTR("No label"), 3600, 0, 0),
    Relay(1, 0, 0, 0, 0, 0, 0, 0, false, PSTR("No label"), 3600, 0, 0),
    Relay(2, 0, 0, 0, 0, 0, 0, 0, false, PSTR("No label"), 3600, 0, 0),
    Relay(3, 0, 0, 0, 0, 0, 0, 0, false, PSTR("No label"), 3600, 0, 0)
};

struct State {
    bool fPowerSensor = false;
    bool fResetPowerSensor = false;
    bool fIOExtender = false;
    TaskHandle_t xHandlePowerSensor = NULL;
    BaseType_t xReturnedPowerSensor;
    unsigned long powerSensorTaskRoutineLastActivity = 0;
    TaskHandle_t xHandleRelayControl = NULL;
    BaseType_t xReturnedRelayControl;
    unsigned long relayControlTaskRoutineLastActivity = 0;
    bool fPanic = false;
    bool fsaveAppRelay = false;
    bool fSaveAppState = false;
    bool fsyncClientAttributes = false;
};
State state;

GenericConfig appConfig(PSTR("/appConfig.json"));
GenericConfig appState(PSTR("/appState.json"));
GenericConfig appRelay(PSTR("/appRelay.json"));

void loadAppConfig();
void saveAppConfig();
void convertAppConfig(JsonDocument &doc, bool direction);
void loadAppState();
void saveAppState();
void convertAppState(JsonDocument &doc, bool direction);
void loadAppRelay();
void saveAppRelay();
void convertAppRelay(JsonDocument &doc, bool direction);
void powerSensorTaskRoutine(void *arg);
void relayControlTaskRoutine(void *arg);
void setRelay(uint8_t index, bool output);

#ifdef USE_IOT
void tbRun();
void processProvisionResponse(const JsonDocument &data);
void provisionRequestTimedOut();
void onTbConnected();
#endif

#ifdef USE_LOCAL_WEB_INTERFACE
void _onWsEventMain(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
#endif


void _onSyncClientAttributesCallback(uint8_t direction);
void _onFSDownloadedCallback();

Udawa* udawa = Udawa::getInstance();
PCF8575 IOExtender(IOEXTENDER_ADDRESS);

/**
 * @brief UDAWA Common Alarm Code Definition
 * 110 Light sensor
 * 110 = The light sensor failed to initialize; please check the module integration and wiring.
 * 111 = The light sensor measurement is abnormal; please check the module integrity.
 * 112 = The light sensor measurement is showing an extreme value; please monitor the device's operation closely.
 *
 * 120 Weather sensor
 * 120 = The weather sensor failed to initialize; please check the module integration and wiring.
 * 121 = The weather sensor measurement is abnormal; The ambient temperature is out of range.
 * 122 = The weather sensor measurement is showing an extreme value; The ambient temperature is exceeding safe threshold; please monitor the device's operation closely.
 * 123 = The weather sensor measurement is showing an extreme value; The ambient temperature is less than safe threshold; please monitor the device's operation closely.
 * 124 = The weather sensor measurement is abnormal; The ambient humidity is out of range.
 * 125 = The weather sensor measurement is showing an extreme value; The ambient humidity is exceeding safe threshold; please monitor the device's operation closely.
 * 126 = The weather sensor measurement is showing an extreme value; The ambient humidity is below safe threshold; please monitor the device's operation closely.
 * 127 = The weather sensor measurement is abnormal; The barometric pressure is out of range.
 * 128 = The weather sensor measurement is showing an extreme value; The barometric pressure is more than safe threshold; please monitor the device's operation closely.
 * 129 = The weather sensor measurement is showing an extreme value; The barometric pressure is less than safe threshold; please monitor the device's operation closely.
 *
 * 130 SD Card
 * 130 = The SD Card failed to initialize; please check the module integration and wiring.
 * 131 = The SD Card failed to attatch; please check if the card is inserted properly.
 * 132 = The SD Card failed to create log file; please check if the card is ok.
 * 133 = The SD Card failed to write to the log file; please check if the card is ok.
 * * 140 AC Power sensor
 * 140 = The power sensor failed to initialize; please check the module integration and wiring.
 * 141 = The power sensor measurement is abnormal; The voltage reading is out of range.
 * 142 = The power sensor measurement is abnormal; The current reading is out of range.
 * 143 = The power sensor measurement is abnormal; The power reading is out of range.
 * 144 = The power sensor measurement is abnormal; The power factor and frequency reading is out of range.
 * 145 = The power sensor measurement is showing an overlimit; Please check the connected instruments.
 * * 150 Real Time Clock
 * 150 = The device timing information is incorrect; please update the device time manually. Any function that requires precise timing will malfunction!
 * * 210 Switch Relay
 * 210 = Switch number one is active, but the power sensor detects no power utilization. Please check the connected instrument to prevent failures.
 * 211 = Switch number two is active, but the power sensor detects no power utilization. Please check the connected instrument to prevent failures.
 * 212 = Switch number three is active, but the power sensor detects no power utilization. Please check the connected instrument to prevent failures.
 * 213 = Switch number four is active, but the power sensor detects no power utilization. Please check the connected instrument to prevent failures.
 * 214 = All switches are inactive, but the power sensor detects large power utilization. Please check the device relay module to prevent relay malfunction.
 * 215 = Switch numner one is active for more than safe duration!
 * 216 = Switch number two is active for more than safe duration!
 * 217 = Switch number three is active for more than safe duration!
 * 218 = Switch number four is active for more than safe duration! 
 *
 * 220 IOExtender
 * 220 = The IOExtender failed to initialize; please check the module integration and wiring.
 * */
#endif