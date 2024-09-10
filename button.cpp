#include "button.h"

const int switch1Pin = 8;
const int switch2Pin = 9;

volatile bool sosPressed = false;
volatile bool okPressed = false;

void buttonSetup() {
    pinMode(switch1Pin, INPUT_PULLUP);  // Enable internal pull-up resistor
    pinMode(switch2Pin, INPUT_PULLUP);  // Enable internal pull-up resistor

    attachInterrupt(digitalPinToInterrupt(switch1Pin), handleSosPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(switch2Pin), handleOkPress, FALLING);
}

void handleSosPress() {
    delay(50);  // Simple debounce
    if (digitalRead(switch1Pin) == LOW) {  // Check if the button is still pressed
        sosPressed = true;
    }
}

void handleOkPress() {
    delay(50);  // Simple debounce
    if (digitalRead(switch2Pin) == LOW) {  // Check if the button is still pressed
        okPressed = true;
    }
}
