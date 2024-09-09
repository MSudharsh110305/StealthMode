#ifndef SENDDATA_H
#define SENDDATA_H

#include <Arduino.h>

bool sendHTTPRequest(String url, String jsonPayload, bool breath) {
    if (breath) {
        delay(50);  // Adjusted for minimal delay
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

#endif
