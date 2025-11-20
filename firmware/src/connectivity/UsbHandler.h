#pragma once
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

class UsbHandler {
private:
    // HID Object for Keyboard emulation
    Adafruit_USBD_HID usb_hid;

public:
    UsbHandler();
    
    void begin();
    
    // Standard HID Keyboard methods
    void sendKey(uint8_t keycode, uint8_t modifiers = 0);
    void releaseAll();
    
    // Serial Communication (For Python Host Listener)
    void sendSerialCommand(String cmd);
    
    // Check if USB is mounted and ready
    bool isReady();
};
