#ifndef TRACKING_H
#define TRACKING_H

#include <Arduino.h>

String extractNMEA(String response) {
    int start = response.indexOf("+CGPSINFO: ");
    if (start != -1) {
        start += 11; // Move past "+CGPSINFO: "
        int end = response.indexOf("OK", start);
        if (end != -1 && end > start) {
            String nmea = response.substring(start, end);
            nmea.trim();
            return nmea;
        }
    }
    return "";
}

void sendTrackData() {
    String gpsInfo = sendData("AT+CGPSINFO", 1300, DEBUG);
    String nmeaSentence = extractNMEA(gpsInfo);
    SerialUSB.println(nmeaSentence + " nmea");

    if (nmeaSentence.length() > 8) {
        SerialUSB.println("Sending Track data");

        String jsonPayload = "{\"nmea\": \"" + nmeaSentence + "\",\"carId\":2}";
        String requestBinURL = "<YOUR_TRACK_ENDPOINT>";
        if (sendHTTPRequest(requestBinURL, jsonPayload, false)) {
            // Retry logic if needed
        }
    }
}

#endif
