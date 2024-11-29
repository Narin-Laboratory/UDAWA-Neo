#include "main.h"

Udawa udawa;

void setup() {
  udawa.begin();

  loadAppConfig();
  loadAppState();
  loadAppRelay();

  udawa.addOnWsEvent(_onWsEventMain);
  udawa.addOnSyncClientAttributesCallback(_onSyncClientAttributesCallback);

  udawa.logger->verbose(PSTR(__func__), PSTR("Initial config.relayON value: %d\n"), config.relayON);
  
  if (!udawa.crashState.fSafeMode) {
    if(state.xHandlePowerSensor == NULL){
      state.xReturnedPowerSensor = xTaskCreatePinnedToCore(powerSensorTaskRoutine, PSTR("powerSensor"), POWERSENSOR_STACKSIZE, NULL, 1, &state.xHandlePowerSensor, 1);
      if(state.xReturnedPowerSensor == pdPASS){
        udawa.logger->warn(PSTR(__func__), PSTR("Task powerSensor has been created.\n"));
      }
    }

    if(state.xHandleRelayControl == NULL){
      state.xReturnedRelayControl = xTaskCreatePinnedToCore(relayControlTaskRoutine, PSTR("relayControl"), RELAYCONTROL_STACKSIZE, NULL, 1, &state.xHandleRelayControl, 1);
      if(state.xReturnedRelayControl == pdPASS){
        udawa.logger->warn(PSTR(__func__), PSTR("Task relayControl has been created.\n"));
      }
    }
  }
}

unsigned long timer = millis();
void loop() {
  udawa.run();

  if (!udawa.crashState.fSafeMode) {
    if (millis() - timer > 1000) {
      
      #ifdef USE_LOCAL_WEB_INTERFACE
      if(udawa.ws.count() > 0){
        JsonDocument doc;
        JsonObject sysInfo = doc[PSTR("sysInfo")].to<JsonObject>();

        sysInfo[PSTR("heap")] = ESP.getFreeHeap();
        sysInfo[PSTR("uptime")] = millis();
        sysInfo[PSTR("datetime")] = udawa.RTC.getDateTime();
        sysInfo[PSTR("rssi")] = udawa.wiFiHelper.rssiToPercent(WiFi.RSSI());

        udawa.wsBroadcast(doc);
      }
      #endif

      if(state.fsaveAppRelay){
        saveAppRelay();
        state.fsaveAppRelay = false;
      }

      if(state.fsyncClientAttributes){
        _onSyncClientAttributesCallback(3);
        state.fsyncClientAttributes = false;
      }

      timer = millis();
    }
  }
}

void loadAppConfig(){
  JsonDocument doc;
  bool status = appConfig.load(doc);
  udawa.logger->debug(PSTR(__func__), PSTR("%d\n"), (int)status);
  if(status){
    if(doc[PSTR("s1tx")] != nullptr){config.s1tx = doc[PSTR("s1tx")].as<uint8_t>();} else{config.s1tx = s1tx;}
    if(doc[PSTR("s1rx")] != nullptr){config.s1rx = doc[PSTR("s1rx")].as<uint8_t>();} else{config.s1rx = s1rx;}
    if(doc[PSTR("intvWeb")] != nullptr){config.intvWeb = doc[PSTR("intvWeb")].as<unsigned long>();} else{config.intvWeb = intvWeb;}
    if(doc[PSTR("intvAttr")] != nullptr){config.intvAttr = doc[PSTR("intvAttr")].as<unsigned long>();} else{config.intvAttr = intvAttr;}
    if(doc[PSTR("intvTele")] != nullptr){config.intvTele = doc[PSTR("intvTele")].as<unsigned long>();} else{config.intvTele = intvTele;}
    if(doc[PSTR("maxWatt")] != nullptr){config.maxWatt = doc[PSTR("maxWatt")].as<int>();} else{config.maxWatt = maxWatt;}
    if(doc[PSTR("relayON")] != nullptr){config.relayON = doc[PSTR("relayON")].as<bool>();} else{config.relayON = relayON;}
  }
}

