/**
 * @file config.cpp
 * @brief Configuration file management implementation.
 */

#include "config.h"
#include "debug.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

// ── State ───────────────────────────────────────────────────────
static bool g_config_loaded = false;
static JsonDocument g_config_doc;  // Parsed JSON document
static RGBConfig g_rgb_config;

// ── Initialization ──────────────────────────────────────────────
bool config_init() {
    DBG_INFO("CFG", "Initializing configuration system...");
    
    // Mount LittleFS
    if (!LittleFS.begin(true)) {  // format on fail = true
        DBG_ERROR("CFG", "Failed to mount LittleFS");
        return false;
    }
    
    // Get filesystem info
    size_t total, used;
    config_get_fs_info(&total, &used);
    DBG_INFO("CFG", "  LittleFS mounted: %u KB total, %u KB used", 
             total / 1024, used / 1024);
    
    // Attempt to load config
    if (config_load()) {
        DBG_INFO("CFG", "Configuration loaded successfully");
        return true;
    } else {
        DBG_WARN("CFG", "Using default configuration");
        return false;
    }
}

// ── Loading ─────────────────────────────────────────────────────
bool config_load() {
    // Check if config file exists
    if (!LittleFS.exists(CONFIG_FILE_PATH)) {
        DBG_WARN("CFG", "Config file not found: %s", CONFIG_FILE_PATH);
        g_config_loaded = false;
        return false;
    }
    
    // Open file
    File file = LittleFS.open(CONFIG_FILE_PATH, "r");
    if (!file) {
        DBG_ERROR("CFG", "Failed to open config file");
        g_config_loaded = false;
        return false;
    }
    
    // Check file size
    size_t file_size = file.size();
    if (file_size == 0 || file_size > CONFIG_MAX_SIZE) {
        DBG_ERROR("CFG", "Invalid config file size: %u bytes", file_size);
        file.close();
        g_config_loaded = false;
        return false;
    }
    
    DBG_VERBOSE("CFG", "Loading config file (%u bytes)...", file_size);
    
    // Parse JSON
    DeserializationError error = deserializeJson(g_config_doc, file);
    file.close();
    
    if (error) {
        DBG_ERROR("CFG", "Failed to parse config JSON: %s", error.c_str());
        g_config_loaded = false;
        return false;
    }
    
    // Validate basic structure
    if (!g_config_doc.is<JsonObject>()) {
        DBG_ERROR("CFG", "Config root must be a JSON object");
        g_config_loaded = false;
        return false;
    }
    
    // Extract RGB config if present
    if (g_config_doc["rgb"].is<JsonObject>()) {
        JsonObject rgb = g_config_doc["rgb"].as<JsonObject>();
        if (rgb["brightness"].is<int>()) {
            g_rgb_config.brightness = rgb["brightness"];
        }
        if (rgb["mode"].is<const char*>()) {
            g_rgb_config.mode = rgb["mode"];
        }
        if (rgb["speed"].is<int>()) {
            g_rgb_config.speed = rgb["speed"];
        }
        if (rgb["color"].is<const char*>()) {
            // Color can be a string like "#FF00FF" or a number
            if (rgb["color"].is<const char*>()) {
                const char* color_str = rgb["color"];
                if (color_str[0] == '#') {
                    g_rgb_config.color = strtoul(color_str + 1, nullptr, 16);
                }
            } else {
                g_rgb_config.color = rgb["color"];
            }
        }
    }
    
    g_config_loaded = true;
    DBG_INFO("CFG", "Config parsed OK");
    return true;
}

bool config_is_loaded() {
    return g_config_loaded;
}

// ── Saving ──────────────────────────────────────────────────────
bool config_save(const char* json_data, size_t length) {
    if (length == 0 || length > CONFIG_MAX_SIZE) {
        DBG_ERROR("CFG", "Invalid config size: %u bytes", length);
        return false;
    }
    
    DBG_INFO("CFG", "Saving config (%u bytes)...", length);
    
    // Validate JSON before saving
    JsonDocument test_doc;
    DeserializationError error = deserializeJson(test_doc, json_data, length);
    if (error) {
        DBG_ERROR("CFG", "Invalid JSON: %s", error.c_str());
        return false;
    }
    
    // Write to file
    File file = LittleFS.open(CONFIG_FILE_PATH, "w");
    if (!file) {
        DBG_ERROR("CFG", "Failed to open config file for writing");
        return false;
    }
    
    size_t written = file.write((const uint8_t*)json_data, length);
    file.close();
    
    if (written != length) {
        DBG_ERROR("CFG", "Failed to write complete config");
        return false;
    }
    
    DBG_INFO("CFG", "Config saved successfully");
    
    // Reload the config
    return config_load();
}

