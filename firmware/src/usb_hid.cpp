/**
 * @file usb_hid.cpp
 * @brief USB HID implementation for keyboard and consumer control.
 * 
 * This file handles USB HID initialization, report sending, and event callbacks.
 * It uses TinyUSB to implement a composite device (Keyboard + Consumer Control).
 */

#include <Arduino.h>
#include <tusb.h>
#include "debug.h"

// HID Report Descriptor
const uint8_t desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(),
    TUD_HID_REPORT_DESC_CONSUMER()
};

// USB Device Callbacks
void tud_mount_cb(void) {
    DBG_INFO("USB mounted");
}

void tud_umount_cb(void) {
    DBG_INFO("USB unmounted");
}

void tud_suspend_cb(bool remote_wakeup_en) {
    DBG_INFO("USB suspended");
}

void tud_resume_cb(void) {
    DBG_INFO("USB resumed");
}

// Send a keyboard report
void send_hid_report(uint8_t keycode) {
    if (tud_hid_ready()) {
        DBG_VERBOSE("Sending HID report: keycode=0x%02X", keycode);
        uint8_t key_report[6] = {0};
        key_report[0] = keycode;
        tud_hid_keyboard_report(0, 0, key_report);
    } else {
        DBG_WARN("HID not ready, report dropped");
    }
}

// Send a consumer control report
void send_consumer_report(uint16_t usage_code) {
    if (tud_hid_ready()) {
        DBG_VERBOSE("Sending Consumer Control report: usage=0x%04X", usage_code);
        tud_hid_report(1, &usage_code, sizeof(usage_code));
    } else {
        DBG_WARN("HID not ready, consumer report dropped");
    }
}

// TinyUSB HID Task
void hid_task(void) {
    // Example condition: Check if HID is ready
    if (tud_hid_ready()) {
        send_hid_report(HID_KEY_A);  // Example: Send 'A' key
    }
}

// Initialize USB HID
void setup_usb_hid() {
    tusb_init();
}

// Call this in the main loop
void usb_hid_task() {
    tud_task();  // Handle USB events
    hid_task();  // Handle HID reports
}