void saveAppConfig(){
  JsonDocument doc;
  doc[PSTR("s1tx")] = config.s1tx;
  doc[PSTR("s1rx")] = config.s1rx;
  doc[PSTR("intvWeb")] = config.intvWeb;
  doc[PSTR("intvAttr")] = config.intvAttr;
  doc[PSTR("intvTele")] = config.intvTele;
  doc[PSTR("maxWatt")] = config.maxWatt;
  doc[PSTR("relayON")] = config.relayON;

  bool status = appConfig.save(doc);
  udawa.logger->debug(PSTR(__func__), PSTR("%d\n"), (int)status);
}

void loadAppState(){
  JsonDocument doc;
  bool status = appState.load(doc);
  udawa.logger->debug(PSTR(__func__), PSTR("%d\n"), (int)status);
  if(status){
    if(doc[PSTR("fPanic")] != nullptr){state.fPanic = doc[PSTR("fPanic")].as<bool>();} else{state.fPanic = false;}

  }
}

void saveAppState(){
  JsonDocument doc;

  doc[PSTR("fPanic")] = state.fPanic;

  bool status = appState.save(doc);
  udawa.logger->debug(PSTR(__func__), PSTR("%d\n"), (int)status);
}

void loadAppRelay(){
  JsonDocument doc;
  bool status = appRelay.load(doc);
  udawa.logger->debug(PSTR(__func__), PSTR("%d\n"), (int)status);
  if(status){
    for(uint8_t i = 0; i < countof(relays); i++){
      if(doc[PSTR("relays")][i] != nullptr){
        relays[i].pin = doc[PSTR("relays")][i][PSTR("pin")].as<uint8_t>();
        relays[i].mode = doc[PSTR("relays")][i][PSTR("mode")].as<uint8_t>();
        relays[i].wattage = doc[PSTR("relays")][i][PSTR("wattage")].as<uint16_t>();
        relays[i].lastActive = doc[PSTR("relays")][i][PSTR("lastActive")].as<unsigned long>();
        relays[i].dutyCycle = doc[PSTR("relays")][i][PSTR("dutyCycle")].as<uint8_t>();
        relays[i].autoOff = doc[PSTR("relays")][i][PSTR("autoOff")].as<unsigned long>();
        relays[i].state = doc[PSTR("relays")][i][PSTR("state")].as<bool>();
        relays[i].label = doc[PSTR("relays")][i][PSTR("label")].as<String>();
      }
    }
  }
}

void saveAppRelay(){
  JsonDocument doc;
  JsonArray _relays = doc[PSTR("relays")].to<JsonArray>();
  for(uint8_t i = 0; i < countof(relays); i++){
    _relays[i][PSTR("pin")] = relays[i].pin;
    _relays[i][PSTR("mode")] = relays[i].mode;
    _relays[i][PSTR("wattage")] = relays[i].wattage;
    _relays[i][PSTR("lastActive")] = relays[i].lastActive;
    _relays[i][PSTR("dutyCycle")] = relays[i].dutyCycle;
    _relays[i][PSTR("autoOff")] = relays[i].autoOff;
    _relays[i][PSTR("state")] = relays[i].state;
    _relays[i][PSTR("label")] = relays[i].label;
  }
  bool status = appRelay.save(doc);
  udawa.logger->debug(PSTR(__func__), PSTR("%d\n"), (int)status);
}

