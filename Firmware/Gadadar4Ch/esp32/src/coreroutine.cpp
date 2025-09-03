#include "coreroutine.h"

void coreroutineSetup(){
    
}

void coreroutineCrashStateTruthKeeper(uint8_t direction){
  JsonDocument doc;
  crashState.rtcp = millis();

  if(direction == 1 || direction == 3){
    crashStateConfig.load(doc);
    crashState.rtcp = doc[PSTR("rtcp")];
    crashState.crashCnt = doc[PSTR("crashCnt")];
    crashState.fSafeMode = doc[PSTR("fSafeMode")];
    crashState.lastRecordedDatetime = doc[PSTR("lastRecordedDatetime")];
  } 

   if(direction == 2 || direction == 3){
    doc[PSTR("rtcp")] = crashState.rtcp;
    doc[PSTR("crashCnt")] = crashState.crashCnt;
    doc[PSTR("fSafeMode")] = crashState.fSafeMode;
    doc[PSTR("lastRecordedDatetime")] = RTC.getEpoch();
    crashStateConfig.save(doc);
  }
}