#define DEBUG false

#define LTE_PWRKEY_PIN 5
#define LTE_RESET_PIN 6
#define LTE_FLIGHT_PIN 7

const String allowedNumbers[] = {"+918637447158", "+919944546840"};
const int allowedNumbersCount = 2;

const int switch1Pin = 8;
const int switch2Pin = 9;
const int stopwatchButtonPin = 2;

volatile bool sosPressed = false;
volatile bool okPressed = false;
volatile bool stopwatchPressed = false;

bool stopwatchRunning = false;
unsigned long stopwatchStartTime = 0;
unsigned long stopwatchElapsedTime = 0;
unsigned long lastUpdate = 0;

unsigned long lastTrackSendTime = 0;
const unsigned long trackSendInterval = 500; // 0.5 seconds

#define PHONE_NUMBER "+919944546840"

String sendData(String command, const int timeout, boolean debug = false) {
    String response = "";
    Serial1.println(command);

    long int startTime = millis();
    while (millis() - startTime < timeout) {
        while (Serial1.available()) {
            char c = Serial1.read();
            response += c;
        }
    }

    if (debug) {
        SerialUSB.print(command);
        SerialUSB.print(" Response: ");
        SerialUSB.println(response);
    }

    return response;
}

String extractNMEA(String response) {
    int start = response.indexOf("+CGPSINFO: ");
    if (start != -1) {
        start += 11;
        int end = response.indexOf("OK", start);
        if (end != -1 && end > start) {
            String nmea = response.substring(start, end);
            nmea.trim();
            return nmea;
        }
    }
    return "";
}

bool sendHTTPRequest(String url, String jsonPayload, bool breath) {
    if (breath) {
        delay(50);
    }
    SerialUSB.println("Sending HTTP request to " + url);

    sendData("AT+HTTPINIT", 100, DEBUG);
    sendData("AT+HTTPPARA=\"CID\",1", 100, DEBUG);
    sendData("AT+HTTPPARA=\"URL\",\"" + url + "\"", 100, DEBUG);
    sendData("AT+HTTPPARA=\"CONTENT\",\"application/json\"", 100, DEBUG);
    sendData("AT+HTTPDATA=" + String(jsonPayload.length()) + ",2000", 100);
    sendData(jsonPayload, 100, DEBUG);
    String response = sendData("AT+HTTPACTION=1", 2000, DEBUG);
    sendData("AT+HTTPTERM", 100, DEBUG);

    if (breath) {
        if (response.indexOf("200") != -1) {
            SerialUSB.println("HTTP request sent successfully.");
            return false;
        } else {
            SerialUSB.println("Failed to send HTTP request.");
            return true;
        }
    } else {
        return true;
    }
}

void sendTrackData() {
    String gpsInfo = sendData("AT+CGPSINFO", 1300, DEBUG);
    String nmeaSentence = extractNMEA(gpsInfo);
    SerialUSB.println(nmeaSentence + " nmea");

    if (nmeaSentence.length() > 8) {
        SerialUSB.println("Sending Track data");
        String jsonPayload = "{\"nmea\": \"" + nmeaSentence + "\",\"carId\":2}";
        String requestBinURL = "https://blueband-speed-zr7gm6w4cq-el.a.run.app/track";
        sendHTTPRequest(requestBinURL, jsonPayload, false);
    }
}

void handleSosPress() {
    delay(50);
    if (digitalRead(switch1Pin) == LOW) {
        sosPressed = true;
    }
}

void handleOkPress() {
    delay(50);
    if (digitalRead(switch2Pin) == LOW) {
        okPressed = true;
    }
}

void handleStopwatchPress() {
    delay(50);
    if (digitalRead(stopwatchButtonPin) == LOW) {
        stopwatchPressed = true;
    }
}

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

    pinMode(switch1Pin, INPUT_PULLUP);
    pinMode(switch2Pin, INPUT_PULLUP);
    pinMode(stopwatchButtonPin, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(switch1Pin), handleSosPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(switch2Pin), handleOkPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(stopwatchButtonPin), handleStopwatchPress, FALLING);

    SerialUSB.println("Initializing...");
    sendData("AT+CLIP=1", 2000, DEBUG);
    sendData("AT+CGATT=0", 2000, DEBUG);
    sendData("AT+CGATT=1", 2000, DEBUG);
    sendData("AT+CGACT=1,1", 2000, DEBUG);
    String response = sendData("AT+CGPADDR=1", 1300, DEBUG);
    if (response.indexOf("OK") != -1 && response.indexOf(".") != -1) {
        SerialUSB.println("Internet connected.");
    } else {
        SerialUSB.println("Internet not connected.");
    }
    sendData("AT+CGPS=0", 3000, DEBUG);
    sendData("AT+CGPS=1", 3000, DEBUG);
}


