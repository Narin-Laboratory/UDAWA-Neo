#include "main.h"

Udawa udawa;
Config config;

void setup() {
  udawa.begin();
  
  if (!udawa.crashState.fSafeMode) {
    loadConfig();
  
    saveConfig();
  }
}

unsigned long timer = millis();
void loop() {
  udawa.run();

  if (!udawa.crashState.fSafeMode) {
    if (millis() - timer > 1000) {
      udawa.logger->debug(PSTR(__func__), PSTR("%d\n"), ESP.getFreeHeap());
      JsonDocument doc;
      doc["powerSensor"] 
      timer = millis();
    }
  }
}

void loadConfig(){
  JsonDocument doc;
  bool status = configHelper.load(doc);
  udawa.logger->debug(PSTR(__func__), PSTR("%d\n"), (int)status);
  if(status){
    //if(doc[PSTR("sowingDatetime")] != nullptr){config.sowingDatetime = doc[PSTR("sowingDatetime")].as<String>();} else{config.sowingDatetime = "2024-07-09 08:00:00";}

  }
}

void saveConfig(){
  JsonDocument doc;
  //doc[PSTR("sowingDatetime")] = config.sowingDatetime;
  bool status = configHelper.save(doc);
  udawa.logger->debug(PSTR(__func__), PSTR("%d\n"), (int)status);
}