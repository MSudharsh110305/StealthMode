#include "sending.h"

String sendData(String command, const int timeout, boolean debug) {
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
