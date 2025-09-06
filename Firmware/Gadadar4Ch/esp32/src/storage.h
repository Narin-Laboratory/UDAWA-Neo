#ifndef STORAGE_H
#define STORAGE_H

#include "main.h"

struct CrashState{
    unsigned long rtcp = 0;
    int crashCnt = 0;
    bool fSafeMode = false;
    unsigned long crashStateCheckTimer = millis();
    bool crashStateCheckedFlag = false;
    unsigned long plannedRebootTimer = millis();
    unsigned int plannedRebootCountDown = 0;
    bool fPlannedReboot = false;
    bool fRTCHwDetected = false;
    unsigned long lastRecordedDatetime = 0;
    unsigned long lastRecordedDatetimeSavedTimer = 0;
    bool fFSDownloading = false;
    bool fStartServices = false;
    bool fStopServices = false;
    bool fDoInit = false;
};

extern UdawaConfig config;
extern CrashState crashState;
extern GenericConfig crashStateConfig;

void storageSetup();

#endif