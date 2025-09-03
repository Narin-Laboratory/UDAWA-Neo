#ifndef LOGGING_H
#define LOGGING_H

#include "params.h"
#include "UdawaLogger.h"
#include "UdawaSerialLogger.h"
#include "UdawaWiFiLogger.h"

struct AlarmMessage
{
    uint16_t code;
    uint8_t color; 
    int32_t blinkCount; 
    uint16_t blinkDelay;
};

extern UdawaLogger *logger;
extern UdawaSerialLogger *serialLogger;
#ifdef USE_WIFI_LOGGER
extern UdawaWiFiLogger *wiFiLogger;
#endif

void loggingSetup();

#endif