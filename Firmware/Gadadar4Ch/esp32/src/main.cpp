#include <ArduinoJson.h>
#include "main.h"

void setup(){
    loggingSetup();
    storageSetup();
    networkingSetup();
    coreroutineSetup();

    strcpy(config.state.wssid, "Govinda");
    strcpy(config.state.wpass, "goloka8881");
    strcpy(config.state.provDK, "a2l5rmf3ezpwygyhkqxs");
    strcpy(config.state.provDS, "bw6vxpkm66v2c1iwbpuw");
    config.save();
    
    logger->debug(PSTR(__func__), PSTR("Hello world!\n"));
}

void loop(){
    coreroutineLoop();
}