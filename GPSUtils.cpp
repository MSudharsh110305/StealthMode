#include "GPSUtils.h"

void setupGPS(Adafruit_GPS& gps) {
    gps.begin(9600);
}

String getGPSTime(Adafruit_GPS& gps) {
    if (gps.newNMEAreceived()) {
        gps.parse(gps.lastNMEA());
    }

    if (gps.hour < 10) {
        return String("0") + gps.hour + ":" + (gps.minute < 10 ? "0" + String(gps.minute) : String(gps.minute));
    }
    return String(gps.hour) + ":" + (gps.minute < 10 ? "0" + String(gps.minute) : String(gps.minute));
}
