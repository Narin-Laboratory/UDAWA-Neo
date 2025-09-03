#ifndef NETWORKING_H
#define NETWORKING_H

#include "UdawaWiFiHelper.h"
#include <LittleFS.h>
#include "storage.h"

extern UdawaWiFiHelper wiFiHelper;

void networkingSetup();
void networkingOnWiFiConnected();
void networkingOnWiFiDisconnected();
void networkingOnWiFiGotIP();
void networkingOnWiFiAPNewClientIP();
void networkingOnWiFiAPStart();

#endif