#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <U8g2lib.h>

/**
 * @brief A class to manage OLED display functionality for ESP32 projects
 * 
 * This class provides a simplified interface for OLED display operations,
 * including rendering screens, logging messages, and display control.
 */
class DisplayManager {
public:
    // Constants
    static const int SCREEN_WIDTH = 128;
    static const int SCREEN_HEIGHT = 64;
    static const int LINE_HEIGHT = 10;
    static const int MAX_LOG_LINES = 10;
    
    // Heltec board versions
    enum BoardVersion {
        V3_0, // V3.0 or V3.1
        V3_2  // V3.2 (inverted VEXT)
    };

    /**
     * @brief Constructor
     */
    DisplayManager();
    
    /**
     * @brief Initialize the display
     * 
     * @param sda SDA pin (pass -1 to use default)
     * @param scl SCL pin (pass -1 to use default)
     * @param boardVersion Board version (V3_0 for V3.0/V3.1, V3_2 for V3.2)
     */
    void begin(int sda = -1, int scl = -1, BoardVersion boardVersion = V3_0);
    
    /**
     * @brief Control VEXT power pin for display
     * 
     * @param state true to enable display power, false to disable
     * @param inverted true for V3.2 boards where VEXT is inverted, false for other boards
     */
    void controlDisplayPower(bool state, bool inverted = false);
    
    /**
     * @brief Clear the display
     */
    void clear();
    
    /**
     * @brief Send buffer to display
     */
    void refresh();
    
    /**
     * @brief Put display in power save mode
     */
    void sleep();
    
    /**
     * @brief Wake display from power save mode
     */
    void wakeup();
    
    /**
     * @brief Set display contrast
     * 
     * @param contrast Contrast value (0-255)
     */
    void setContrast(uint8_t contrast);
    
    /**
     * @brief Set display to normal mode (white text on black background)
     */
    void setNormalMode();
    
    /**
     * @brief Draw a string at a specific position
     * 
     * @param x X coordinate
     * @param y Y coordinate
     * @param text Text to draw
     */
    void drawString(int x, int y, const String& text);
    
    /**
     * @brief Draw a centered string at a specific Y position
     * 
     * @param y Y coordinate
     * @param text Text to draw
     */
    void drawCenteredString(int y, const String& text);
    
    /**
     * @brief Draw a right-aligned string at a specific Y position
     * 
     * @param y Y coordinate
     * @param text Text to draw
     */
    void drawRightAlignedString(int y, const String& text);
    
    /**
     * @brief Draw a progress bar
     * 
     * @param x X coordinate of top-left corner
     * @param y Y coordinate of top-left corner
     * @param width Width of progress bar
     * @param height Height of progress bar
     * @param progress Progress value (0-100)
     */
    void drawProgressBar(int x, int y, int width, int height, uint8_t progress);
    
    /**
     * @brief Draw a rectangle
     * 
     * @param x X coordinate of top-left corner
     * @param y Y coordinate of top-left corner
     * @param width Width of rectangle
     * @param height Height of rectangle
     */
    void drawRect(int x, int y, int width, int height);
    
    /**
     * @brief Fill a rectangle
     * 
     * @param x X coordinate of top-left corner
     * @param y Y coordinate of top-left corner
     * @param width Width of rectangle
     * @param height Height of rectangle
     */
    void fillRect(int x, int y, int width, int height);
    
    /**
     * @brief Draw a line
     * 
     * @param x0 Start X coordinate
     * @param y0 Start Y coordinate
     * @param x1 End X coordinate
     * @param y1 End Y coordinate
     */
    void drawLine(int x0, int y0, int x1, int y1);
    
    /**
     * @brief Change to a specific screen
     * 
     * @param screenIndex Index of screen to display
     * 0 = Main screen
     * 1 = Startup screen
     * 2 = LoRaWAN status screen
     * 3 = Sensor data screen
     * 4 = Log screen
     * @param redraw If true, clear and redraw the screen; if false, just update the screen index
     */
    void setScreen(uint8_t screenIndex, bool redraw = true);
    
    /**
     * @brief Get the current screen index
     * 
     * @return uint8_t Current screen index
     */
    uint8_t getCurrentScreen() const { return currentScreen; }
    
    /**
     * @brief Add a message to the log
     * 
     * @param message Message to log
     */
    void log(const String& message);
    
    /**
     * @brief Clear the log
     */
    void clearLog();
    
    /**
     * @brief Refresh the log screen
     */
    void refreshLogScreen();
    
    /**
     * @brief Set the font for text rendering
     * 
     * @param font Font to use
     */
    void setFont(const uint8_t* font);
    
    /**
     * @brief Draw the startup screen
     */
    void drawStartupScreen();
    
    /**
     * @brief Update the progress bar on the startup screen
     * 
     * @param progress Progress value (0-100)
     * @param statusText Status text to display
     */
    void updateStartupProgress(uint8_t progress, const String& statusText);
    
    /**
     * @brief Draw the LoRaWAN status screen
     */
    void drawLoRaWANStatusScreen();
    
    /**
     * @brief Update the LoRaWAN status screen
     * 
     * @param joined Whether device is joined to network
     * @param rssi Last RSSI value
     * @param uplinks Number of uplink messages
     * @param downlinks Number of downlink messages
     */
    void updateLoRaWANStatus(bool joined, int16_t rssi, uint32_t uplinks, uint32_t downlinks);
    
    /**
     * @brief Draw the sensor data screen
     */
    void drawSensorDataScreen();
    
    /**
     * @brief Update the sensor data screen
     * 
     * @param temperature Temperature in degrees C
     * @param humidity Humidity in percent
     * @param pressure Pressure in hPa
     * @param battery Battery voltage
     */
    void updateSensorData(float temperature, float humidity, float pressure, float battery);
    
    /**
     * @brief Show an error screen
     * 
     * @param title Error title
     * @param errorMsg Error message
     */
    void showErrorScreen(const String& title, const String& errorMsg);
    
    /**
     * @brief Show a LoRaWAN-specific error screen
     * 
     * @param errorCode LoRaWAN error code
     */
    void showLoRaError(int errorCode);
    
    /**
     * @brief Draw the log screen
     */
    void drawLogScreen();
    
private:
    // OLED display instance
    U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2;
    
    // Log buffer
    String logBuffer[MAX_LOG_LINES];
    int currentLogLine;
    
    // Current screen index
    uint8_t currentScreen;
};

#endif // DISPLAY_MANAGER_H 