#include <Adafruit_NeoPixel.h>

// NeoPixel configuration
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// Pin for battery voltage monitoring
// The Adafruit Feather boards use pin A13 for this measurement
#define VBATPIN A13

// Define voltage thresholds for your LiPo battery
// These values are typical for a 3.7V LiPo
const float VOLTAGE_FULL = 4.20; // Corresponds to 100%
const float VOLTAGE_EMPTY = 3.30; // Corresponds to 0%

// Define the color hues for the gradient
const unsigned long FLASH_DELAY_MS = 500;
unsigned long lastFlashTime = 0;
bool pixelOn = false;

void setup() {
  Serial.begin(115200);

  // Initialize NeoPixel
  pixels.begin();
  pixels.setBrightness(255); // Full brightness
  pixels.show(); // Clear the pixel at startup

  // Set up the battery pin as an input
  pinMode(VBATPIN, INPUT);

  Serial.println("NeoPixel Battery Indicator with Adafruit Method");
}

void loop() {
  // Read the battery voltage using the Adafruit recommended method
  float measuredvbat = analogReadMilliVolts(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat /= 1000; // convert to volts!

  // Calculate the battery percentage
  float batteryPercentage = mapFloat(measuredvbat, VOLTAGE_EMPTY, VOLTAGE_FULL, 0, 100);
  
  // Print current battery status
  Serial.print("VBat: "); 
  Serial.print(measuredvbat, 2);
  Serial.print("V, Percentage: "); 
  Serial.print(batteryPercentage, 0);
  Serial.println("%");

  // Determine the color based on the percentage
  if (batteryPercentage > 80) {
    // Solid green for > 80%
    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // Green
    pixels.show();

  } else if (batteryPercentage >= 20) {
    // Gradient from green (80%) to red (20%)
    uint32_t color = getGradientColor(batteryPercentage);
    pixels.setPixelColor(0, color);
    pixels.show();

  } else { // Less than 20%
    // Flashing red
    unsigned long currentTime = millis();
    if (currentTime - lastFlashTime >= FLASH_DELAY_MS) {
      lastFlashTime = currentTime;
      pixelOn = !pixelOn; // Toggle the state

      if (pixelOn) {
        pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Red
      } else {
        pixels.setPixelColor(0, 0); // Turn off
      }
      pixels.show();
    }
  }
  
  delay(1000); // Update every second
}

// Function to map a float value from one range to another
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Function to generate a color in the green-to-red gradient
uint32_t getGradientColor(float percentage) {
  percentage = constrain(percentage, 20.0, 80.0);

  // Normalize the percentage to a 0-1.0 scale
  float normalized = mapFloat(percentage, 20.0, 80.0, 0.0, 1.0);

  uint8_t r, g;

  if (normalized > 0.5) {
    // Transition from yellow-green to green
    r = (1.0 - normalized) * 255 * 2;
    g = 255;
  } else {
    // Transition from red to yellow-green
    r = normalized * 255 * 2;
    g = 255;
  }
  
  // To create a more visually appealing hue transition, swap the logic.
  // At 80% (normalized=1.0) we want green. At 20% (normalized=0.0) we want red.
  r = 255 * (1.0 - normalized);
  g = 255 * normalized;

  // The actual color is determined by the combination of red and green
  return pixels.Color(r, g, 0); 
}
