#include <Arduino.h>
#include "Config.h"
#include "DisplayManager.h"
#include "SensorManager.h"
#include "LoRaManager.h"

// Include secrets for LoRaWAN credentials
#include "secrets.h"

// Global instances
DisplayManager display;
SensorManager sensors;
LoRaManager lora;

// RTC variables (preserved during deep sleep)
RTC_DATA_ATTR uint32_t bootCount = 0;
RTC_DATA_ATTR int16_t lastRssi = 0;
RTC_DATA_ATTR bool hadSuccessfulTransmission = false;
RTC_DATA_ATTR int consecutiveErrors = 0;
RTC_DATA_ATTR uint32_t errorBackoffTime = MINIMUM_DELAY;
RTC_DATA_ATTR bool pirWake = false;
RTC_DATA_ATTR int lastJoinError = 0;

// Timers
uint32_t lastDisplayUpdate = 0;
uint32_t displayTimeout = 0;
uint32_t lastDataSendTime = 0;
uint32_t lastButtonCheck = 0;

// Button state
bool lastButtonState = HIGH;

// Function prototypes
void goToSleep(uint32_t sleepTime);
void updateDisplay();
void sendSensorData(bool motionDetected = false);
void processDownlink();
String getBmeStatusString();
void checkButton();

void setup() {
  // Initialize Serial
  Serial.begin(115200);
  delay(300);  // Longer delay to ensure serial is ready
  
  Serial.println("\n\n=== Heltec ESP32 LoRa Sensor Node ===");
  Serial.println("Boot count: " + String(++bootCount));
  
  // Check wake-up reason
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("Wakeup caused by external signal using RTC_IO (PIR sensor)");
    pirWake = true;
  } else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("Wakeup caused by timer");
    pirWake = false;
  } else {
    Serial.println("Wakeup was not caused by deep sleep");
    pirWake = false;
  }
  
  // Initialize I2C once for all devices that use it
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(100);  // Short delay after I2C init
  
  // Initialize display with startup screen
  display.begin();
  display.setNormalMode(); // Ensure display is in normal mode
  display.drawStartupScreen();
  display.updateStartupProgress(10, "Initializing...");
  delay(300);
  
  // Initialize button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize PIR motion sensor
  #ifdef PIR_PIN
  pinMode(PIR_PIN, INPUT_PULLUP);
  Serial.println("PIR motion sensor initialized on pin " + String(PIR_PIN));
  display.log("Motion sensor ready");
  #endif
  
  // Initialize sensors
  display.updateStartupProgress(30, "Initializing sensors...");
  if (!sensors.begin()) {
    Serial.println("BME280 sensor not found!");
    display.log("BME280 sensor not found!");
  } else {
    Serial.println("BME280 sensor initialized");
    display.log("BME280 sensor found");
  }
  delay(300);
  
  // Initialize LoRa radio
  display.updateStartupProgress(50, "Initializing LoRa...");
  if (!lora.begin(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY)) {
    Serial.println("Failed to initialize LoRa!");
    display.showErrorScreen("Hardware Error", "LoRa initialization failed");
    delay(5000);
  } else {
    Serial.println("LoRa initialized and ready");
    display.log("LoRa initialized");
  }
  delay(300);
  
  // Convert hex strings to bytes (if using secrets.h approach)
  uint8_t appKey[16], nwkKey[16];
  uint64_t joinEUI = strtoull(APPEUI, NULL, 16);
  uint64_t devEUI = strtoull(DEVEUI, NULL, 16);
  
  // Convert the App Key string to bytes
  for (int i = 0; i < 16; i++) {
    sscanf(APPKEY + i*2, "%2hhx", &appKey[i]);
  }
  
  // Convert the Network Key string to bytes
  for (int i = 0; i < 16; i++) {
    sscanf(NWKKEY + i*2, "%2hhx", &nwkKey[i]);
  }
  
  // Set credentials
  display.updateStartupProgress(70, "Setting credentials...");
  lora.setCredentials(joinEUI, devEUI, appKey, nwkKey);
  display.log("LoRaWAN credentials set");
  delay(300);
  
  // Join network
  display.updateStartupProgress(90, "Joining network...");
  bool joined = lora.joinNetwork();
  if (joined) {
    display.updateStartupProgress(100, "Network joined!");
    display.log("LoRaWAN network joined");
    
    // Check if this was a new session
    if (lora.getLastErrorCode() == -1118) {
      display.log("New LoRaWAN session");
      Serial.println("New LoRaWAN session created successfully");
    }
    
    hadSuccessfulTransmission = true;
    consecutiveErrors = 0;
    
    // Send sensor data immediately after joining
    Serial.println("Sending sensor data immediately after joining the network");
    display.log("Sending initial data");
    
    // Small delay to ensure network is ready
    delay(1000);
    
    // Send sensor data
    sendSensorData();
    
    // Update the last send time to maintain the regular schedule
    lastDataSendTime = millis();
    
    // Process any potential downlink messages
    processDownlink();
    
  } else {
    display.updateStartupProgress(100, "Join failed!");
    // Get and save the last error code
    lastJoinError = RADIOLIB_ERR_NONE; // Default
    
    // Get the error code from the SX1262 if possible
    if (lora.getLastErrorCode() != 0) {
      lastJoinError = lora.getLastErrorCode();
      // Show error on display
      display.showLoRaError(lastJoinError);
    } else {
      display.showErrorScreen("Network Error", "Failed to join LoRaWAN network");
    }
    
    display.log("Failed to join network");
    hadSuccessfulTransmission = false;
    consecutiveErrors++;
  }
  
  delay(500);
  
  // Reset display timeout
  displayTimeout = millis() + DISPLAY_TIMEOUT;
  
  // Update display with initial data
  updateDisplay();
  
  // If we woke up due to motion detection, send data immediately
  if (pirWake && lora.isNetworkJoined()) {
    Serial.println("Motion detected during sleep - sending data immediately");
    display.log("Motion detected");
    
    // Small delay to ensure system is ready
    delay(1000);
    
    sendSensorData(true);
    lastDataSendTime = millis();
    
    // Process any downlink messages
    processDownlink();
  }
}

