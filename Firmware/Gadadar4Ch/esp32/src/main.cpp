#include "main.h"
void setup() {
  udawa->begin();

  loadAppConfig();
  loadAppState();
  loadAppRelay();

  udawa->addOnWsEvent(_onWsEventMain);
  udawa->addOnSyncClientAttributesCallback(_onSyncClientAttributesCallback);
  udawa->addOnThingsboardConnected(_onThingsboardConnectedCallback);
  udawa->addOnFSDownloadedCallback(_onFSDownloadedCallback);

  udawa->logger->verbose(PSTR(__func__), PSTR("Initial config.relayON value: %d\n"), config.relayON);
  
  if (!udawa->crashState.fSafeMode) {
    if(state.xHandlePowerSensor == NULL){
      state.xReturnedPowerSensor = xTaskCreatePinnedToCore(powerSensorTaskRoutine, PSTR("powerSensor"), POWERSENSOR_STACKSIZE, NULL, 1, &state.xHandlePowerSensor, 1);
      if(state.xReturnedPowerSensor == pdPASS){
        udawa->logger->warn(PSTR(__func__), PSTR("Task powerSensor has been created.\n"));
      }
    }

    if(state.xHandleRelayControl == NULL){
      state.xReturnedRelayControl = xTaskCreatePinnedToCore(relayControlTaskRoutine, PSTR("relayControl"), RELAYCONTROL_STACKSIZE, NULL, 1, &state.xHandleRelayControl, 1);
      if(state.xReturnedRelayControl == pdPASS){
        udawa->logger->warn(PSTR(__func__), PSTR("Task relayControl has been created.\n"));
      }
    }
  }

}

unsigned long timer = millis();
void loop() {
  udawa->run();

  unsigned long now = millis();
  if (!udawa->crashState.fSafeMode) {
    if (now - timer > 1000) {
      
      #ifdef USE_LOCAL_WEB_INTERFACE
      if(udawa->ws.count() > 0){
        JsonDocument doc;
        JsonObject sysInfo = doc[PSTR("sysInfo")].to<JsonObject>();

        sysInfo[PSTR("heap")] = ESP.getFreeHeap();
        sysInfo[PSTR("uptime")] = now;
        sysInfo[PSTR("datetime")] = udawa->RTC.getDateTime();
        sysInfo[PSTR("rssi")] = udawa->wiFiHelper.rssiToPercent(WiFi.RSSI());

        udawa->wsBroadcast(doc);
      }
      #endif

      #ifdef USE_IOT
      JsonDocument doc;
      doc[PSTR("heap")] = ESP.getFreeHeap();
      udawa->iotSendTelemetry(doc);
      #endif

      if(state.fsaveAppRelay){
        saveAppRelay();
        state.fsaveAppRelay = false;
      }

      if(state.fsyncClientAttributes){
        _onSyncClientAttributesCallback(3);
        state.fsyncClientAttributes = false;
      }

      if(state.fSaveAppState){
        saveAppState();
        state.fSaveAppState = false;
      }

      if(state.fPanic){
        udawa->logger->warn(PSTR(__func__), PSTR("Panic mode is activated.\n"));
        for(uint8_t i = 0; i < countof(relays); i++){
          udawa->logger->warn(PSTR(__func__), PSTR("Relay %d is turned off and changed to manual.\n"), i+1);
          relays[i].mode = 0;
          setRelay(i, false);
        }
      }

      timer = now;
    }
  }
}

void loadAppConfig(){
  JsonDocument doc;
  bool status = appConfig.load(doc);
  udawa->logger->debug(PSTR(__func__), PSTR("%d\n"), (int)status);
  if(status){
    convertAppConfig(doc, true);
    if(config.s1rx == 0){config.s1rx = s1rx;}
    if(config.s1tx == 0){config.s1rx = s1tx;}
    if(config.intvWeb == 0){config.intvWeb = 1;}
    if(config.intvAttr == 0){config.intvAttr = 5;}
    if(config.intvTele == 0){config.intvTele = 900;}
    if(config.maxWatt == 0){config.maxWatt = 2000;}
  }else{
    udawa->crashState.fFSDownloading = true;
  }
  
}

void saveAppConfig(){
  JsonDocument doc;
  convertAppConfig(doc, false);

  bool status = appConfig.save(doc);
  udawa->logger->debug(PSTR(__func__), PSTR("%d\n"), (int)status);
}

