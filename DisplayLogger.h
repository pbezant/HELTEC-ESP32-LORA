#pragma once
#include <heltec_unofficial.h>

class DisplayLogger {
private:
    static const int SCREEN_WIDTH = 128;
    static const int SCREEN_HEIGHT = 64;
    static const int LINE_HEIGHT = 10;
    static const int MAX_LINES = 6;
    
    String buffer[MAX_LINES];
    int currentLine;
    bool initialized;

public:
    enum class TextAlignment {
        LEFT,
        CENTER,
        RIGHT
    };

    DisplayLogger() : currentLine(0), initialized(false) {
        for (int i = 0; i < MAX_LINES; i++) {
            buffer[i] = "";
        }
    }

    void begin() {
        if (!initialized) {
            Heltec.display->init();
            Heltec.display->clear();
            Heltec.display->setFont(ArialMT_Plain_10);
            Heltec.display->display();
            initialized = true;
        }
    }

    void log(const String& message) {
        if (!initialized) begin();

        // Shift existing lines up
        for (int i = 0; i < MAX_LINES - 1; i++) {
            buffer[i] = buffer[i + 1];
        }
        buffer[MAX_LINES - 1] = message;

        // Redraw display
        refreshDisplay();
    }

    void clear() {
        if (!initialized) begin();
        
        for (int i = 0; i < MAX_LINES; i++) {
            buffer[i] = "";
        }
        Heltec.display->clear();
        Heltec.display->display();
    }

    void displayText(const String& message, int row, TextAlignment align) {
        if (!initialized) begin();
        
        int yPos = row * LINE_HEIGHT;
        int xPos = 0;
        
        switch(align) {
            case TextAlignment::LEFT:
                xPos = 0;
                break;
            case TextAlignment::CENTER:
                xPos = (SCREEN_WIDTH - Heltec.display->getStringWidth(message)) / 2;
                break;
            case TextAlignment::RIGHT:
                xPos = SCREEN_WIDTH - Heltec.display->getStringWidth(message);
                break;
        }
        
        Heltec.display->drawString(xPos, yPos, message);
        Heltec.display->display();
    }

    void displayProgressBar(uint8_t percentage) {
        if (!initialized) begin();
        
        const int barHeight = 6;
        const int barWidth = 120;
        const int x = 4;
        const int y = 58;
        
        Heltec.display->drawRect(x, y, barWidth, barHeight);
        int fillWidth = (barWidth - 2) * percentage / 100;
        Heltec.display->fillRect(x+1, y+1, fillWidth, barHeight-2);
        Heltec.display->display();
    }

    void serialMirror(const String& message) {
        log(message);
        Serial.println(message);
    }

    void runTest() {
        clear();
        
        serialMirror("Display Test Start");
        delay(1000);
        
        // Test basic logging
        for (int i = 1; i <= 6; i++) {
            serialMirror("Test line " + String(i));
            delay(500);
        }
        delay(1000);
        
        // Test alignment
        clear();
        displayText("Alignment Test", 0, TextAlignment::CENTER);
        displayText("Left", 1, TextAlignment::LEFT);
        displayText("Center", 2, TextAlignment::CENTER);
        displayText("Right", 3, TextAlignment::RIGHT);
        delay(2000);
        
        // Test progress bar
        clear();
        displayText("Progress Test", 0, TextAlignment::CENTER);
        for (int i = 0; i <= 100; i += 10) {
            displayProgressBar(i);
            displayText("Loading: " + String(i) + "%", 3, TextAlignment::CENTER);
            delay(200);
        }
        delay(1000);
        
        clear();
        serialMirror("Test Complete");
    }

private:
    void refreshDisplay() {
        Heltec.display->clear();
        for (int i = 0; i < MAX_LINES; i++) {
            Heltec.display->drawString(0, i * LINE_HEIGHT, buffer[i]);
        }
        Heltec.display->display();
    }
};

