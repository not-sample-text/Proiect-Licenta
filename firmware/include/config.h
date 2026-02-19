/**
 * @file config.h
 * @brief Configuration file management for the PROS3 Macropad.
 *
 * Handles loading and parsing the config.json file from LittleFS.
 * The config structure stores all user-defined settings:
 * - Layer 1-3 key mappings
 * - RGB lighting preferences
 * - Power management timeouts
 * - Layer names (optional)
 *
 * Responsibilities:
 * - Mount LittleFS filesystem
 * - Load config.json from storage
 * - Parse and validate JSON structure
 * - Provide parsed configuration to other modules
 * - Handle config upload from host (future)
 */

#pragma once

#include <Arduino.h>

// ── Configuration File ──────────────────────────────────────────
#define CONFIG_FILE_PATH "/config.json"
#define CONFIG_MAX_SIZE  8192  // 8KB max config file size

// ── Initialization ──────────────────────────────────────────────
/**
 * Initialize the configuration system.
 * - Mounts LittleFS filesystem
 * - Attempts to load config.json
 * - Falls back to defaults if config is missing/invalid
 * 
 * @return true if config was loaded successfully, false if using defaults
 */
bool config_init();

// ── Loading ─────────────────────────────────────────────────────
/**
 * Load and parse the config file from storage.
 * This is called automatically by config_init() but can be called
 * again to reload after a config upload.
 * 
 * @return true if config was loaded and parsed successfully
 */
bool config_load();

/**
 * Check if a valid config is currently loaded.
 * Returns false if using default/fallback configuration.
 */
bool config_is_loaded();

// ── Saving ──────────────────────────────────────────────────────
/**
 * Save a new config file to storage.
 * Used when receiving config from host via serial.
 * 
 * @param json_data The JSON string to save
 * @param length Length of the JSON string
 * @return true if saved successfully
 */
bool config_save(const char* json_data, size_t length);

// ── Key Mapping Access ──────────────────────────────────────────
/**
 * Get the action string for a specific key on a layer.
 * 
 * @param layer Layer number (0-3)
 * @param col Column (0-2)
 * @param row Row (0-3)
 * @return Action string from config, or NULL if not defined
 * 
 * Format examples:
 *   "F13"              - Single keycode
 *   "Ctrl+Shift+P"     - Modifier combo
 *   "Media:VolUp"      - Media key
 *   "open_url:http..." - Host listener action (layers 2-3)
 */
const char* config_get_key_action(uint8_t layer, uint8_t col, uint8_t row);

// ── RGB Lighting Settings ───────────────────────────────────────
struct RGBConfig {
    uint8_t  brightness;     // 0-255
    uint8_t  mode;           // 0=solid, 1=breathing, 2=rainbow, etc.
    uint8_t  speed;          // Animation speed (0-255)
    uint32_t color;          // RGB color (0xRRGGBB)
    
    RGBConfig()
        : brightness(128)
        , mode(0)
        , speed(128)
        , color(0xFF00FF)  // Default: magenta
    {}
};

/**
 * Get RGB lighting configuration from config file.
 */
const RGBConfig& config_get_rgb();

// ── Power Settings ──────────────────────────────────────────────
/**
 * Get idle timeout from config (in seconds).
 * Returns 0 to use firmware default.
 */
uint32_t config_get_idle_timeout_sec();

/**
 * Get OLED dim timeout from config (in seconds).
 * Returns 0 to use firmware default.
 */
uint32_t config_get_oled_dim_timeout_sec();

// ── Filesystem Info ─────────────────────────────────────────────
/**
 * Get filesystem usage info for diagnostics.
 * 
 * @param[out] total_bytes Total filesystem size
 * @param[out] used_bytes Used space
 */
void config_get_fs_info(size_t* total_bytes, size_t* used_bytes);
