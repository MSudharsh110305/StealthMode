#ifndef TRACKING_H
#define TRACKING_H

#include <Arduino.h>
#include "sending.h"

extern volatile bool sendingMessage;  // Changed to 'volatile'
extern const bool DEBUG;  // Declare DEBUG as extern

String extractNMEA(String response);

void sendTrackData();

#endif
