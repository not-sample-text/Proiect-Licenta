#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "../Config.h"

class StorageManager {
public:
    StorageManager();
    void begin();

    // Loads JSON from file -> Fills the SystemState struct
    bool loadConfig(SystemState &state);

    // Saves a raw JSON string (from Python) -> To file
    bool saveConfig(String jsonString);

    // Helper: Returns the JSON string for Python to read (Sync)
    String getRawConfig();
};
