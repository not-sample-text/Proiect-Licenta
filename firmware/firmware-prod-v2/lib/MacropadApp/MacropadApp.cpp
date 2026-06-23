#include "MacropadApp.h"
#include "config.h"
#include "BoardSupport.h"
#include "BatteryManager.h"
#include "MatrixScanner.h"
#include "EncoderHandler.h"
#include "OledHandler.h"
#include "RgbHandler.h"
#include "EventQueue.h"
#include "LayoutLoader.h"
#include "KeymapTranslator.h"
#include "UsbHid.h"
#include "BleHid.h"

enum EncoderMode : uint8_t {
    MODE_NAV = 0,   
    MODE_VOL = 1,   
    MODE_LYR = 2    
};

uint8_t MacropadApp::currentLayer = 0;
bool MacropadApp::bleModeActive = false;
bool MacropadApp::lastBleSwitchState = false;
uint32_t MacropadApp::lastActivityMs = 0;
const char* MacropadApp::lastKeyLabel = "IDLE";

EncoderMode currentEncoderMode = MODE_NAV;
uint32_t bleSecurePasskeyCached = 0;
bool displayPasskeyActive = false;

static uint32_t switchDebounceTimer = 0;
static bool candidateSwitchState = false;

void displayPasskeyOnOled(uint32_t passkey) {
    bleSecurePasskeyCached = passkey;
    displayPasskeyActive = true;
    MacropadApp::resetActivityTimer(); 
    OledHandler::update(); 
}

void MacropadApp::begin() {
    delay(2000);
    
    BoardSupport::begin();
    
    LayoutLoader::begin();
    KeymapTranslator::init();

    if (!LayoutLoader::begin()) {
        Serial.println("[DEBUG] Failed to load config.json from SPIFFS!");
    }

    delay(50);
    bleModeActive = BoardSupport::isBleSwitchActive();
    lastBleSwitchState = bleModeActive;
    candidateSwitchState = bleModeActive;

    MatrixScanner::begin();
    EncoderHandler::begin();
    
    OledHandler::begin();
    OledHandler::showBootAnimation(); 
    
    RgbHandler::begin();

    BleHid::setPasskeyShowCallback(displayPasskeyOnOled);

    if (bleModeActive) {
        Serial.println("Booting strictly into BLE Transport...");
        BleHid::begin();
        RgbHandler::setBleState(BleLedState::kPairing);
    } else {
        Serial.println("Booting strictly into Native USB Transport...");
        UsbHid::begin();
    }

    lastActivityMs = millis();
    Serial.println("ApexPad Application Vector Ready.");
    
    OledHandler::update(); 
}

void MacropadApp::run() {
    uint32_t now = millis();

    bool currentBleSwitch = BoardSupport::isBleSwitchActive();
    
    if (currentBleSwitch != candidateSwitchState) {
        candidateSwitchState = currentBleSwitch;
        switchDebounceTimer = now;
    }

    if ((currentBleSwitch != lastBleSwitchState) && ((now - switchDebounceTimer) > 500)) {
        Serial.println("Transport Layer switch state stabilized. Forcing clean state restart...");
        delay(100);
        ESP.restart();
    }

    static uint32_t lastBleBatteryUpdate = 0;
    if (bleModeActive && (now - lastBleBatteryUpdate >= 30000 || lastBleBatteryUpdate == 0)) {
        uint8_t batPct = 0;
        if (BatteryManager::getPercent(batPct)) {
            BleHid::updateBatteryLevel(batPct);
        }
        lastBleBatteryUpdate = now;
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
                KeyAction action = LayoutLoader::getKeyAction(currentLayer, event.row, event.col);
                
                if (!action.isValid) {
                    lastKeyLabel = "UNUSED";
                    OledHandler::update();
                    return; 
                }

                lastKeyLabel = action.label.c_str();

                if (action.type == "KEY" || action.type == "SHORTCUT") {
                    HidCode code = KeymapTranslator::translate(action.value);
                    if (bleModeActive) BleHid::sendKey(code.keycode, code.modifiers, true); 
                    else               UsbHid::sendKey(code.keycode, code.modifiers, true);
                } 
                
                if (action.type == "APP" || action.type == "SCRIPT") {
                    uint8_t packed = LayoutLoader::getPackedByte(currentLayer, event.row, event.col);
                    Serial.print("[CMD]");
                    Serial.write(packed);
                }
                
                OledHandler::update();
                break;
            }

            case EventType::KeyRelease: {
                KeyAction action = LayoutLoader::getKeyAction(currentLayer, event.row, event.col);
                if (action.isValid && (action.type == "KEY" || action.type == "SHORTCUT")) {
                    HidCode code = KeymapTranslator::translate(action.value);
                    if (bleModeActive) BleHid::sendKey(code.keycode, code.modifiers, false);
                    else               UsbHid::sendKey(code.keycode, code.modifiers, false);
                }
                break;
            }

            case EventType::EncoderCW:
                if (currentEncoderMode == MODE_NAV) OledHandler::nextScreen();
                else if (currentEncoderMode == MODE_VOL) {
                    if (bleModeActive) BleHid::sendConsumerKey(0x00E9, true);  
                    else               UsbHid::sendConsumerKey(0x00E9, true);
                    delay(5);
                    if (bleModeActive) BleHid::sendConsumerKey(0x00E9, false); 
                    else               UsbHid::sendConsumerKey(0x00E9, false);
                } else if (currentEncoderMode == MODE_LYR) {
                    currentLayer = (currentLayer + 1) % 4;
                    OledHandler::update();
                }
                break;

            case EventType::EncoderCCW:
                if (currentEncoderMode == MODE_NAV) OledHandler::previousScreen();
                else if (currentEncoderMode == MODE_VOL) {
                    if (bleModeActive) BleHid::sendConsumerKey(0x00EA, true);  
                    else               UsbHid::sendConsumerKey(0x00EA, true);
                    delay(5);
                    if (bleModeActive) BleHid::sendConsumerKey(0x00EA, false); 
                    else               UsbHid::sendConsumerKey(0x00EA, false);
                } else if (currentEncoderMode == MODE_LYR) {
                    currentLayer = (currentLayer == 0) ? 3 : currentLayer - 1;
                    OledHandler::update();
                }
                break;

            case EventType::EncoderButton:
                currentEncoderMode = (EncoderMode)((currentEncoderMode + 1) % 3);
                OledHandler::update();
                break;
        }
    }
}

void MacropadApp::checkInactivityTimeout(uint32_t now) {
    if (BoardSupport::isUsbConnected() || displayPasskeyActive) {
        lastActivityMs = now;
        return;
    }
    uint32_t elapsed = now - lastActivityMs;
    if (elapsed >= DEEP_SLEEP_TIMEOUT_MS) {
        OledHandler::showSleepAnimation();
        OledHandler::clear(); 
        BoardSupport::enterDeepSleep(false); 
    } 
}
