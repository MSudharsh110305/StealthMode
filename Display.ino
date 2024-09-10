#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>  // For real-time clock functionality

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_MOSI 9
#define OLED_CLK 10
#define OLED_DC 11
#define OLED_CS 12
#define OLED_RESET 13
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// Define variables for UI
bool blinkState = false;  // For blinking effect when battery is low
unsigned long lastBlinkTime = 0;  // To track blinking interval
unsigned long lastScreenChangeTime = 0;  // To track splash screen timing
const unsigned long BLINK_INTERVAL = 500;  // 500 ms blink interval
const unsigned long SPLASH_DURATION = 2000;  // 2-second splash screen

// Current page of the UI
enum Pages { SPLASH, MAIN_UI, STOPWATCH_UI, END_RECORD_UI, RESULTS_UI };
Pages currentPage = SPLASH;

// Stopwatch variables
unsigned long stopwatchStartTime = 0;
bool isStopwatchRunning = false;
bool showElapsedTime = false;

RTC_DS3231 rtc;  // Real-Time Clock

void setup() {
    SerialUSB.begin(115200);

    // Initialize the OLED display
    if (!display.begin(SSD1306_SWITCHCAPVCC)) {
        SerialUSB.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    // Initialize the RTC
    if (!rtc.begin()) {
        SerialUSB.println("Couldn't find RTC");
        while (1);
    }

    // Set the current time to RTC (optional)
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    display.clearDisplay();
    display.display();
    lastScreenChangeTime = millis();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Handle splash screen transition
    if (currentPage == SPLASH && (currentTime - lastScreenChangeTime >= SPLASH_DURATION)) {
        currentPage = MAIN_UI;  // Move to the main UI after splash screen
    }

    // Handle blink state for low battery
    if (currentTime - lastBlinkTime >= BLINK_INTERVAL) {
        blinkState = !blinkState;  // Toggle blink state
        lastBlinkTime = currentTime;
    }

    // Update the display based on the current page
    if (currentPage == SPLASH) {
        showSplashScreen();
    } else if (currentPage == MAIN_UI) {
        showMainUI();
    } else if (currentPage == STOPWATCH_UI) {
        showStopwatchUI();
    } else if (currentPage == END_RECORD_UI) {
        showStartEndRecordUI();
    } else if (currentPage == RESULTS_UI) {
        showResultsUI();
    }

    delay(10);  // Small delay to avoid overwhelming the loop
}

// Function to show the splash screen
void showSplashScreen() {
    display.clearDisplay();
    display.setTextSize(2);  // Larger text size for splash screen
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(SCREEN_WIDTH / 6, SCREEN_HEIGHT / 2 - 10);
    display.println("BLUEBAND");
    display.setCursor(SCREEN_WIDTH / 6, SCREEN_HEIGHT / 2 + 10);
    display.println("SPORTS");
    display.display();
}

// Function to show the main UI
void showMainUI() {
    display.clearDisplay();
    DateTime now = rtc.now();  // Get current time from RTC
    String timeStr = String(now.hour()) + ":" + String(now.minute());

    // Draw Real-Time Clock
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 1);
    display.print(timeStr);

    // Draw Battery Status
    drawBatteryStatus(20);  // Example with 20% battery

    // Draw Car ID
    display.setCursor(SCREEN_WIDTH - 20, 1);
    display.print("C1");

    // Draw Dashed Line
    drawDashedLine(0, 10, SCREEN_WIDTH, 10);

    // Display Speed and Distance
    display.setCursor(0, 20);
    display.print("spd: --.-- km/h");
    display.setCursor(0, 30);
    display.print("dis: --.-- km");

    // Another Dashed Line
    drawDashedLine(0, 40, SCREEN_WIDTH, 40);

    // Placeholder for broadcasting message
    display.setCursor(0, 50);
    display.print("Broadcast message here");

    display.display();
}

// Function to show the stopwatch UI
void showStopwatchUI() {
    display.clearDisplay();
    DateTime now = rtc.now();  // Get current time from RTC
    String timeStr = String(now.hour()) + ":" + String(now.minute());

    // Draw Real-Time Clock
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 1);
    display.print(timeStr);

    // Draw Battery Status
    drawBatteryStatus(20);  // Example with 20% battery

    // Draw Car ID
    display.setCursor(SCREEN_WIDTH - 20, 1);
    display.print("C1");

    // Draw Dashed Line
    drawDashedLine(0, 10, SCREEN_WIDTH, 10);

    // Display Stopwatch or Elapsed Time
    display.setCursor(0, 20);
    if (isStopwatchRunning) {
        unsigned long elapsedTime = (millis() - stopwatchStartTime) / 1000;
        display.print("Stopwatch: " + formatTime(elapsedTime));
    } else {
        display.print("Elapsed time: " + formatTime((millis() - stopwatchStartTime) / 1000));
    }

    // Another Dashed Line
    drawDashedLine(0, 40, SCREEN_WIDTH, 40);

    // Placeholder for broadcasting message
    display.setCursor(0, 50);
    display.print("Broadcast message here");

    display.display();
}

// Function to show the start/end time record UI
void showStartEndRecordUI() {
    display.clearDisplay();
    DateTime now = rtc.now();  // Get current time from RTC
    String timeStr = String(now.hour()) + ":" + String(now.minute());

    // Draw Real-Time Clock
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 1);
    display.print(timeStr);

    // Draw Battery Status
    drawBatteryStatus(20);  // Example with 20% battery

    // Draw Car ID
    display.setCursor(SCREEN_WIDTH - 20, 1);
    display.print("C1");

    // Draw Dashed Line
    drawDashedLine(0, 10, SCREEN_WIDTH, 10);

    // Display Start/End Times
    display.setCursor(0, 20);
    display.print("st: 00:00:00");  // Example placeholders
    display.setCursor(0, 30);
    display.print("en: 00:00:00");

    display.display();
}

// Function to show the overall results page
void showResultsUI() {
    display.clearDisplay();

    // Display Average Speed
    display.setCursor(0, 10);
    display.print("avg spd: --.-- km/h");

    // Display Total Time
    display.setCursor(0, 20);
    display.print("total time: 00:00:00");

    // Display Penalties
    display.setCursor(0, 30);
    display.print("penalties: --");

    display.display();
}

// Function to draw battery status (with low battery warning)
void drawBatteryStatus(int percentage) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    String displayText;
    if (percentage < 25 && blinkState) {
        displayText = "LOW";
    } else {
        displayText = String(percentage) + "%";
    }

    // Calculate box dimensions
    int textWidth = displayText.length() * 6;
    int boxPadding = 2;
    int boxWidth = textWidth + 2 * boxPadding;
    int boxHeight = 12;
    int boxX = SCREEN_WIDTH - boxWidth - 5;
    int boxY = 0;

    // Draw the box
    display.drawRect(boxX, boxY, boxWidth, boxHeight, SSD1306_WHITE);
    display.setCursor(boxX + boxPadding, boxY + boxPadding);
    display.print(displayText);
}

// Function to draw a dashed line
void drawDashedLine(int x0, int y0, int x1, int y1) {
    for (int i = x0; i < x1; i += 2) {
        display.drawPixel(i, y0, SSD1306_WHITE);  // Adjust spacing for dash effect
    }
}

// Helper function to format time (seconds to HH:MM:SS)
String formatTime(unsigned long timeInSeconds) {
    unsigned long hours = timeInSeconds / 3600;
    unsigned long minutes = (timeInSeconds % 3600) / 60;
    unsigned long seconds = timeInSeconds % 60;
    return String(hours) + ":" + String(minutes) + ":" + String(seconds);
}
