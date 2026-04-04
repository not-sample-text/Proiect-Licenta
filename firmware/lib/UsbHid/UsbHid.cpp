/**
 * @file usb_hid.cpp
 * @brief USB HID implementation for keyboard and consumer control.
 * 
 * This file handles USB HID initialization, report sending, and event callbacks.
 * It uses TinyUSB to implement a composite device (Keyboard + Consumer Control).
 */

#include <Arduino.h>
#include <tusb.h>
#include "UsbHid.h"
#include "debug.h"

// HID Report Descriptor
const uint8_t desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(),
    TUD_HID_REPORT_DESC_CONSUMER()
};

// USB Device Callbacks
void tud_mount_cb(void) {
    DBG_INFO("USB", "USB mounted");
}

void tud_umount_cb(void) {
    DBG_INFO("USB", "USB unmounted");
}

void tud_suspend_cb(bool remote_wakeup_en) {
    DBG_INFO("USB", "USB suspended");
}

void tud_resume_cb(void) {
    DBG_INFO("USB", "USB resumed");
}

// Forward declaration
void hid_send_key(uint8_t keycode, uint8_t modifiers, bool is_press);

// Send a keyboard report (single key)
void send_hid_report(uint8_t keycode) {
    hid_send_key(keycode, 0, true);
}

// Send a keyboard report with modifiers and press/release
void hid_send_key(uint8_t keycode, uint8_t modifiers, bool is_press) {
    if (tud_hid_ready()) {
        uint8_t key_report[6] = {0};
        if (is_press) {
            key_report[0] = keycode;
            DBG_VERBOSE("USB", "HID Key Press: keycode=0x%02X, mods=0x%02X", keycode, modifiers);
        } else {
            // Release: all zeros
            DBG_VERBOSE("USB", "HID Key Release: keycode=0x%02X, mods=0x%02X", keycode, modifiers);
        }
        tud_hid_keyboard_report(0, modifiers, key_report);
    } else {
        DBG_WARN("USB", "HID not ready, key report dropped");
    }
}

// Send a consumer control report
void send_consumer_report(uint16_t usage_code) {
    if (tud_hid_ready()) {
        DBG_VERBOSE("USB", "Sending Consumer Control report: usage=0x%04X", usage_code);
        tud_hid_report(1, &usage_code, sizeof(usage_code));
    } else {
        DBG_WARN("USB", "HID not ready, consumer report dropped");
    }
}

// Initialize USB HID
void setup_usb_hid() {
    tusb_init();
    DBG_INFO("USB", "USB HID initialized (TinyUSB)");
}

// Call this in the main loop
void usb_hid_task() {
    tud_task();  // Handle USB events
}

bool usb_hid_is_ready() {
    return tud_hid_ready();
}

// ── Volume Control Helpers ──────────────────────────────────────
void hid_volume_up() {
    send_consumer_report(HID_USAGE_CONSUMER_VOLUME_INCREMENT);
    delay(10);  // Brief delay
    send_consumer_report(0);  // Release
}

void hid_volume_down() {
    send_consumer_report(HID_USAGE_CONSUMER_VOLUME_DECREMENT);
    delay(10);  // Brief delay
    send_consumer_report(0);  // Release
}

void hid_volume_mute() {
    send_consumer_report(HID_USAGE_CONSUMER_MUTE);
    delay(10);  // Brief delay
    send_consumer_report(0);  // Release
}