void powerSensorTaskRoutine(void *arg){
  float volt; float amp; float watt; float freq; float pf; float ener;  

  unsigned long timerTelemetry = millis();
  unsigned long timerAttribute = millis();
  unsigned long timerWebIface = millis();
  unsigned long timerAlarm = millis();

  while (true)
  {
    HardwareSerial PZEMSerial(1);
    PZEM004Tv30 PZEM(PZEMSerial, config.s1rx, config.s1tx);
    state.fPowerSensor = !isnan(PZEM.voltage());
    if(!state.fPowerSensor){
      udawa.logger->warn(PSTR(__func__), PSTR("Failed to initialize powerSensor!\n"));
    }

    bool fFailureReadings = false;
    if(state.fPowerSensor){
      volt = PZEM.voltage();
      amp = PZEM.current();
      watt = PZEM.power();
      freq = PZEM.frequency();
      pf = PZEM.pf();
      ener = PZEM.energy();
    }

    if(isnan(volt) || isnan(amp) || isnan(watt) || isnan(freq) || isnan(pf) ||
    isnan(ener) || volt < 0.0 || volt > 1000.0 || amp < 0.0 || amp > 100.0 || watt < .0 || 
    watt > 22000.0 || freq < 0.0 || freq > 100.0 || ener > 9999.0 ){
      fFailureReadings = true;
      udawa.logger->debug(PSTR(__func__), PSTR("Power sensor abnormal reading: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n"), volt, amp, watt, freq, pf, ener);
    }

    unsigned long now = millis();
    if(state.fPowerSensor && !fFailureReadings)
    {
      JsonDocument doc;
      String buffer;
      JsonObject powerSensor = doc[PSTR("powerSensor")].to<JsonObject>();

      #ifdef USE_LOCAL_WEB_INTERFACE
      if( (now - timerWebIface) > (config.intvWeb * 1000))
      {
        powerSensor[PSTR("volt")] = volt;
        powerSensor[PSTR("amp")] = amp;
        powerSensor[PSTR("watt")] = watt;
        powerSensor[PSTR("freq")] = freq;
        powerSensor[PSTR("pf")] = pf;
        powerSensor[PSTR("ener")] = ener;
        udawa.wsBroadcast(doc);
        doc.clear();

        timerWebIface = now;
      }
      #endif
      
      #ifdef USE_IOT
      if( (now - timerAttribute) > (config.intvAttr * 1000))
      {
        doc[PSTR("_volt")] = volt;
        doc[PSTR("_amp")] = amp;
        doc[PSTR("_watt")] = watt;
        doc[PSTR("_freq")] = freq;
        doc[PSTR("_pf")] = pf;
        doc[PSTR("_ener")] = ener;
        udawa.iotSendAttributes(doc);
        doc.clear();

        timerAttribute = now;
      }

      if( (now - timerTelemetry) > (config.intvTele * 1000) )
      {

        doc[PSTR("volt")] = volt;  
        doc[PSTR("amp")] = amp; 
        doc[PSTR("watt")] = watt;
        doc[PSTR("pf")] = pf; 
        doc[PSTR("freq")] = freq;
        doc[PSTR("ener")] = ener;
        #ifdef USE_DISK_LOG  
        writeCardLogger(doc);
        #endif
        udawa.iotSendTelemetry(doc);
        doc.clear();
      
        timerTelemetry = now;
      }
      #endif

    }
  
    if( (now - timerAlarm) > 30000 )
    {
      if(!state.fPowerSensor){udawa.setAlarm(140, 1, 5, 1000);}
      else{
        if(volt < 0 || volt > 1000){udawa.setAlarm(141, 1, 5, 1000);}
        if(amp < 0 || amp > 100){udawa.setAlarm(142, 1, 5, 1000);}
        if(watt < 0 || watt > config.maxWatt){udawa.setAlarm(143, 1, 5, 1000);}
        if(pf < 0 || pf > 100){udawa.setAlarm(144, 1, 5, 1000);}
        if(freq < 0 || freq > 100){udawa.setAlarm(144, 1, 5, 1000);}
        if(watt > config.maxWatt || volt > 275){udawa.setAlarm(145, 1, 5, 1000);}

        uint8_t activeRelayCounter = 0;
        for(uint8_t i = 0; i < 4; i++){
          if(relays[i].state == true &&
          watt < 6.0){udawa.setAlarm(210+i, 1, 5, 1000);}

          if(relays[i].state == true){activeRelayCounter++;}

          if( relays[i].state == true && 
            (millis() - relays[i].lastActive) > relays[i].overrunInSec * 1000 && relays[i].overrunInSec != 0){
              udawa.setAlarm(215+i, 1, 5, 1000);
            }
        }

        if(activeRelayCounter == 0 && watt > 6){udawa.setAlarm(214, 1, 5, 1000);}    
      }

      
      timerAlarm = now;
    }

    if(state.fResetPowerSensor){
      state.fResetPowerSensor = false;
      udawa.logger->warn(PSTR(__func__), PSTR("Resetting power sensor.\n"));
      PZEM.resetEnergy();
    }

    state.powerSensorTaskRoutineLastActivity = millis();
    vTaskDelay((const TickType_t) 1000 / portTICK_PERIOD_MS);
  }
}

