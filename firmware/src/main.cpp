#include <Arduino.h>
#include "Config.h"
#include <Adafruit_TinyUSB.h>

#include "managers/InputManager.h"
#include "managers/DisplayManager.h"
#include "managers/LightingManager.h"
#include "managers/StorageManager.h"
#include "connectivity/UsbHandler.h"

// Global State
SystemState systemState;
int currentLayer = 0; // 0 to 3

// Managers
InputManager inputManager;
DisplayManager displayManager;
LightingManager lightingManager;
StorageManager storageManager;
UsbHandler usbHandler;

// Helper: Convert key char to Row/Col
bool getKeyCoords(char keyChar, int &row, int &col) {
    if (keyChar >= '0' && keyChar <= '9') {
        int val = keyChar - '0';
        row = val / 3;
        col = val % 3;
        return true;
    } else if (keyChar >= 'A' && keyChar <= 'B') {
        row = 3;
        col = (keyChar == 'A') ? 1 : 2;
        return true;
    }
    return false;
}

void updateScreen() {
    // Get Layer Name from Config (or default)
    String name = "LAYER " + String(currentLayer + 1);
    
    // Check if we have a custom name in the JSON config
    // (Layer names are not stored in keys array, but we assume defaults for now)
    if(currentLayer == 0) name = "FN KEYS";
    if(currentLayer == 1) name = "SHORTCUTS";
    if(currentLayer == 2) name = "COMMANDS";
    if(currentLayer == 3) name = "LAUNCHER";
    
    displayManager.drawScreen(name, true, 90); // Dummy battery for now
}

void setup() {
    TinyUSBDevice.setProductDescriptor(DEVICE_NAME);
    TinyUSBDevice.setManufacturerDescriptor(MANUFACTURER_NAME);
    TinyUSBDevice.begin();

    Serial.begin(SERIAL_BAUD_RATE);
    
    // 1. Load Storage
    storageManager.begin();
    if (!storageManager.loadConfig(systemState)) {
        Serial.println("Config load failed, using defaults.");
    }

    // 2. Init Hardware
    displayManager.begin();
    lightingManager.begin();
    inputManager.begin();
    usbHandler.begin();
    
    // 3. Initial State
    updateScreen();
    
    // Apply loaded lighting settings
    if (systemState.lighting.mode > 0) {
         lightingManager.setBrightness(systemState.lighting.brightness);
         CRGB c; c.setColorCode(systemState.lighting.color);
         lightingManager.setColor(c);
         lightingManager.setMode((LightingMode)systemState.lighting.mode); // Cast int to Enum
    }
}

void loop() {
    // 1. Hardware Update
    inputManager.update();
    lightingManager.update();

    // 2. Process Queue
    InputEvent evt;
    while (inputManager.getEvent(evt)) {
        
        // --- LAYER SWITCHING (Knob Click) ---
        if (evt.type == InputEvent::KNOB_CLICK) {
            currentLayer++;
            if (currentLayer > 3) currentLayer = 0;
            
            Serial.print("Switched to Layer: ");
            Serial.println(currentLayer);
            updateScreen();
        }

        // --- VOLUME (Knob Turn) ---
        if (evt.type == InputEvent::KNOB_TURN) {
            if (evt.value > 0) usbHandler.sendKey(HID_KEY_VOLUME_UP);
            else usbHandler.sendKey(HID_KEY_VOLUME_DOWN);
            usbHandler.releaseAll();
        }

        // --- KEY PRESS ---
        if (evt.type == InputEvent::KEY_PRESS) {
            int r, c;
            if (getKeyCoords(evt.keyChar, r, c)) {
                
                // Determine Key ID string (e.g., "C0R0")
                char keyId[5];
                sprintf(keyId, "C%dR%d", c, r);
                
                // LOGIC BRANCH
                if (currentLayer == 0) {
                    // --- LAYER 0: HARDCODED F-KEYS ---
                    // Map 0-11 -> F13-F24
                    // F13 is code 0x68 (104)
                    uint8_t fKey = 0x68 + (r * 3 + c); 
                    usbHandler.sendKey(fKey);
                }
                else if (currentLayer == 1) {
                    // --- LAYER 1: SHORTCUTS (Complex HID) ---
                    // TODO: Parse systemState.layers[1][r][c].value (e.g. "Ctrl+C")
                    // For now, just blink screen to show we registered it
                    displayManager.showMessage("HID: " + String(keyId));
                }
                else {
                    // --- LAYER 2 & 3: SERIAL COMMANDS ---
                    // Format: L{layer}:{KeyID} -> "L2:C0R0"
                    String cmd = "L" + String(currentLayer) + ":" + String(keyId);
                    usbHandler.sendSerialCommand(cmd);
                    
                    // Visual feedback
                    String label = systemState.layers[currentLayer][r][c].label;
                    if (label == "") label = "EXECUTE";
                    displayManager.showMessage(label);
                }
            }
        }
        
        if (evt.type == InputEvent::KEY_RELEASE) {
            usbHandler.releaseAll();
        }
    }

    // 3. Host Sync Check
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        // Logic for file upload will go here later
    }
    
    delay(2); 
}
