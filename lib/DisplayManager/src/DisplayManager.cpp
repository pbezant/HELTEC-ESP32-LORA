#include "DisplayManager.h"

// Default pins for OLED display
#define DEFAULT_OLED_SDA 17
#define DEFAULT_OLED_SCL 18
#define DEFAULT_OLED_RST 21
#define VEXT_PIN 36  // VEXT control pin on Heltec boards (GPIO36)

DisplayManager::DisplayManager() : 
    u8g2(U8G2_R0, DEFAULT_OLED_SCL, DEFAULT_OLED_SDA, DEFAULT_OLED_RST),
    currentLogLine(0),
    currentScreen(0) {
    
    // Initialize log buffer
    for (int i = 0; i < MAX_LOG_LINES; i++) {
        logBuffer[i] = "";
    }
}

void DisplayManager::controlDisplayPower(bool state, bool inverted) {
    pinMode(VEXT_PIN, OUTPUT);
    
    // For V3.2 boards, VEXT is inverted (LOW turns on the display, HIGH turns it off)
    // For older boards, VEXT is normal (LOW turns off the display, HIGH turns it on)
    if (inverted) {
        digitalWrite(VEXT_PIN, state ? LOW : HIGH);
    } else {
        digitalWrite(VEXT_PIN, state ? HIGH : LOW);
    }
    
    delay(10); // Small delay to ensure power stabilizes
}

void DisplayManager::begin(int sda, int scl, BoardVersion boardVersion) {
    // Control display power based on board version
    controlDisplayPower(true, boardVersion == V3_2);
    
    // Initialize the OLED display
    if (sda != -1 && scl != -1) {
        u8g2.setBusClock(400000);  // Set I2C clock speed to 400kHz
        u8g2.begin();
    } else {
        u8g2.begin();
    }
    
    // Set initial font
    u8g2.setFont(u8g2_font_profont12_tf);
    
    // Set normal display mode (white text on black background)
    u8g2.setFlipMode(0);
    u8g2.sendF("ca", 0x0A8, 0x03F); // Set MUX ratio
    u8g2.setContrast(255);          // Maximum contrast
    u8g2.setDisplayRotation(U8G2_R0); // Normal orientation
    
    // Set drawing color to white (1)
    u8g2.setDrawColor(1);           // White drawing color for text/lines
    u8g2.setFontMode(0);            // Solid font (not transparent)
    u8g2.setColorIndex(1);          // White pixels active
    
    // Clear display and set screen to startup
    clear();
    currentScreen = 0; // Default screen
    currentLogLine = 0;
    
    // Clear log buffer
    for (int i = 0; i < MAX_LOG_LINES; i++) {
        logBuffer[i] = "";
    }
    
    Serial.println(F("Display initialized"));
}

void DisplayManager::clear() {
    // Save current color index
    uint8_t colorIndex = u8g2.getColorIndex();
    
    // Set color to black (0) for clearing
    u8g2.setColorIndex(0);
    
    // Fill entire screen with black
    u8g2.clearBuffer();
    
    // Restore the original color index (usually white/1)
    u8g2.setColorIndex(colorIndex);
}

void DisplayManager::refresh() {
    u8g2.sendBuffer();
}

void DisplayManager::sleep() {
    u8g2.setPowerSave(1);
}

void DisplayManager::wakeup() {
    u8g2.setPowerSave(0);  // Wake up display
}

void DisplayManager::setContrast(uint8_t contrast) {
    // Set contrast value (0-255), higher values make display brighter
    u8g2.setContrast(contrast);
    refresh();  // Apply changes to display
}

void DisplayManager::setNormalMode() {
    // Reset display to normal mode (white text on black background)
    u8g2.setFlipMode(0);
    u8g2.setContrast(255);  // Maximum contrast
    
    // These are SSD1306 specific commands
    u8g2.sendF("ca", 0x0A8, 0x03F); // Set MUX ratio
    u8g2.sendF("c", 0x0A6);         // Normal display (not inverted)
    
    // Set drawing color to white (1)
    u8g2.setDrawColor(1);           // White drawing color for text/lines
    u8g2.setFontMode(0);            // Solid font (not transparent)
    u8g2.setColorIndex(1);          // White pixels active
    
    // Force a display refresh
    refresh();
}

void DisplayManager::drawString(int x, int y, const String& text) {
    u8g2.drawStr(x, y, text.c_str());
}

void DisplayManager::drawCenteredString(int y, const String& text) {
    int width = u8g2.getStrWidth(text.c_str());
    int x = (SCREEN_WIDTH - width) / 2;
    if (x < 0) x = 0;
    
    drawString(x, y, text);
}

