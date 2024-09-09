#include "CallManager.h"
#include "Stopwatch.h"
#include "SendData.h"
#include "Tracking.h"

#define LTE_PWRKEY_PIN 5
#define LTE_RESET_PIN 6
#define LTE_FLIGHT_PIN 7

const int switch1Pin = 8;
volatile bool sosPressed = false;
volatile bool okPressed = false;
volatile bool sendingMessage = false;

unsigned long lastTrackSendTime = 0;
const unsigned long trackSendInterval = 500;  // 0.5 seconds

void handleSosPress() {
    delay(50);  // Debounce
    if (digitalRead(switch1Pin) == LOW) {
        sosPressed = true;
    }
}

void handleOkPress() {
    delay(50);  // Debounce
    if (digitalRead(switch2Pin) == LOW) {
        okPressed = true;
    }
}

void setup() {
    SerialUSB.begin(115200);
    Serial1.begin(115200);

    pinMode(LTE_PWRKEY_PIN, OUTPUT);
    pinMode(LTE_RESET_PIN, OUTPUT);
    pinMode(LTE_FLIGHT_PIN, OUTPUT);
    pinMode(switch1Pin, INPUT_PULLUP);
    pinMode(switch2Pin, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(switch1Pin), handleSosPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(switch2Pin), handleOkPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(stopwatchButtonPin), handleStopwatchPress, FALLING);

    delay(1000);  // Give modules time to initialize
    sendData("AT+CMGF=1", 1000, DEBUG);  // Initialize GSM module
    sendData("AT+CNMI=2,2,0,0,0", 1000, DEBUG);  // Set SMS to show up on serial

    sendData("AT+CLIP=1", 1000, DEBUG);  // Enable caller ID
    sendData("AT+CGNSPWR=1", 1000, DEBUG);  // Enable GPS
}

void loop() {
    handleIncomingCalls();  
    manageStopwatch();

    if (millis() - lastTrackSendTime >= trackSendInterval) {
        lastTrackSendTime = millis();
        sendTrackData(); 
    }

    if (sosPressed && !sendingMessage) {
        sendingMessage = true;
        SerialUSB.println("SOS Button Pressed. Sending data...");

        String jsonPayload = "{\"event\": \"sosButtonPressed\", \"carId\": 2}";
        String url = "<YOUR_SOS_ENDPOINT>";
        if (sendHTTPRequest(url, jsonPayload, true)) {
            sosPressed = false;
        }
        sendingMessage = false;
    }

    if (okPressed && !sendingMessage) {
        sendingMessage = true;
        SerialUSB.println("OK Button Pressed. Sending data...");

        String jsonPayload = "{\"event\": \"okButtonPressed\", \"carId\": 2}";
        String url = "<YOUR_OK_ENDPOINT>";
        if (sendHTTPRequest(url, jsonPayload, true)) {
            okPressed = false;
        }
        sendingMessage = false;
    }
}
