#include "SerialControl.h"
#include <Arduino.h>
#include "debug.h"
#include "Config.h"
#include "Oled.h"
#include "Power.h"
#include "Rgb.h"

namespace {

enum ConfigState {
    CFG_IDLE,
    CFG_RECEIVING,
};

ConfigState g_cfg_state = CFG_IDLE;
String g_cfg_buffer;
size_t g_cfg_expected_size = 0;

} // namespace

void SerialControl::begin() {
    g_cfg_state = CFG_IDLE;
    g_cfg_buffer = "";
    g_cfg_expected_size = 0;
}

void SerialControl::run() {
    static String cmd_buffer;

    while (Serial.available()) {
        char c = Serial.read();

        if (c == '\n') {
            cmd_buffer.trim();

            if (cmd_buffer == "DBG:ON") {
                g_debug_enabled = true;
                Serial.println("#Debug output: ENABLED");
            } else if (cmd_buffer == "DBG:OFF") {
                Serial.println("#Debug output: DISABLED");
                g_debug_enabled = false;
            } else if (cmd_buffer.startsWith("DBG:")) {
                Serial.println("#Unknown debug command");
            } else if (cmd_buffer == "PWR:SHUTDOWN") {
                power_request_shutdown();
                Serial.println("#PWR:SHUTDOWN:ACK");
            } else if (cmd_buffer.startsWith("CFG:START:")) {
                String size_str = cmd_buffer.substring(10);
                g_cfg_expected_size = size_str.toInt();

                if (g_cfg_expected_size > 0 && g_cfg_expected_size <= CONFIG_MAX_SIZE) {
                    g_cfg_state = CFG_RECEIVING;
                    g_cfg_buffer = "";
                    g_cfg_buffer.reserve(g_cfg_expected_size);
                    Serial.printf("#CFG:READY:%u\n", g_cfg_expected_size);
                    DBG_INFO("MAIN", "Config upload started: %u bytes expected", g_cfg_expected_size);
                } else {
                    Serial.println("#CFG:ERROR:Invalid size");
                }
            } else if (cmd_buffer.startsWith("CFG:DATA:") && g_cfg_state == CFG_RECEIVING) {
                String data = cmd_buffer.substring(9);
                g_cfg_buffer += data;
                Serial.printf("#CFG:ACK:%u/%u\n", g_cfg_buffer.length(), g_cfg_expected_size);
            } else if (cmd_buffer == "CFG:END" && g_cfg_state == CFG_RECEIVING) {
                if (g_cfg_buffer.length() == g_cfg_expected_size) {
                    if (config_save(g_cfg_buffer.c_str(), g_cfg_buffer.length())) {
                        Serial.println("#CFG:SUCCESS");
                        DBG_INFO("MAIN", "Config saved and reloaded");

                        const RGBConfig& cfg = config_get_rgb();
                        rgb_set_mode((RGBMode)cfg.mode);
                        rgb_set_brightness(cfg.brightness);
                        rgb_set_speed(cfg.speed);
                        rgb_set_color((cfg.color >> 16) & 0xFF,
                                      (cfg.color >> 8) & 0xFF,
                                      cfg.color & 0xFF);

                        oled_show_status("Config Updated", 2000);
                    } else {
                        Serial.println("#CFG:ERROR:Save failed");
                        DBG_ERROR("MAIN", "Config save failed");
                    }
                } else {
                    Serial.printf("#CFG:ERROR:Size mismatch %u/%u\n",
                                  g_cfg_buffer.length(), g_cfg_expected_size);
                }

                g_cfg_state = CFG_IDLE;
                g_cfg_buffer = "";
                g_cfg_expected_size = 0;
            } else if (cmd_buffer == "CFG:ABORT") {
                g_cfg_state = CFG_IDLE;
                g_cfg_buffer = "";
                g_cfg_expected_size = 0;
                Serial.println("#CFG:ABORTED");
            }

            cmd_buffer = "";
        } else if (c >= 32 && c < 127) {
            cmd_buffer += c;

            if (cmd_buffer.length() > 256) {
                cmd_buffer = "";
                Serial.println("#Error: Command too long");
            }
        }
    }
}
