/**
 * @file layers.h
 * @brief Layer state management for the PROS3 Macropad.
 *
 * Manages the 4-layer state machine:
 * - Layer 0: Function keys (F13-F24) - hardcoded
 * - Layer 1: User-defined actions (from config)
 * - Layer 2: Host listener protocol (keys send serial data)
 * - Layer 3: Host listener protocol (keys send serial data)
 *
 * The encoder button cycles through layers.
 *
 * Responsibilities:
 * - Track current active layer (0-3)
 * - Cycle between layers
 * - Notify observers of layer changes (for OLED, LEDs, etc.)
 * - Provide layer query API
 */

#pragma once

#include <Arduino.h>

// ── Layer Definitions ───────────────────────────────────────────
#define LAYER_COUNT 4

enum Layer : uint8_t {
    LAYER_0_FN_KEYS = 0,      // F13-F24 hardcoded
    LAYER_1_USER    = 1,      // User-defined from config
    LAYER_2_SERIAL  = 2,      // Serial protocol to host
    LAYER_3_SERIAL  = 3,      // Serial protocol to host
};

// ── Layer Change Callback ───────────────────────────────────────
/**
 * Callback function type for layer change notifications.
 * Called whenever the active layer changes.
 * 
 * @param old_layer Previous layer
 * @param new_layer New active layer
 */
typedef void (*LayerChangeCallback)(Layer old_layer, Layer new_layer);

// ── Configuration ───────────────────────────────────────────────
struct LayerConfig {
    bool cycle_enabled;         // Allow layer cycling via encoder button
    Layer default_layer;        // Layer to activate at boot (default: LAYER_0)
    
    LayerConfig()
        : cycle_enabled(true)
        , default_layer(LAYER_0_FN_KEYS)
    {}
};

extern LayerConfig g_layer_config;

// ── Initialization ──────────────────────────────────────────────
/**
 * Initialize the layer system.
 * Sets the current layer to the configured default.
 */
void layers_init();

// ── Layer Control ───────────────────────────────────────────────
/**
 * Get the currently active layer.
 */
Layer layers_get_current();

/**
 * Set the active layer directly.
 * Triggers layer change callbacks if the layer actually changes.
 * 
 * @param layer The layer to switch to (0-3)
 * @return true if layer was changed, false if invalid or already active
 */
bool layers_set(Layer layer);

/**
 * Cycle to the next layer.
 * Wraps around from Layer 3 back to Layer 0.
 * 
 * @return The new active layer
 */
Layer layers_cycle_next();

/**
 * Cycle to the previous layer.
 * Wraps around from Layer 0 back to Layer 3.
 * 
 * @return The new active layer
 */
Layer layers_cycle_prev();

// ── Name Lookup ─────────────────────────────────────────────────
/**
 * Get a human-readable name for a layer.
 * Useful for OLED display.
 * 
 * @param layer The layer
 * @return Static string with layer name (never NULL)
 */
const char* layers_get_name(Layer layer);

// ── Callbacks ───────────────────────────────────────────────────
/**
 * Register a callback to be notified of layer changes.
 * Only one callback is supported (keeps it simple).
 * 
 * @param callback Function to call on layer change, or NULL to unregister
 */
void layers_set_change_callback(LayerChangeCallback callback);
