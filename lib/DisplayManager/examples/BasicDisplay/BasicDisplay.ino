#include <Arduino.h>
#include <DisplayManager.h>
#include <DisplayLogger.h>

// Create display manager instance
DisplayManager display;
// Create logger that uses the display
DisplayLogger logger(display);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Initialize display with default pins
  display.begin();
  
  // Start with the startup screen
  display.setScreen(1);
  
  // Log information
  logger.info("Display initialized");
  
  // Show initialization progress
  for (int i = 0; i <= 100; i += 20) {
    String statusText = "Progress: " + String(i) + "%";
    display.updateStartupProgress(i, statusText);
    logger.debug(statusText);
    delay(500);
  }
}

void loop() {
  // Cycle through different screens
  
  // Sensor data screen
  display.setScreen(3);
  logger.info("Showing sensor data");
  
  // Update with some example sensor data
  float temp = 23.5 + random(-10, 10) / 10.0;
  float humidity = 65.0 + random(-50, 50) / 10.0;
  float pressure = 1013.2 + random(-20, 20) / 10.0;
  float batteryVoltage = 3.7 + random(-2, 2) / 10.0;
  
  display.updateSensorData(temp, humidity, pressure, batteryVoltage);
  delay(3000);
  
  // LoRaWAN status screen
  display.setScreen(2);
  logger.info("Showing LoRaWAN status");
  
  // Update with some example LoRaWAN data
  bool joined = true;
  int16_t rssi = -100 + random(-15, 15);
  uint32_t uplinks = millis() / 30000;  // Approximate uplink count based on time
  uint32_t downlinks = uplinks / 5;     // Fewer downlinks than uplinks
  
  display.updateLoRaWANStatus(joined, rssi, uplinks, downlinks);
  delay(3000);
  
  // Log screen to see the messages we've been logging
  display.setScreen(4);
  logger.info("Showing log screen");
  delay(3000);
  
  // Error screen example
  logger.warning("Showing error screen example");
  display.showErrorScreen("Demo Error", "This is an example error message to show formatting");
  delay(3000);
  
  // Show LoRa error example
  logger.warning("Showing LoRa error example");
  display.showLoRaError(-1106);  // Join Accept not received
  delay(3000);
} 