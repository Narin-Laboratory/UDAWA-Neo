#include "coreroutine.h"

ESP32Time RTC(0);
#ifdef USE_HW_RTC
ErriezDS3231 hwRTC;
#endif

TaskHandle_t xHandleAlarm = NULL;
BaseType_t xReturnedAlarm;
QueueHandle_t xQueueAlarm = xQueueCreate( 10, sizeof( struct AlarmMessage ) );

void reboot(int countDown = 0){
  crashState.plannedRebootCountDown = countDown;
  crashState.fPlannedReboot = true;
}

void coreroutineSetup(){
    #ifdef USE_I2C
    Wire.begin();
    Wire.setClock(400000);
    #endif

    coreroutineCrashStateTruthKeeper(1);
    if(crashState.rtcp < 30000){
        crashState.crashCnt++;
        if(crashState.crashCnt >= MAX_CRASH_COUNTER){
            crashState.fSafeMode = true;
            logger->warn(PSTR(__func__), PSTR("** SAFEMODE ACTIVATED **\n"));
        }
    }
    logger->debug(PSTR(__func__), PSTR("Runtime Counter: %d, Crash Counter: %d, Safemode Status: %s\n"), crashState.rtcp, crashState.crashCnt, crashState.fSafeMode ? PSTR("ENABLED") : PSTR("DISABLED"));
    
    if (!crashState.fSafeMode){
      logger->info(PSTR(__func__), PSTR("Hardware ID: %s\n"), config.state.hwid);

      #ifdef USE_WIFI_LOGGER
      wiFiLogger->setConfig(config.state.logIP, config.state.logPort, WIFI_LOGGER_BUFFER_SIZE);
      #endif

    if(xHandleAlarm == NULL){
        xReturnedAlarm = xTaskCreatePinnedToCore(coreroutineAlarmTaskRoutine, PSTR("coreroutineAlarmTaskRoutine"), ALARM_STACKSIZE, NULL, 1, &xHandleAlarm, 1);
        if(xReturnedAlarm == pdPASS){
          logger->warn(PSTR(__func__), PSTR("Task alarmTaskRoutine has been created.\n"));
        }
      }
    }
    coreroutineSetAlarm(0, 0, 3, 50);

    crashState.rtcp = 0;
    coreroutineCrashStateTruthKeeper(2);

}

void coreroutineDoInit(){
    logger->warn(PSTR(__func__), PSTR("Starting services setup protocol!\n"));
    if (!MDNS.begin(config.state.hname)) {
    logger->error(PSTR(__func__), PSTR("Error setting up MDNS responder!\n"));
    }
    else{
    logger->debug(PSTR(__func__), PSTR("mDNS responder started at %s\n"), config.state.hname);
    }

    MDNS.addService("http", "tcp", 80);

    #ifdef USE_WIFI_OTA
    if(config.state.fWOTA){
        logger->debug(PSTR(__func__), PSTR("Starting WiFi OTA at %s\n"), config.state.hname);
        ArduinoOTA.setHostname(config.state.hname);
        ArduinoOTA.setPasswordHash(config.state.upass);

        ArduinoOTA.onStart(coreroutineOnWiFiOTAStart);
        ArduinoOTA.onEnd(coreroutineOnWiFiOTAEnd);
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        coreroutineOnWiFiOTAProgress(progress, total);
        });
        ArduinoOTA.onError([](ota_error_t error) {
            coreroutineOnWiFiOTAError(error);
        });
        ArduinoOTA.begin();
    }
    #endif

    #ifdef USE_LOCAL_WEB_INTERFACE
    logger->debug(PSTR(__func__), PSTR("Starting Web Service...\n"));
    http.serveStatic("/", LittleFS, "/ui").setDefaultFile("index.html");
    http.serveStatic("/css/pico.blue.min.css", LittleFS, "/ui/css/pico.blue.min.css");
    http.serveStatic("/css/index.css", LittleFS, "/ui/css/index.css");
    http.serveStatic("/assets/bundle.js", LittleFS, "/ui/assets/bundle.js");


    ws.onEvent([](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
        coreroutineOnWsEvent(server, client, type, arg, data, len);
    });

    http.addHandler(&ws);
    http.begin();
    #endif
}