void DisplayManager::drawRightAlignedString(int y, const String& text) {
    int width = u8g2.getStrWidth(text.c_str());
    int x = SCREEN_WIDTH - width;
    if (x < 0) x = 0;
    
    drawString(x, y, text);
}

void DisplayManager::drawProgressBar(int x, int y, int width, int height, uint8_t progress) {
    // Draw border
    drawRect(x, y, width, height);
    
    // Calculate progress width (accounting for border)
    int progressWidth = (width - 2) * progress / 100;
    
    // Draw filled progress area
    if (progressWidth > 0) {
        u8g2.setDrawColor(1); // Ensure we're drawing in white
        u8g2.drawBox(x + 1, y + 1, progressWidth, height - 2);
    }
}

void DisplayManager::drawRect(int x, int y, int width, int height) {
    u8g2.drawFrame(x, y, width, height);
}

void DisplayManager::fillRect(int x, int y, int width, int height) {
    u8g2.setDrawColor(0); // 0 for black, 1 for white
    u8g2.drawBox(x, y, width, height);
    u8g2.setDrawColor(1); // Reset to white for subsequent drawing
}

void DisplayManager::drawLine(int x0, int y0, int x1, int y1) {
    u8g2.drawLine(x0, y0, x1, y1);
}

void DisplayManager::setScreen(uint8_t screenIndex, bool redraw) {
    // Update the screen index
    currentScreen = screenIndex;
    
    // If redraw is false, just update the screen index without clearing or drawing
    if (!redraw) {
        return;
    }
    
    // Otherwise, clear and draw the new screen
    clear();
    
    switch (currentScreen) {
        case 0: // Main info screen
            // Empty, to be filled by main application
            break;
        case 1: // Startup screen
            drawStartupScreen();
            break;
        case 2: // LoRaWAN status screen
            drawLoRaWANStatusScreen();
            break;
        case 3: // Sensor data screen
            drawSensorDataScreen();
            break;
        case 4: // Log screen
            drawLogScreen();
            break;
        default:
            // Default to main screen
            currentScreen = 0;
            break;
    }
    
    refresh();
}

void DisplayManager::log(const String& message) {
    // Shift messages up
    for (int i = 0; i < MAX_LOG_LINES - 1; i++) {
        logBuffer[i] = logBuffer[i + 1];
    }
    
    // Add new message to bottom
    logBuffer[MAX_LOG_LINES - 1] = message;
    
    // If we're on the log screen, refresh it
    if (currentScreen == 4) {
        refreshLogScreen();
    }
}

void DisplayManager::clearLog() {
    for (int i = 0; i < MAX_LOG_LINES; i++) {
        logBuffer[i] = "";
    }
    
    if (currentScreen == 4) {
        refreshLogScreen();
    }
}

void DisplayManager::refreshLogScreen() {
    if (currentScreen != 4) return;
    
    // Clear the log area
    fillRect(0, 16, SCREEN_WIDTH, SCREEN_HEIGHT - 16);
    
    // Draw log entries
    u8g2.setFont(u8g2_font_profont10_tf);
    
    int y = 25;
    int startIdx = MAX_LOG_LINES - 5; // Show last 5 entries
    if (startIdx < 0) startIdx = 0;
    
    for (int i = startIdx; i < MAX_LOG_LINES; i++) {
        if (logBuffer[i].length() > 0) {
            drawString(0, y, logBuffer[i]);
            y += LINE_HEIGHT;
        }
    }
    
    refresh();
}

void DisplayManager::setFont(const uint8_t* font) {
    u8g2.setFont(font);
}

void DisplayManager::drawStartupScreen() {
    clear();
    
    // Draw logo
    u8g2.setFont(u8g2_font_profont12_tf);
    drawCenteredString(15, "StructureSense");
    
    u8g2.setFont(u8g2_font_profont10_tf);
    drawCenteredString(30, "ESP32 LoRaWAN Node");
    drawCenteredString(45, "Initializing...");
    
    // Draw progress bar
    drawProgressBar(20, 50, SCREEN_WIDTH - 40, 10, 0);
    
    refresh();
}

void DisplayManager::updateStartupProgress(uint8_t progress, const String& statusText) {
    // Update progress bar
    drawProgressBar(20, 50, SCREEN_WIDTH - 40, 10, progress);
    
    // Clear status area
    fillRect(0, 35, SCREEN_WIDTH, 10);
    
    // Update status text
    drawCenteredString(45, statusText);
    
    refresh();
}

void DisplayManager::drawLoRaWANStatusScreen() {
    clear();
    
    u8g2.setFont(u8g2_font_profont12_tf);
    drawCenteredString(12, "LoRaWAN Status");
    
    u8g2.setFont(u8g2_font_profont10_tf);
    drawLine(0, 15, SCREEN_WIDTH, 15);
    
    // This screen will be updated with actual data by the main application
    
    refresh();
}