void loop() {
  // Handle LoRa events
  lora.handleEvents();
  
  // Check button presses
  checkButton();
  
  // Check for motion detection
  #ifdef PIR_PIN
  static bool lastMotionState = LOW;
  bool currentMotionState = digitalRead(PIR_PIN);
  
  // If motion is detected (rising edge)
  if (currentMotionState == PIR_WAKE_LEVEL && lastMotionState != PIR_WAKE_LEVEL) {
    Serial.println("Motion detected!");
    display.wakeup(); // Wake up display if it was sleeping
    display.log("Motion detected");
    displayTimeout = millis() + DISPLAY_TIMEOUT; // Reset display timeout
    
    // If we're joined to the network, send data immediately
    if (lora.isNetworkJoined()) {
      // Only send if it's been at least 10 seconds since last transmission
      // This prevents too frequent transmissions when motion is continuous
      if (millis() - lastDataSendTime > 10000) {
        Serial.println("Sending sensor data due to motion detection");
        display.log("Sending motion alert");
        
        // Add a small delay to make sure everything is ready
        delay(500);
        
        sendSensorData(true);
        lastDataSendTime = millis();
        
        // Process any downlink data
        processDownlink();
      } else {
        Serial.println("Motion detected but skipping transmission (too soon after last send)");
      }
    }
  }
  lastMotionState = currentMotionState;
  #endif
  
  // Update display periodically
  if (millis() - lastDisplayUpdate > 5000) {
    updateDisplay();
    lastDisplayUpdate = millis();
  }
  
  // Check if we should turn off the display to save power
  if (millis() > displayTimeout) {
    display.sleep();
  }
  
  // If we're joined to the network and it's time to send data
  if (lora.isNetworkJoined() && (millis() - lastDataSendTime > (MINIMUM_DELAY * 1000))) {
    Serial.println("Network joined, preparing to send sensor data");
    display.log("Preparing to send data");
    
    // Add a small delay to make sure everything is ready
    delay(1000);
    
    sendSensorData(false); // Regular scheduled transmission, not motion triggered
    lastDataSendTime = millis();
    
    // Process any downlink data
    processDownlink();
    
    // Add delay after sending to ensure transmission completes
    Serial.println("Waiting 5 seconds before sleep to ensure transmission completes");
    delay(5000);
    
    // Sleep after sending data
    goToSleep(MINIMUM_DELAY);
  } else if (lora.isNetworkJoined()) {
    // Debug: print time until next transmission
    unsigned long timeToNext = (MINIMUM_DELAY * 1000) - (millis() - lastDataSendTime);
    if (millis() % 10000 < 10) { // Print only occasionally to avoid flooding
      Serial.println("Network joined. Next transmission in " + String(timeToNext/1000) + " seconds");
    }
  }
  
  // Prevent watchdog resets
  delay(10);
}

