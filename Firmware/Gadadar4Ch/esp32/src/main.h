#ifndef MAIN_H
#define MAIN_H

#define countof(a) (sizeof(a) / sizeof(a[0]))

#include "params.h"
#include "secret.h"

#include "UdawaConfig.h"
#include "UdawaWiFiHelper.h"
#include "UdawaLogger.h"
#include "UdawaSerialLogger.h"
#include "UdawaWiFiLogger.h"
#include <LittleFS.h>
#include <NTPClient.h>
#include <ESP32Time.h>
#ifdef USE_I2C
#include <Wire.h>
#endif
#ifdef USE_HW_RTC
#include <ErriezDS3231.h>
#endif
#ifdef USE_WIFI_OTA
#include <ArduinoOTA.h>
#endif
#include <ESPmDNS.h>

#include "storage.h"
#include "logging.h"
#include "networking.h"
#include "coreroutine.h"

#endif