#include "storage.h"

// Existing global variables
UdawaConfig config(PSTR("/config.json"));
CrashState crashState;
GenericConfig crashStateConfig(PSTR("/crash.json"));

// New global variables for application-specific data
AppConfig appConfig;
AppState appState;
GenericConfig appConfigGeneric(PSTR("/appConfig.json"));
GenericConfig appStateGeneric(PSTR("/appState.json"));

void storageSetup(){
    logger->debug(PSTR(__func__), PSTR("Initializing LittleFS: %d\n"), config.begin());
    config.load();
    
    logger->setLogLevel((LogLevel)config.state.logLev);

    // Load application-specific config and state
    JsonDocument doc;
    appConfigGeneric.load(doc);
    storageConvertAppConfig(doc, true); // from doc to struct
    doc.clear();
    appStateGeneric.load(doc);
    storageConvertAppState(doc, true); // from doc to struct
}

void storageSync(uint8_t direction, uint8_t source = 0) {
    JsonDocument doc;
    // Direction: 1 = from source to others, 2 = from memory to others
    // Source: 0 = memory, 1 = ws, 2 = iot, 3 = file

    if (direction == 1) { // from source to others
        if (source == 1) { // from ws
            // Data is already in memory from ws callback, now sync to iot and file
            storageConvertAppState(doc, false);
            iotSendAttr(doc);
            appStateGeneric.save(doc);
        } else if (source == 2) { // from iot
            // Data is already in memory from iot callback, now sync to ws and file
            storageConvertAppState(doc, false);
            wsBcast(doc);
            appStateGeneric.save(doc);
        }
    } else if (direction == 2) { // from memory to others
        // Sync AppConfig
        doc.clear();
        storageConvertAppConfig(doc, false);
        wsBcast(doc);
        iotSendAttr(doc);

        // Sync AppState
        doc.clear();
        storageConvertAppState(doc, false);
        wsBcast(doc);
        iotSendAttr(doc);
    }
}

void storageConvertAppConfig(JsonDocument &doc, bool direction) {
    if (direction) { // from doc to struct
        if (!doc[PSTR("s1tx")].isNull()) appConfig.s1tx = doc[PSTR("s1tx")]; else appConfig.s1tx = 33;
        if (!doc[PSTR("s1rx")].isNull()) appConfig.s1rx = doc[PSTR("s1rx")]; else appConfig.s1rx = 32;
        if (!doc[PSTR("intvWeb")].isNull()) appConfig.intvWeb = doc[PSTR("intvWeb")]; else appConfig.intvWeb = 1;
        if (!doc[PSTR("intvAttr")].isNull()) appConfig.intvAttr = doc[PSTR("intvAttr")]; else appConfig.intvAttr = 60;
        if (!doc[PSTR("intvTele")].isNull()) appConfig.intvTele = doc[PSTR("intvTele")]; else appConfig.intvTele = 300;
        if (!doc[PSTR("maxWatt")].isNull()) appConfig.maxWatt = doc[PSTR("maxWatt")]; else appConfig.maxWatt = 2200;
        if (!doc[PSTR("relayON")].isNull()) appConfig.relayON = doc[PSTR("relayON")]; else appConfig.relayON = false;
    } else { // from struct to doc
        doc[PSTR("s1tx")] = appConfig.s1tx;
        doc[PSTR("s1rx")] = appConfig.s1rx;
        doc[PSTR("intvWeb")] = appConfig.intvWeb;
        doc[PSTR("intvAttr")] = appConfig.intvAttr;
        doc[PSTR("intvTele")] = appConfig.intvTele;
        doc[PSTR("maxWatt")] = appConfig.maxWatt;
        doc[PSTR("relayON")] = appConfig.relayON;
    }
}

