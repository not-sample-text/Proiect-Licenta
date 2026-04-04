/**
 * @file ble_hid.cpp
 * @brief BLE HID implementation using NimBLE.
 */

#include "BleHid.h"
#include "pins.h"
#include "debug.h"
#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEHIDDevice.h>

// ── Configuration ───────────────────────────────────────────────
#define BLE_DEVICE_NAME     "PROS3 Macropad"
#define BLE_MANUFACTURER    "Custom"
#define BLE_MODEL_NUMBER    "v1.0"
#define BLE_SERIAL_NUMBER   "0001"

// ── HID Report Descriptor ───────────────────────────────────────
// Standard HID keyboard + consumer control descriptor
static const uint8_t hid_report_descriptor[] = {
    // Keyboard Report (Report ID 1)
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x05, 0x07,        //   Usage Page (Key Codes)
    0x19, 0xE0,        //   Usage Minimum (224)
    0x29, 0xE7,        //   Usage Maximum (231)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data, Variable, Absolute) - Modifier byte
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x01,        //   Input (Constant) - Reserved byte
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x65,        //   Logical Maximum (101)
    0x05, 0x07,        //   Usage Page (Key Codes)
    0x19, 0x00,        //   Usage Minimum (0)
    0x29, 0x65,        //   Usage Maximum (101)
    0x81, 0x00,        //   Input (Data, Array) - Key array
    0xC0,              // End Collection
    
    // Consumer Control Report (Report ID 2)
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x02,        //   Report ID (2)
    0x19, 0x00,        //   Usage Minimum (0)
    0x2A, 0x3C, 0x02,  //   Usage Maximum (572)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0x3C, 0x02,  //   Logical Maximum (572)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x10,        //   Report Size (16)
    0x81, 0x00,        //   Input (Data, Array)
    0xC0,              // End Collection
};

// ── State Variables ─────────────────────────────────────────────
static bool                 g_ble_initialized = false;
static BLEState             g_ble_state = BLE_DISCONNECTED;
static NimBLEServer*        g_ble_server = nullptr;
static NimBLEHIDDevice*     g_ble_hid = nullptr;
static NimBLECharacteristic* g_input_keyboard = nullptr;
static NimBLECharacteristic* g_input_consumer = nullptr;

// ── Server Callbacks ────────────────────────────────────────────
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        g_ble_state = BLE_CONNECTED;
        DBG_INFO("BLE", "BLE client connected");
    }

    void onDisconnect(NimBLEServer* pServer) {
        g_ble_state = BLE_DISCONNECTED;
        DBG_INFO("BLE", "BLE client disconnected");
        // Restart advertising
        ble_start_advertising();
    }
};

// ── Initialization ──────────────────────────────────────────────
bool ble_hid_init() {
    DBG_INFO("BLE", "Initializing BLE HID...");
    
    // Initialize NimBLE
    NimBLEDevice::init(BLE_DEVICE_NAME);
    
    // Create BLE Server
    g_ble_server = NimBLEDevice::createServer();
    g_ble_server->setCallbacks(new ServerCallbacks());
    
    // Create HID Device
    g_ble_hid = new NimBLEHIDDevice(g_ble_server);
    g_ble_hid->manufacturer()->setValue(BLE_MANUFACTURER);
    g_ble_hid->pnp(0x02, 0x05AC, 0x820A, 0x0100);  // PnP ID
    g_ble_hid->hidInfo(0x00, 0x01);
    
    // Set HID report map
    g_ble_hid->reportMap((uint8_t*)hid_report_descriptor, sizeof(hid_report_descriptor));
    
    // Create input report characteristics
    g_input_keyboard = g_ble_hid->inputReport(1);  // Report ID 1 = Keyboard
    g_input_consumer = g_ble_hid->inputReport(2);  // Report ID 2 = Consumer
    
    // Start HID service
    g_ble_hid->startServices();
    
    // Start advertising
    ble_start_advertising();
    
    g_ble_initialized = true;
    g_ble_state = BLE_ADVERTISING;
    
    DBG_INFO("BLE", "BLE HID initialized and advertising");
    return true;
}