void loadAppState(){
  JsonDocument doc;
  bool status = appState.load(doc);
  udawa->logger->debug(PSTR(__func__), PSTR("%d\n"), (int)status);
  if(status){
    convertAppState(doc, true);
  }
}

void saveAppState(){
  JsonDocument doc;

  convertAppState(doc, false);

  bool status = appState.save(doc);
  udawa->logger->debug(PSTR(__func__), PSTR("%d\n"), (int)status);
}

void loadAppRelay(){
  JsonDocument doc;
  bool status = appRelay.load(doc);
  udawa->logger->debug(PSTR(__func__), PSTR("%d\n"), (int)status);
  if(status){
    convertAppRelay(doc, true);
  }
}

void saveAppRelay(){
  JsonDocument doc;
  convertAppRelay(doc, false);
  bool status = appRelay.save(doc);
  udawa->logger->debug(PSTR(__func__), PSTR("%d\n"), (int)status);
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
      //udawa->logger->warn(PSTR(__func__), PSTR("Failed to initialize powerSensor!\n"));
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
      udawa->logger->debug(PSTR(__func__), PSTR("Power sensor abnormal reading: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n"), volt, amp, watt, freq, pf, ener);
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
        udawa->wsBroadcast(doc);
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
        udawa->iotSendAttributes(doc);
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
        udawa->iotSendTelemetry(doc);
        doc.clear();
      
        timerTelemetry = now;
      }
      #endif

    }
  
    if( (now - timerAlarm) > 30000 )
    {
      if(!state.fPowerSensor){udawa->setAlarm(140, 1, 5, 1000);}
      else{
        if(volt < 0 || volt > 1000){udawa->setAlarm(141, 1, 5, 1000);}
        if(amp < 0 || amp > 100){udawa->setAlarm(142, 1, 5, 1000);}
        if(watt < 0 || watt > config.maxWatt){udawa->setAlarm(143, 1, 5, 1000);}
        if(pf < 0 || pf > 100){udawa->setAlarm(144, 1, 5, 1000);}
        if(freq < 0 || freq > 100){udawa->setAlarm(144, 1, 5, 1000);}
        if(watt > config.maxWatt || volt > 275){udawa->setAlarm(145, 1, 5, 1000);}

        uint8_t activeRelayCounter = 0;
        for(uint8_t i = 0; i < 4; i++){
          if(relays[i].state == true &&
          watt < 6.0){udawa->setAlarm(210+i, 1, 5, 1000);}

          if(relays[i].state == true){activeRelayCounter++;}

          if( relays[i].state == true && 
            (millis() - relays[i].lastActive) > relays[i].overrunInSec * 1000 && relays[i].overrunInSec != 0){
              udawa->setAlarm(215+i, 1, 5, 1000);
            }
        }

        if(activeRelayCounter == 0 && watt > 6){udawa->setAlarm(214, 1, 5, 1000);}    
      }

      
      timerAlarm = now;
    }

    if(state.fResetPowerSensor){
      state.fResetPowerSensor = false;
      udawa->logger->warn(PSTR(__func__), PSTR("Resetting power sensor.\n"));
      PZEM.resetEnergy();
    }

    state.powerSensorTaskRoutineLastActivity = millis();
    vTaskDelay((const TickType_t) 1000 / portTICK_PERIOD_MS);
  }
}

