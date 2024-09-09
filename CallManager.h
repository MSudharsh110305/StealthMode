#ifndef CALLMANAGER_H
#define CALLMANAGER_H

#include <Arduino.h>

const String allowedNumbers[] = {"+918637447158", "+919944546840"};
const int allowedNumbersCount = 2;
#define PHONE_NUMBER "9944546840"

String sendData(String command, const int timeout, boolean debug = false);

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

void hangUpRestrictedCall() {
    String response = sendData("ATH", 5000, DEBUG);  // Hang up the call
    delay(200);  // Small delay after sending ATH
}

void makeVoiceCall(String phoneNumber) {
    SerialUSB.println("Dialing number: " + phoneNumber);
    String response = sendData("ATD" + phoneNumber + ";", 10000, DEBUG);
  
    if (response.indexOf("OK") != -1 || response.indexOf("CONNECT") != -1) {
        SerialUSB.println("Call initiated successfully.");
        sendData("AT+CSDVC=3", 2000, DEBUG);  // Speakerphone
        sendData("AT+CLVL=2", 2000, DEBUG);   // Set loudspeaker volume to 2
    } else {
        SerialUSB.println("Failed to initiate call. Response: " + response);
    }
}

void handleIncomingCalls() {
    if (Serial1.available()) {
        String incomingData = Serial1.readString();
        if (incomingData.indexOf("RING") != -1) {
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
            }
        }
    }
}

#endif