void coreroutuneStartServices(){
    #ifdef USE_WIFI_LOGGER
    logger->addLogger(wiFiLogger);
    #endif
    coreroutineRTCUpdate(0);
    #ifdef USE_WIFI_OTA
    if(config.state.fWOTA){
        logger->debug(PSTR(__func__), PSTR("Starting WiFi OTA at %s\n"), config.state.hname);
        ArduinoOTA.setHostname(config.state.hname);
        ArduinoOTA.setPasswordHash(config.state.upass);

        ArduinoOTA.onStart(coreroutineOnWiFiOTAStart);
        ArduinoOTA.onEnd(coreroutineOnWiFiOTAEnd);
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        coreroutineOnWiFiOTAProgress(progress, total);
        });
        ArduinoOTA.onError([](ota_error_t error) {
            coreroutineOnWiFiOTAError(error);
        });
        ArduinoOTA.begin();
    }
    #endif

    if(!MDNS.begin(config.state.hname)) {
        logger->error(PSTR(__func__), PSTR("Error setting up MDNS responder!\n"));
    }
    else{
        logger->debug(PSTR(__func__), PSTR("mDNS responder started at %s\n"), config.state.hname);
    }

    MDNS.addService("http", "tcp", 80);

    #ifdef USE_LOCAL_WEB_INTERFACE
    if(config.state.fWeb && !crashState.fSafeMode){
      logger->debug(PSTR(__func__), PSTR("Starting Web Service...\n"));
      http.serveStatic("/", LittleFS, "/ui").setDefaultFile("index.html");
      http.serveStatic("/css/pico.blue.min.css", LittleFS, "/ui/css/pico.blue.min.css");
      http.serveStatic("/css/index.css", LittleFS, "/ui/css/index.css");
      http.serveStatic("/assets/bundle.js", LittleFS, "/ui/assets/bundle.js");

      ws.onEvent([](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
          coreroutineOnWsEvent(server, client, type, arg, data, len);
      });

      http.addHandler(&ws);
      http.begin();
    }
    #endif
}

