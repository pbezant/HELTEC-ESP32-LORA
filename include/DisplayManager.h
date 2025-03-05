#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "Config.h"

class DisplayManager {
public:
    // Constructor
    DisplayManager();
    
    // Initialization
    void begin(int sda = -1, int scl = -1);
    
    // Basic display functions
    void clear();
    void refresh();
    void sleep();
    void wakeup();
    void setContrast(uint8_t contrast); // Set display contrast (brightness)
    void setNormalMode(); // Set display to normal mode (not inverted)
    
    // Text functions
    void drawString(int x, int y, const String& text);
    void drawCenteredString(int y, const String& text);
    void drawRightAlignedString(int y, const String& text);
    
    // Drawing functions
    void drawProgressBar(int x, int y, int width, int height, uint8_t progress);
    void drawRect(int x, int y, int width, int height);
    void fillRect(int x, int y, int width, int height);
    void drawLine(int x0, int y0, int x1, int y1);
    
    // Multi-screen management
    void setScreen(uint8_t screenIndex);
    uint8_t getCurrentScreen() { return currentScreen; }
    
    // Logging capability 
    void log(const String& message);
    void clearLog();
    void drawLogScreen();
    
    // Font management
    void setFont(const uint8_t* font);
    
    // LoRaWAN specific screens
    void drawStartupScreen();
    void updateStartupProgress(uint8_t progress, const String& statusText);
    void drawLoRaWANStatusScreen();
    void updateLoRaWANStatus(bool joined, int16_t rssi, uint32_t uplinks, uint32_t downlinks);
    void drawSensorDataScreen();
    void updateSensorData(float temperature, float humidity, float pressure, float battery);
    
    // Error display
    void showErrorScreen(const String& title, const String& errorMsg);
    void showLoRaError(int errorCode);
    
    // Direct access to display (use carefully)
    U8G2& getDisplay() { return u8g2; }
    
private:
    U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2;
    static const uint8_t SCREEN_WIDTH = OLED_WIDTH;
    static const uint8_t SCREEN_HEIGHT = OLED_HEIGHT;
    static const uint8_t MAX_LOG_LINES = 6;
    static const uint8_t LINE_HEIGHT = 10;
    
    String logBuffer[MAX_LOG_LINES];
    uint8_t currentLogLine;
    uint8_t currentScreen;
    
    void refreshLogScreen();
}; 