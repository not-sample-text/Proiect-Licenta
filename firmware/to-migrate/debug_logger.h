#pragma once

#include <cstdarg>
#include <cstring>
#include "config.h"

/**
 * DebugLogger: Handles formatted debug output to both Serial and OLED display
 */
class DebugLogger {
 public:
  DebugLogger() {
    clearLogs();
  }

  void begin() {
    // Logger initializes without hardware
  }

  void run() {
    // Logger doesn't need periodic updates
  }

  /**
   * Log a formatted message to serial and in-memory buffer
   */
  void log(const char* format, ...) {
    char buffer[kMaxLogLength];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Send to serial
    Serial.println(buffer);

    // Shift existing lines up
    for (uint8_t i = 0; i < kMaxLogLines - 1; ++i) {
      strncpy(messages_[i], messages_[i + 1], kMaxLogLength);
    }
    
    // Add new line at bottom
    strncpy(messages_[kMaxLogLines - 1], buffer, kMaxLogLength);
    messages_[kMaxLogLines - 1][kMaxLogLength - 1] = '\0'; 
  }

  /**
   * Get log message at given index (0 = oldest, kMaxLogLines-1 = newest)
   */
  const char* getMessage(uint8_t index) const {
    if (index >= kMaxLogLines) return "";
    return messages_[index];
  }

  void clearLogs() {
    for (uint8_t i = 0; i < kMaxLogLines; ++i) {
      messages_[i][0] = '\0';
    }
  }

 private:
  char messages_[kMaxLogLines][kMaxLogLength] = {0};
};
