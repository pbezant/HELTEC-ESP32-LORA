#pragma once

// ===== Board Configuration =====
// I2C Pins
#define I2C_SDA 3 //Green wire
#define I2C_SCL 4 //White wire

// BME280 Sensor
#define BME_ADDRESS 0x76  // Try 0x77 if 0x76 doesn't work
#define SEALEVELPRESSURE_HPA (1013.25)

// INMP441 Microphone Configuration (I2S)
#define I2S_SCK 16  // Serial Clock
#define I2S_WS 15   // Word Select or Left/Right Clock
#define I2S_SD 7    // Serial Data
#define I2S_PORT I2S_NUM_0

// Sound level configuration
#define SAMPLE_RATE 16000      // Sample rate in Hz
#define SAMPLE_BITS 32         // Bits per sample
#define BUFFER_SIZE 512        // Buffer size for I2S and FFT
#define DB_SMOOTHING_FACTOR 0.8 // Smoothing factor for dB readings (0-1)
#define DB_SAMPLING_PERIOD 1000 // How often to sample sound in milliseconds
#define DB_OFFSET 35           // Calibration offset for dB calculation

// PIR Motion Sensor
#define PIR_PIN 5 //White wire
#define PIR_WAKE_LEVEL HIGH  // HIGH for active-high PIR, LOW for active-low

// Button for UI navigation
#define BUTTON_PIN 0  // PRG button on Heltec board

// ===== LoRaWAN Configuration =====
// Spreading Factor (7-12)
#define LORA_SF 7  
#define TX_POWER 14  // Transmit power in dBm (max 20dBm)

// LoRaWAN Join Timeout (in milliseconds)
#define LORAWAN_JOIN_TIMEOUT 60000  // 60 seconds

// LoRaWAN Confirmed Messages
#define LORAWAN_CONFIRMED_MESSAGES true  // Set to true for confirmed uplinks

// ===== RadioLib SX1262 pins for Heltec ESP32 LoRa V3 =====
// Correct pin definitions for SX1262 on Heltec WiFi LoRa 32 V3
#define LORA_CS 8     // NSS pin
#define LORA_DIO1 14  // DIO1 pin (interrupt)
#define LORA_RST 12   // Reset pin
#define LORA_BUSY 13  // Busy pin

// SPI pins for SX1262
#define LORA_SCK 9    // SPI clock
#define LORA_MOSI 10  // SPI MOSI
#define LORA_MISO 11  // SPI MISO

// ===== LoRaWAN Frequency =====
#define LORA_FREQUENCY 915.0  // MHz (US915)
#define LORA_BANDWIDTH 125.0  // kHz
#define LORA_CODING_RATE 7    // 4/7
#define LORA_SYNC_WORD 0x34   // Private LoRa networks

// ===== Application Configuration =====
#define MINIMUM_DELAY 120  // Minimum delay between transmissions in seconds
#define MAX_BACKOFF_DELAY 3600  // Maximum backoff delay in seconds (1 hour)
#define DEBUG_SERIAL true  // Enable serial debug output

// ===== Display Configuration =====
#define DISPLAY_ENABLED true
#define DISPLAY_TIMEOUT 30000  // Turn off display after this many ms of inactivity

// ===== OLED Display Configuration =====
#define OLED_SDA 17
#define OLED_SCL 18
#define OLED_RST 21
#define OLED_WIDTH 128
#define OLED_HEIGHT 64 