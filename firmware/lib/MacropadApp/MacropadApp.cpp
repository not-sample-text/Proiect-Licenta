#include "MacropadApp.h"
#include "Core/EventQueue.h"
#include "Keymap/Keymap.h"
#include "BoardSupport/BoardSupport.h"
#include "MatrixScanner/MatrixScanner.h"
#include "EncoderHandler/EncoderHandler.h"
#include "OledHandler/OledHandler.h"
#include "RgbHandler/RgbHandler.h"
#include "BleHid/BleHid.h"
#include "UsbHid/UsbHid.h"

uint8_t MacropadApp::currentLayer = 0;
MacropadApp::EncoderMode MacropadApp::encoderMode = MacropadApp::EncoderMode::Volume;
bool MacropadApp::needsDisplayUpdate = true;
bool MacropadApp::g_ble_mode = false;

void MacropadApp::begin() {
    BoardSupport::begin();
    
    // Decide HID mode
    g_ble_mode = ble_is_enabled();
    
    MatrixScanner::begin();
    EncoderHandler::begin();
    OledHandler::begin();
    RgbHandler::begin();
    
    if (g_ble_mode) {
        ble_hid_init();
        OledHandler::showStatus("Mode: BLE");
    } else {
        setup_usb_hid();
        OledHandler::showStatus("Mode: USB");
    }
    
    Serial.println("Macropad App Started");
}

void MacropadApp::run() {
    MatrixScanner::scan();
    EncoderHandler::run();
    processEvents();
    
    OledHandler::run(needsDisplayUpdate);
    RgbHandler::run();
    
    if (g_ble_mode) {
        ble_hid_task();
    } else {
        usb_hid_task();
    }
    
    // Reset dirty flag after processing
    needsDisplayUpdate = false;
}

void MacropadApp::processEvents() {
    InputEvent event;
    while (EventQueue::dequeue(event)) {
        needsDisplayUpdate = true;
        
        switch (event.type) {
            case EventType::KeyPress: {
                const char* label = Keymap::getLabel(currentLayer, event.row, event.col);
                uint8_t keycode = Keymap::getKeyCode(currentLayer, event.row, event.col);
                
                Serial.printf("Key Press: %s (R%d C%d)\n", label, event.row, event.col);
                OledHandler::showStatus(label);
                
                if (g_ble_mode) ble_send_key(keycode, 0, true);
                else hid_send_key(keycode, 0, true);
                break;
            }
                
            case EventType::KeyRelease: {
                uint8_t keycode = Keymap::getKeyCode(currentLayer, event.row, event.col);
                if (g_ble_mode) ble_send_key(keycode, 0, false);
                else hid_send_key(keycode, 0, false);
                break;
            }
                
            case EventType::EncoderCW:
                if (encoderMode == EncoderMode::Volume) {
                    if (g_ble_mode) ble_volume_up();
                    else hid_volume_up();
                    OledHandler::showStatus("Vol +");
                } else {
                    currentLayer = (currentLayer + 1) % 4;
                    OledHandler::showStatus("Layer Up");
                }
                break;
                
            case EventType::EncoderCCW:
                if (encoderMode == EncoderMode::Volume) {
                    if (g_ble_mode) ble_volume_down();
                    else hid_volume_down();
                    OledHandler::showStatus("Vol -");
                } else {
                    currentLayer = (currentLayer == 0) ? 3 : currentLayer - 1;
                    OledHandler::showStatus("Layer Down");
                }
                break;
                
            case EventType::EncoderButton:
                encoderMode = (encoderMode == EncoderMode::Volume) ? EncoderMode::Layer : EncoderMode::Volume;
                OledHandler::showStatus(encoderMode == EncoderMode::Volume ? "Mode: VOL" : "Mode: LYR");
                break;
        }
    }
}
