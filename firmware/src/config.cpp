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
    DBG_INFO("Initializing configuration system...");
    
    // Mount LittleFS
    if (!LittleFS.begin(true)) {  // format on fail = true
        DBG_ERROR("Failed to mount LittleFS");
        return false;
    }
    
    // Get filesystem info
    size_t total, used;
    config_get_fs_info(&total, &used);
    DBG_INFO("  LittleFS mounted: %u KB total, %u KB used", 
             total / 1024, used / 1024);
    
    // Attempt to load config
    if (config_load()) {
        DBG_INFO("Configuration loaded successfully");
        return true;
    } else {
        DBG_WARN("Using default configuration");
        return false;
    }
}

// ── Loading ─────────────────────────────────────────────────────
bool config_load() {
    // Check if config file exists
    if (!LittleFS.exists(CONFIG_FILE_PATH)) {
        DBG_WARN("Config file not found: %s", CONFIG_FILE_PATH);
        g_config_loaded = false;
        return false;
    }
    
    // Open file
    File file = LittleFS.open(CONFIG_FILE_PATH, "r");
    if (!file) {
        DBG_ERROR("Failed to open config file");
        g_config_loaded = false;
        return false;
    }
    
    // Check file size
    size_t file_size = file.size();
    if (file_size == 0 || file_size > CONFIG_MAX_SIZE) {
        DBG_ERROR("Invalid config file size: %u bytes", file_size);
        file.close();
        g_config_loaded = false;
        return false;
    }
    
    DBG_VERBOSE("Loading config file (%u bytes)...", file_size);
    
    // Parse JSON
    DeserializationError error = deserializeJson(g_config_doc, file);
    file.close();
    
    if (error) {
        DBG_ERROR("Failed to parse config JSON: %s", error.c_str());
        g_config_loaded = false;
        return false;
    }
    
    // Validate basic structure
    if (!g_config_doc.is<JsonObject>()) {
        DBG_ERROR("Config root must be a JSON object");
        g_config_loaded = false;
        return false;
    }
    
    // Extract RGB config if present
    if (g_config_doc.containsKey("rgb")) {
        JsonObject rgb = g_config_doc["rgb"];
        if (rgb.containsKey("brightness")) {
            g_rgb_config.brightness = rgb["brightness"];
        }
        if (rgb.containsKey("mode")) {
            g_rgb_config.mode = rgb["mode"];
        }
        if (rgb.containsKey("speed")) {
            g_rgb_config.speed = rgb["speed"];
        }
        if (rgb.containsKey("color")) {
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
    DBG_INFO("Config parsed OK");
    return true;
}

bool config_is_loaded() {
    return g_config_loaded;
}

// ── Saving ──────────────────────────────────────────────────────
bool config_save(const char* json_data, size_t length) {
    if (length == 0 || length > CONFIG_MAX_SIZE) {
        DBG_ERROR("Invalid config size: %u bytes", length);
        return false;
    }
    
    DBG_INFO("Saving config (%u bytes)...", length);
    
    // Validate JSON before saving
    JsonDocument test_doc;
    DeserializationError error = deserializeJson(test_doc, json_data, length);
    if (error) {
        DBG_ERROR("Invalid JSON: %s", error.c_str());
        return false;
    }
    
    // Write to file
    File file = LittleFS.open(CONFIG_FILE_PATH, "w");
    if (!file) {
        DBG_ERROR("Failed to open config file for writing");
        return false;
    }
    
    size_t written = file.write((const uint8_t*)json_data, length);
    file.close();
    
    if (written != length) {
        DBG_ERROR("Failed to write complete config");
        return false;
    }
    
    DBG_INFO("Config saved successfully");
    
    // Reload the config
    return config_load();
}

// ── Key Mapping Access ──────────────────────────────────────────
const char* config_get_key_action(uint8_t layer, uint8_t col, uint8_t row) {
    if (!g_config_loaded) {
        return nullptr;
    }
    
    // Build key path: layers[L].keys[R][C]
    // (row-major order in config)
    char path[32];
    snprintf(path, sizeof(path), "layers[%d].keys[%d][%d]", layer, row, col);
    
    // Navigate JSON path
    if (!g_config_doc.containsKey("layers")) {
        return nullptr;
    }
    
    JsonArray layers = g_config_doc["layers"];
    if (layer >= layers.size()) {
        return nullptr;
    }
    
    JsonObject layer_obj = layers[layer];
    if (!layer_obj.containsKey("keys")) {
        return nullptr;
    }
    
    JsonArray keys = layer_obj["keys"];
    if (row >= keys.size()) {
        return nullptr;
    }
    
    JsonArray key_row = keys[row];
    if (col >= key_row.size()) {
        return nullptr;
    }
    
    // Return action string (or null if not a string)
    if (key_row[col].is<const char*>()) {
        return key_row[col];
    }
    
    return nullptr;
}

// ── RGB Lighting Settings ───────────────────────────────────────
const RGBConfig& config_get_rgb() {
    return g_rgb_config;
}

// ── Power Settings ──────────────────────────────────────────────
uint32_t config_get_idle_timeout_sec() {
    if (!g_config_loaded || !g_config_doc.containsKey("power")) {
        return 0;  // Use firmware default
    }
    
    JsonObject power = g_config_doc["power"];
    if (power.containsKey("idle_timeout_sec")) {
        return power["idle_timeout_sec"];
    }
    
    return 0;
}

uint32_t config_get_oled_dim_timeout_sec() {
    if (!g_config_loaded || !g_config_doc.containsKey("power")) {
        return 0;  // Use firmware default
    }
    
    JsonObject power = g_config_doc["power"];
    if (power.containsKey("oled_dim_timeout_sec")) {
        return power["oled_dim_timeout_sec"];
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