void storageConvertAppState(JsonDocument &doc, bool direction) {
    if (direction) { // from doc to struct
        if (!doc[PSTR("fPanic")].isNull()) appState.fPanic = doc[PSTR("fPanic")]; else appState.fPanic = false;

        JsonArray relaysArr = doc[PSTR("relays")].as<JsonArray>();
        for(uint8_t i = 0; i < maxRelays; i++) {
            JsonObject relayObj = relaysArr[i].as<JsonObject>();
            if (!relayObj.isNull()) {
                if(!relayObj[PSTR("pin")].isNull()) appState.relays[i].pin = relayObj[PSTR("pin")];
                if(!relayObj[PSTR("mode")].isNull()) appState.relays[i].mode = relayObj[PSTR("mode")];
                if(!relayObj[PSTR("wattage")].isNull()) appState.relays[i].wattage = relayObj[PSTR("wattage")];
                if(!relayObj[PSTR("dutyCycle")].isNull()) appState.relays[i].dutyCycle = relayObj[PSTR("dutyCycle")];
                if(!relayObj[PSTR("dutyRange")].isNull()) appState.relays[i].dutyRange = relayObj[PSTR("dutyRange")];
                if(!relayObj[PSTR("autoOff")].isNull()) appState.relays[i].autoOff = relayObj[PSTR("autoOff")];
                if(!relayObj[PSTR("label")].isNull()) appState.relays[i].label = relayObj[PSTR("label")].as<String>();
                if(!relayObj[PSTR("overrunInSec")].isNull()) appState.relays[i].overrunInSec = relayObj[PSTR("overrunInSec")];
                if(!relayObj[PSTR("datetime")].isNull()) appState.relays[i].datetime = relayObj[PSTR("datetime")];
                if(!relayObj[PSTR("duration")].isNull()) appState.relays[i].duration = relayObj[PSTR("duration")];

                JsonArray timersArr = relayObj[PSTR("timers")].as<JsonArray>();
                for(uint8_t j = 0; j < maxTimers; j++) {
                    JsonObject timerObj = timersArr[j].as<JsonObject>();
                    if (!timerObj.isNull()) {
                        if(!timerObj[PSTR("h")].isNull()) appState.relays[i].timers[j].hour = timerObj[PSTR("h")];
                        if(!timerObj[PSTR("i")].isNull()) appState.relays[i].timers[j].minute = timerObj[PSTR("i")];
                        if(!timerObj[PSTR("s")].isNull()) appState.relays[i].timers[j].second = timerObj[PSTR("s")];
                        if(!timerObj[PSTR("d")].isNull()) appState.relays[i].timers[j].duration = timerObj[PSTR("d")];
                    }
                }
            }
        }
    } else { // from struct to doc
        doc[PSTR("fPanic")] = appState.fPanic;
        JsonArray _relays = doc[PSTR("relays")].to<JsonArray>();
        for(uint8_t i = 0; i < maxRelays; i++){
            JsonObject relayObj = _relays.add<JsonObject>();
            relayObj[PSTR("pin")] = appState.relays[i].pin;
            relayObj[PSTR("mode")] = appState.relays[i].mode;
            relayObj[PSTR("wattage")] = appState.relays[i].wattage;
            relayObj[PSTR("lastActive")] = appState.relays[i].lastActive;
            relayObj[PSTR("lastChanged")] = appState.relays[i].lastChanged;
            relayObj[PSTR("dutyCycle")] = appState.relays[i].dutyCycle;
            relayObj[PSTR("dutyRange")] = appState.relays[i].dutyRange;
            relayObj[PSTR("autoOff")] = appState.relays[i].autoOff;
            relayObj[PSTR("state")] = appState.relays[i].state;
            relayObj[PSTR("label")] = appState.relays[i].label;
            relayObj[PSTR("overrunInSec")] = appState.relays[i].overrunInSec;
            relayObj[PSTR("duration")] = appState.relays[i].duration;
            relayObj[PSTR("datetime")] = appState.relays[i].datetime;

            JsonArray timers = relayObj[PSTR("timers")].to<JsonArray>();
            for (uint8_t j = 0; j < maxTimers; j++) {
                JsonObject timerObj = timers.add<JsonObject>();
                timerObj[PSTR("h")] = appState.relays[i].timers[j].hour;
                timerObj[PSTR("i")] = appState.relays[i].timers[j].minute;
                timerObj[PSTR("s")] = appState.relays[i].timers[j].second;
                timerObj[PSTR("d")] = appState.relays[i].timers[j].duration;
            }
        }
    }
}
void storageSetRelay(uint8_t index, bool output){
  if(index < maxRelays){
    if(output){
      // Assuming IOExtender is available globally or passed somehow
      // IOExtender.digitalWrite(appState.relays[index].pin, appConfig.relayON);
      appState.relays[index].state = true;
      appState.relays[index].lastActive = millis();
      appState.relays[index].lastChanged = millis();
      logger->debug(PSTR(__func__), PSTR("Relay %d is ON. %d was written to relay.\n"), index+1, appConfig.relayON);
    }
    else{
      // IOExtender.digitalWrite(appState.relays[index].pin, !appConfig.relayON);
      appState.relays[index].state = false;
      appState.relays[index].lastChanged = millis();
      logger->debug(PSTR(__func__), PSTR("Relay %d is OFF. %d was written to relay.\n"), index+1, !appConfig.relayON);
    }
    storageSync(1, 1); // Sync from memory to others
  }
}