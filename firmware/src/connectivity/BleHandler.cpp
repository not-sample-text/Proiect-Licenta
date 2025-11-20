#include "BleHandler.h"

BleHandler::BleHandler() {
    // Constructor
}

void BleHandler::begin() {
    // NimBLE initialization
    Serial.println("[BLE] Initialized (Skeleton)");
}

void BleHandler::sendKey(uint8_t keycode) {
    Serial.print("[BLE] Sending Key: ");
    Serial.println(keycode);
}

bool BleHandler::isConnected() {
    return false; // Always false for skeleton
}
