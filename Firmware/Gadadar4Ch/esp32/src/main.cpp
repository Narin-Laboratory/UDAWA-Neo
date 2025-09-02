#include "main.h"
#include <ArduinoJson.h>

void setup() {
  udawa->begin();

  loadAppConfig();
  loadAppState();
  loadAppRelay();

  udawa->addOnWsEvent(_onWsEventMain);
  udawa->addOnFSDownloadedCallback(_onFSDownloadedCallback);

  udawa->logger->verbose(PSTR(__func__), PSTR("Initial config.relayON value: %d\n"), config.relayON);

  #ifdef USE_IOT_SECURE
  espClient.setCACert(CA_CERT);
  #endif
  
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
  
  tbRun();

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

        String jsonString;
        serializeJson(doc, jsonString);
        udawa->wsBroadcast(jsonString.c_str());
      }
      #endif

      if(state.fsaveAppRelay){
        saveAppRelay();
        state.fsaveAppRelay = false;
      }

      if(state.fsyncClientAttributes){
        _onSyncClientAttributesCallback(1);
        _onSyncClientAttributesCallback(1);
        _onSyncClientAttributesCallback(1);
        _onSyncClientAttributesCallback(1);
        _onSyncClientAttributesCallback(1);
        _onSyncClientAttributesCallback(1);
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

void tbRun(){
  if(!udawa->crashState.fSafeMode){
    if (udawa->config.state.provSent) {
      if (!tb.connected()) {
        // Connect to the ThingsBoard server as a client wanting to provision a new device
        udawa->logger->debug(PSTR(__func__), PSTR("Connecting to: %s\n"), udawa->config.state.tbAddr);
        if (!tb.connect(udawa->config.state.tbAddr, "provision", udawa->config.state.tbPort)) {
          udawa->logger->debug(PSTR(__func__), PSTR("Failed to connect to: %s\n"), udawa->config.state.tbAddr);
          return;
        }
      }
      udawa->logger->debug(PSTR(__func__), PSTR("Sending provisioning request: %s\n"), udawa->config.state.name);

      const Provision_Callback provisionCallback(Access_Token(), &processProvisionResponse, udawa->config.state.provDK, udawa->config.state.provDS, udawa->config.state.name, IOT_REQUEST_TIMEOUT_MICROSECONDS, &provisionRequestTimedOut);
      udawa->config.state.provSent = prov.Provision_Request(provisionCallback);
    }
    else if (udawa->config.state.provSent) {
      if (!tb.connected()) {
        // Connect to the ThingsBoard server, as the provisioned client
        udawa->logger->debug(PSTR(__func__), PSTR("Connecting to: %s\n"), udawa->config.state.tbAddr);
        if (!tb.connect(udawa->config.state.tbAddr, udawa->config.state.accTkn, udawa->config.state.tbPort, udawa->config.state.name)) {
          udawa->logger->warn(PSTR(__func__), PSTR("Failed to connect: %s:%d using clientID %s\n"), udawa->config.state.tbAddr, udawa->config.state.tbPort, udawa->config.state.name);
          return;
        } else {
          udawa->logger->warn(PSTR(__func__), PSTR("Connected!\n"));
          onTbConnected();
        }
      }
    }

    tb.loop();
  }
}

void provisionRequestTimedOut() {
  udawa->logger->warn((PSTR(__func__)), PSTR("Provision request timed out did not receive a response in (%llu) microseconds. Ensure client is connected to the MQTT broker\n"), IOT_REQUEST_TIMEOUT_MICROSECONDS);
}

void processProvisionResponse(const JsonDocument &data) {
  String buffer;
  serializeJson(data, buffer);
  Serial.printf("Received device provision response (%s)\n", buffer.c_str());

  if (strcmp(data["status"].as<const char*>(), "SUCCESS") != 0) {
    Serial.printf("Provision response contains the error: (%s)\n", data["errorMsg"].as<const char*>());
    return;
  }

  const char* credentialsType = data["credentialsType"].as<const char*>();
  if (strcmp(credentialsType, "ACCESS_TOKEN") == 0) {
    strlcpy(udawa->config.state.accTkn, data["credentialsValue"].as<const char*>(), sizeof(udawa->config.state.accTkn));
  }
  else if (strcmp(credentialsType, "MQTT_BASIC") == 0) {
    JsonObjectConst credentialsValue = data["credentialsValue"].as<JsonObjectConst>();
    strlcpy(udawa->config.state.name, credentialsValue["clientId"].as<const char*>(), sizeof(udawa->config.state.name));
    strlcpy(udawa->config.state.accTkn, credentialsValue["userName"].as<const char*>(), sizeof(udawa->config.state.accTkn));
    // password is not used for token-based auth with ThingsBoard MQTT
  }
  else {
    Serial.printf("Unexpected provision credentialsType: (%s)\n", credentialsType);
    return;
  }

  udawa->config.state.provSent = true;
  udawa->config.save();

  // Disconnect from the cloud client connected to the provision account, because it is no longer needed the device has been provisioned
  // and we can reconnect to the cloud with the newly generated credentials.
  if (tb.connected()) {
    tb.disconnect();
  }
}

void loadAppConfig(){
  // Attempt to load the config file
  JsonDocument doc;
  bool status = appConfig.load(doc);
  
  if(status){
    // If successful, parse the values from the file into the config object
    convertAppConfig(doc, true);
  } else {
    // This block runs if the file doesn't exist or is invalid.
    // You can put first-time setup logic here if needed.
    loadAppRelay();
    loadAppState();
    udawa->crashState.fFSDownloading = true;
  }

  // --- This is the new, corrected logic ---
  // After attempting to load, check if any values are still invalid (0)
  // and apply the compile-time defaults from params.h.
  if(config.s1rx == 0){config.s1rx = s1rx;}
  if(config.s1tx == 0){config.s1tx = s1tx;}
  if(config.intvWeb == 0){config.intvWeb = intvWeb;}
  if(config.intvAttr == 0){config.intvAttr = intvAttr;}
  if(config.intvTele == 0){config.intvTele = intvTele;}
  if(config.maxWatt == 0){config.maxWatt = maxWatt;}
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
      //udawa->logger->debug(PSTR(__func__), PSTR("Power sensor abnormal reading: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n"), volt, amp, watt, freq, pf, ener);
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
        powerSensor[PSTR("volt")] = String(volt, 2);
        powerSensor[PSTR("amp")] = String(amp, 2);
        powerSensor[PSTR("watt")] = String(watt, 2);
        powerSensor[PSTR("freq")] = String(freq, 2);
        powerSensor[PSTR("pf")] = String(pf, 2);
        powerSensor[PSTR("ener")] = String(ener, 2);
        String jsonString;
        serializeJson(doc, jsonString);
        udawa->wsBroadcast(jsonString.c_str());
        doc.clear();

        timerWebIface = now;
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

  }
}