void relayControlTaskRoutine(void *arg){  
  for(uint8_t i = 0; i < countof(relays); i++){
    IOExtender.pinMode(relays[i].pin, OUTPUT);
    udawa->logger->verbose(PSTR(__func__), PSTR("Relay %d initialized as output.\n"), relays[i].pin);
  }

  state.fIOExtender =  IOExtender.begin();

  if(!state.fIOExtender){
    //udawa->logger->error(PSTR(__func__), PSTR("Failed to initialize IOExtender!\n"));
  }
  else{
    udawa->logger->verbose(PSTR(__func__), PSTR("IOExtender initialized.\n"));
  }

  for(uint8_t i = 0; i < countof(relays); i++){
    if(relays[i].mode == 0){
      setRelay(i, relays[i].state);
      udawa->logger->debug(PSTR(__func__), PSTR("Relay %d is initialized as %d.\n"), i+1, relays[i].state);
    }
  }

  unsigned long timerAlarm = millis();
  while (true)
  {
    if(!state.fIOExtender){
      for(uint8_t i = 0; i < countof(relays); i++){
        IOExtender.pinMode(relays[i].pin, OUTPUT);
      }
      state.fIOExtender = IOExtender.begin();
    }

    vTaskDelay((const TickType_t) 1000 / portTICK_PERIOD_MS);
    if(state.fPanic){continue;}
    unsigned long now = millis();

    /* Start alarm section */
    if(now - timerAlarm > 30000){
      if(!state.fIOExtender){
        udawa->setAlarm(220, 1, 10, 500);
      }
      
      timerAlarm = now;
    }
    /* End alarm section */

    for(uint8_t i = 0; i < countof(relays); i++){
      /* Start Manual Control Mode*/
      if(relays[i].mode == 0 && relays[i].autoOff != 0){
        if( (now - relays[i].lastActive) >= ( relays[i].autoOff * 1000 ) && relays[i].state == true){
          setRelay(i, false);
          udawa->logger->debug(PSTR(__func__), PSTR("Relay %d is turned off due to autoOff.\n"), i+1);
        } 
      }
      /* End Manual Control Mode*/
      /* Start Duty Cycle Control Mode */
      if(relays[i].dutyRange < 2){relays[i].dutyRange = 2;} //safenet
      if(relays[i].dutyCycle != 0 && relays[i].mode == 1) {
        if( relays[i].state == true )
        {
          if( relays[i].dutyCycle != 100 && (now - relays[i].lastChanged ) >= (float)(( ((float)relays[i].dutyCycle / 100) * (float)relays[i].dutyRange) * 1000))
          {
            setRelay(i, false);
            relays[i].state = false;
            relays[i].lastChanged = now;
          }
        }
        else
        {
          if( relays[i].dutyCycle != 0 && (now - relays[i].lastChanged ) >= (float) ( ((100 - (float) relays[i].dutyCycle) / 100) * (float)relays[i].dutyRange) * 1000)
          {
            setRelay(i, true);
            relays[i].state = true;
            relays[i].lastChanged = now;
          }
        }
      }
      /* End Duty Cycle Control Mode */
      /* Start Time Daily Control Mode */
      if(relays[i].mode == 2){
        bool flag_isInTimeWindow = false;
        int activeTimeWindowCounter = 0;
        for(uint8_t j = 0; j < maxTimers; j++)
        {
          int currHour = udawa->RTC.getHour(true);
          int currHourToSec = currHour * 3600;
          int currMinute = udawa->RTC.getMinute();
          int currMinuteToSec = currMinute * 60;
          int currSecond = udawa->RTC.getSecond();
          String currDT = udawa->RTC.getDateTime();
          int currentTimeInSec = currHourToSec + currMinuteToSec + currSecond;

          int duration = relays[i].timers[j].duration;
          int targetHour = relays[i].timers[j].hour;
          int targetHourToSec = targetHour * 3600;
          int targetMinute = relays[i].timers[j].minute;
          int targetMinuteToSec =targetMinute * 60;
          int targetSecond = relays[i].timers[j].second;
          int targetTimeInSec = targetHourToSec + targetMinuteToSec + targetSecond;

          int activationOffset = targetTimeInSec - currentTimeInSec;
          int deactivationOffset = activationOffset + duration;
          int activeTimeWindow = deactivationOffset - activationOffset;
          flag_isInTimeWindow = ( activationOffset <= 0 && deactivationOffset >= 0 ) ? true : false;
          const char * isInTimeWindow = flag_isInTimeWindow ? "TRUE" : "FALSE";

          //udawa->logger->debug(PSTR(__func__), PSTR("Relay %d timer index %d hour %d minute %d second %d duration %d IsInTImeWindow %s activeTimeWindowCounter %d\n"), i+1, j, targetHour, targetMinute, targetSecond, duration, isInTimeWindow, activeTimeWindowCounter);
          if(flag_isInTimeWindow){
            activeTimeWindowCounter++;
          }
        }
        if (relays[i].state == false && activeTimeWindowCounter > 0){
          relays[i].state = true;
          setRelay(i, true);
        }else if(relays[i].state == true && activeTimeWindowCounter < 1) {
          relays[i].state = false;
          setRelay(i, false);
        }
      }
      /* Stop Time Daily Control Mode */
      /* Start Exact Datetime Control Mode */
      if(relays[i].duration > 0 && relays[i].mode == 3){
        if(relays[i].datetime <= (udawa->RTC.getEpoch()) && (relays[i].duration) >=
          (udawa->RTC.getEpoch() - relays[i].datetime) && relays[i].state == false){
            relays[i].state = true;
            setRelay(i, true);
        }
        else if(relays[i].state == true && (relays[i].duration) <=
          (udawa->RTC.getEpoch() - relays[i].datetime)){
            relays[i].state = false;
            setRelay(i, false);
        }
      }
      /* Start Exact Datetime Control Mode */
    }

    state.relayControlTaskRoutineLastActivity = millis();
  }
}

