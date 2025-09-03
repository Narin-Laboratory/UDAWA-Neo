#include "storage.h"

UdawaConfig config(PSTR("/config.json"));
CrashState crashState;
GenericConfig crashStateConfig(PSTR("/crash.json"));

void storageSetup(){
    logger->debug(PSTR(__func__), PSTR("Initializing LittleFS: %d\n"), config.begin());
    config.load();
    
    logger->setLogLevel((LogLevel)config.state.logLev);
}
