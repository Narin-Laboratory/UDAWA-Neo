#include <ArduinoJson.h>
#include "main.h"

void setup(){
    loggingSetup();
    storageSetup();
    networkingSetup();
    coreroutineSetup();

    //strcpy(config.state.wssid, "Govinda");
    //strcpy(config.state.wpass, "goloka8881");
    //config.save();
    
    logger->debug(PSTR(__func__), PSTR("Hello world!\n"));
}

void loop(){
    coreroutineLoop();
}