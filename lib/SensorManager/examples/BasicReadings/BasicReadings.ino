#include <Arduino.h>
#include <SensorManager.h>

SensorManager sensors;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); // Wait for serial console to open
  
  Serial.println("\nSensorManager Basic Readings Example");
  Serial.println("===================================");
  
  // Initialize with default I2C pins
  if (!sensors.begin()) {
    Serial.println("Could not find BME280 sensor!");
    Serial.println("Check your wiring and I2C address.");
    while (1) delay(10); // Pause forever
  }
  
  Serial.println("BME280 sensor initialized successfully!");
  Serial.println();
}

void loop() {
  // Method 1: Reading individual values
  Serial.println("Individual readings:");
  Serial.print("Temperature: ");
  Serial.print(sensors.readTemperature());
  Serial.println(" °C");
  
  Serial.print("Humidity: ");
  Serial.print(sensors.readHumidity());
  Serial.println(" %");
  
  Serial.print("Pressure: ");
  Serial.print(sensors.readPressure());
  Serial.println(" hPa");
  
  Serial.print("Altitude: ");
  Serial.print(sensors.readAltitude());
  Serial.println(" m");
  
  Serial.println();
  
  // Method 2: Reading all values at once (more efficient)
  float temperature, humidity, pressure, altitude;
  
  Serial.println("Efficient combined reading:");
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