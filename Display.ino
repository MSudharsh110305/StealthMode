#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GPS.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// GSM/GPS variables (assuming GPS is used for time)
Adafruit_GPS gps = Adafruit_GPS(&Serial1);  // Initialize GPS on Serial1

bool blinkState = false;
unsigned long lastBlinkTime = 0;
const unsigned long BLINK_INTERVAL = 500;
unsigned long lastPageSwitchTime = 0;
const unsigned long PAGE_SWITCH_DELAY = 2000;

int currentPage = 0;
unsigned long stopwatchStart = 0;
unsigned long stopwatchElapsed = 0;
bool stopwatchRunning = false;

int scrollOffset = 0; // Variable for scrolling message

String carID = "C1"; // Variable for car ID

// Splash screen timing variables
bool splashScreenShown = false;  // Track if the splash screen has been displayed
unsigned long splashScreenStartTime = 0;
const unsigned long SPLASH_SCREEN_DURATION = 3000; // 3 seconds total duration
int blinkCount = 0;

void setup() {
    SerialUSB.begin(115200);
    gps.begin(9600);  // Start GPS at 9600 baud

    // Initialize the OLED display
    if (!display.begin(SSD1306_SWITCHCAPVCC)) {
        SerialUSB.println(F("SSD1306 allocation failed"));
        for (;;);
    }

    display.clearDisplay();
    display.display();
}

void loop() {
    unsigned long currentTime = millis();

    // Show the splash screen if not yet displayed
    if (!splashScreenShown) {
        if (currentTime - splashScreenStartTime >= SPLASH_SCREEN_DURATION) {
            splashScreenShown = true;  // Splash screen has been displayed for 3 seconds
            currentPage = 1;  // Start from Page 1 after splash screen
            lastPageSwitchTime = currentTime;  // Reset page switch timer
            return;  // Skip the rest of the loop to avoid drawing other content during splash
        } else {
            // Handle the splash screen blink effect
            if (currentTime - lastBlinkTime >= BLINK_INTERVAL && blinkCount < 6) {
                blinkState = !blinkState;  // Toggle blink state
                lastBlinkTime = currentTime;
                blinkCount++;  // Count the blinks (3 blinks = 6 state changes)

                // Clear display before drawing the splash screen
                display.clearDisplay();

                if (blinkState) {
                    SplashscreenUI();  // Show splash screen only when blinkState is true
                }

                display.display();  // Update display with current content
            }
            return;  // Skip further processing during splash screen
        }
    }

    // Regular operation after splash screen
    // Update GPS to fetch real-time clock (RTC)
    if (gps.newNMEAreceived()) {
        gps.parse(gps.lastNMEA());  // Parse new NMEA data
    }

    // Handle page switching
    if (currentTime - lastPageSwitchTime >= PAGE_SWITCH_DELAY) {
        currentPage = (currentPage + 1) % 5;  // Cycle through 5 pages (0 to 4)
        lastPageSwitchTime = currentTime;     // Reset the switch time
    }

    // Blinking logic for battery low warning
    if (currentTime - lastBlinkTime >= BLINK_INTERVAL) {
        blinkState = !blinkState;  // Toggle blink state
        lastBlinkTime = currentTime;
    }

    // Clear the display for each page
    display.clearDisplay();

    // Draw the top bar once
    Topbar(gpsTime(), carID, 100, blinkState);

    // Draw the center content based on the current page
    switch (currentPage) {
        case 1:
            SpeedAndDistanceUI("--.--", "--.--");
            break;
        case 2:
            StopwatchUI("00:03:59");
            break;
        case 3:
            LapTimingUI("00:00:00", "00:00:00");
            break;
        case 4:
            OverallResultsUI("--.--", "00:00:00", "--");
            break;
    }

    // Draw the bottom bar once
    Bottombar("incoming call");

    // Update the display with the new content
    display.display();

    delay(10);  // Small delay to avoid hogging CPU
}

// PAGE 0: Splash screen page (center content only)
void SplashscreenUI() {
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(SCREEN_WIDTH / 6, SCREEN_HEIGHT / 2 - 10);
    display.println("BLUEBAND  SPORTS");
}