void updateDisplay() {
  // Wake up display if it was sleeping
  display.wakeup();
  
  // Ensure display is in normal mode (not inverted)
  display.setNormalMode();
  
  // Reset display timeout
  displayTimeout = millis() + DISPLAY_TIMEOUT;
  
  // Read sensor data
  float temperature = 0, humidity = 0, pressure = 0, altitude = 0;
  float batteryVoltage = 3.7; // Default value, replace with actual battery reading if available
  
  if (sensors.isBME280Available()) {
    sensors.readBME280(temperature, humidity, pressure, altitude);
  }
  
  // If we're on screen 1 (startup), move to sensor or status screen
  if (display.getCurrentScreen() == 1) {
    if (!lora.isNetworkJoined() && lastJoinError != 0) {
      // Show error screen if join failed
      display.showLoRaError(lastJoinError);
    } else {
      // Show the appropriate screen based on network status
      if (lora.isNetworkJoined()) {
        display.setScreen(3); // Sensor data screen
      } else {
        display.setScreen(2); // LoRaWAN status screen
      }
    }
  }
  
  // Update sensor data screen
  display.updateSensorData(temperature, humidity, pressure, batteryVoltage);
  
  // Update LoRaWAN status screen
  display.updateLoRaWANStatus(
    lora.isNetworkJoined(),
    lastRssi,
    0, // We don't have uplink counter in the new API
    0  // We don't have downlink counter in the new API
  );
  
  // Refresh the current screen
  display.refresh();
}

