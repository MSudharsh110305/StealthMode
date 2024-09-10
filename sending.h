#ifndef SENDING_H
#define SENDING_H

#include <Arduino.h>

extern const bool DEBUG;  // Declare DEBUG as extern

String sendData(String command, const int timeout, boolean debug = false);
bool sendHTTPRequest(String url, String jsonPayload, bool breath);

#endif