#ifdef USE_LOCAL_WEB_INTERFACE
void _onWsEventMain(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  JsonDocument doc;
  if (type == WS_EVT_CONNECT) {
    // Handle WebSocket connection event
  } else if (type == WS_EVT_DATA) {
    // Handle WebSocket data event
    DeserializationError err = deserializeJson(doc, (const char*)data, len);
    if (err) {
      udawa->logger->error(PSTR(__func__), PSTR("JSON deserialization failed: %s\n"), err.c_str());
      doc.clear();
      return;
    }
    if (doc.containsKey(PSTR("setRelayState"))) {
      JsonObject setRelayState = doc[PSTR("setRelayState")];
      if (setRelayState.containsKey(PSTR("pin")) && setRelayState.containsKey(PSTR("state"))) {
        setRelay(setRelayState[PSTR("pin")].as<uint8_t>(), setRelayState[PSTR("state")].as<bool>());
      }
    } else if (doc.containsKey(PSTR("resetPowerSensor"))) {
      state.fResetPowerSensor = true;
    } else if (doc.containsKey(PSTR("getConfig"))) {
      state.fsyncClientAttributes = true;
    } else if (doc.containsKey(PSTR("setRelay"))) {
      JsonObject setRelayObj = doc[PSTR("setRelay")];
      if (setRelayObj.containsKey(PSTR("relay")) && setRelayObj.containsKey(PSTR("index"))) {
        uint8_t index = setRelayObj[PSTR("index")].as<uint8_t>();
        if (index < countof(relays)) {
          JsonObject relay = setRelayObj[PSTR("relay")];
          if (relay.containsKey(PSTR("pin"))) { relays[index].pin = relay[PSTR("pin")].as<uint8_t>(); }
          if (relay.containsKey(PSTR("mode"))) { relays[index].mode = relay[PSTR("mode")].as<uint8_t>(); }
          if (relay.containsKey(PSTR("wattage"))) { relays[index].wattage = relay[PSTR("wattage")].as<uint16_t>(); }
          if (relay.containsKey(PSTR("dutyCycle"))) { relays[index].dutyCycle = relay[PSTR("dutyCycle")].as<uint8_t>(); }
          if (relay.containsKey(PSTR("dutyRange"))) { relays[index].dutyRange = relay[PSTR("dutyRange")].as<unsigned long>(); }
          if (relay.containsKey(PSTR("autoOff"))) { relays[index].autoOff = relay[PSTR("autoOff")].as<unsigned long>(); }
          if (relay.containsKey(PSTR("label"))) { relays[index].label = relay[PSTR("label")].as<String>(); }
          if (relay.containsKey(PSTR("overrunInSec"))) { relays[index].overrunInSec = relay[PSTR("overrunInSec")].as<uint16_t>(); }
          if (relay.containsKey(PSTR("datetime"))) { relays[index].datetime = relay[PSTR("datetime")].as<unsigned long>(); }
          if (relay.containsKey(PSTR("duration"))) { relays[index].duration = relay[PSTR("duration")].as<unsigned long>(); }
          if (relay.containsKey(PSTR("timers"))) {
            JsonArray timers = relay[PSTR("timers")];
            for (uint8_t j = 0; j < maxTimers && j < timers.size(); j++) {
              JsonObject timer = timers[j];
              if (timer.containsKey(PSTR("h"))) { relays[index].timers[j].hour = timer[PSTR("h")].as<uint8_t>(); }
              if (timer.containsKey(PSTR("i"))) { relays[index].timers[j].minute = timer[PSTR("i")].as<uint8_t>(); }
              if (timer.containsKey(PSTR("s"))) { relays[index].timers[j].second = timer[PSTR("s")].as<uint8_t>(); }
              if (timer.containsKey(PSTR("d"))) { relays[index].timers[j].duration = timer[PSTR("d")].as<unsigned long>(); }
            }
          }
        }
        state.fsaveAppRelay = true;
      }
    } else if (doc.containsKey(PSTR("fPanic"))) {
      state.fPanic = doc[PSTR("fPanic")].as<bool>();
    }
    doc.clear();
  }
}

