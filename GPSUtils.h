#ifndef GPSUTILS_H
#define GPSUTILS_H

#include <Adafruit_GPS.h>

// Function prototypes for GPS utilities
void setupGPS(Adafruit_GPS& gps);
String getGPSTime(Adafruit_GPS& gps);

#endif
