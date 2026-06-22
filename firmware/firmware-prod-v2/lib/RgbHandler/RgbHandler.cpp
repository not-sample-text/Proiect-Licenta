#include "RgbHandler.h"
#include "BoardSupport.h"
#include "config.h"
#include "BatteryManager.h"

Adafruit_NeoPixel RgbHandler::statusLed(1, kStatusLedPin, NEO_GRB + NEO_KHZ800);
BleLedState RgbHandler::currentBleState = BleLedState::kDisconnected;
uint32_t RgbHandler::stateTransitionTimer = 0;
bool RgbHandler::blinkToggle = false;
uint32_t RgbHandler::lastBlinkChangeMs = 0;

void RgbHandler::begin() {
    pinMode(kLegacyTpsEnablePin, OUTPUT);
    digitalWrite(kLegacyTpsEnablePin, LOW);
    pinMode(kLegacyRgbDataPin, OUTPUT);
    digitalWrite(kLegacyRgbDataPin, LOW);

    pinMode(kLdo2EnablePin, OUTPUT);
    digitalWrite(kLdo2EnablePin, HIGH);

    statusLed.begin();
    statusLed.setBrightness(128); 
    statusLed.show();
}

void RgbHandler::setBleState(BleLedState state) {
    currentBleState = state;
    if (state == BleLedState::kJustConnected) {
        stateTransitionTimer = millis() + 5000;
    } else if (state == BleLedState::kDisconnected) {
        stateTransitionTimer = millis() + 2000;
    }
}

void RgbHandler::run() {
    uint32_t now = millis();
    
    if (now - lastBlinkChangeMs >= 250) {
        blinkToggle = !blinkToggle;
        lastBlinkChangeMs = now;
    }

    handleStatusLed(now);
}

void RgbHandler::handleStatusLed(uint32_t now) {
    if (!BoardSupport::isBleSwitchActive()) {
        uint8_t battPercent = 0;
        bool percentOk = BatteryManager::getPercent(battPercent);
        
        if (!percentOk) {
            if (blinkToggle) statusLed.setPixelColor(0, statusLed.Color(128, 0, 128));
            else statusLed.setPixelColor(0, statusLed.Color(0, 0, 0));
            statusLed.show();
            return;
        }

        uint8_t red = 0, green = 0;
        if (battPercent >= 60) {
            red = map(battPercent, 60, 100, 255, 0);
            green = 255;
        } else {
            red = 255;
            green = map(battPercent, 21, 59, 0, 255);
        }
        
        if (battPercent <= 20) {
            if (blinkToggle) statusLed.setPixelColor(0, statusLed.Color(255, 0, 0));
            else statusLed.setPixelColor(0, statusLed.Color(0, 0, 0));
        } else {
            statusLed.setPixelColor(0, statusLed.Color(red, green, 0));
        }
        statusLed.show();
        return;
    }

    switch (currentBleState) {
        case BleLedState::kPairing:
            if (blinkToggle) statusLed.setPixelColor(0, statusLed.Color(0, 0, 255));
            else statusLed.setPixelColor(0, statusLed.Color(0, 0, 0));
            break;

        case BleLedState::kJustConnected:
            statusLed.setPixelColor(0, statusLed.Color(0, 0, 255));
            if (now >= stateTransitionTimer) {
                currentBleState = BleLedState::kConnectedTracking;
            }
            break;

        case BleLedState::kDisconnected:
            if (blinkToggle) statusLed.setPixelColor(0, statusLed.Color(255, 128, 0));
            else statusLed.setPixelColor(0, statusLed.Color(0, 0, 0));
            
            if (now >= stateTransitionTimer) {
                currentBleState = BleLedState::kPairing; 
            }
            break;

        case BleLedState::kConnectedTracking:
            uint8_t battPercent = 0;
            if (BatteryManager::getPercent(battPercent)) {
                uint8_t red = 0, green = 0;
                if (battPercent >= 60) {
                    red = map(battPercent, 60, 100, 255, 0);
                    green = 255;
                } else {
                    red = 255;
                    green = map(battPercent, 21, 59, 0, 255);
                }
                
                if (battPercent <= 20 && blinkToggle) {
                    statusLed.setPixelColor(0, statusLed.Color(255, 0, 0));
                } else if (battPercent <= 20 && !blinkToggle) {
                    statusLed.setPixelColor(0, statusLed.Color(0, 0, 0));
                } else {
                    statusLed.setPixelColor(0, statusLed.Color(red, green, 0));
                }
            }
            break;
    }
    statusLed.show();
}
