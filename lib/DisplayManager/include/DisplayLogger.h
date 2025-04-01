#pragma once

#include <Arduino.h>
#include "DisplayManager.h"

// A simple logger class that works with DisplayManager
class DisplayLogger {
private:
    DisplayManager& display;
    bool serialEcho;  // Whether to echo log messages to Serial

public:
    /**
     * @brief Construct a new Display Logger object
     * 
     * @param displayManager Reference to a DisplayManager instance
     * @param echoToSerial Whether to also print log messages to Serial (default: true)
     */
    DisplayLogger(DisplayManager& displayManager, bool echoToSerial = true) : 
        display(displayManager), 
        serialEcho(echoToSerial) {}

    /**
     * @brief Log an info message
     * 
     * @param message The message to log
     */
    void info(const String& message) {
        String formattedMsg = "INFO: " + message;
        display.log(formattedMsg);
        
        if (serialEcho) {
            Serial.println(formattedMsg);
        }
    }
    
    /**
     * @brief Log a warning message
     * 
     * @param message The message to log
     */
    void warning(const String& message) {
        String formattedMsg = "WARN: " + message;
        display.log(formattedMsg);
        
        if (serialEcho) {
            Serial.println(formattedMsg);
        }
    }
    
    /**
     * @brief Log an error message
     * 
     * @param message The message to log
     */
    void error(const String& message) {
        String formattedMsg = "ERROR: " + message;
        display.log(formattedMsg);
        
        if (serialEcho) {
            Serial.println(formattedMsg);
        }
    }
    
    /**
     * @brief Log a debug message
     * 
     * @param message The message to log
     */
    void debug(const String& message) {
        String formattedMsg = "DEBUG: " + message;
        display.log(formattedMsg);
        
        if (serialEcho) {
            Serial.println(formattedMsg);
        }
    }
    
    /**
     * @brief Clear the log buffer
     */
    void clear() {
        display.clearLog();
    }
    
    /**
     * @brief Set whether to echo log messages to Serial
     * 
     * @param echo True to echo to Serial, false to disable
     */
    void setSerialEcho(bool echo) {
        serialEcho = echo;
    }
    
    /**
     * @brief Show the log screen on the display
     */
    void showLogScreen() {
        display.setScreen(4);  // 4 is the log screen index
    }
}; 