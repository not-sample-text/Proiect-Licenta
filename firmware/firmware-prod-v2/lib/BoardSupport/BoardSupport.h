#pragma once
#include <Arduino.h>

class BoardSupport {
public:
    static void begin();
    static bool isUsbConnected();
    static bool isBleSwitchActive();
    static void enterDeepSleep(bool shutdownMode);
    static void configureInactivityWakeup();
};
