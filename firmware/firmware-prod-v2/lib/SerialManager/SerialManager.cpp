#include "SerialManager.h"
#include <SPIFFS.h>
#include "OledHandler.h"

char SerialManager::serialBuffer[64];
uint8_t SerialManager::bufferIndex = 0;

void SerialManager::begin() {
    Serial.begin(115200);
    bufferIndex = 0;
    memset(serialBuffer, 0, sizeof(serialBuffer));
}

void SerialManager::check() {
    while (Serial.available() > 0) {
        char c = Serial.read();
        
        if (c == '\n' || c == '\r' || c == ']') {
            if (c == ']') {
                if (bufferIndex < sizeof(serialBuffer) - 1) {
                    serialBuffer[bufferIndex++] = c;
                }
            }
            
            serialBuffer[bufferIndex] = '\0';
            String cmd(serialBuffer);

            if (cmd.indexOf("[PING]") >= 0) {
                Serial.print("[PONG:APEXPAD]\n");
            } else if (cmd.indexOf("[CFG_READ_REQ]") >= 0) {
                handleConfigRead();
            } else if (cmd.indexOf("[CFG_WRITE_REQ]") >= 0) {
                handleConfigWrite();
            }

            bufferIndex = 0;
            memset(serialBuffer, 0, sizeof(serialBuffer));
        } else {
            if (bufferIndex < sizeof(serialBuffer) - 1) {
                serialBuffer[bufferIndex++] = c;
            }
        }
    }
}

void SerialManager::handleConfigRead() {
    Serial.print("[CFG_READ_START]\n");
    
    File file = SPIFFS.open("/config.json", FILE_READ);
    if (file) {
        while (file.available()) {
            Serial.write(file.read());
        }
        file.close();
    }
    
    Serial.print("\n[CFG_READ_END]\n");
}

void SerialManager::handleConfigWrite() {
    OledHandler::showSystemMessage("SYNCING...");
    
    if (SPIFFS.exists("/config.json")) {
        SPIFFS.rename("/config.json", "/config.bak");
    }
    
    File file = SPIFFS.open("/config.json", FILE_WRITE);
    Serial.print("[CFG_WRITE_ACK]\n");
    
    uint32_t lastDataTime = millis();
    String currentLine = "";
    
    // Blocking loop to securely capture the file stream
    while (true) {
        while (Serial.available()) {
            char c = Serial.read();
            if (c == '\n' || c == '\r') {
                if (currentLine.indexOf("[CFG_WRITE_EOF]") >= 0) {
                    file.close();
                    SPIFFS.remove("/config.bak");
                    Serial.print("[CFG_WRITE_OK]\n");
                    OledHandler::showSystemMessage("SUCCESS");
                    delay(1000);
                    ESP.restart();
                } else {
                    if (currentLine.length() > 0) {
                        file.print(currentLine);
                        file.print("\n");
                    }
                }
                currentLine = "";
            } else {
                currentLine += c;
            }
            lastDataTime = millis();
        }
        
        // Timeout Failsafe (5 Seconds)
        if (millis() - lastDataTime > 5000) {
            file.close();
            SPIFFS.remove("/config.json");
            
            // Restore from backup
            if (SPIFFS.exists("/config.bak")) {
                SPIFFS.rename("/config.bak", "/config.json");
            }
            
            Serial.print("[CFG_WRITE_ERR]\n");
            OledHandler::showSystemMessage("ERROR");
            delay(2000);
            ESP.restart();
        }
    }
}