void _onSyncClientAttributesCallback(uint8_t direction) {
  JsonDocument doc;
  if (direction == 0 || direction == 1) {
    String buffer;
    // Serialize relay data and broadcast
    convertAppRelay(doc, false);
    serializeJson(doc, buffer);
    udawa->wsBroadcast(buffer.c_str());
    doc.clear();
    buffer.clear();

    // Clear and prepare the document for available relay modes
    JsonArray availableRelayModes = doc.createNestedArray(F("availableRelayMode"));
    for (uint8_t i = 0; i < countof(availableRelayMode); i++) {
      availableRelayModes.add(availableRelayMode[i]);
    }
    // Serialize and broadcast available relay modes
    serializeJson(doc, buffer);
    udawa->wsBroadcast(buffer.c_str());
    doc.clear();
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
 * - `true`: Transfer data from the JSON document to the relay objects.
 * - `false`: Transfer data from the relay objects to the JSON document.
 */
void convertAppRelay(JsonDocument &doc, bool direction){
  // direction = true means from doc to relays
  if(direction){
    udawa->logger->debug(PSTR(__func__), PSTR("Converting from JSON document to relays.\n"));
    for(uint8_t i = 0; i < countof(relays); i++){
      if(doc[PSTR("relays")][i].containsKey(PSTR("pin"))){relays[i].pin = doc[PSTR("relays")][i][PSTR("pin")].as<uint8_t>();}else{relays[i].pin = i;}
      if(doc[PSTR("relays")][i].containsKey(PSTR("mode"))){relays[i].mode = doc[PSTR("relays")][i][PSTR("mode")].as<uint8_t>();}else{relays[i].mode = 0;}
      if(doc[PSTR("relays")][i].containsKey(PSTR("wattage"))){relays[i].wattage = doc[PSTR("relays")][i][PSTR("wattage")].as<uint16_t>();}else{relays[i].wattage = 0;}
      if(doc[PSTR("relays")][i].containsKey(PSTR("lastActive"))){relays[i].lastActive = doc[PSTR("relays")][i][PSTR("lastActive")].as<unsigned long>();}else{relays[i].lastActive = 0;}
      if(doc[PSTR("relays")][i].containsKey(PSTR("lastChanged"))){relays[i].lastChanged = doc[PSTR("relays")][i][PSTR("lastChanged")].as<unsigned long>();}else{relays[i].lastChanged = 0;}
      if(doc[PSTR("relays")][i].containsKey(PSTR("dutyCycle"))){relays[i].dutyCycle = doc[PSTR("relays")][i][PSTR("dutyCycle")].as<uint8_t>();}else{relays[i].dutyCycle = 0;}
      if(doc[PSTR("relays")][i].containsKey(PSTR("dutyRange"))){relays[i].dutyRange = doc[PSTR("relays")][i][PSTR("dutyRange")].as<unsigned long>();}else{relays[i].dutyRange = 0;}
      if(doc[PSTR("relays")][i].containsKey(PSTR("autoOff"))){relays[i].autoOff = doc[PSTR("relays")][i][PSTR("autoOff")].as<unsigned long>();}else{relays[i].autoOff = 0;}
      if(doc[PSTR("relays")][i].containsKey(PSTR("state"))){relays[i].state = doc[PSTR("relays")][i][PSTR("state")].as<bool>();}else{relays[i].state = false;}
      if(doc[PSTR("relays")][i].containsKey(PSTR("label"))){relays[i].label = doc[PSTR("relays")][i][PSTR("label")].as<String>();}else{relays[i].label = PSTR("No label");}
      if(doc[PSTR("relays")][i].containsKey(PSTR("overrunInSec"))){relays[i].overrunInSec = doc[PSTR("relays")][i][PSTR("overrunInSec")].as<uint16_t>();}else{relays[i].overrunInSec = 3600;}
      if(doc[PSTR("relays")][i].containsKey(PSTR("duration"))){relays[i].duration = doc[PSTR("relays")][i][PSTR("duration")].as<unsigned long>();}else{relays[i].duration = 0;}
      if(doc[PSTR("relays")][i].containsKey(PSTR("datetime"))){relays[i].datetime = doc[PSTR("relays")][i][PSTR("datetime")].as<unsigned long>();}else{relays[i].datetime = 0;}
      if(doc[PSTR("relays")][i].containsKey(PSTR("timers"))){
        for(uint8_t j = 0; j < countof(relays[i].timers); j++){
          if(doc[PSTR("relays")][i][PSTR("timers")][j].containsKey(PSTR("h"))){relays[i].timers[j].hour = doc[PSTR("relays")][i][PSTR("timers")][j][PSTR("h")].as<uint8_t>();}else{relays[i].timers[j].hour = 0;}
          if(doc[PSTR("relays")][i][PSTR("timers")][j].containsKey(PSTR("i"))){relays[i].timers[j].minute = doc[PSTR("relays")][i][PSTR("timers")][j][PSTR("i")].as<uint8_t>();}else{relays[i].timers[j].minute = 0;}
          if(doc[PSTR("relays")][i][PSTR("timers")][j].containsKey(PSTR("s"))){relays[i].timers[j].second = doc[PSTR("relays")][i][PSTR("timers")][j][PSTR("s")].as<uint8_t>();}else{relays[i].timers[j].second = 0;}
          if(doc[PSTR("relays")][i][PSTR("timers")][j].containsKey(PSTR("d"))){relays[i].timers[j].duration = doc[PSTR("relays")][i][PSTR("timers")][j][PSTR("d")].as<unsigned long>();}else{relays[i].timers[j].duration = 0;}
        }
      }
      udawa->logger->debug(PSTR(__func__), PSTR("Relay %d loaded: pin=%d, mode=%d, state=%d\n"), i, relays[i].pin, relays[i].mode, relays[i].state);
    }
  }
  // direction = false means from relays to doc
  else{
    udawa->logger->debug(PSTR(__func__), PSTR("Converting from relays to JSON document.\n"));
    JsonArray _relays = doc.createNestedArray(PSTR("relays"));
    for(uint8_t i = 0; i < countof(relays); i++){
      JsonObject relay = _relays.createNestedObject();
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
      
      // Add timers array
      JsonArray timers = relay.createNestedArray(PSTR("timers"));
      for (uint8_t j = 0; j < maxTimers; j++) {
          JsonObject timer = timers.createNestedObject();
          timer[PSTR("h")] = relays[i].timers[j].hour;
          timer[PSTR("i")] = relays[i].timers[j].minute;
          timer[PSTR("s")] = relays[i].timers[j].second;
          timer[PSTR("d")] = relays[i].timers[j].duration;
      }
      udawa->logger->debug(PSTR(__func__), PSTR("Relay %d saved: pin=%d, mode=%d, state=%d\n"), i, relays[i].pin, relays[i].mode, relays[i].state);
    }
  }
}


void convertAppState(JsonDocument &doc, bool direction){
  // direction = true means from doc to state
  if(direction){
    if(doc.containsKey(PSTR("fPanic"))){state.fPanic = doc[PSTR("fPanic")].as<bool>();}else{state.fPanic = false;}
  }
  // direction = false means from state to doc
  else{
    doc[PSTR("fPanic")] = state.fPanic;
  }
}

void convertAppConfig(JsonDocument &doc, bool direction){
  // direction = true means from doc to config
  if(direction){
    if(doc.containsKey(PSTR("s1tx"))){config.s1tx = doc[PSTR("s1tx")].as<uint8_t>();} else{config.s1tx = s1tx;}
    if(doc.containsKey(PSTR("s1rx"))){config.s1rx = doc[PSTR("s1rx")].as<uint8_t>();} else{config.s1rx = s1rx;}
    if(doc.containsKey(PSTR("intvWeb"))){config.intvWeb = doc[PSTR("intvWeb")].as<unsigned long>();} else{config.intvWeb = intvWeb;}
    if(doc.containsKey(PSTR("intvAttr"))){config.intvAttr = doc[PSTR("intvAttr")].as<unsigned long>();} else{config.intvAttr = intvAttr;}
    if(doc.containsKey(PSTR("intvTele"))){config.intvTele = doc[PSTR("intvTele")].as<unsigned long>();} else{config.intvTele = intvTele;}
    if(doc.containsKey(PSTR("maxWatt"))){config.maxWatt = doc[PSTR("maxWatt")].as<int>();} else{config.maxWatt = maxWatt;}
    if(doc.containsKey(PSTR("relayON"))){config.relayON = doc[PSTR("relayON")].as<bool>();} else{config.relayON = relayON;}
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

void _onFSDownloadedCallback(){
  saveAppConfig();
  saveAppRelay();
  saveAppState();
}

#endif