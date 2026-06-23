#pragma once

#include <Arduino.h>

class BoardSupport {
public:
    static void begin();
    
    static bool isBleSwitchActive();
    static bool isUsbConnected();
    
    // Configures auto light sleep and triggers deep sleep vectors
    static void enterDeepSleep(bool hardShutdown);
};