void relayControlTaskRoutine(void *arg){  
  for(uint8_t i = 0; i < countof(relays); i++){
    IOExtender.pinMode(relays[i].pin, OUTPUT);
    udawa.logger->verbose(PSTR(__func__), PSTR("Relay %d initialized as output.\n"), relays[i].pin);
  }

  state.fIOExtender =  IOExtender.begin();

  if(!state.fIOExtender){
    udawa.logger->error(PSTR(__func__), PSTR("Failed to initialize IOExtender!\n"));
  }
  else{
    udawa.logger->verbose(PSTR(__func__), PSTR("IOExtender initialized.\n"));
  }


  unsigned long timerAlarm = millis();
  while (true)
  {
    if(!state.fIOExtender){
      for(uint8_t i = 0; i < countof(relays); i++){
        IOExtender.pinMode(relays[i].pin, OUTPUT);
      }
      state.fIOExtender = IOExtender.begin();
      udawa.logger->error(PSTR(__func__), PSTR("Failed to initialize IOExtender!\n"));

      vTaskDelay((const TickType_t) 1000 / portTICK_PERIOD_MS);
      continue;
    }

    vTaskDelay((const TickType_t) 1000 / portTICK_PERIOD_MS);
    if(state.fPanic){continue;}
    unsigned long now = millis();

    /* Start alarm section */
    if(now - timerAlarm > 30000){
      if(!state.fIOExtender){
        udawa.setAlarm(220, 1, 10, 500);
      }
      
      timerAlarm = now;
    }
    /* End alarm section */

    /* Start Manual Control Mode*/
    for(uint8_t i = 0; i < countof(relays); i++){
      if(relays[i].mode == 0 && relays[i].autoOff != 0){
        if( (now - relays[i].lastActive) >= ( relays[i].autoOff * 1000 ) && relays[i].state == config.relayON){
          setRelay(i, !config.relayON);
        } 
      }
    }
    /* End Manual Control Mode*/

    state.relayControlTaskRoutineLastActivity = millis();
  }
}

void setRelay(uint8_t index, bool output){
  udawa.logger->debug(PSTR(__func__), PSTR("config.relayON value: %d\n"), config.relayON);
  if(index < countof(relays)){
    if(output){
      IOExtender.digitalWrite(relays[index].pin, config.relayON);
      relays[index].state = true;
      relays[index].lastActive = millis();
      udawa.logger->debug(PSTR(__func__), PSTR("Relay %d is ON. %d was written to relay.\n"), index+1, config.relayON);
    
      state.fsyncClientAttributes = true;
    }
    else{
      IOExtender.digitalWrite(relays[index].pin, !config.relayON);
      relays[index].state = false;
      udawa.logger->debug(PSTR(__func__), PSTR("Relay %d is OFF. %d was written to relay.\n"), index+1, !config.relayON);
      state.fsyncClientAttributes = true;
    }
  }
}