void sendSensorData(bool motionDetected) {
  Serial.println("Starting sendSensorData function");
  
  if (!lora.isNetworkJoined()) {
    Serial.println("Cannot send data - not joined to network");
    return;
  }
  
  // Read sensor data
  float temperature = 0, humidity = 0, pressure = 0, altitude = 0;
  
  if (sensors.isBME280Available()) {
    Serial.println("Reading BME280 sensor data");
    sensors.readBME280(temperature, humidity, pressure, altitude);
    Serial.println("Sensor readings - Temp: " + String(temperature) + "°C, Humidity: " + String(humidity) + 
                  "%, Pressure: " + String(pressure) + " hPa");
  } else {
    Serial.println("BME280 not available, using dummy values");
  }
  
  // Show sensor data screen before sending
  display.drawSensorDataScreen();
  display.updateSensorData(
    temperature, 
    humidity, 
    pressure, 
    3.7 // Default battery value, replace with actual reading if available
  );
  
  // Prepare payload (simple binary format)
  uint8_t payload[8];
  
  // Convert float to int16_t (2 bytes) with 1 decimal place precision
  int16_t temp_int = (int16_t)(temperature * 10);
  int16_t hum_int = (int16_t)(humidity * 10);
  int16_t press_int = (int16_t)(pressure / 10); // Reduce to fit in int16_t
  
  // Pack the data into the payload
  payload[0] = temp_int >> 8;
  payload[1] = temp_int & 0xFF;
  payload[2] = hum_int >> 8;
  payload[3] = hum_int & 0xFF;
  payload[4] = press_int >> 8;
  payload[5] = press_int & 0xFF;
  payload[6] = 0; // Use bit 0 of byte 6 as motion flag
  payload[7] = 0; // Reserved for future use
  
  // Set motion flag if data is being sent due to motion detection
  if (motionDetected) {
    payload[6] |= 0x01; // Set bit 0 of byte 6
    Serial.println("Motion flag set in payload");
  }
  
  // Display sending status
  if (motionDetected) {
    display.log("Sending motion alert...");
  } else {
    display.log("Sending data...");
  }
  display.refresh();
  
  // Send the data
  Serial.println("Sending sensor data packet of " + String(sizeof(payload)) + " bytes");
  Serial.print("Payload hex: ");
  for (int i = 0; i < sizeof(payload); i++) {
    if (payload[i] < 16) Serial.print("0");
    Serial.print(payload[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  bool success = lora.sendData(payload, sizeof(payload), 1, false);
  
  if (success) {
    Serial.println("Data sent successfully!");
    display.log("Data sent successfully");
    
    // Update RSSI
    lastRssi = lora.getLastRssi();
    
    // Reset error counters
    hadSuccessfulTransmission = true;
    consecutiveErrors = 0;
    errorBackoffTime = MINIMUM_DELAY;
  } else {
    Serial.println("Failed to send data! Error code: " + String(lora.getLastErrorCode()));
    display.log("Failed to send data");
    
    // Get the error code
    int errorCode = lora.getLastErrorCode();
    
    // Show error on display
    if (errorCode != RADIOLIB_ERR_NONE) {
      display.showLoRaError(errorCode);
    } else {
      display.showErrorScreen("Transmission Error", "Failed to send data");
    }
    
    // Increment error counter and adjust backoff
    consecutiveErrors++;
    
    // Exponential backoff for consecutive errors
    if (consecutiveErrors > 1) {
      errorBackoffTime = errorBackoffTime * 2;
      if (errorBackoffTime > MAX_BACKOFF_DELAY) {
        errorBackoffTime = MAX_BACKOFF_DELAY;
      }
    }
    
    // If we've never had a successful transmission, we might need to rejoin
    if (!hadSuccessfulTransmission && consecutiveErrors > 3) {
      display.log("Attempting to rejoin network...");
      if (lora.joinNetwork()) {
        display.log("Network rejoined!");
        consecutiveErrors = 0;
      } else {
        display.log("Rejoin failed!");
        // Show the error
        int joinError = lora.getLastErrorCode();
        if (joinError != RADIOLIB_ERR_NONE) {
          display.showLoRaError(joinError);
        }
      }
    }
  }
  
  // Update display with latest status
  updateDisplay();
}

void processDownlink() {
  // This function would process any downlink data received
  // In the new implementation, this is handled in the sendData method
}

void goToSleep(uint32_t sleepTime) {
  Serial.println("Going to sleep for " + String(sleepTime) + " seconds");
  display.log("Sleep: " + String(sleepTime) + "s");
  display.refresh();
  delay(100);
  
  // Turn off display to save power
  display.sleep();
  
  // Configure wake sources
  esp_sleep_enable_timer_wakeup(sleepTime * 1000000ULL);
  
  // Enable PIR wake if configured
  #ifdef PIR_PIN
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIR_PIN, PIR_WAKE_LEVEL);
  #endif
  
  // Go to deep sleep
  esp_deep_sleep_start();
}

String getBmeStatusString() {
  if (sensors.isBME280Available()) {
    return "BME280: OK";
  } else {
    return "BME280: Not Found";
  }
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
    
    // Reset display timeout
    displayTimeout = millis() + DISPLAY_TIMEOUT;
    
    // Update display immediately
    updateDisplay();
  }
  
  // Save button state for next check
  lastButtonState = buttonState;
} 