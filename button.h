#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

extern volatile bool sosPressed;
extern volatile bool okPressed;

void buttonSetup();
void handleSosPress();
void handleOkPress();

#endif
