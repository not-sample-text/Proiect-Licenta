/**
 * @file debug.h
 * @brief Leveled debug logging for the PROS3 Macropad.
 *
 * Compile-time levels (set -DDEBUG_LEVEL=n in platformio.ini):
 *   0 = OFF   — all macros compile to nothing
 *   1 = ERROR
 *   2 = WARN
 *   3 = INFO   (default for development)
 *   4 = VERBOSE
 *
 * Runtime control:
 *   Send "DBG:ON\n" or "DBG:OFF\n" over serial to enable/disable
 *   verbose logging without reflashing.
 *
 * Every line is prefixed with '#' so the host listener can
 * trivially separate debug output from protocol data.
 *
 * Format:  #[LEVEL] [millis] message\n
 * Example: #[I] [  4321] Booted OK
 */

#pragma once

#include <Arduino.h>

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 3  // default to INFO if not set via build flags
#endif

// ── Runtime debug toggle ────────────────────────────────────────
extern bool g_debug_enabled;  // Defined in main.cpp

// ── Internal helper — do not call directly ──────────────────────
#define _DBG_PRINT(tag, fmt, ...)                                    \
    do {                                                             \
        if (g_debug_enabled) {                                       \
            Serial.printf("#[" tag "] [%7u] " fmt "\n",              \
                          (unsigned int)millis(), ##__VA_ARGS__);    \
        }                                                            \
    } while (0)

// ── Public macros ───────────────────────────────────────────────
#if DEBUG_LEVEL >= 1
#define DBG_ERROR(fmt, ...) _DBG_PRINT("E", fmt, ##__VA_ARGS__)
#else
#define DBG_ERROR(fmt, ...) ((void)0)
#endif

#if DEBUG_LEVEL >= 2
#define DBG_WARN(fmt, ...)  _DBG_PRINT("W", fmt, ##__VA_ARGS__)
#else
#define DBG_WARN(fmt, ...)  ((void)0)
#endif

#if DEBUG_LEVEL >= 3
#define DBG_INFO(fmt, ...)  _DBG_PRINT("I", fmt, ##__VA_ARGS__)
#else
#define DBG_INFO(fmt, ...)  ((void)0)
#endif

#if DEBUG_LEVEL >= 4
#define DBG_VERBOSE(fmt, ...) _DBG_PRINT("V", fmt, ##__VA_ARGS__)
#else
#define DBG_VERBOSE(fmt, ...) ((void)0)
#endif
