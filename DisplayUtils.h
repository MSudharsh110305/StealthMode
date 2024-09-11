#ifndef DISPLAYUTILS_H
#define DISPLAYUTILS_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GPS.h>

// Function prototypes for display handling
void initDisplay();
void renderPage(int currentPage, String gpsTime, Adafruit_GPS gps);
void drawMainUI(String time, String carID, int batteryPercentage, String speed, String distance, String message);
void drawStopwatchAndElapsedUI(String time, String carID, int batteryPercentage, String stopwatchTime, String elapsedTime, String message);
void drawStartEndTimePage(String time, String carID, int batteryPercentage, String startTime, String endTime);
void drawOverallResultsPage(String time, String carID, int batteryPercentage, String avgSpeed, String totalTime, String penalties);
void drawBatteryStatus(int batteryPercentage);
void drawLine(int x0, int y0, int x1, int y1);
void drawCircleWithToggle(int x, int y, bool isFilled);
void drawStartupPage();
String gpsTime(Adafruit_GPS gps);

#endif