void DisplayManager::updateLoRaWANStatus(bool joined, int16_t rssi, uint32_t uplinks, uint32_t downlinks) {
    if (currentScreen != 2) return;
    
    // Clear the data area
    fillRect(0, 16, SCREEN_WIDTH, SCREEN_HEIGHT - 16);
    
    // Draw network status
    drawString(0, 25, "Network:");
    drawRightAlignedString(25, joined ? "JOINED" : "NOT JOINED");
    
    // Draw RSSI
    drawString(0, 35, "RSSI:");
    drawRightAlignedString(35, String(rssi) + " dBm");
    
    // Draw uplink/downlink counters
    drawString(0, 45, "Uplinks:");
    drawRightAlignedString(45, String(uplinks));
    
    drawString(0, 55, "Downlinks:");
    drawRightAlignedString(55, String(downlinks));
    
    refresh();
}

void DisplayManager::drawSensorDataScreen() {
    clear();
    
    u8g2.setFont(u8g2_font_profont12_tf);
    drawCenteredString(12, "Sensor Data");
    
    u8g2.setFont(u8g2_font_profont10_tf);
    drawLine(0, 15, SCREEN_WIDTH, 15);
    
    // This screen will be updated with actual data by the main application
    
    refresh();
}

void DisplayManager::updateSensorData(float temperature, float humidity, float pressure, float battery) {
    if (currentScreen != 3) return;
    
    // Clear the data area
    fillRect(0, 16, SCREEN_WIDTH, SCREEN_HEIGHT - 16);
    
    // Draw temperature
    drawString(0, 25, "Temp:");
    drawRightAlignedString(25, String(temperature, 1) + " C");
    
    // Draw humidity
    drawString(0, 35, "Humidity:");
    drawRightAlignedString(35, String(humidity, 1) + " %");
    
    // Draw pressure
    drawString(0, 45, "Pressure:");
    drawRightAlignedString(45, String(pressure, 1) + " hPa");
    
    // Draw battery
    drawString(0, 55, "Battery:");
    drawRightAlignedString(55, String(battery, 1) + " V");
    
    // Always refresh the display immediately after updating sensor data
    refresh();
}

void DisplayManager::showErrorScreen(const String& title, const String& errorMsg) {
    clear();
    
    // Set title font and draw title
    u8g2.setFont(u8g2_font_profont12_tf);
    drawCenteredString(12, title);
    
    // Draw separation line
    u8g2.setFont(u8g2_font_profont10_tf);
    drawLine(0, 15, SCREEN_WIDTH, 15);
    
    // Draw error message - handle long messages by splitting across lines if needed
    if (errorMsg.length() <= 21) { // Fits on a single line
        drawCenteredString(35, errorMsg);
    } else {
        // Simple word wrap for error message
        String line1 = errorMsg.substring(0, 21);
        String line2 = errorMsg.substring(21);
        
        // If line1 doesn't end with a space but cuts a word, try to find a better break point
        int lastSpace = line1.lastIndexOf(' ');
        if (lastSpace > 0) {
            line2 = line1.substring(lastSpace + 1) + line2;
            line1 = line1.substring(0, lastSpace);
        }
        
        drawCenteredString(30, line1);
        drawCenteredString(42, line2);
    }
    
    refresh();
    
    // Also log the error
    log("ERROR: " + errorMsg);
}

void DisplayManager::showLoRaError(int errorCode) {
    String errorMessage;
    
    // Translate error codes to human-readable messages
    switch (errorCode) {
        case -1:
            errorMessage = "Unknown error";
            break;
        case -2:
            errorMessage = "Invalid state";
            break;
        case -3:
            errorMessage = "Secondary header not found";
            break;
        case -4:
            errorMessage = "Invalid header format";
            break;
        case -5:
            errorMessage = "Invalid packet length";
            break;
        case -1106:
            errorMessage = "Join Accept not received";
            break;
        case -1118:
            errorMessage = "CFList missing";
            break;
        default:
            errorMessage = "Error code: " + String(errorCode);
            break;
    }
    
    showErrorScreen("LoRaWAN Error", errorMessage);
}

void DisplayManager::drawLogScreen() {
    clear();
    
    // Set title font and draw title
    u8g2.setFont(u8g2_font_profont12_tf);
    drawCenteredString(12, "System Log");
    
    // Draw separation line
    u8g2.setFont(u8g2_font_profont10_tf);
    drawLine(0, 15, SCREEN_WIDTH, 15);
    
    // Refresh the log content
    refreshLogScreen();
} 