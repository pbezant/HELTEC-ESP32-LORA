#include <Arduino.h>
#include <SensorManager.h>

// Define custom I2C pins for ESP32/ESP8266 or other boards that support custom I2C pins
const int SDA_PIN = 16;  // Change to your SDA pin
const int SCL_PIN = 17;  // Change to your SCL pin

SensorManager sensors;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); // Wait for serial console to open
  
  Serial.println("\nSensorManager Custom Pins Example");
  Serial.println("==================================");
  
  Serial.printf("Initializing BME280 with custom I2C pins (SDA: %d, SCL: %d)\n", SDA_PIN, SCL_PIN);
  
  // Initialize with custom I2C pins
  if (!sensors.begin(SDA_PIN, SCL_PIN)) {
    Serial.println("Could not find BME280 sensor!");
    Serial.println("Check your wiring, I2C address, and pin configuration.");
    while (1) delay(10); // Pause forever
  }
  
  Serial.println("BME280 sensor initialized successfully with custom pins!");
  Serial.println();
}

void loop() {
  // Get all values in a single call for efficiency
  float temperature, humidity, pressure, altitude;
  sensors.readBME280(temperature, humidity, pressure, altitude);
  
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");
  
  Serial.print("Altitude: ");
  Serial.print(altitude);
  Serial.println(" m");
  
  Serial.println("\n-----------------------------------\n");
  
  delay(5000); // Wait 5 seconds between readings
} 