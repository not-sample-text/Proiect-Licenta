#include "KeymapTranslator.h"
#include <SPIFFS.h>

JsonDocument KeymapTranslator::libDoc;
JsonObject KeymapTranslator::lib;

void KeymapTranslator::init() {
    File file = SPIFFS.open("/library.json", "r");
    if (file) {
        deserializeJson(libDoc, file);
        lib = libDoc["library"].as<JsonObject>();
        file.close();
    }
}

HidCode KeymapTranslator::translate(String value) {
    value.toUpperCase();
    value.replace(" ", ""); // Handle "CTRL + T" -> "CTRL+T"

    uint8_t totalMod = 0;
    uint8_t keyCode = 0;

    // Split string by '+'
    int from = 0;
    int to = value.indexOf('+');
    while (to >= 0) {
        String part = value.substring(from, to);
        if (lib.containsKey(part)) totalMod |= (uint8_t)lib[part]["mod"].as<int>();
        from = to + 1;
        to = value.indexOf('+', from);
    }
    // Handle the final key
    String finalKey = value.substring(from);
    if (lib.containsKey(finalKey)) {
        keyCode = (uint8_t)lib[finalKey]["code"].as<int>();
        totalMod |= (uint8_t)lib[finalKey]["mod"].as<int>();
    }

    return {keyCode, totalMod};
}
