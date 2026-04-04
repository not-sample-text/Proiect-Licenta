#include <Arduino.h>
#include <MacropadApp.h>

namespace {
MacropadApp g_app;
}

void setup() {
    g_app.begin();
}

void loop() {
    g_app.run();
}
