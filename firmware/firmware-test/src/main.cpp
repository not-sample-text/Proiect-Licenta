#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>
#include "system_organizer.h"

// Global system organizer instance
SystemOrganizer app;

void setup() {
  Serial.begin(115200);
  app.begin();
}

void loop() {
  app.run();
}
