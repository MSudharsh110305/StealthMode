#include "DisplayUtils.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_MOSI 9
#define OLED_CLK 10
#define OLED_DC 11
#define OLED_CS 12
#define OLED_RESET 13

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

unsigned long lastBlinkTime = 0;
const unsigned long BLINK_INTERVAL = 500;
bool blinkState = false;
int scrollOffset = 0;

void initDisplay() {
    if (!display.begin(SSD1306_SWITCHCAPVCC)) {
        SerialUSB.println(F("SSD1306 allocation failed"));
        for (;;);
    }

    display.clearDisplay();
    display.display();
}

void renderPage(int currentPage, String gpsTime, Adafruit_GPS gps) {
    display.clearDisplay();

    switch (currentPage) {
        case 0:
            drawStartupPage();
            break;
        case 1:
            drawMainUI(gpsTime, "C1", 100, "--.--", "--.--", "Hazard 153m ahead! Car3 pressed SOS. Stay safe!");
            break;
        case 2:
            drawStopwatchAndElapsedUI(gpsTime, "C1", 20, "00:00:00", "00:00:00", "Broadcasting message");
            break;
        case 3:
            drawStartEndTimePage(gpsTime, "C1", 20, "00:00:00", "00:00:00");
            break;
        case 4:
            drawOverallResultsPage(gpsTime, "C1", 20, "--.--", "00:00:00", "--");
            break;
    }

    display.display();
}

// Function to fetch time from GPS (RTC)
String gpsTime(Adafruit_GPS gps) {
    if (gps.hour < 10) {
        return String("0") + gps.hour + ":" + (gps.minute < 10 ? "0" + String(gps.minute) : String(gps.minute));
    }
    return String(gps.hour) + ":" + (gps.minute < 10 ? "0" + String(gps.minute) : String(gps.minute));
}

// Drawing functions for various UI elements

void drawStartupPage() {
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(SCREEN_WIDTH / 6, SCREEN_HEIGHT / 2 - 10);
    display.println("BLUEBAND  SPORTS");
}

void drawMainUI(String time, String carID, int batteryPercentage, String speed, String distance, String message) {
    // Top section
    display.setTextSize(1);
    display.setCursor(0, 3);
    display.print(time);

    // Car ID with circle (toggle between filled and hollow)
    display.setCursor(SCREEN_WIDTH / 2 - 15, 3);
    display.print(carID);
    drawCircleWithToggle(SCREEN_WIDTH / 2 + 18, 5, blinkState);

    drawBatteryStatus(batteryPercentage);

    // Divider line
    drawLine(0, 12, SCREEN_WIDTH, 12);

    // Speed and Distance
    display.setCursor(5, 16);
    display.setTextSize(2);
    display.println("spd:" + (speed != "--.--" ? speed : "N/A"));

    display.setCursor(5, 36);
    display.println("dis:" + (distance != "--.--" ? distance : "N/A"));
    display.setTextSize(1);

    // Divider line
    drawLine(0, 52, SCREEN_WIDTH, 36);

    // Broadcasting message with scrolling
    int messageWidth = message.length() * 6;
    display.setCursor(-scrollOffset, 55);
    display.print(message + " ");
    display.print(message);

    scrollOffset++;
    if (scrollOffset > messageWidth) scrollOffset = 0;
    drawLine(0, 63, SCREEN_WIDTH, 36);
}

void drawStopwatchAndElapsedUI(String time, String carID, int batteryPercentage, String stopwatchTime, String elapsedTime, String message) {
    // Top section
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(time);

    // Car ID with circle (toggle between filled and hollow)
    display.setCursor(SCREEN_WIDTH / 2 - 10, 0);
    display.print(carID);
    drawCircleWithToggle(SCREEN_WIDTH / 2 + 18, 4, blinkState);

    drawBatteryStatus(batteryPercentage);

    // Divider line
    drawLine(0, 12, SCREEN_WIDTH, 12);

    // Stopwatch and Elapsed Time
    display.setCursor(0, 16);
    display.println("SW: " + (stopwatchTime != "00:00:00" ? stopwatchTime : "N/A"));

    display.setCursor(0, 26);
    display.println("ET: " + (elapsedTime != "00:00:00" ? elapsedTime : "N/A"));

    // Divider line
    drawLine(0, 35, SCREEN_WIDTH, 36);

    // Broadcasting message with scrolling
    display.setCursor(-scrollOffset, 40);
    display.println(message);
    scrollOffset++;
    if (scrollOffset > SCREEN_WIDTH) scrollOffset = 0;
}

void drawStartEndTimePage(String time, String carID, int batteryPercentage, String startTime, String endTime) {
    // Top section
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(time);

    // Car ID with circle (toggle between filled and hollow)
    display.setCursor(SCREEN_WIDTH / 2 - 10, 0);
    display.print(carID);
    drawCircleWithToggle(SCREEN_WIDTH / 2 + 18, 4, blinkState);

    drawBatteryStatus(batteryPercentage);

    // Divider line
    drawLine(0, 12, SCREEN_WIDTH, 12);

    // Start and End times
    display.setCursor(0, 16);
    display.println("st: " + (startTime != "00:00:00" ? startTime : "N/A"));
    
    display.setCursor(0, 26);
    display.println("en: " + (endTime != "00:00:00" ? endTime : "N/A"));
}

void drawOverallResultsPage(String time, String carID, int batteryPercentage, String avgSpeed, String totalTime, String penalties) {
    // Top section
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(time);

    // Car ID with circle (toggle between filled and hollow)
    display.setCursor(SCREEN_WIDTH / 2 - 10, 0);
    display.print(carID);
    drawCircleWithToggle(SCREEN_WIDTH / 2 + 18, 4, blinkState);

    drawBatteryStatus(batteryPercentage);

    // Divider line
    drawLine(0, 12, SCREEN_WIDTH, 12);

    // Average speed, total time, and penalties
    display.setCursor(0, 16);
    display.println("avg spd: " + (avgSpeed != "--.--" ? avgSpeed : "N/A"));

    display.setCursor(0, 26);
    display.println("tot time: " + (totalTime != "00:00:00" ? totalTime : "N/A"));

    display.setCursor(0, 36);
    display.println("penalty: " + penalties);
}

// Helper functions to draw battery status and circle toggle
void drawBatteryStatus(int batteryPercentage) {
    display.setCursor(SCREEN_WIDTH - 25, 3);
    display.print(batteryPercentage);
    display.print("%");
}

void drawLine(int x0, int y0, int x1, int y1) {
    display.drawLine(x0, y0, x1, y1, SSD1306_WHITE);
}

void drawCircleWithToggle(int x, int y, bool isFilled) {
    if (isFilled) {
        display.fillCircle(x, y, 3, SSD1306_WHITE);
    } else {
        display.drawCircle(x, y, 3, SSD1306_WHITE);
    }
}
