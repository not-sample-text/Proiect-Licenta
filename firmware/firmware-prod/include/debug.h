/**
 * @file debug.h
 * @brief Debug logging with timestamp and tag.
 * 
 * Format: #[timestamp][tag] message
 * Example: #[12345][BLE] Connected to host
 * 
 * Lines start with '#' so the host listener can filter them.
 */

#pragma once

#include <Arduino.h>

// ── Debug Level Configuration ───────────────────────────────────
// 0 = Off
// 1 = Error only
// 2 = Error + Warning
// 3 = Error + Warning + Info
// 4 = All (including Verbose)

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 3  // Default from platformio.ini
#endif

// ── Runtime Debug Toggle ────────────────────────────────────────
extern bool g_debug_enabled;

// ── Base Debug Macro ────────────────────────────────────────────
#if DEBUG_LEVEL > 0

    #define DBG(level, tag, fmt, ...) \
        do { \
            if (DEBUG_LEVEL >= level && g_debug_enabled) { \
                Serial.printf("#[%lu][%s] " fmt "\n", \
                             millis(), tag, ##__VA_ARGS__); \
            } \
        } while(0)

#else
    #define DBG(level, tag, fmt, ...) ((void)0)
#endif

// ── Convenience Macros ──────────────────────────────────────────
#define DBG_ERROR(tag, fmt, ...)   DBG(1, tag, fmt, ##__VA_ARGS__)
#define DBG_WARN(tag, fmt, ...)    DBG(2, tag, fmt, ##__VA_ARGS__)
#define DBG_INFO(tag, fmt, ...)    DBG(3, tag, fmt, ##__VA_ARGS__)
#define DBG_VERBOSE(tag, fmt, ...) DBG(4, tag, fmt, ##__VA_ARGS__)
