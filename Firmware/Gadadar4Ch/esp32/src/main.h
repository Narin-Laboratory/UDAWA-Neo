#ifndef main_h
#define main_h
#include <Arduino.h>
#include "UdawaLogger.h"
#include "UdawaSerialLogger.h"
#include "Udawa.h"
#include "UdawaConfig.h"
#include <time.h>
#include <PZEM004Tv30.h>
#include "PCF8575.h"

struct Config {
    uint8_t s1tx; //Neo 26, V3.1 33, V3 32
    uint8_t s1rx; //Neo 25, V3.1 32, V3 4
    unsigned long intvWeb;
    unsigned long intvAttr;
    unsigned long intvTele;
    int maxWatt;
    bool relayON;
};
Config config;

struct Relay {
    uint8_t pin;
    uint8_t mode;
    uint16_t wattage;
    unsigned long lastActive;

    uint8_t dutyCycle;
    unsigned long autoOff;
    bool state;

    String label;
    uint16_t overrunInSec;

    Relay(uint8_t pin, uint8_t mode, uint16_t wattage,  long lastActive, 
        uint8_t dutyCycle, unsigned long autoOff, bool state, String label, 
        uint16_t overrunInSec) : pin(pin), 
        mode(mode), wattage(wattage), lastActive(lastActive), dutyCycle(dutyCycle), 
        autoOff(autoOff), state(state), label(label), overrunInSec(overrunInSec) {}
};
Relay relays[4] = {
    Relay(0, 0, 0, 0, 0, 0, false, PSTR("No label"), 3600),
    Relay(1, 0, 0, 0, 0, 0, false, PSTR("No label"), 3600),
    Relay(2, 0, 0, 0, 0, 0, false, PSTR("No label"), 3600),
    Relay(3, 0, 0, 0, 0, 0, false, PSTR("No label"), 3600)
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
    bool fsyncClientAttributes = false;
};
State state;

GenericConfig appConfig(PSTR("/appConfig.json"));
GenericConfig appState(PSTR("/appState.json"));
GenericConfig appRelay(PSTR("/appRelay.json"));

PCF8575 IOExtender(IOEXTENDER_ADDRESS);

void loadAppConfig();
void saveAppConfig();
void loadAppState();
void saveAppState();
void loadAppRelay();
void saveAppRelay();
void powerSensorTaskRoutine(void *arg);
void relayControlTaskRoutine(void *arg);
void setRelay(uint8_t index, bool output);

#ifdef USE_LOCAL_WEB_INTERFACE
void _onWsEventMain(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
#endif
void _onSyncClientAttributesCallback(uint8_t direction);

/**
 * @brief UDAWA Common Alarm Code Definition
 *   110 Light sensor
 *      110 = The light sensor failed to initialize; please check the module integration and wiring.
 *      111 = The light sensor measurement is abnormal; please check the module integrity.
 *      112 = The light sensor measurement is showing an extreme value; please monitor the device's operation closely.
 *
 *   120 Weather sensor
 *      120 = The weather sensor failed to initialize; please check the module integration and wiring.
 *      121 = The weather sensor measurement is abnormal; The ambient temperature is out of range.
 *      122 = The weather sensor measurement is showing an extreme value; The ambient temperature is exceeding safe threshold; please monitor the device's operation closely.
 *      123 = The weather sensor measurement is showing an extreme value; The ambient temperature is less than safe threshold; please monitor the device's operation closely.
 *      124 = The weather sensor measurement is abnormal; The ambient humidity is out of range.
 *      125 = The weather sensor measurement is showing an extreme value; The ambient humidity is exceeding safe threshold; please monitor the device's operation closely.
 *      126 = The weather sensor measurement is showing an extreme value; The ambient humidity is below safe threshold; please monitor the device's operation closely.
 *      127 = The weather sensor measurement is abnormal; The barometric pressure is out of range.
 *      128 = The weather sensor measurement is showing an extreme value; The barometric pressure is more than safe threshold; please monitor the device's operation closely.
 *      129 = The weather sensor measurement is showing an extreme value; The barometric pressure is less than safe threshold; please monitor the device's operation closely.
 *
 *   130 SD Card
 *      130 = The SD Card failed to initialize; please check the module integration and wiring.
 *      131 = The SD Card failed to attatch; please check if the card is inserted properly.
 *      132 = The SD Card failed to create log file; please check if the card is ok.
 *      133 = The SD Card failed to write to the log file; please check if the card is ok.
 * 
 *   140 AC Power sensor
 *      140 = The power sensor failed to initialize; please check the module integration and wiring.
 *      141 = The power sensor measurement is abnormal; The voltage reading is out of range.
 *      142 = The power sensor measurement is abnormal; The current reading is out of range.
 *      143 = The power sensor measurement is abnormal; The power reading is out of range.
 *      144 = The power sensor measurement is abnormal; The power factor and frequency reading is out of range.
 *      145 = The power sensor measurement is showing an overlimit; Please check the connected instruments.
 * 
 *   150 Real Time Clock
 *      150 = The device timing information is incorrect; please update the device time manually. Any function that requires precise timing will malfunction!
 * 
 *   210 Switch Relay
 *      210 = Switch number one is active, but the power sensor detects no power utilization. Please check the connected instrument to prevent failures.
 *      211 = Switch number two is active, but the power sensor detects no power utilization. Please check the connected instrument to prevent failures.
 *      212 = Switch number three is active, but the power sensor detects no power utilization. Please check the connected instrument to prevent failures.
 *      213 = Switch number four is active, but the power sensor detects no power utilization. Please check the connected instrument to prevent failures.
 *      214 = All switches are inactive, but the power sensor detects large power utilization. Please check the device relay module to prevent relay malfunction.
 *      215 = Switch numner one is active for more than safe duration!
 *      216 = Switch number two is active for more than safe duration!
 *      217 = Switch number three is active for more than safe duration!
 *      218 = Switch number four is active for more than safe duration! 
 *
 *    220 IOExtender
 *      220 = The IOExtender failed to initialize; please check the module integration and wiring.
 * 
 */
#endif