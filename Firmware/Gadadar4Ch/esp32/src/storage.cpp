#include "storage.h"

UdawaConfig config(PSTR("/config.json"));
CrashState crashState;
GenericConfig crashStateConfig(PSTR("/crash.json"));

// New definitions
AppConfig appConfig;
AppState appState;
Relay relays[4] = {
    Relay(0, 0, 0, 0, 0, 0, 0, 0, false, "No label", 3600, 0, 0),
    Relay(1, 0, 0, 0, 0, 0, 0, 0, false, "No label", 3600, 0, 0),
    Relay(2, 0, 0, 0, 0, 0, 0, 0, false, "No label", 3600, 0, 0),
    Relay(3, 0, 0, 0, 0, 0, 0, 0, false, "No label", 3600, 0, 0)
};

GenericConfig appConfigGC(PSTR("/appConfig.json"));
GenericConfig appStateGC(PSTR("/appState.json"));
GenericConfig appRelaysGC(PSTR("/appRelays.json"));


void storageSetup(){
    logger->debug(PSTR(__func__), PSTR("Initializing LittleFS: %d\n"), config.begin());
    config.load();
    
    logger->setLogLevel((LogLevel)config.state.logLev);

    JsonDocument doc;
    appConfigGC.load(doc);
    storageConvertAppConfig(doc, true, true);
    doc.clear();
    appStateGC.load(doc);
    storageConvertAppState(doc, true, true);
    doc.clear();
    appRelaysGC.load(doc);
    storageConvertAppRelay(doc, true, true);
    doc.clear();
}

void storageConvertAppConfig(JsonDocument &doc, bool direction, bool load_defaults){
  if(direction){ // from doc to config
    if(!doc["s1tx"].isNull()) appConfig.s1tx = doc["s1tx"].as<uint8_t>(); else if(load_defaults) appConfig.s1tx = 33;
    if(!doc["s1rx"].isNull()) appConfig.s1rx = doc["s1rx"].as<uint8_t>(); else if(load_defaults) appConfig.s1rx = 32;
    if(!doc["intvWeb"].isNull()) appConfig.intvWeb = doc["intvWeb"].as<unsigned long>(); else if(load_defaults) appConfig.intvWeb = 1000;
    if(!doc["intvAttr"].isNull()) appConfig.intvAttr = doc["intvAttr"].as<unsigned long>(); else if(load_defaults) appConfig.intvAttr = 60000;
    if(!doc["intvTele"].isNull()) appConfig.intvTele = doc["intvTele"].as<unsigned long>(); else if(load_defaults) appConfig.intvTele = 300000;
    if(!doc["maxWatt"].isNull()) appConfig.maxWatt = doc["maxWatt"].as<int>(); else if(load_defaults) appConfig.maxWatt = 22000;
    if(!doc["relayON"].isNull()) appConfig.relayON = doc["relayON"].as<bool>(); else if(load_defaults) appConfig.relayON = true;
  }
  else{ // from config to doc
    doc[PSTR("s1tx")] = appConfig.s1tx;
    doc[PSTR("s1rx")] = appConfig.s1rx;
    doc[PSTR("intvWeb")] = appConfig.intvWeb;
    doc[PSTR("intvAttr")] = appConfig.intvAttr;
    doc[PSTR("intvTele")] = appConfig.intvTele;
    doc[PSTR("maxWatt")] = appConfig.maxWatt;
    doc[PSTR("relayON")] = appConfig.relayON;
  }
}

void storageConvertAppState(JsonDocument &doc, bool direction, bool load_defaults){
  if(direction){ // from doc to state
    if(!doc["fPanic"].isNull()) appState.fPanic = doc["fPanic"].as<bool>(); else if(load_defaults) appState.fPanic = false;
  }
  else{ // from state to doc
    doc[PSTR("fPanic")] = appState.fPanic;
  }
}

