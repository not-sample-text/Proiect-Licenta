#include "MacropadApp.h"
#include "config.h"
#include "BoardSupport.h"
#include "MatrixScanner.h"
#include "EncoderHandler.h"
#include "OledHandler.h"
#include "RgbHandler.h"
#include "EventQueue.h"
#include "Keymap.h"
#include "UsbHid.h"
#include "BleHid.h"

enum EncoderMode : uint8_t {
    MODE_NAV = 0,   // Screens change on rotation
    MODE_VOL = 1,   // System volume ticks on rotation
    MODE_LYR = 2    // Keymap layers cycle on rotation
};

uint8_t MacropadApp::currentLayer = 0;
bool MacropadApp::bleModeActive = false;
bool MacropadApp::lastBleSwitchState = false;
uint32_t MacropadApp::lastActivityMs = 0;
char MacropadApp::lastKey = '-';

EncoderMode currentEncoderMode = MODE_NAV;
uint32_t bleSecurePasskeyCached = 0;
bool displayPasskeyActive = false;

// Callback function targeted from BleHid when secure keys generate
void displayPasskeyOnOled(uint32_t passkey) {
    bleSecurePasskeyCached = passkey;
    displayPasskeyActive = true;
    OledHandler::update(); // Forces display to instantly render passkey numbers
}

void MacropadApp::begin() {
    BoardSupport::begin();

    bleModeActive = BoardSupport::isBleSwitchActive();
    lastBleSwitchState = bleModeActive;

    MatrixScanner::begin();
    EncoderHandler::begin();
    OledHandler::begin();
    RgbHandler::begin();

    BleHid::setPasskeyShowCallback(displayPasskeyOnOled);

    if (bleModeActive) {
        BleHid::begin();
        RgbHandler::setBleState(BleLedState::kPairing);
    } else {
        UsbHid::begin();
    }

    lastActivityMs = millis();
    Serial.println("ApexPad Application Vector Ready.");
}

void MacropadApp::run() {
    uint32_t now = millis();

    bool currentBleSwitch = BoardSupport::isBleSwitchActive();
    if (currentBleSwitch != lastBleSwitchState) {
        lastBleSwitchState = currentBleSwitch;
        bleModeActive = currentBleSwitch;
        displayPasskeyActive = false;

        if (bleModeActive) {
            Serial.println("Routing transport to BLE...");
            BleHid::begin();
            RgbHandler::setBleState(BleLedState::kPairing);
        } else {
            Serial.println("Routing transport to USB...");
            UsbHid::begin();
        }
        OledHandler::update();
    }

    MatrixScanner::scan();
    EncoderHandler::run();
    processEvents();
    RgbHandler::run();

    checkInactivityTimeout(now);
    delay(1);
}

void MacropadApp::processEvents() {
    InputEvent event;
    
    while (EventQueue::dequeue(event)) {
        lastActivityMs = millis();

        switch (event.type) {
            case EventType::KeyPress: {
                KeyAction dynamicAction = Keymap::getAction(currentLayer, event.row, event.col);
                
                if (dynamicAction.type == ActionType::SerialMessage) {
                    lastKey = 'S'; 
                    Serial.println(dynamicAction.label);
                } else {
                    lastKey = dynamicAction.label[0];
                    if (bleModeActive) {
                        BleHid::sendKey(dynamicAction.keycode, dynamicAction.modifiers, true);
                    } else {
                        UsbHid::sendKey(dynamicAction.keycode, dynamicAction.modifiers, true);
                    }
                }
                OledHandler::update();
                break;
            }

            case EventType::KeyRelease: {
                KeyAction dynamicAction = Keymap::getAction(currentLayer, event.row, event.col);
                if (dynamicAction.type != ActionType::SerialMessage) {
                    if (bleModeActive) {
                        BleHid::sendKey(dynamicAction.keycode, dynamicAction.modifiers, false);
                    } else {
                        UsbHid::sendKey(dynamicAction.keycode, dynamicAction.modifiers, false);
                    }
                }
                break;
            }

            case EventType::EncoderCW:
                if (currentEncoderMode == MODE_NAV) {
                    OledHandler::nextScreen();
                } 
                else if (currentEncoderMode == MODE_VOL) {
                    if (bleModeActive) BleHid::sendConsumerKey(0x00E9, true);  // Vol Up
                    else               UsbHid::sendConsumerKey(0x00E9, true);
                    delay(5);
                    if (bleModeActive) BleHid::sendConsumerKey(0x00E9, false); // Clear report
                    else               UsbHid::sendConsumerKey(0x00E9, false);
                } 
                else if (currentEncoderMode == MODE_LYR) {
                    currentLayer = (currentLayer + 1) % 4;
                    OledHandler::update();
                }
                break;

            case EventType::EncoderCCW:
                if (currentEncoderMode == MODE_NAV) {
                    OledHandler::previousScreen();
                } 
                else if (currentEncoderMode == MODE_VOL) {
                    if (bleModeActive) BleHid::sendConsumerKey(0x00EA, true);  // Vol Down
                    else               UsbHid::sendConsumerKey(0x00EA, true);
                    delay(5);
                    if (bleModeActive) BleHid::sendConsumerKey(0x00EA, false); // Clear report
                    else               UsbHid::sendConsumerKey(0x00EA, false);
                } 
                else if (currentEncoderMode == MODE_LYR) {
                    currentLayer = (currentLayer == 0) ? 3 : currentLayer - 1;
                    OledHandler::update();
                }
                break;

            case EventType::EncoderButton:
                // Tap advances the actual functional tracking mode state
                currentEncoderMode = (EncoderMode)((currentEncoderMode + 1) % 3);
                OledHandler::update();
                break;
        }
    }
}

void MacropadApp::checkInactivityTimeout(uint32_t now) {
    if (BoardSupport::isUsbConnected()) {
        lastActivityMs = now;
        return;
    }
    uint32_t elapsed = now - lastActivityMs;
    if (elapsed >= DEEP_SLEEP_TIMEOUT_MS) {
        OledHandler::showSleepAnimation();
        BoardSupport::enterDeepSleep(false); 
    } 
    else if (elapsed >= LIGHT_SLEEP_TIMEOUT_MS) {
        esp_sleep_enable_timer_wakeup(100000);
        esp_light_sleep_start();
        lastActivityMs = millis(); 
    }
}
