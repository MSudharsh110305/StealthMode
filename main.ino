#include "tracking.h"
#include "button.h"
#include "sending.h"

#define LTE_PWRKEY_PIN 5
#define LTE_RESET_PIN 6
#define LTE_FLIGHT_PIN 7

const bool DEBUG = false;  // Define DEBUG as a constant
volatile bool sendingMessage = false;

const unsigned long trackSendInterval = 500; // 0.5 seconds

void setup() {
    SerialUSB.begin(115200);
    Serial1.begin(115200);

    pinMode(LTE_RESET_PIN, OUTPUT);
    digitalWrite(LTE_RESET_PIN, LOW);

    pinMode(LTE_PWRKEY_PIN, OUTPUT);
    digitalWrite(LTE_PWRKEY_PIN, LOW);
    delay(100);
    digitalWrite(LTE_PWRKEY_PIN, HIGH);
    delay(2000);
    digitalWrite(LTE_PWRKEY_PIN, LOW);

    pinMode(LTE_FLIGHT_PIN, OUTPUT);
    digitalWrite(LTE_FLIGHT_PIN, LOW);

    buttonSetup();  // Initialize buttons and interrupts

    while (!SerialUSB) {
        delay(10);
    }

    SerialUSB.println("Switch test initialized.");
    String response = sendData("AT+CGATT=0", 2000, DEBUG);
    response = sendData("AT+CGATT=1", 2000, DEBUG);
    response = sendData("AT+CGACT=1,1", 2000, DEBUG);
    response = sendData("AT+CGPADDR=1", 1300, DEBUG);
    
    if (response.indexOf("OK") != -1 && response.indexOf(".") != -1) {
        SerialUSB.println("Internet connected.");
    } else {
        SerialUSB.println("Internet not connected.");
    }

    response = sendData("AT+CGPS=0", 3000, DEBUG);
    response = sendData("AT+CGPS=1", 3000, DEBUG);
}

void loop() {
    if (sosPressed) {
        sendingMessage = true;
        sosPressed = false;
        int max_sos_attempt = 0;
        while (max_sos_attempt++ < 25) {
            if (!sendHTTPRequest("https://blueband-speed-zr7gm6w4cq-el.a.run.app/sos", "{\"carId\":2, \"message\": \"SOS\"}", true)) {
                SerialUSB.println("SOS DONE");
                break;
            }
            SerialUSB.println("SOS NOT DONE");
        }
        sendingMessage = false;
    }

    if (okPressed) {
        sendingMessage = true;
        okPressed = false;
        int max_ok_attempt = 0;
        while (max_ok_attempt++ < 25) {
            if (!sendHTTPRequest("https://blueband-speed-zr7gm6w4cq-el.a.run.app/ok", "{\"carId\":2, \"message\": \"OK\"}", true)) {
                SerialUSB.println("OK DONE");
                break;
            }
            SerialUSB.println("OK NOT DONE");
        }
        sendingMessage = false;
    }

    if (!sendingMessage) {
        sendTrackData();  // Attempt to send track data only if no SOS or OK is being sent
    }

    delay(trackSendInterval); // Add a delay to prevent constant looping
}
