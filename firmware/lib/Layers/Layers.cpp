/**
 * @file layers.cpp
 * @brief Layer state management implementation.
 */

#include "Layers.h"
#include "debug.h"

// ── Configuration ───────────────────────────────────────────────
LayerConfig g_layer_config;

// ── State ───────────────────────────────────────────────────────
static Layer g_current_layer = LAYER_0_FN_KEYS;
static LayerChangeCallback g_change_callback = nullptr;

// ── Layer Names ─────────────────────────────────────────────────
static const char* LAYER_NAMES[LAYER_COUNT] = {
    "FN Keys",      // Layer 0
    "User",         // Layer 1
    "Serial A",     // Layer 2
    "Serial B",     // Layer 3
};

// ── Initialization ──────────────────────────────────────────────
void layers_init() {
    DBG_INFO("LYR", "Initializing layer system...");
    
    g_current_layer = g_layer_config.default_layer;
    
    DBG_INFO("LYR", "Layer system initialized (default: %s)", 
             layers_get_name(g_current_layer));
}

// ── Layer Control ───────────────────────────────────────────────
Layer layers_get_current() {
    return g_current_layer;
}

bool layers_set(Layer layer) {
    // Validate layer
    if (layer >= LAYER_COUNT) {
        DBG_ERROR("LYR", "Invalid layer: %d", layer);
        return false;
    }
    
    // Check if already active
    if (layer == g_current_layer) {
        return false;  // No change
    }
    
    // Store old layer for callback
    Layer old_layer = g_current_layer;
    
    // Switch layer
    g_current_layer = layer;
    
    DBG_INFO("LYR", "Layer changed: %s → %s", 
             layers_get_name(old_layer), 
             layers_get_name(layer));
    
    // Notify callback
    if (g_change_callback) {
        g_change_callback(old_layer, layer);
    }
    
    return true;
}

Layer layers_cycle_next() {
    if (!g_layer_config.cycle_enabled) {
        return g_current_layer;
    }
    
    Layer next_layer = (Layer)((g_current_layer + 1) % LAYER_COUNT);
    layers_set(next_layer);
    return next_layer;
}

Layer layers_cycle_prev() {
    if (!g_layer_config.cycle_enabled) {
        return g_current_layer;
    }
    
    Layer prev_layer = (Layer)((g_current_layer + LAYER_COUNT - 1) % LAYER_COUNT);
    layers_set(prev_layer);
    return prev_layer;
}

// ── Name Lookup ─────────────────────────────────────────────────
const char* layers_get_name(Layer layer) {
    if (layer >= LAYER_COUNT) {
        return "Invalid";
    }
    return LAYER_NAMES[layer];
}

// ── Callbacks ───────────────────────────────────────────────────
void layers_set_change_callback(LayerChangeCallback callback) {
    g_change_callback = callback;
}
