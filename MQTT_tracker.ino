#define TINY_GSM_MODEM_SIM7600

#include <TinyGsmClient.h>
#include <PubSubClient.h>

#define DEBUG false  // Declaring DEBUG constant

#define LTE_PWRKEY_PIN 5
#define LTE_RESET_PIN 6
#define LTE_FLIGHT_PIN 7

 const int switch1Pin = 8;
 const int switch2Pin = 9;

 volatile bool sosPressed = false;
 volatile bool okPressed = false;
 volatile bool sendingMessage = false;

 unsigned long lastTrackSendTime = 0;
 const unsigned long trackSendInterval = 2000; // 2  seconds

 int gpsSendAttempts = 0;
 const int maxAttempts = 25;

// Set up APN credentials
const char apn[] = "bsnlnet";  
const char gprsUser[] = "";    
const char gprsPass[] = "";     
const char broker[] = "35.200.163.26";  
const int brokerPort = 1883;    

TinyGsm modem(Serial1);
TinyGsmClient gsmClient(modem);  // GSM Client for MQTT
PubSubClient mqtt(gsmClient);  // Initialize MQTT client

// Define the sendData function
String sendData(String command, const int timeout, boolean debug = false) {
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

// Define the extractNMEA function
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

// Function to send track data
void sendTrackData() {
    String gpsInfo = sendData("AT+CGPSINFO", 1300, DEBUG);
    String nmeaSentence = extractNMEA(gpsInfo);
    SerialUSB.println(nmeaSentence + " nmea");

    if (nmeaSentence.length() > 8) {
        SerialUSB.println("Sending Track data");

        String jsonPayload = "{\"nmea\": \"" + nmeaSentence + "\",\"carId\":2}";

        // Publish the data to the MQTT broker
        mqtt.publish("blueband/car2/data", jsonPayload.c_str());
    }
}

// Function to connect to the cellular network and initialize modem
void setupModem() {
    SerialUSB.println("Initializing modem...");
    
    if (!modem.restart()) {
        SerialUSB.println("Failed to restart modem.");
        while (true);  // Halt execution
    }

    SerialUSB.println("Modem restarted.");
    
    modem.gprsConnect(apn);
    if (!modem.isGprsConnected()) {
        SerialUSB.println("GPRS not connected.");
        return;
    }

    SerialUSB.println("GPRS connected.");
}


// Function to connect to MQTT broker
void connectToMQTT() {
    while (!mqtt.connected()) {
        SerialUSB.print("Connecting to MQTT...");
        if (mqtt.connect("CarDevice_2")) {
            SerialUSB.println("connected");

            mqtt.subscribe("blueband/car2/status");
        } else {
            SerialUSB.print("failed, rc=");
            SerialUSB.print(mqtt.state());
            SerialUSB.println(" try again in 4 seconds");
            delay(4000);
        }
    }
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

void setup() {
    SerialUSB.begin(115200);
    Serial1.begin(115200);

    pinMode(LTE_RESET_PIN, OUTPUT);
    digitalWrite(LTE_RESET_PIN, LOW);

    pinMode(LTE_PWRKEY_PIN, OUTPUT);
    digitalWrite(LTE_PWRKEY_PIN, LOW);
    delay(100);
    digitalWrite(LTE_PWRKEY_PIN, HIGH);
    delay(2000);
    digitalWrite(LTE_PWRKEY_PIN, LOW);

    pinMode(LTE_FLIGHT_PIN, OUTPUT);
    digitalWrite(LTE_FLIGHT_PIN, LOW);

    pinMode(switch1Pin, INPUT_PULLUP);  // Enable internal pull-up resistor
    pinMode(switch2Pin, INPUT_PULLUP);  // Enable internal pull-up resistor

    attachInterrupt(digitalPinToInterrupt(switch1Pin), handleSosPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(switch2Pin), handleOkPress, FALLING);

    while (!SerialUSB) {
        delay(10);
    }

    sosPressed = false;
    okPressed = false;

    SerialUSB.println("Switch test initialized.");

    setupModem();  // Initialize modem and connect to network

    mqtt.setServer(broker, brokerPort);  // Set the MQTT broker server
    connectToMQTT();
}

void loop() {
    if (!mqtt.connected()) {
        connectToMQTT();
    }
    mqtt.loop();

    if (sosPressed) {
        sendingMessage = true;
        sosPressed = false;
        int max_sos_attempt = 0;
        while (max_sos_attempt++ < 25) {
            String payload = "{\"carId\":2, \"message\": \"SOS\"}";
            if (mqtt.publish("blueband/car2/sos", payload.c_str())) {
                SerialUSB.println("SOS DONE");
                break;
            }
            //SerialUSB.println("SOS NOT DONE");
        }
        sendingMessage = false;
    }

    if (okPressed) {
        sendingMessage = true;
        okPressed = false;
        int max_ok_attempt = 0;
        while (max_ok_attempt++ < 25) {
            String payload = "{\"carId\":2, \"message\": \"OK\"}";
            if (mqtt.publish("blueband/car2/ok", payload.c_str())) {
                SerialUSB.println("OK DONE");
                break;
            }
            //SerialUSB.println("OK NOT DONE");
        }
        sendingMessage = false;
    }

    if (!sendingMessage) {
        sendTrackData();  // Attempt to send track data only if no SOS or OK is being sent
    }

    delay(trackSendInterval); // Add a delay to prevent constant looping

}