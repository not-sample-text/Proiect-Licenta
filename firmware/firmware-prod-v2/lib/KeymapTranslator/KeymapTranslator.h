#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

struct HidCode {
    uint8_t keycode;
    uint8_t modifiers;
};

class KeymapTranslator {
public:
    static void init(); // Added declaration
    static HidCode translate(String value);
private:
    static JsonDocument libDoc;
    static JsonObject lib;
};