// ── Key Mapping Access ──────────────────────────────────────────
const char* config_get_key_action(uint8_t layer, uint8_t col, uint8_t row) {
    if (!g_config_loaded) {
        return nullptr;
    }
    
    // Build key position string: "C{col}R{row}"
    char key_pos[16];  // Increased buffer size to avoid truncation warnings
    snprintf(key_pos, sizeof(key_pos), "C%dR%d", col, row);
    
    // Navigate to layers[layer].keys[key_pos]
    if (!g_config_doc["layers"].is<JsonArray>()) {
        return nullptr;
    }
    
    JsonArray layers = g_config_doc["layers"];
    if (layer >= layers.size()) {
        return nullptr;
    }
    
    JsonObject layer_obj = layers[layer];
    if (!layer_obj["keys"].is<JsonObject>()) {
        return nullptr;
    }
    
    JsonObject keys = layer_obj["keys"];
    
    // Check if key exists
    JsonVariant key_variant = keys[key_pos];
    if (key_variant.isNull() || !key_variant.is<JsonObject>()) {
        return nullptr;
    }
    
    JsonObject key_obj = key_variant.as<JsonObject>();
    
    // Return "value" field
    if (key_obj["value"].is<const char*>()) {
        return key_obj["value"];
    }
    
    return nullptr;
}

const char* config_get_key_label(uint8_t layer, uint8_t col, uint8_t row) {
    if (!g_config_loaded) {
        return nullptr;
    }
    
    // Build key position string: "C{col}R{row}"
    char key_pos[16];  // Increased buffer size to avoid truncation warnings
    snprintf(key_pos, sizeof(key_pos), "C%dR%d", col, row);
    
    // Navigate to layers[layer].keys[key_pos]
    if (!g_config_doc["layers"].is<JsonArray>()) {
        return nullptr;
    }
    
    JsonArray layers = g_config_doc["layers"];
    if (layer >= layers.size()) {
        return nullptr;
    }
    
    JsonObject layer_obj = layers[layer];
    if (!layer_obj["keys"].is<JsonObject>()) {
        return nullptr;
    }
    
    JsonObject keys = layer_obj["keys"];
    
    // Check if key exists
    JsonVariant key_variant = keys[key_pos];
    if (key_variant.isNull() || !key_variant.is<JsonObject>()) {
        return nullptr;
    }
    
    JsonObject key_obj = key_variant.as<JsonObject>();
    
    // Return "label" field
    if (key_obj["label"].is<const char*>()) {
        return key_obj["label"];
    }
    
    return nullptr;
}

// ── RGB Lighting Settings ───────────────────────────────────────
const RGBConfig& config_get_rgb() {
    return g_rgb_config;
}

// ── Power Settings ──────────────────────────────────────────────
uint32_t config_get_idle_timeout_sec() {
    if (!g_config_loaded || !g_config_doc.is<JsonObject>()) {
        return 0;  // Use firmware default
    }
    
    // Correct JSON key access
    if (g_config_doc["power"]["idle_timeout_sec"].is<int>()) {
        return g_config_doc["power"]["idle_timeout_sec"].as<int>();
    }
    
    return 0;
}

uint32_t config_get_oled_dim_timeout_sec() {
    if (!g_config_loaded || !g_config_doc.is<JsonObject>()) {
        return 0;  // Use firmware default
    }
    
    // Correct JSON key access
    if (g_config_doc["power"]["oled_dim_timeout_sec"].is<int>()) {
        return g_config_doc["power"]["oled_dim_timeout_sec"].as<int>();
    }
    
    return 0;
}

// ── Filesystem Info ─────────────────────────────────────────────
void config_get_fs_info(size_t* total_bytes, size_t* used_bytes) {
    if (total_bytes) {
        *total_bytes = LittleFS.totalBytes();
    }
    if (used_bytes) {
        *used_bytes = LittleFS.usedBytes();
    }
}
