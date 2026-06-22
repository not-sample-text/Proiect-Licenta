#include "BleHid.h"
#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>
#include "RgbHandler.h"

static NimBLEHIDDevice* hidDevice = nullptr;
static NimBLECharacteristic* inputKeyboard = nullptr;
static NimBLECharacteristic* inputConsumer = nullptr;
static NimBLECharacteristic* batteryLevelChar = nullptr;

static bool connectedState = false;
static bool bondedState = false;
static void (*onPasskeyDisplay)(uint32_t) = nullptr;

static uint8_t keyboardReportBuffer[8] = {0};

class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) override {
        connectedState = true;
        RgbHandler::setBleState(BleLedState::kPairing);
    }

    void onDisconnect(NimBLEServer* pServer) override {
        connectedState = false;
        bondedState = false;
        RgbHandler::setBleState(BleLedState::kDisconnected);
        NimBLEDevice::startAdvertising();
    }
};

class SecurityCallbacks : public NimBLESecurityCallbacks {
    // FIX 1: Explicitly match Capital "K" signatures
    uint32_t onPassKeyRequest() override { 
        return 0; 
    }
    
    void onPassKeyNotify(uint32_t pass_key) override {
        Serial.printf("BLE Secure Passkey generated: %06u\n", pass_key);
        if (onPasskeyDisplay) {
            onPasskeyDisplay(pass_key);
        }
    }

    // FIX 2: Remove arguments to match 1.4.1 specification exactly
    bool onSecurityRequest() override { 
        return true; 
    }
    
    // FIX 3: Implement missing pure virtual PIN validation step
    bool onConfirmPIN(uint32_t pin) override {
        Serial.printf("Confirming BLE PIN: %06u\n", pin);
        return true; // Match authorization requirements
    }
    
    void onAuthenticationComplete(ble_gap_conn_desc* desc) override {
        if (desc->sec_state.bonded) {
            bondedState = true;
            Serial.println("Secure Bonding Completed.");
            RgbHandler::setBleState(BleLedState::kJustConnected);
        }
    }
};

void BleHid::setPasskeyShowCallback(void (*callback)(uint32_t)) {
    onPasskeyDisplay = callback;
}

void BleHid::begin() {
    NimBLEDevice::init("ApexPad-v1");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    
    NimBLEServer* server = NimBLEDevice::createServer();
    server->setCallbacks(new ServerCallbacks());

    hidDevice = new NimBLEHIDDevice(server);
    inputKeyboard = hidDevice->inputReport(1); 
    inputConsumer = hidDevice->inputReport(2); 

    NimBLEService* batteryService = server->createService(NimBLEUUID((uint16_t)0x180F));
    batteryLevelChar = batteryService->createCharacteristic(
        NimBLEUUID((uint16_t)0x2A19),
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    batteryService->start();

    hidDevice->startServices();

    NimBLEDevice::setSecurityAuth(true, true, true); // MITM, Bonding, SC
    NimBLEDevice::setSecurityCallbacks(new SecurityCallbacks());

    NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
    advertising->setAppearance(0x03C1); 
    advertising->addServiceUUID(hidDevice->hidService()->getUUID());
    advertising->start();

    Serial.println("Secure BLE Stack fully armed and broadcasting.");
}

bool BleHid::isBondedAndConnected() {
    return connectedState && bondedState;
}

void BleHid::updateBatteryLevel(uint8_t percentage) {
    if (batteryLevelChar) {
        batteryLevelChar->setValue(&percentage, 1);
        batteryLevelChar->notify();
    }
}

void BleHid::sendKey(uint16_t keycode, uint8_t modifiers, bool isPressed) {
    if (!isBondedAndConnected()) return;

    memset(keyboardReportBuffer, 0, 8);

    if (isPressed) {
        if (modifiers & (1 << 0)) keyboardReportBuffer[0] |= 0x01;
        if (modifiers & (1 << 1)) keyboardReportBuffer[0] |= 0x02;
        if (modifiers & (1 << 2)) keyboardReportBuffer[0] |= 0x04;
        if (modifiers & (1 << 3)) keyboardReportBuffer[0] |= 0x08;
        
        if (keycode != 0) {
            keyboardReportBuffer[2] = (uint8_t)keycode;
        }
    }

    inputKeyboard->setValue(keyboardReportBuffer, 8);
    inputKeyboard->notify();
}

void BleHid::sendConsumerKey(uint16_t consumerUsageId, bool isPressed) {
    if (!isBondedAndConnected()) return;

    uint8_t consumerBuffer[2] = {0};
    if (isPressed) {
        consumerBuffer[0] = (uint8_t)(consumerUsageId & 0xFF);
        consumerBuffer[1] = (uint8_t)((consumerUsageId >> 8) & 0xFF);
    }

    inputConsumer->setValue(consumerBuffer, 2);
    inputConsumer->notify();
}