void setRelay(uint8_t index, bool output){
  if(index < countof(relays)){
    if(output){
      IOExtender.digitalWrite(relays[index].pin, config.relayON);
      relays[index].state = true;
      relays[index].lastActive = millis();
      relays[index].lastChanged = millis();
      udawa->logger->debug(PSTR(__func__), PSTR("Relay %d is ON. %d was written to relay.\n"), index+1, config.relayON);
      state.fsyncClientAttributes = true;
      state.fsaveAppRelay = true;
    }
    else{
      IOExtender.digitalWrite(relays[index].pin, !config.relayON);
      relays[index].state = false;
      relays[index].lastChanged = millis();
      udawa->logger->debug(PSTR(__func__), PSTR("Relay %d is OFF. %d was written to relay.\n"), index+1, !config.relayON);
      state.fsyncClientAttributes = true;
      state.fsaveAppRelay = true;
    }

    #ifdef USE_IOT
    JsonDocument doc;
    doc[(String("ch") + String(index+1)).c_str()] = relays[index].state;
    #endif
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
    //String incomming;
    //serializeJson(doc, incomming);
    //udawa->logger->debug(PSTR(__func__), PSTR("Incoming: %s\n"), incomming.c_str());

    if(doc[PSTR("setRelayState")].is<JsonObject>()){
      if(doc[PSTR("setRelayState")][PSTR("pin")].is<uint8_t>() && doc[PSTR("setRelayState")][PSTR("state")].is<bool>()){
        setRelay(doc[PSTR("setRelayState")][PSTR("pin")].as<uint8_t>(), doc[PSTR("setRelayState")][PSTR("state")].as<bool>());
      }
    }
    else if(doc[PSTR("resetPowerSensor")].is<bool>()){
      state.fResetPowerSensor = true;
    }
    else if(doc[PSTR("setRelay")].is<JsonObject>() && doc[PSTR("setRelay")][PSTR("relay")].is<JsonObject>() && doc[PSTR("setRelay")][PSTR("index")].is<uint8_t>()){
      uint8_t index = doc[PSTR("setRelay")][PSTR("index")].as<uint8_t>();
      if(index < countof(relays)){
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("pin")].is<uint8_t>()){relays[index].pin = doc[PSTR("setRelay")][PSTR("relay")][PSTR("pin")].as<uint8_t>();} 
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("mode")].is<uint8_t>()){relays[index].mode = doc[PSTR("setRelay")][PSTR("relay")][PSTR("mode")].as<uint8_t>();}
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("wattage")].is<uint16_t>()){relays[index].wattage = doc[PSTR("setRelay")][PSTR("relay")][PSTR("wattage")].as<uint16_t>();}
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("dutyCycle")].is<uint8_t>()){relays[index].dutyCycle = doc[PSTR("setRelay")][PSTR("relay")][PSTR("dutyCycle")].as<uint8_t>();}
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("dutyRange")].is<unsigned long>()){relays[index].dutyRange = doc[PSTR("setRelay")][PSTR("relay")][PSTR("dutyRange")].as<unsigned long>();}
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("autoOff")].is<unsigned long>()){relays[index].autoOff = doc[PSTR("setRelay")][PSTR("relay")][PSTR("autoOff")].as<unsigned long>();}
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("label")].is<String>()){relays[index].label = doc[PSTR("setRelay")][PSTR("relay")][PSTR("label")].as<String>();}
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("overrunInSec")].is<uint16_t>()){relays[index].overrunInSec = doc[PSTR("setRelay")][PSTR("relay")][PSTR("overrunInSec")].as<uint16_t>();}
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("datetime")].is<unsigned long>()){relays[index].datetime = doc[PSTR("setRelay")][PSTR("relay")][PSTR("datetime")].as<unsigned long>();}
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("duration")].is<unsigned long>()){relays[index].duration = doc[PSTR("setRelay")][PSTR("relay")][PSTR("duration")].as<unsigned long>();}
        if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("timers")].is<JsonArray>()){
          for(uint8_t j = 0; j < maxTimers; j++){
            if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("timers")][j][PSTR("h")].is<uint8_t>()){relays[index].timers[j].hour = doc[PSTR("setRelay")][PSTR("relay")][PSTR("timers")][j][PSTR("h")].as<uint8_t>();}
            if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("timers")][j][PSTR("i")].is<uint8_t>()){relays[index].timers[j].minute = doc[PSTR("setRelay")][PSTR("relay")][PSTR("timers")][j][PSTR("i")].as<uint8_t>();}
            if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("timers")][j][PSTR("s")].is<uint8_t>()){relays[index].timers[j].second = doc[PSTR("setRelay")][PSTR("relay")][PSTR("timers")][j][PSTR("s")].as<uint8_t>();}
            if(doc[PSTR("setRelay")][PSTR("relay")][PSTR("timers")][j][PSTR("d")].is<unsigned long>()){relays[index].timers[j].duration = doc[PSTR("setRelay")][PSTR("relay")][PSTR("timers")][j][PSTR("d")].as<unsigned long>();} 
          }
        }
      }
      
      state.fsaveAppRelay = true;
    }
    else if(doc[PSTR("fPanic")].is<bool>()){
      state.fPanic = doc[PSTR("fPanic")].as<bool>();
    } 

  }
}

