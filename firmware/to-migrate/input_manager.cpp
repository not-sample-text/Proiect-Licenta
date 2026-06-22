#include "handlers/input_manager.h"

// Global interrupt pending flag - can be safely accessed from IRAM
volatile bool g_encoder_interrupt_pending = false;

// Standalone ISR function (runs in IRAM, no class relocation issues)
void IRAM_ATTR inputManagerEncoderISR() {
  g_encoder_interrupt_pending = true;
}