#ifdef USE_LOCAL_WEB_INTERFACE
void _onWsEventMain(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    // Handle WebSocket connection event
  } else if (type == WS_EVT_DATA) {
    // Handle WebSocket data event
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data);
    String incomming;
    serializeJson(doc, incomming);
    udawa.logger->debug(PSTR(__func__), PSTR("Incoming: %s\n"), incomming.c_str());

    if(doc[PSTR("setRelayState")] != nullptr){
      if(doc[PSTR("setRelayState")][PSTR("pin")] != nullptr && doc[PSTR("setRelayState")][PSTR("state")] != nullptr){
        setRelay(doc[PSTR("setRelayState")][PSTR("pin")].as<uint8_t>(), doc[PSTR("setRelayState")][PSTR("state")].as<bool>());
      }
    }
    else if(doc[PSTR("resetPowerSensor")] != nullptr){
      state.fResetPowerSensor = true;
    }
    else if(doc[PSTR("setRelay")] != nullptr && doc[PSTR("setRelay")][PSTR("relay")] != nullptr && doc[PSTR("setRelay")][PSTR("index")] != nullptr){
      uint8_t index = doc[PSTR("setRelay")][PSTR("index")].as<uint8_t>();
      if(index < countof(relays)){
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("pin")] != nullptr){relays[index].pin = doc[PSTR("setRelay")][PSTR("relay")][PSTR("pin")].as<uint8_t>();} 
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("pin")] != nullptr){relays[index].mode = doc[PSTR("setRelay")][PSTR("relay")][PSTR("mode")].as<uint8_t>();}
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("pin")] != nullptr){relays[index].wattage = doc[PSTR("setRelay")][PSTR("relay")][PSTR("wattage")].as<uint16_t>();}
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("pin")] != nullptr){relays[index].lastActive = doc[PSTR("setRelay")][PSTR("relay")][PSTR("lastActive")].as<unsigned long>();}
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("pin")] != nullptr){relays[index].dutyCycle = doc[PSTR("setRelay")][PSTR("relay")][PSTR("dutyCycle")].as<uint8_t>();}
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("pin")] != nullptr){relays[index].autoOff = doc[PSTR("setRelay")][PSTR("relay")][PSTR("autoOff")].as<unsigned long>();}
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("pin")] != nullptr){relays[index].state = doc[PSTR("setRelay")][PSTR("relay")][PSTR("state")].as<bool>();}
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("pin")] != nullptr){relays[index].label = doc[PSTR("setRelay")][PSTR("relay")][PSTR("label")].as<String>();}
      }
      
      state.fsaveAppRelay = true;
    } 

  }
}

void _onSyncClientAttributesCallback(uint8_t direction){
  JsonDocument doc;
  if(direction == 1 || direction == 3){
    doc[PSTR("relays")] = state.fPanic;
    //udawa.iotSendAttributes(doc);
  }
  else if(direction == 2 || direction == 3){
    doc.clear();
    JsonArray _relays = doc[PSTR("relays")].to<JsonArray>();
    for (uint8_t i = 0; i < countof(relays); i++) {
      _relays[i][PSTR("pin")] = relays[i].pin;
      _relays[i][PSTR("mode")] = relays[i].mode;
      _relays[i][PSTR("wattage")] = relays[i].wattage;
      _relays[i][PSTR("lastActive")] = relays[i].lastActive;
      _relays[i][PSTR("dutyCycle")] = relays[i].dutyCycle;
      _relays[i][PSTR("autoOff")] = relays[i].autoOff;
      _relays[i][PSTR("state")] = relays[i].state;
      _relays[i][PSTR("label")] = relays[i].label;
    }
    udawa.wsBroadcast(doc);

    doc.clear();
    JsonArray _availableRelayMode = doc[PSTR("availableRelayMode")].to<JsonArray>();
    for(uint8_t i = 0; i < countof(availableRelayMode); i++){
      _availableRelayMode.add(availableRelayMode[i]);
    }
    udawa.wsBroadcast(doc);
  }
}

#endif