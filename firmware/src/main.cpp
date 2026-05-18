#include <Arduino.h>
#include "MacropadApp/MacropadApp.h"

void setup() {
    MacropadApp::begin();
}

void loop() {
    MacropadApp::run();
    delay(1); // Small yield for RTOS
}