// ── State Management ────────────────────────────────────────────
bool ble_is_enabled() {
    // Read BT select switch (LOW = BLE mode, HIGH = USB mode)
    return digitalRead(PIN_BT_SELECT) == LOW;
}

BLEState ble_get_state() {
    return g_ble_state;
}

bool ble_is_ready() {
    return g_ble_initialized && g_ble_state == BLE_CONNECTED;
}

// ── HID Reports ─────────────────────────────────────────────────
void ble_send_key(uint8_t keycode, uint8_t modifiers, bool is_press) {
    if (!ble_is_ready() || !g_input_keyboard) {
        return;
    }
    
    // Build keyboard report: [modifiers, reserved, key1-6]
    uint8_t report[8] = {0};
    report[0] = modifiers;
    
    if (is_press) {
        report[2] = keycode;  // First key slot
        DBG_VERBOSE("BLE", "BLE Key Press: keycode=0x%02X, mods=0x%02X", keycode, modifiers);
    } else {
        DBG_VERBOSE("BLE", "BLE Key Release: keycode=0x%02X", keycode);
    }
    
    g_input_keyboard->setValue(report, sizeof(report));
    g_input_keyboard->notify();
}

void ble_send_consumer(uint16_t usage_code) {
    if (!ble_is_ready() || !g_input_consumer) {
        return;
    }
    
    DBG_VERBOSE("BLE", "BLE Consumer: usage=0x%04X", usage_code);
    
    // Send press
    uint8_t report[2];
    report[0] = usage_code & 0xFF;
    report[1] = (usage_code >> 8) & 0xFF;
    g_input_consumer->setValue(report, sizeof(report));
    g_input_consumer->notify();
    
    // Send release (0x0000) after a short delay
    delay(50);
    report[0] = 0;
    report[1] = 0;
    g_input_consumer->setValue(report, sizeof(report));
    g_input_consumer->notify();
}

// ── Connection Management ───────────────────────────────────────
void ble_start_advertising() {
    if (!g_ble_initialized || !g_ble_server) {
        return;
    }
    
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setAppearance(0x03C1);  // HID Keyboard
    pAdvertising->addServiceUUID(g_ble_hid->hidService()->getUUID());
    pAdvertising->start();
    
    g_ble_state = BLE_ADVERTISING;
    DBG_INFO("BLE", "BLE advertising started");
}

void ble_stop_advertising() {
    if (!g_ble_initialized) {
        return;
    }
    
    NimBLEDevice::getAdvertising()->stop();
    g_ble_state = BLE_DISCONNECTED;
    DBG_INFO("BLE", "BLE advertising stopped");
}

void ble_disconnect() {
    if (!g_ble_initialized || !g_ble_server) {
        return;
    }
    
    // Disconnect all connected clients
    g_ble_server->disconnect(0);
    DBG_INFO("BLE", "BLE disconnected");
}

void ble_clear_bonds() {
    NimBLEDevice::deleteAllBonds();
    DBG_INFO("BLE", "BLE bonds cleared");
}

// ── Task ────────────────────────────────────────────────────────
void ble_hid_task() {
    // NimBLE runs its own task, so nothing specific needed here
    // This is just a placeholder for future extensions
}

// ── Volume Control Helpers ──────────────────────────────────────
#define HID_USAGE_CONSUMER_VOLUME_INCREMENT  0x00E9
#define HID_USAGE_CONSUMER_VOLUME_DECREMENT  0x00EA
#define HID_USAGE_CONSUMER_MUTE              0x00E2

void ble_volume_up() {
    ble_send_consumer(HID_USAGE_CONSUMER_VOLUME_INCREMENT);
    delay(10);
    ble_send_consumer(0);  // Release
}

void ble_volume_down() {
    ble_send_consumer(HID_USAGE_CONSUMER_VOLUME_DECREMENT);
    delay(10);
    ble_send_consumer(0);  // Release
}

void ble_volume_mute() {
    ble_send_consumer(HID_USAGE_CONSUMER_MUTE);
    delay(10);
    ble_send_consumer(0);  // Release
}
