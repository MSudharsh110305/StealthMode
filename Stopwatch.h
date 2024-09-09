#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <Arduino.h>

const int stopwatchButtonPin = 10;  
volatile bool stopwatchPressed = false;  
bool stopwatchRunning = false;
unsigned long stopwatchStartTime = 0;
unsigned long stopwatchElapsedTime = 0;
unsigned long lastUpdate = 0;

void handleStopwatchPress() {
    delay(50);  // Debounce
    if (digitalRead(stopwatchButtonPin) == LOW) {
        stopwatchPressed = true;
    }
}

void manageStopwatch() {
    if (stopwatchPressed) {
        stopwatchPressed = false;
        if (!stopwatchRunning) {
            stopwatchStartTime = millis();
            stopwatchRunning = true;
            SerialUSB.println("Stopwatch started");
        } else {
            stopwatchElapsedTime = millis() - stopwatchStartTime;
            stopwatchRunning = false;
            unsigned long totalSeconds = stopwatchElapsedTime / 1000;
            unsigned long hours = totalSeconds / 3600;
            unsigned long minutes = (totalSeconds % 3600) / 60;
            unsigned long seconds = totalSeconds % 60;
            unsigned long milliseconds = stopwatchElapsedTime % 1000;

            SerialUSB.print("Stopwatch stopped. Time: ");
            SerialUSB.print(hours);
            SerialUSB.print("h ");
            SerialUSB.print(minutes);
            SerialUSB.print("m ");
            SerialUSB.print(seconds);
            SerialUSB.print("s ");
            SerialUSB.print(milliseconds);
            SerialUSB.println("ms");
        }
    }

    if (stopwatchRunning) {
        unsigned long currentMillis = millis();
        stopwatchElapsedTime = currentMillis - stopwatchStartTime;

        if (currentMillis - lastUpdate >= 100) {
            lastUpdate = currentMillis;
            unsigned long totalSeconds = stopwatchElapsedTime / 1000;
            unsigned long hours = totalSeconds / 3600;
            unsigned long minutes = (totalSeconds % 3600) / 60;
            unsigned long seconds = totalSeconds % 60;
            unsigned long milliseconds = stopwatchElapsedTime % 1000;

            SerialUSB.print("Running time: ");
            SerialUSB.print(hours);
            SerialUSB.print("h ");
            SerialUSB.print(minutes);
            SerialUSB.print("m ");
            SerialUSB.print(seconds);
            SerialUSB.print("s ");
            SerialUSB.print(milliseconds);
            SerialUSB.println("ms");
        }
    }
}

#endif