void storageConvertAppRelay(JsonDocument &doc, bool direction, bool load_defaults){
    if(direction){ // from doc to relays
        if (!doc["relays"].isNull()) {
            JsonArrayConst relays_arr = doc["relays"].as<JsonArrayConst>();
            for(uint8_t i = 0; i < 4; i++){
                if (i < relays_arr.size() && !relays_arr[i].isNull()) {
                    JsonObjectConst relay_obj = relays_arr[i].as<JsonObjectConst>();
                    if(!relay_obj["pin"].isNull()) relays[i].pin = relay_obj["pin"].as<uint8_t>(); else if(load_defaults) relays[i].pin = i;
                    if(!relay_obj["mode"].isNull()) relays[i].mode = relay_obj["mode"].as<uint8_t>(); else if(load_defaults) relays[i].mode = 0;
                    if(!relay_obj["wattage"].isNull()) relays[i].wattage = relay_obj["wattage"].as<uint16_t>(); else if(load_defaults) relays[i].wattage = 0;
                    if(!relay_obj["lastActive"].isNull()) relays[i].lastActive = relay_obj["lastActive"].as<unsigned long>(); else if(load_defaults) relays[i].lastActive = 0;
                    if(!relay_obj["lastChanged"].isNull()) relays[i].lastChanged = relay_obj["lastChanged"].as<unsigned long>(); else if(load_defaults) relays[i].lastChanged = 0;
                    if(!relay_obj["dutyCycle"].isNull()) relays[i].dutyCycle = relay_obj["dutyCycle"].as<uint8_t>(); else if(load_defaults) relays[i].dutyCycle = 0;
                    if(!relay_obj["dutyRange"].isNull()) relays[i].dutyRange = relay_obj["dutyRange"].as<unsigned long>(); else if(load_defaults) relays[i].dutyRange = 0;
                    if(!relay_obj["autoOff"].isNull()) relays[i].autoOff = relay_obj["autoOff"].as<unsigned long>(); else if(load_defaults) relays[i].autoOff = 0;
                    if(!relay_obj["state"].isNull()) relays[i].state = relay_obj["state"].as<bool>(); else if(load_defaults) relays[i].state = false;
                    if(!relay_obj["label"].isNull()) relays[i].label = relay_obj["label"].as<String>(); else if(load_defaults) relays[i].label = PSTR("No label");
                    if(!relay_obj["overrunInSec"].isNull()) relays[i].overrunInSec = relay_obj["overrunInSec"].as<uint16_t>(); else if(load_defaults) relays[i].overrunInSec = 3600;
                    if(!relay_obj["duration"].isNull()) relays[i].duration = relay_obj["duration"].as<unsigned long>(); else if(load_defaults) relays[i].duration = 0;
                    if(!relay_obj["datetime"].isNull()) relays[i].datetime = relay_obj["datetime"].as<unsigned long>(); else if(load_defaults) relays[i].datetime = 0;

                    if(!relay_obj["timers"].isNull()){
                        JsonArrayConst timers_arr = relay_obj["timers"].as<JsonArrayConst>();
                        for(uint8_t j = 0; j < maxTimers; j++){
                            if (j < timers_arr.size() && !timers_arr[j].isNull()) {
                                JsonObjectConst timer_obj = timers_arr[j].as<JsonObjectConst>();
                                if(!timer_obj["h"].isNull()) relays[i].timers[j].hour = timer_obj["h"].as<uint8_t>(); else if(load_defaults) relays[i].timers[j].hour = 0;
                                if(!timer_obj["i"].isNull()) relays[i].timers[j].minute = timer_obj["i"].as<uint8_t>(); else if(load_defaults) relays[i].timers[j].minute = 0;
                                if(!timer_obj["s"].isNull()) relays[i].timers[j].second = timer_obj["s"].as<uint8_t>(); else if(load_defaults) relays[i].timers[j].second = 0;
                                if(!timer_obj["d"].isNull()) relays[i].timers[j].duration = timer_obj["d"].as<unsigned long>(); else if(load_defaults) relays[i].timers[j].duration = 0;
                            } else if (load_defaults) {
                                relays[i].timers[j] = TimerConfig();
                            }
                        }
                    } else if (load_defaults) {
                        for(uint8_t j = 0; j < maxTimers; j++) relays[i].timers[j] = TimerConfig();
                    }
                } else if (load_defaults) {
                    relays[i] = Relay();
                }
            }
        } else if (load_defaults) {
            for(uint8_t i = 0; i < 4; i++) relays[i] = Relay();
        }
    }
    else{ // from relays to doc
    JsonArray _relays = doc[PSTR("relays")].to<JsonArray>();
        for(uint8_t i = 0; i < 4; i++){
      JsonObject relay = _relays.add<JsonObject>();
        relay[PSTR("pin")] = relays[i].pin;
        relay[PSTR("mode")] = relays[i].mode;
        relay[PSTR("wattage")] = relays[i].wattage;
        relay[PSTR("lastActive")] = relays[i].lastActive;
        relay[PSTR("lastChanged")] = relays[i].lastChanged;
        relay[PSTR("dutyCycle")] = relays[i].dutyCycle;
        relay[PSTR("dutyRange")] = relays[i].dutyRange;
        relay[PSTR("autoOff")] = relays[i].autoOff;
        relay[PSTR("state")] = relays[i].state;
        relay[PSTR("label")] = relays[i].label;
        relay[PSTR("overrunInSec")] = relays[i].overrunInSec;
        relay[PSTR("duration")] = relays[i].duration;
        relay[PSTR("datetime")] = relays[i].datetime;

      JsonArray timers = relay[PSTR("timers")].to<JsonArray>();
        for (uint8_t j = 0; j < maxTimers; j++) {
          JsonObject timer = timers.add<JsonObject>();
            timer[PSTR("h")] = relays[i].timers[j].hour;
            timer[PSTR("i")] = relays[i].timers[j].minute;
            timer[PSTR("s")] = relays[i].timers[j].second;
            timer[PSTR("d")] = relays[i].timers[j].duration;
        }
        }
    }
}