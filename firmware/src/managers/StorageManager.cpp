#include "StorageManager.h"

StorageManager::StorageManager() {}

void StorageManager::begin() {
    // Try to mount the file system
    // true = format if failed (important for first run!)
    if (!LittleFS.begin(true)) {
        Serial.println("[Storage] LittleFS Mount Failed");
        return;
    }
    Serial.println("[Storage] Initialized");
}

bool StorageManager::saveConfig(String jsonString) {
    File file = LittleFS.open(CONFIG_FILE_PATH, "w");
    if (!file) {
        Serial.println("[Storage] Failed to open file for writing");
        return false;
    }
    
    int bytesWritten = file.print(jsonString);
    file.close();
    
    Serial.print("[Storage] Saved ");
    Serial.print(bytesWritten);
    Serial.println(" bytes");
    return true;
}

String StorageManager::getRawConfig() {
    if (!LittleFS.exists(CONFIG_FILE_PATH)) return "{}";
    
    File file = LittleFS.open(CONFIG_FILE_PATH, "r");
    String raw = file.readString();
    file.close();
    return raw;
}

bool StorageManager::loadConfig(SystemState &state) {
    if (!LittleFS.exists(CONFIG_FILE_PATH)) {
        Serial.println("[Storage] No config file found");
        return false;
    }

    File file = LittleFS.open(CONFIG_FILE_PATH, "r");
    
    // Allocate a temporary JSON document
    // 4KB should be enough for this specific structure
    JsonDocument doc;
    
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.print("[Storage] JSON Parse Error: ");
        Serial.println(error.c_str());
        return false;
    }

    // 1. PARSE LIGHTING
    JsonObject lighting = doc["lighting"];
    String modeStr = lighting["mode"] | "SOLID";
    
    // Simple Enum mapping
    if (modeStr == "RAINBOW") state.lighting.mode = 1;
    else if (modeStr == "BREATHING") state.lighting.mode = 2;
    else if (modeStr == "REACTIVE") state.lighting.mode = 3;
    else if (modeStr == "OFF") state.lighting.mode = 4;
    else state.lighting.mode = 0; // SOLID

    state.lighting.brightness = lighting["brightness"] | 128;
    state.lighting.speed = lighting["speed"] | 10;
    
    // Parse Color Hex String (#RRGGBB) to uint32
    String colorStr = lighting["color"] | "#000000";
    colorStr.replace("#", "");
    state.lighting.color = strtoul(colorStr.c_str(), NULL, 16);

    // 2. PARSE LAYERS
    JsonArray layers = doc["layers"];
    for (JsonObject layer : layers) {
        int id = layer["id"];
        if (id < 0 || id > 3) continue; // Safety check

        JsonObject keys = layer["keys"];
        
        // Iterate over C0R0, C1R0...
        for (JsonPair kv : keys) {
            String keyCoord = String(kv.key().c_str()); // e.g. "C2R1"
            JsonObject keyData = kv.value().as<JsonObject>();

            // Extract coordinates from string "C{col}R{row}"
            int cIndex = keyCoord.indexOf('C');
            int rIndex = keyCoord.indexOf('R');
            
            if (cIndex != -1 && rIndex != -1) {
                int col = keyCoord.substring(cIndex + 1, rIndex).toInt();
                int row = keyCoord.substring(rIndex + 1).toInt();

                if (row < 4 && col < 3) {
                    // ! FIX: Apply default operator | directly to JsonVariant
                    state.layers[id][row][col].label = keyData["label"] | "";
                    state.layers[id][row][col].value = keyData["value"] | "";
                    state.layers[id][row][col].type  = keyData["type"]  | "EMPTY";
                }
            }
        }
    }

    Serial.println("[Storage] Config Loaded into RAM");
    return true;
}
