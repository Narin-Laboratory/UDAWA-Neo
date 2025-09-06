#ifndef COREROUTINE_H
#define COREROUTINE_H

#include "main.h"

struct AlarmMessage
{
    uint16_t code;
    uint8_t color; 
    int32_t blinkCount; 
    uint16_t blinkDelay;
};


extern ESP32Time RTC;
#ifdef USE_HW_RTC
extern ErriezDS3231 hwRTC;
#endif
extern TaskHandle_t xHandleAlarm;
extern BaseType_t xReturnedAlarm;
extern QueueHandle_t xQueueAlarm;

void reboot(int countDown);
void coreroutineSetup();
void coreroutineDoInit();
void coreroutuneStartServices();
void coreroutineStopServices();
void coreroutineCrashStateTruthKeeper(uint8_t direction);
void coreroutineRTCUpdate(long ts);
void coreroutineSetAlarm(uint16_t code, uint8_t color, int32_t blinkCount, uint16_t blinkDelay);
static void coreroutineAlarmTaskRoutine(void *arg);
void coreroutineSetLEDBuzzer(uint8_t color, uint8_t isBlink, int32_t blinkCount, uint16_t blinkDelay);
#ifdef USE_WIFI_OTA
    void coreroutineOnWiFiOTAStart();
    void coreroutineOnWiFiOTAEnd();
    void coreroutineOnWiFiOTAProgress(unsigned int progress, unsigned int total);
    void coreroutineOnWiFiOTAError(ota_error_t error);
#endif

#endif