void _onSyncClientAttributesCallback(uint8_t direction){
  JsonDocument doc;
  if(direction == 1 || direction == 3){
    //udawa->iotSendAttributes(doc);
  }
  if(direction == 2 || direction == 3){
    doc.clear();
    convertAppRelay(doc, false);
    udawa->wsBroadcast(doc);

    doc.clear();
    JsonArray _availableRelayMode = doc[PSTR("availableRelayMode")].to<JsonArray>();
    for(uint8_t i = 0; i < countof(availableRelayMode); i++){
      _availableRelayMode.add(availableRelayMode[i]);
    }
    udawa->wsBroadcast(doc);
  }
}



/**
 * @brief Converts relay data between a JSON document and relay objects.
 *
 * This function transfers relay data between a JSON document and an array of relay objects.
 * The direction of the transfer is determined by the `direction` parameter.
 *
 * @param doc The JSON document containing relay data.
 * @param direction The direction of the data transfer.
 *                  - `true`: Transfer data from the JSON document to the relay objects.
 *                  - `false`: Transfer data from the relay objects to the JSON document.
 */
void convertAppRelay(JsonDocument &doc, bool direction){
  // direction = true means from doc to relays
  if(direction){
    for(uint8_t i = 0; i < countof(relays); i++){
      if(doc[PSTR("relays")][i][PSTR("pin")].is<uint8_t>()){relays[i].pin = doc[PSTR("relays")][i][PSTR("pin")].as<uint8_t>();}else{relays[i].pin = i;}
      if(doc[PSTR("relays")][i][PSTR("mode")].is<uint8_t>()){relays[i].mode = doc[PSTR("relays")][i][PSTR("mode")].as<uint8_t>();}else{relays[i].mode = 0;}
      if(doc[PSTR("relays")][i][PSTR("wattage")].is<uint16_t>()){relays[i].wattage = doc[PSTR("relays")][i][PSTR("wattage")].as<uint16_t>();}else{relays[i].wattage = 0;}
      if(doc[PSTR("relays")][i][PSTR("lastActive")].is<unsigned long>()){relays[i].lastActive = doc[PSTR("relays")][i][PSTR("lastActive")].as<unsigned long>();}else{relays[i].lastActive = 0;}
      if(doc[PSTR("relays")][i][PSTR("lastChanged")].is<unsigned long>()){relays[i].lastChanged = doc[PSTR("relays")][i][PSTR("lastChanged")].as<unsigned long>();}else{relays[i].lastChanged = 0;}
      if(doc[PSTR("relays")][i][PSTR("dutyCycle")].is<uint8_t>()){relays[i].dutyCycle = doc[PSTR("relays")][i][PSTR("dutyCycle")].as<uint8_t>();}else{relays[i].dutyCycle = 0;}
      if(doc[PSTR("relays")][i][PSTR("dutyRange")].is<unsigned long>()){relays[i].dutyRange = doc[PSTR("relays")][i][PSTR("dutyRange")].as<unsigned long>();}else{relays[i].dutyRange = 0;}
      if(doc[PSTR("relays")][i][PSTR("autoOff")].is<unsigned long>()){relays[i].autoOff = doc[PSTR("relays")][i][PSTR("autoOff")].as<unsigned long>();}else{relays[i].autoOff = 0;}
      if(doc[PSTR("relays")][i][PSTR("state")].is<bool>()){relays[i].state = doc[PSTR("relays")][i][PSTR("state")].as<bool>();}else{relays[i].state = false;}
      if(doc[PSTR("relays")][i][PSTR("label")].is<String>()){relays[i].label = doc[PSTR("relays")][i][PSTR("label")].as<String>();}else{relays[i].label = PSTR("No label");}
      if(doc[PSTR("relays")][i][PSTR("overrunInSec")].is<uint16_t>()){relays[i].overrunInSec = doc[PSTR("relays")][i][PSTR("overrunInSec")].as<uint16_t>();}else{relays[i].overrunInSec = 3600;}
      if(doc[PSTR("relays")][i][PSTR("duration")].is<unsigned long>()){relays[i].duration = doc[PSTR("relays")][i][PSTR("duration")].as<unsigned long>();}else{relays[i].duration = 0;}
      if(doc[PSTR("relays")][i][PSTR("datetime")].is<unsigned long>()){relays[i].datetime = doc[PSTR("relays")][i][PSTR("datetime")].as<unsigned long>();}else{relays[i].datetime = 0;}
      if(doc[PSTR("relays")][i][PSTR("timers")].is<JsonArray>()){
        for(uint8_t j = 0; j < countof(relays[i].timers); j++){
          if(doc[PSTR("relays")][i][PSTR("timers")][j][PSTR("h")].is<uint8_t>()){relays[i].timers[j].hour = doc[PSTR("relays")][i][PSTR("timers")][j][PSTR("h")].as<uint8_t>();}else{relays[i].timers[j].hour = 0;}
          if(doc[PSTR("relays")][i][PSTR("timers")][j][PSTR("i")].is<uint8_t>()){relays[i].timers[j].minute = doc[PSTR("relays")][i][PSTR("timers")][j][PSTR("i")].as<uint8_t>();}else{relays[i].timers[j].minute = 0;}
          if(doc[PSTR("relays")][i][PSTR("timers")][j][PSTR("s")].is<uint8_t>()){relays[i].timers[j].second = doc[PSTR("relays")][i][PSTR("timers")][j][PSTR("s")].as<uint8_t>();}else{relays[i].timers[j].second = 0;}
          if(doc[PSTR("relays")][i][PSTR("timers")][j][PSTR("d")].is<unsigned long>()){relays[i].timers[j].duration = doc[PSTR("relays")][i][PSTR("timers")][j][PSTR("d")].as<unsigned long>();}else{relays[i].timers[j].duration = 0;}
        }
      }
    }
  }
  // direction = false means from relays to doc
  else{
    JsonArray _relays = doc[PSTR("relays")].to<JsonArray>();
    for(uint8_t i = 0; i < countof(relays); i++){
      _relays[i][PSTR("pin")] = relays[i].pin;
      _relays[i][PSTR("mode")] = relays[i].mode;
      _relays[i][PSTR("wattage")] = relays[i].wattage;
      _relays[i][PSTR("lastActive")] = relays[i].lastActive;
      _relays[i][PSTR("lastChanged")] = relays[i].lastChanged;
      _relays[i][PSTR("dutyCycle")] = relays[i].dutyCycle;
      _relays[i][PSTR("dutyRange")] = relays[i].dutyRange;
      _relays[i][PSTR("autoOff")] = relays[i].autoOff;
      _relays[i][PSTR("state")] = relays[i].state;
      _relays[i][PSTR("label")] = relays[i].label;
      _relays[i][PSTR("overrunInSec")] = relays[i].overrunInSec;
      _relays[i][PSTR("duration")] = relays[i].duration;
      _relays[i][PSTR("datetime")] = relays[i].datetime;
      
      // Add timers array
      JsonArray timers = _relays[i][PSTR("timers")].to<JsonArray>();
      for (uint8_t j = 0; j < maxTimers; j++) {
          timers[j][PSTR("h")] = relays[i].timers[j].hour;
          timers[j][PSTR("i")] = relays[i].timers[j].minute;
          timers[j][PSTR("s")] = relays[i].timers[j].second;
          timers[j][PSTR("d")] = relays[i].timers[j].duration;
      }
    }
  }
}


