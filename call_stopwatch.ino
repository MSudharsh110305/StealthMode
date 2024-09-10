#define DEBUG false

#define LTE_PWRKEY_PIN 5
#define LTE_RESET_PIN 6
#define LTE_FLIGHT_PIN 7

const String allowedNumbers[] = {"+918637447158", "+919944546840"};
const int allowedNumbersCount = 2;

const int switch1Pin = 8;
const int stopwatchButtonPin = 10;  // Button for stopwatch
volatile bool stopwatchPressed = false;  // Track stopwatch button press
bool stopwatchRunning = false;           // Track if stopwatch is running
unsigned long stopwatchStartTime = 0;
unsigned long stopwatchElapsedTime = 0;
unsigned long lastUpdate = 0;  // To limit the update rate of the Serial Monitor

#define PHONE_NUMBER "9944546840"

// New function to handle stopwatch button press
void handleStopwatchPress() {
    delay(50);  // Debounce
    if (digitalRead(stopwatchButtonPin) == LOW) {
        stopwatchPressed = true;
    }
}

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

    pinMode(switch1Pin, INPUT_PULLUP);  // Enable internal pull-up resistor
    pinMode(stopwatchButtonPin, INPUT_PULLUP);  
    attachInterrupt(digitalPinToInterrupt(stopwatchButtonPin), handleStopwatchPress, FALLING);  // Interrupt for stopwatch button

    SerialUSB.println("Initializing...");
    sendData("AT+CLIP=1", 2000, DEBUG);  // Enable Caller ID notifications
    SerialUSB.println("Waiting for incoming calls...");
}

void loop() {
    // Handle incoming calls
    if (Serial1.available()) {
        String incomingData = Serial1.readString();
        if (incomingData.indexOf("RING") != -1) {
            SerialUSB.println("Incoming call detected.");
            String callerID = sendData("AT+CLCC", 2000, DEBUG);

            // Extract and display the caller ID
            int start = callerID.indexOf('"');
            int end = callerID.indexOf('"', start + 1);
            if (start != -1 && end != -1) {
                String callerNumber = callerID.substring(start + 1, end);
                SerialUSB.println("Incoming call from: " + callerNumber);

                // Check if the number is allowed
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
                    hangUpRestrictedCall();  // Hang up restricted numbers
                }
            } else {
                SerialUSB.println("Unable to get caller ID.");
            }
        }
    }

    // Handle outgoing call
    if (digitalRead(switch1Pin) == LOW) {
        SerialUSB.println("Switch pressed. Making call...");
        delay(300);  // Debounce the switch
        makeVoiceCall(PHONE_NUMBER);  // Make the call
    }

    // Handle stopwatch logic
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
    delay(100);  // Small delay to avoid continuously reading the switch
}

// Function to make a voice call
void makeVoiceCall(String phoneNumber) {
    SerialUSB.println("Dialing number: " + phoneNumber);
  
    // Dial the number using ATD command
    String response = sendData("ATD" + phoneNumber + ";", 10000, DEBUG);
  
    // Check if call is initiated (look for "OK" or "CONNECT")
    if (response.indexOf("OK") != -1 || response.indexOf("CONNECT") != -1) {
        SerialUSB.println("Call initiated successfully.");
        
        // Set speaker mode and volume
        sendData("AT+CSDVC=3", 2000, DEBUG);  // 3 for Speakerphone
        sendData("AT+CLVL=2", 2000, DEBUG);   // Set loudspeaker volume level to 2 (range: 0-5)
        
        monitorCall();
    } else {
        SerialUSB.println("Failed to initiate call. Response: " + response);
    }
}

// Function to monitor the call
void monitorCall() {
    SerialUSB.println("Monitoring call status...");

    bool callEnded = false;
    
    while (!callEnded) {
        // Read response from AT+CLCC command
        String response = sendData("AT+CLCC", 1300, DEBUG);  // Check call status
        
        // Check for the call ended status
        if (response.indexOf("NO CARRIER") != -1) {
            SerialUSB.println("Call ended.");
            callEnded = true;  // Exit the loop once the call has ended
        } else if (response.indexOf("ERROR") != -1) {
            SerialUSB.println("Error while monitoring call status.");
            callEnded = true;  // Exit the loop on error
        } else {
            SerialUSB.println("Waiting for call end status...");
        }

        delay(500);  // Check every half second
    }

    // Hang up the call
    String hangupResponse = sendData("ATH", 1300, DEBUG);
    if (hangupResponse.indexOf("OK") != -1) {
        SerialUSB.println("Call hung up successfully.");
    } else {
        SerialUSB.println("Failed to hang up call. Response: " + hangupResponse);
    }
}

// Function to answer the call
void answerCall() {
    String response = sendData("ATA", 5000, DEBUG);  // Answer the call
    if (response.indexOf("OK") != -1 || response.indexOf("CONNECT") != -1) {
        SerialUSB.println("Call answered successfully.");
        sendData("AT+CSDVC=3", 2000, DEBUG);  // Switch to speakerphone
        sendData("AT+CLVL=2", 2000, DEBUG);   // Set loudspeaker volume
    } else {
        SerialUSB.println("Failed to answer the call.");
    }
}

// Function to hang up a restricted call
void hangUpRestrictedCall() {
    String response = sendData("ATH", 5000, DEBUG);  // Hang up the call
    delay(200);  // Small delay after sending ATH
}