void coreroutineStopServices(){
    #ifdef USE_WIFI_LOGGER
    logger->addLogger(wiFiLogger);
    #endif

    #ifdef USE_WIFI_OTA
    if(config.state.fWOTA){
        logger->debug(PSTR(__func__), PSTR("Stopping WiFi OTA...\n"), config.state.hname);
        ArduinoOTA.end();
    }
    #endif

    logger->error(PSTR(__func__), PSTR("Stopping MDNS...\n"));
    MDNS.end();


    #ifdef USE_LOCAL_WEB_INTERFACE
    logger->error(PSTR(__func__), PSTR("Stopping HTTP...\n"));
    http.end();
    #endif
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

void coreroutineRTCUpdate(long ts){
  #ifdef USE_HW_RTC
  crashState.fRTCHwDetected = false;
  if(!hwRTC.begin()){
    logger->error(PSTR(__func__), PSTR("RTC module not found. Any function that requires precise timing will malfunction! \n"));
    logger->warn(PSTR(__func__), PSTR("Trying to recover last recorded time from flash file...! \n"));
    RTC.setTime(crashState.lastRecordedDatetime);
    logger->debug(PSTR(__func__), PSTR("Updated time via last recorded time from flash file: %s\n"), RTC.getDateTime().c_str());
  }
  else{
    crashState.fRTCHwDetected = true;
    hwRTC.setSquareWave(SquareWaveDisable);
  }
  #endif
  if(ts == 0){
    WiFiUDP ntpUDP;
    NTPClient timeClient(ntpUDP, "pool.ntp.org");
    timeClient.setTimeOffset(config.state.gmtOff);
    bool ntpSuccess = timeClient.update();
    if (ntpSuccess){
      long epochTime = timeClient.getEpochTime();
      RTC.setTime(epochTime);
      logger->debug(PSTR(__func__), PSTR("Updated time via NTP: %s GMT Offset:%d (%d) \n"), RTC.getDateTime().c_str(), config.state.gmtOff, config.state.gmtOff / 3600);
      #ifdef USE_HW_RTC
      if(crashState.fRTCHwDetected){
        logger->debug(PSTR(__func__), PSTR("Updating RTC HW from NTP...\n"));
        hwRTC.setDateTime(RTC.getHour(), RTC.getMinute(), RTC.getSecond(), RTC.getDay(), RTC.getMonth()+1, RTC.getYear(), RTC.getDayofWeek());
        logger->debug(PSTR(__func__), PSTR("Updated RTC HW from NTP with epoch %d | H:I:S W D-M-Y. -> %d:%d:%d %d %d-%d-%d\n"), 
        hwRTC.getEpoch(), RTC.getHour(), RTC.getMinute(), RTC.getSecond(), RTC.getDayofWeek(), RTC.getDay(), RTC.getMonth()+1, RTC.getYear());
      }
      #endif
    }else{
      #ifdef USE_HW_RTC
      if(crashState.fRTCHwDetected){
        logger->debug(PSTR(__func__), PSTR("Updating RTC from RTC HW with epoch %d.\n"), hwRTC.getEpoch());
        RTC.setTime(hwRTC.getEpoch());
        logger->debug(PSTR(__func__), PSTR("Updated time via RTC HW: %s GMT Offset:%d (%d) \n"), RTC.getDateTime().c_str(), config.state.gmtOff, config.state.gmtOff / 3600);
      }
      #endif
    }
  }else{
      RTC.setTime(ts);
      logger->debug(PSTR(__func__), PSTR("Updated time via timestamp: %s\n"), RTC.getDateTime().c_str());
  }
}

void coreroutineSetAlarm(uint16_t code, uint8_t color, int32_t blinkCount, uint16_t blinkDelay){
  if( xQueueAlarm != NULL ){
    AlarmMessage alarmMsg;
    alarmMsg.code = code; alarmMsg.color = color; alarmMsg.blinkCount = blinkCount; alarmMsg.blinkDelay = blinkDelay;
    if( xQueueSend( xQueueAlarm, &alarmMsg, ( TickType_t ) 1000 ) != pdPASS )
    {
        logger->debug(PSTR(__func__), PSTR("Failed to set alarm. Queue is full. \n"));
    }
  }
}

void coreroutineAlarmTaskRoutine(void *arg){
  pinMode(config.state.pinLEDR, OUTPUT);
  pinMode(config.state.pinLEDG, OUTPUT);
  pinMode(config.state.pinLEDB, OUTPUT);
  pinMode(config.state.pinBuzz, OUTPUT);
  while(true){
    if( xQueueAlarm != NULL ){
      AlarmMessage alarmMsg;
      if( xQueueReceive( xQueueAlarm,  &( alarmMsg ), ( TickType_t ) 100 ) == pdPASS )
      {
        if(alarmMsg.code > 0){
          JsonDocument doc;
          JsonObject alarm = doc[PSTR("alarm")].to<JsonObject>();
          alarm[PSTR("code")] = alarmMsg.code;
          alarm[PSTR("time")] = RTC.getDateTime();

          #ifdef USE_LOCAL_WEB_INTERFACE
          char buffer[JSON_DOC_SIZE_MEDIUM];
          serializeJson(doc, buffer);
          wsBroadcast(buffer);
          #endif
        }
        coreroutineSetLEDBuzzer(alarmMsg.color, alarmMsg.blinkCount > 0 ? true : false, alarmMsg.blinkCount, alarmMsg.blinkDelay);
        logger->debug(PSTR(__func__), PSTR("Alarm code: %d, color: %d, blinkCount: %d, blinkDelay: %d\n"), alarmMsg.code, alarmMsg.color, alarmMsg.blinkCount, alarmMsg.blinkDelay);
        vTaskDelay((const TickType_t) (alarmMsg.blinkCount * alarmMsg.blinkDelay) / portTICK_PERIOD_MS);
      }
    }
    vTaskDelay((const TickType_t) 100 / portTICK_PERIOD_MS);
  }
}

void coreroutineSetLEDBuzzer(uint8_t color, uint8_t isBlink, int32_t blinkCount, uint16_t blinkDelay){
  uint8_t r, g, b;
  switch (color)
  {
  //Auto by network
  case 0:
    if(false){
      r = config.state.LEDOn == false ? true : false;
      g = config.state.LEDOn == false ? true : false;
      b = config.state.LEDOn;
    }
    else if(WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_MODE_STA){
      r = config.state.LEDOn == false ? true : false;
      g = config.state.LEDOn;
      b = config.state.LEDOn == false ? true : false;
    }
    else if(WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_MODE_AP && WiFi.softAPgetStationNum() > 0){
      r = config.state.LEDOn == false ? true : false;
      g = config.state.LEDOn;
      b = config.state.LEDOn == false ? true : false;
    }
    else{
      r = config.state.LEDOn;
      g = config.state.LEDOn == false ? true : false;
      b = config.state.LEDOn == false ? true : false;
    }
    break;
  //RED
  case 1:
    r = config.state.LEDOn;
    g = config.state.LEDOn == false ? true : false;
    b = config.state.LEDOn == false ? true : false;
    break;
  //GREEN
  case 2:
    r = config.state.LEDOn == false ? true : false;
    g = config.state.LEDOn;
    b = config.state.LEDOn == false ? true : false;
    break;
  //BLUE
  case 3:
    r = config.state.LEDOn == false ? true : false;
    g = config.state.LEDOn == false ? true : false;
    b = config.state.LEDOn;
    break;
  default:
    r = config.state.LEDOn;
    g = config.state.LEDOn;
    b = config.state.LEDOn;
  }

  if(isBlink){
    int32_t blinkCounter = 0;
    while (blinkCounter < blinkCount)
    {
      digitalWrite(config.state.pinLEDR, config.state.LEDOn == false ? true : false);
      digitalWrite(config.state.pinLEDG, config.state.LEDOn == false ? true : false);
      digitalWrite(config.state.pinLEDB, config.state.LEDOn == false ? true : false);
      digitalWrite(config.state.pinBuzz, HIGH);
      //logger->debug(PSTR(__func__), PSTR("Blinking LED and Buzzing, blinkDelay: %d, blinkCount: %d\n"), blinkDelay, blinkCount);
      vTaskDelay(pdMS_TO_TICKS(blinkDelay));
      digitalWrite(config.state.pinLEDR, r);
      digitalWrite(config.state.pinLEDG, g);
      digitalWrite(config.state.pinLEDB, b);
      digitalWrite(config.state.pinBuzz, LOW);
      //logger->debug(PSTR(__func__), PSTR("Stop Blinking LED and Buzzing.\n"));
      vTaskDelay(pdMS_TO_TICKS(blinkDelay));
      blinkCounter++;
    }
  }
  else{
    digitalWrite(config.state.pinLEDR, r);
    digitalWrite(config.state.pinLEDG, g);
    digitalWrite(config.state.pinLEDB, b);
  }
  
}