void convertAppState(JsonDocument &doc, bool direction){
  // direction = true means from doc to state
  if(direction){
    if(doc[PSTR("fPanic")].is<bool>()){state.fPanic = doc[PSTR("fPanic")].as<bool>();}else{state.fPanic = false;}
  }
  // direction = false means from state to doc
  else{
    doc[PSTR("fPanic")] = state.fPanic;
  }
}

void convertAppConfig(JsonDocument &doc, bool direction){
  // direction = true means from doc to config
  if(direction){
    if(doc[PSTR("s1tx")].is<uint8_t>()){config.s1tx = doc[PSTR("s1tx")].as<uint8_t>();} else{config.s1tx = s1tx;}
    if(doc[PSTR("s1rx")].is<uint8_t>()){config.s1rx = doc[PSTR("s1rx")].as<uint8_t>();} else{config.s1rx = s1rx;}
    if(doc[PSTR("intvWeb")].is<unsigned long>()){config.intvWeb = doc[PSTR("intvWeb")].as<unsigned long>();} else{config.intvWeb = intvWeb;}
    if(doc[PSTR("intvAttr")].is<unsigned long>()){config.intvAttr = doc[PSTR("intvAttr")].as<unsigned long>();} else{config.intvAttr = intvAttr;}
    if(doc[PSTR("intvTele")].is<unsigned long>()){config.intvTele = doc[PSTR("intvTele")].as<unsigned long>();} else{config.intvTele = intvTele;}
    if(doc[PSTR("maxWatt")].is<int>()){config.maxWatt = doc[PSTR("maxWatt")].as<int>();} else{config.maxWatt = maxWatt;}
    if(doc[PSTR("relayON")].is<bool>()){config.relayON = doc[PSTR("relayON")].as<bool>();} else{config.relayON = relayON;}
  }
  // direction = false means from config to doc
  else{
    doc[PSTR("s1tx")] = config.s1tx;
    doc[PSTR("s1rx")] = config.s1rx;
    doc[PSTR("intvWeb")] = config.intvWeb;
    doc[PSTR("intvAttr")] = config.intvAttr;
    doc[PSTR("intvTele")] = config.intvTele;
    doc[PSTR("maxWatt")] = config.maxWatt;
    doc[PSTR("relayON")] = config.relayON;
  }
}

