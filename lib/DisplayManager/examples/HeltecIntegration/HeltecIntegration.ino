#include <Arduino.h>
#include <Wire.h>
#include <DisplayManager.h>
#include <DisplayLogger.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Configuration
#define I2C_SDA 45
#define I2C_SCL 46
#define OLED_SDA 17
#define OLED_SCL 18
#define OLED_RST 21
#define BUTTON_PIN 0  // PRG button on Heltec board
#define SCREEN_TIMEOUT 30000  // Turn off display after 30 seconds of inactivity

// Global instances
DisplayManager display;
DisplayLogger logger(display);
Adafruit_BME280 bme;

// Timers
uint32_t lastDisplayUpdate = 0;
uint32_t displayTimeout = 0;
uint32_t lastButtonCheck = 0;

// Button state
bool lastButtonState = HIGH;

// Function prototypes
void updateDisplay();
void checkButton();

void setup() {
  // Initialize Serial
  Serial.begin(115200);
  delay(300);
  
  Serial.println("\n\n=== Heltec ESP32 LoRa DisplayManager Example ===");
  
  // Initialize I2C for sensors
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(100);
  
  // Initialize display
  display.begin();
  display.setScreen(1); // Show startup screen
  display.updateStartupProgress(10, "Initializing...");
  
  // Log the startup
  logger.info("DisplayManager demo started");
  
  // Initialize BME280 sensor
  display.updateStartupProgress(30, "Starting BME280...");
  bool bmeStatus = bme.begin(0x76, &Wire);
  if (!bmeStatus) {
    Serial.println("Could not find BME280 sensor!");
    logger.error("BME280 not found!");
    display.updateStartupProgress(100, "BME280 not found!");
  } else {
    Serial.println("BME280 sensor initialized");
    logger.info("BME280 initialized");
    display.updateStartupProgress(100, "Ready!");
  }
  
  // Initialize button
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  delay(1000);
  
  // Set initial screen after startup
  display.setScreen(3); // Sensor data screen
  
  // Reset display timeout
  displayTimeout = millis() + SCREEN_TIMEOUT;
}

void loop() {
  // Check for button presses to navigate screens
  checkButton();
  
  // Update display periodically
  if (millis() - lastDisplayUpdate > 5000) {
    updateDisplay();
    lastDisplayUpdate = millis();
  }
  
  // Check if we should turn off the display to save power
  if (millis() > displayTimeout) {
    display.sleep();
  }
  
  // Prevent watchdog resets
  delay(10);
}

void updateDisplay() {
  // Wake up display if it was sleeping
  display.wakeup();
  
  // Reset display timeout
  displayTimeout = millis() + SCREEN_TIMEOUT;
  
  // Read sensor data if BME280 is available
  float temperature = 0, humidity = 0, pressure = 0;
  if (bme.begin()) {
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0F; // Convert Pa to hPa
    
    // Log sensor readings periodically
    logger.debug("Temp: " + String(temperature, 1) + "C, Humidity: " + 
                String(humidity, 1) + "%, Pressure: " + String(pressure, 1) + "hPa");
  }
  
  // Update the current screen based on its type
  uint8_t currentScreen = display.getCurrentScreen();
  
  if (currentScreen == 3) {
    // Update sensor data screen
    display.updateSensorData(
      temperature,
      humidity,
      pressure,
      3.7 // Default battery value
    );
  } else if (currentScreen == 2) {
    // Update LoRaWAN status screen with example values
    display.updateLoRaWANStatus(
      true,       // Joined status
      -105,       // RSSI
      12,         // Uplink count
      3           // Downlink count
    );
  }
  
  // Refresh the screen
  display.refresh();
}

void checkButton() {
  // Check button state every 100ms to avoid bouncing
  if (millis() - lastButtonCheck < 100) {
    return;
  }
  
  lastButtonCheck = millis();
  
  // Read button state
  bool buttonState = digitalRead(BUTTON_PIN);
  
  // Check for button press (LOW when pressed)
  if (buttonState == LOW && lastButtonState == HIGH) {
    // Button was pressed, cycle to next screen
    uint8_t currentScreen = display.getCurrentScreen();
    uint8_t nextScreen = (currentScreen + 1) % 5; // Cycle through 5 screens (0-4)
    
    // Skip screen 1 (startup) in normal operation
    if (nextScreen == 1) nextScreen = 2;
    
    // Set the new screen
    display.setScreen(nextScreen);
    
    // Log screen change
    logger.info("Changed to screen " + String(nextScreen));
    
    // Reset display timeout
    displayTimeout = millis() + SCREEN_TIMEOUT;
    
    // Update display immediately
    updateDisplay();
  }
  
  // Save button state for next check
  lastButtonState = buttonState;
} 