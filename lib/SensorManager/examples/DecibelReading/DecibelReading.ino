#include <Arduino.h>
#include <SensorManager.h>

// I2S pins for the INMP441 microphone
#define I2S_SCK 16  // Serial Clock
#define I2S_WS 15   // Word Select (Left/Right Clock)
#define I2S_SD 7    // Serial Data

// Create an instance of SensorManager
SensorManager sensors;

// Variables for tracking time
unsigned long lastReadingTime = 0;
const int readingInterval = 1000; // Read every 1 second

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000); // Wait for serial to be ready
  
  Serial.println("INMP441 Decibel Level Example");
  Serial.println("============================");
  
  // Initialize the microphone with I2S pins
  if (!sensors.beginMicrophone(I2S_SCK, I2S_WS, I2S_SD)) {
    Serial.println("Failed to initialize INMP441 microphone!");
    Serial.println("Please check your connections and I2S pin configuration.");
    while (1) {
      delay(1000); // Halt in an infinite loop
    }
  }
  
  Serial.println("INMP441 microphone initialized successfully!");
  Serial.println("Starting decibel readings...");
  Serial.println();
}

void loop() {
  // Read decibel level at regular intervals
  if (millis() - lastReadingTime >= readingInterval) {
    lastReadingTime = millis();
    
    // Read the decibel level
    float dB = sensors.readDecibelLevel();
    
    // Print the result
    Serial.print("Sound level: ");
    Serial.print(dB, 1); // Print with 1 decimal place
    Serial.println(" dB");
    
    // Classify the noise level
    classifyNoiseLevel(dB);
    
    Serial.println();
  }
}

// Helper function to classify and display the noise level
void classifyNoiseLevel(float dB) {
  if (dB < 40) {
    Serial.println("Noise level: Very quiet (library, whisper)");
  } else if (dB < 50) {
    Serial.println("Noise level: Quiet (quiet office)");
  } else if (dB < 60) {
    Serial.println("Noise level: Moderate (normal conversation)");
  } else if (dB < 70) {
    Serial.println("Noise level: Noisy (busy office)");
  } else if (dB < 80) {
    Serial.println("Noise level: Loud (vacuum cleaner)");
  } else if (dB < 90) {
    Serial.println("Noise level: Very loud (heavy traffic)");
  } else if (dB < 100) {
    Serial.println("Noise level: Extremely loud (power tools)");
  } else {
    Serial.println("Noise level: Potentially dangerous (jackhammer, concert)");
  }
} 