#ifdef USE_IOT
void _onThingsboardConnectedCallback(){
  udawa->tb.RPC_Subscribe(state.callbacks.cbegin(), state.callbacks.cend());
}

void processRPCSwitch(const JsonVariantConst &data, JsonDocument &response) {
  uint8_t index = 255;
  if(data[PSTR("index")].is<uint8_t>()){
    index = data[PSTR("index")].as<uint8_t>();
  }

  if(index < 255 && index >= 0 && index < countof(relays) && data[PSTR("state")].is<bool>()){
    setRelay(index, data[PSTR("state")].as<bool>());
    response.set(PSTR("OK"));
  }
  else if(index < 255 && index >= 0 && index < countof(relays)){
    response[PSTR("channel")] = index + 1;
    response[PSTR("state")] = relays[data[PSTR("index")].as<uint8_t>()].state ? PSTR("ON") : PSTR("OFF");
  }
  else{
    response.set(PSTR("Invalid parameters."));
  }
}

void processRPCFSUpdate(const JsonVariantConst &data, JsonDocument &response) {
  response.set(PSTR("OK"));
  udawa->crashState.fFSDownloading = true;
}
#endif

void _onFSDownloadedCallback(){
  saveAppConfig();
  saveAppRelay();
  saveAppState();
}

#endif