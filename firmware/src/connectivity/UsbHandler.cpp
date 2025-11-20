#include "UsbHandler.h"

// Standard HID Keyboard Report Descriptor
// This tells the PC "I am a Keyboard"
uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD()
};

UsbHandler::UsbHandler() {
    // Constructor
}

void UsbHandler::begin() {
    // Configure HID
    usb_hid.setPollInterval(2); // 2ms polling rate (500Hz)
    usb_hid.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
    usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
    usb_hid.begin();

    // Wait for USB to mount (optional, non-blocking preferred in loop)
    Serial.println("[USB] Composite HID+CDC Initialized");
}

bool UsbHandler::isReady() {
    return TinyUSBDevice.mounted();
}

void UsbHandler::sendKey(uint8_t keycode, uint8_t modifiers) {
    // If USB isn't ready, don't crash
    if (!usb_hid.ready()) return;

    // HID Report: [Modifier, Reserved, Key1, Key2, Key3, Key4, Key5, Key6]
    // We simulate a simple press: Down -> Up
    uint8_t keycode_arr[6] = { keycode, 0, 0, 0, 0, 0 };
    
    // 1. Press
    usb_hid.keyboardReport(0, modifiers, keycode_arr);
    delay(10); // Debounce / Host processing time
    
    // 2. Release
    usb_hid.keyboardRelease(0);
}

void UsbHandler::releaseAll() {
    if (usb_hid.ready()) {
        usb_hid.keyboardRelease(0);
    }
}

void UsbHandler::sendSerialCommand(String cmd) {
    // Send to the Serial Monitor / Python Script
    Serial.println(cmd);
}