void loop() {
    // Handle incoming calls (prioritized)
    if (Serial1.available()) {
        String incomingData = Serial1.readString();
        if (incomingData.indexOf("RING") != -1) {
            handleIncomingCall();
        }
    }

    // Handle SOS button
    if (sosPressed) {
        sosPressed = false;
        SerialUSB.println("SOS button pressed. Sending SOS message...");
        if (sendHTTPRequest("https://blueband-speed-zr7gm6w4cq-el.a.run.app/sos", "{\"carId\":2, \"message\": \"SOS\"}", true)) {
            SerialUSB.println("SOS message sent successfully.");
            makeVoiceCall(PHONE_NUMBER);
        } else {
            SerialUSB.println("Failed to send SOS message.");
        }
    }

    // Handle OK button
    if (okPressed) {
        okPressed = false;
        SerialUSB.println("OK button pressed. Sending OK message...");
        if (sendHTTPRequest("https://blueband-speed-zr7gm6w4cq-el.a.run.app/ok", "{\"carId\":2, \"message\": \"OK\"}", true)) {
            SerialUSB.println("OK message sent successfully.");
        } else {
            SerialUSB.println("Failed to send OK message.");
        }
    }

    // Handle stopwatch
    handleStopwatch();

    // Send track data
    unsigned long currentMillis = millis();
    if (currentMillis - lastTrackSendTime >= trackSendInterval) {
        lastTrackSendTime = currentMillis;
        sendTrackData();
    }

    delay(50);  // Small delay to avoid continuous looping
}

void handleIncomingCall() {
    SerialUSB.println("Incoming call detected.");
    String callerID = sendData("AT+CLCC", 2000, DEBUG);

    int start = callerID.indexOf('"');
    int end = callerID.indexOf('"', start + 1);
    if (start != -1 && end != -1) {
        String callerNumber = callerID.substring(start + 1, end);
        SerialUSB.println("Incoming call from: " + callerNumber);

        bool allowed = false;
        for (int i = 0; i < allowedNumbersCount; i++) {
            if (callerNumber == allowedNumbers[i]) {
                allowed = true;
                break;
            }
        }

        if (allowed) {
            SerialUSB.println("Number is allowed. Answering call...");
            answerCall();
        } else {
            SerialUSB.println("Number is not allowed. Hanging up...");
            hangUpRestrictedCall();
        }
    } else {
        SerialUSB.println("Unable to get caller ID.");
    }
}

void handleStopwatch() {
    if (stopwatchPressed) {
        stopwatchPressed = false;
        if (!stopwatchRunning) {
            stopwatchStartTime = millis();
            stopwatchRunning = true;
            SerialUSB.println("Stopwatch started");
        } else {
            stopwatchElapsedTime = millis() - stopwatchStartTime;
            stopwatchRunning = false;
            printStopwatchTime(stopwatchElapsedTime);
        }
    }

    if (stopwatchRunning) {
        unsigned long currentMillis = millis();
        stopwatchElapsedTime = currentMillis - stopwatchStartTime;

        if (currentMillis - lastUpdate >= 100) {
            lastUpdate = currentMillis;
            printStopwatchTime(stopwatchElapsedTime);
        }
    }
}

void makeVoiceCall(String phoneNumber) {
    SerialUSB.println("Dialing number: " + phoneNumber);
    String response = sendData("ATD" + phoneNumber + ";", 10000, DEBUG);
    if (response.indexOf("OK") != -1 || response.indexOf("CONNECT") != -1) {
        SerialUSB.println("Call initiated successfully.");
        sendData("AT+CSDVC=3", 2000, DEBUG);
        sendData("AT+CLVL=5", 2000, DEBUG);
        monitorCall();
    } else {
        SerialUSB.println("Failed to initiate call. Response: " + response);
    }
}

void monitorCall() {
    SerialUSB.println("Monitoring call status...");
    bool callEnded = false;
    while (!callEnded) {
        String response = sendData("AT+CLCC", 1300, DEBUG);
        if (response.indexOf("NO CARRIER") != -1) {
            SerialUSB.println("Call ended.");
            callEnded = true;
        } else if (response.indexOf("ERROR") != -1) {
            SerialUSB.println("Error while monitoring call status.");
            callEnded = true;
        } else {
            SerialUSB.println("Waiting for call end status...");
        }
        delay(500);
    }
    String hangupResponse = sendData("ATH", 1300, DEBUG);
    if (hangupResponse.indexOf("OK") != -1) {
        SerialUSB.println("Call hung up successfully.");
    } else {
        SerialUSB.println("Failed to hang up call. Response: " + hangupResponse);
    }
}

void answerCall() {
    String response = sendData("ATA", 5000, DEBUG);
    if (response.indexOf("OK") != -1 || response.indexOf("CONNECT") != -1) {
        SerialUSB.println("Call answered successfully.");
        sendData("AT+CSDVC=3", 2000, DEBUG);
        sendData("AT+CLVL=2", 2000, DEBUG);
    } else {
        SerialUSB.println("Failed to answer the call.");
    }
}

void hangUpRestrictedCall() {
    String response = sendData("ATH", 5000, DEBUG);
    delay(200);
}

void printStopwatchTime(unsigned long elapsedTime) {
    unsigned long totalSeconds = elapsedTime / 1000;
    unsigned long hours = totalSeconds / 3600;
    unsigned long minutes = (totalSeconds % 3600) / 60;
    unsigned long seconds = totalSeconds % 60;
    unsigned long milliseconds = elapsedTime % 1000;

    SerialUSB.print(hours);
    SerialUSB.print(":");
    if (minutes < 10) SerialUSB.print("0");
    SerialUSB.print(minutes);
    SerialUSB.print(":");
    if (seconds < 10) SerialUSB.print("0");
    SerialUSB.print(seconds);
    SerialUSB.print(".");
    if (milliseconds < 100) SerialUSB.print("0");
    if (milliseconds < 10) SerialUSB.print("0");
    SerialUSB.println(milliseconds);
}