// PAGE 1: Speed and Distance page (center content only)
void SpeedAndDistanceUI(String speed, String distance) {
    display.setCursor(5, 16);
    display.setTextSize(2);
    display.println("spd:" + (speed != "--.--" ? speed : "N/A"));
    display.setCursor(5, 36);
    display.println("dis:" + (distance != "--.--" ? distance : "N/A"));
}

// PAGE 2: Stopwatch and Elapsed Time page (center content only)
void StopwatchUI(String elapsedTime) {
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.println("Stopwatch");
    display.setTextSize(2);
    display.setCursor(0, 36);
    display.println((elapsedTime != "00:00:00" ? elapsedTime : "N/A"));
}

// PAGE 3: Start/End time record page (center content only)
void LapTimingUI(String startTime, String endTime) {
    display.setTextSize(2);
    display.setCursor(0, 16);
    display.println("ST: " + (startTime != "00:00:00" ? startTime : "N/A"));
    display.setCursor(0, 36);
    display.println("ET: " + (endTime != "00:00:00" ? endTime : "N/A"));
}

// PAGE 4: Overall results page (center content only)
void OverallResultsUI(String avgSpeed, String totalTime, String penalties) {
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.println("avg spd: " + (avgSpeed != "--.--" ? avgSpeed : "N/A") + " km/h");
    display.setCursor(0, 26);
    display.println("total time: " + (totalTime != "00:00:00" ? totalTime : "N/A"));
    display.setCursor(0, 36);
    display.println("penalties: " + (penalties != "--" ? penalties : "N/A"));
}

// Function to draw Battery Status in a box and blink "low" when below 10%
void drawBatteryStatus(int batteryPercentage) {
    display.setCursor(SCREEN_WIDTH - 25, 3);
    display.println(String(batteryPercentage) + "%");
    if (batteryPercentage < 10 && blinkState) {
        display.setCursor(SCREEN_WIDTH - 30, 10);
        display.println("LOW");
    }
}

// Function to draw a dashed line
void drawLine(int x0, int y0, int x1, int y1) {
    for (int x = x0; x < x1; x += 1) {
        display.drawPixel(x, y0, SSD1306_WHITE);
    }
}

// Function for Top Bar
void Topbar(const String& time, const String& carID, int batteryPercentage, bool blinkState) {
    display.setTextSize(1);

    // Display the time
    display.setCursor(0, 3);
    display.print(time);

    // Display Car ID with circle (toggle between filled and hollow)
    display.setCursor(SCREEN_WIDTH / 2 - 15, 3);
    display.print(carID);
    drawCircleWithToggle(SCREEN_WIDTH / 2 + 18, 5, blinkState);

    // Display battery status
    drawBatteryStatus(batteryPercentage);

    // Draw divider line
    drawLine(0, 12, SCREEN_WIDTH, 12);
}

// Function for Bottom Bar
void Bottombar(const String& message) {
    updateScrollingMessage(message);
}

// Scrolling message function
void updateScrollingMessage(const String& message) {
    drawLine(0, 52, SCREEN_WIDTH, 36); // Top divider line

    int messageWidth = message.length() * 6;  // Approximate message width
    display.setCursor(-scrollOffset, 55);
    display.setTextSize(1);
    display.print(message + " ");  // Append space for separation
    display.print(message);  // Repeat message to create loop

    scrollOffset++;
    if (scrollOffset > messageWidth) scrollOffset = 0;  // Reset scroll

    drawLine(0, 63, SCREEN_WIDTH, 36); // Bottom divider line
}

// Function to toggle between filled/hollow circle
void drawCircleWithToggle(int x, int y, bool isFilled) {
    if (isFilled) {
        display.fillCircle(x, y, 4, SSD1306_WHITE);  // Filled circle
    } else {
        display.drawCircle(x, y, 4, SSD1306_WHITE);  // Hollow circle
    }
}

// Function to fetch time from GPS (RTC)
String gpsTime() {
    if (gps.hour < 10) {
        return String("0") + gps.hour + ":" + (gps.minute < 10 ? "0" + String(gps.minute) : String(gps.minute));
    }
    return String(gps.hour) + ":" + (gps.minute < 10 ? "0" + String(gps.minute) : String(gps.minute));
}
