#include "tracking.h"

String extractNMEA(String response) {
    int start = response.indexOf("+CGPSINFO: ");
    if (start != -1) {
        start += 11; // Move past "+CGPSINFO: "
        int end = response.indexOf("OK", start);
        if (end != -1 && end > start) {
            String nmea = response.substring(start, end);
            nmea.trim(); // Trim any leading/trailing whitespace
            return nmea;
        }
    }
    return ""; // Return empty string if NMEA sentence not found or invalid format
}

void sendTrackData() {
    String gpsInfo = sendData("AT+CGPSINFO", 1300, DEBUG);
    String nmeaSentence = extractNMEA(gpsInfo);
    SerialUSB.println(nmeaSentence + " nmea");

    if (nmeaSentence.length() > 8) {
        SerialUSB.println("Sending Track data");

        String jsonPayload = "{\"nmea\": \"" + nmeaSentence + "\",\"carId\":2}";
        String trackURL = "https://blueband-speed-zr7gm6w4cq-el.a.run.app/track";
        if (sendHTTPRequest(trackURL, jsonPayload, false)) {
            // Retry logic if needed
        }
    }
}
