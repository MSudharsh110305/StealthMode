/**********************************************************************************************************************************************************************************************************************************************************************************************
____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________

This files is to study about each snippets used in the other codes, inorder to make final code, this is done to integrate evryyother code


 ______   _                 _______  ______   _______  _        ______     _______  _______  _______  _______ _________ _______ 
(  ___ \ ( \      |\     /|(  ____ \(  ___ \ (  ___  )( (    /|(  __  \   (  ____ \(  ____ )(  ___  )(  ____ )\__   __/(  ____ \
| (   ) )| (      | )   ( || (    \/| (   ) )| (   ) ||  \  ( || (  \  )  | (    \/| (    )|| (   ) || (    )|   ) (   | (    \/
| (__/ / | |      | |   | || (__    | (__/ / | (___) ||   \ | || |   ) |  | (_____ | (____)|| |   | || (____)|   | |   | (_____ 
|  __ (  | |      | |   | ||  __)   |  __ (  |  ___  || (\ \) || |   | |  (_____  )|  _____)| |   | ||     __)   | |   (_____  )
| (  \ \ | |      | |   | || (      | (  \ \ | (   ) || | \   || |   ) |        ) || (      | |   | || (\ (      | |         ) |
| )___) )| (____/\| (___) || (____/\| )___) )| )   ( || )  \  || (__/  )  /\____) || )      | (___) || ) \ \__   | |   /\____) |
|/ \___/ (_______/(_______)(_______/|/ \___/ |/     \||/    )_)(______/   \_______)|/       (_______)|/   \__/   )_(   \_______)
                                                                                                                                


_____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
**********************************************************************************************************************************************************************************************************************************************************************************************/


// debug
#define DEBUG false  

//LTE SIM7600E-H
#define LTE_PWRKEY_PIN 5
#define LTE_RESET_PIN 6
#define LTE_FLIGHT_PIN 7

// Libraries for SPI/I2C communication, graphics on OLED displays, and GPS data handling.
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GPS.h>

// OLED configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// Battery monitoring
#define BATTERY_PIN A1  // Pin for battery voltage monitoring
const float MAX_VOLTAGE = 4.2;  // Max voltage when battery is fully charged
const float MIN_VOLTAGE = 3.0;  // Min voltage when battery is almost empty

// Incoming & Outgoing call
const String allowedNumbers[] = {"+918637447158", "+919944546840"};
const int allowedNumbersCount = 2;

#define PHONE_NUMBER "9944546840"

// Buttons and Interrupts
#define STOPWATCH_BUTTON 2
#define OK_BUTTON 3
#define MODE_BUTTON 4
#define SOS_BUTTON 8

// CARID as a global variable
const int CARID = 2; 
bool blinkState = false;
unsigned long lastBlinkTime = 0;
const unsigned long BLINK_INTERVAL = 500;
unsigned long lastPageSwitchTime = 0;
const unsigned long PAGE_SWITCH_DELAY = 2000;

int currentPage = 0;
unsigned long stopwatchStart = 0;
unsigned long stopwatchElapsed = 0;
int stopwatchState = -1;  // -1 = reset, 1 = running, 2 = stopped

int scrollOffset = 0; // Variable for scrolling message

String carID = "C1"; // Variable for car ID
String broadcastMessage = ""; // Default empty broadcast message
unsigned long broadcastClearTime = 0; // Time when to clear the broadcast message

// SOS call tracking variables
bool sosCallInProgress = false;
unsigned long sosCallStartTime = 0;
unsigned long sosCallElapsedTime = 0;
bool callEnded = false;

// Splash screen timing variables
bool splashScreenShown = false;
unsigned long splashScreenStartTime = 0;
const unsigned long SPLASH_SCREEN_DURATION = 3000; // 3 seconds

// Debounce for the stopwatch button
unsigned long lastStopwatchButtonPress = 0;
const unsigned long DEBOUNCE_DELAY = 300;  // 300 milliseconds debounce delay


/**********************************************************************************************************************************************************************************************************************************************************************************************
  ______                _   _                 
 |  ____|              | | (_)                
 | |__ _   _ _ __   ___| |_ _  ___  _ __  ___ 
 |  __| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
 | |  | |_| | | | | (__| |_| | (_) | | | \__ \
 |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
                                              
**********************************************************************************************************************************************************************************************************************************************************************************************/


/***********************************************************************************************************************************************************************************************************************************************************************************************
  _____                           _          __  _         
 / ___/__  __ _  __ _  __ _____  (_)______ _/ /_(_)__  ___ 
/ /__/ _ \/  ' \/  ' \/ // / _ \/ / __/ _ `/ __/ / _ \/ _ \
\___/\___/_/_/_/_/_/_/\_,_/_//_/_/\__/\_,_/\__/_/\___/_//_/
                                                           
***********************************************************************************************************************************************************************************************************************************************************************************************/

// Sends a command to the LTE module and returns the response, with optional debugging.
String sendData(String command, const int timeout, boolean debug = false) { // debug: Controls debug output.
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

// Sends an HTTP POST request with JSON data and returns success status.
bool sendHTTPRequest(String url, String jsonPayload, bool breath) {// breath: Controls a brief delay and manages response validation
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


/*********************************************************************************************************************************************************************************************************************************************************************************************
 ______             __    _                     __     __         __                         __  _             
/_  __/______ _____/ /__ (_)__  ___ _  _______ / /__ _/ /____ ___/ / ___  ___  ___ _______ _/ /_(_)__  ___  ___
 / / / __/ _ `/ __/  '_// / _ \/ _ `/ / __/ -_) / _ `/ __/ -_) _  / / _ \/ _ \/ -_) __/ _ `/ __/ / _ \/ _ \(_-<
/_/ /_/  \_,_/\__/_/\_\/_/_//_/\_, / /_/  \__/_/\_,_/\__/\__/\_,_/  \___/ .__/\__/_/  \_,_/\__/_/\___/_//_/___/
                              /___/                                    /_/                                     

**********************************************************************************************************************************************************************************************************************************************************************************************/

// Sends GPS track data as a JSON payload to a specified server if valid NMEA sentence is retrieved.
void sendTrackData() {
    String gpsInfo = sendData("AT+CGPSINFO", 1300, DEBUG);
    String nmeaSentence = extractNMEA(gpsInfo);
    SerialUSB.println(nmeaSentence + " nmea");

    if (nmeaSentence.length() > 8) {
        SerialUSB.println("Sending Track data");

        String jsonPayload = "{\"nmea\": \"" + nmeaSentence + "\",\"carId\":" + String(CARID) + "}";
        String requestBinURL = "https://blueband-speed-zr7gm6w4cq-el.a.run.app/track";
        if (sendHTTPRequest(requestBinURL, jsonPayload, false)) {
            // Retry logic if needed
        }
    }
}

// Extracts NMEA sentence from the GPS information string.
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

/* Functions for handling SOS & OK interrupts and mode switch button and stopwatch */

void handleSosPress() {
    delay(50);  
    if (digitalRead(SOS_BUTTON) == LOW) {  
        sosPressed = true;
    }
}

void handleOkPress() {
    delay(50); 
    if (digitalRead(OK_BUTTON) == LOW) {  
        okPressed = true;
    }
}
/***********************************************************************************************************************************************************************************************************************************************************************************************
   ____                    _             ____      ____       __            _             _____     ____  
  /  _/__  _______  __ _  (_)__  ___ _  / __/___  / __ \__ __/ /____ ____  (_)__  ___ _  / ___/__ _/ / /__
 _/ // _ \/ __/ _ \/  ' \/ / _ \/ _ `/  > _/_ _/ / /_/ / // / __/ _ `/ _ \/ / _ \/ _ `/ / /__/ _ `/ / (_-<
/___/_//_/\__/\___/_/_/_/_/_//_/\_, /  |_____/   \____/\_,_/\__/\_, /\___/_/_//_/\_, /  \___/\_,_a_/_/___/
                               /___/                           /___/            /___/                     
**********************************************************************************************************************************************************************************************************************************************************************************************/

// Function to make a voice call
void makeVoiceCall(String phoneNumber) {
    SerialUSB.println("Dialing number: " + phoneNumber);
    String response = sendData("ATD" + phoneNumber + ";", 10000, DEBUG);
    if (response.indexOf("OK") != -1 || response.indexOf("CONNECT") != -1) {
        SerialUSB.println("Call initiated successfully.");
        
        sendData("AT+CSDVC=3", 2000, DEBUG);  // 3 for Speakerphone
        sendData("AT+CLVL=5", 2000, DEBUG);   // Set loudspeaker volume level to 2 (range: 0-5)
        
        monitorCall();
    } else {
        SerialUSB.println("Failed to initiate call. Response: " + response);
    }
}

// Function to monitor the call
void monitorCall() {
    SerialUSB.println("Monitoring call status...");

    bool callEnded = false;
    
    while (!callEnded) {
        // Read response from AT+CLCC command
        String response = sendData("AT+CLCC", 1300, DEBUG);  // Check call status
        
        // Check for the call ended status
        if (response.indexOf("NO CARRIER") != -1) {
            SerialUSB.println("Call ended.");
            callEnded = true;  // Exit the loop once the call has ended
        } else if (response.indexOf("ERROR") != -1) {
            SerialUSB.println("Error while monitoring call status.");
            callEnded = true;  // Exit the loop on error
        } else {
            SerialUSB.println("Waiting for call end status...");
        }

        delay(500);  // Check every half second
    }

    // Hang up the call
    String hangupResponse = sendData("ATH", 1300, DEBUG);
    if (hangupResponse.indexOf("OK") != -1) {
        SerialUSB.println("Call hung up successfully.");
    } else {
        SerialUSB.println("Failed to hang up call. Response: " + hangupResponse);
    }
}

// Function to answer the call
void answerCall() {
    String response = sendData("ATA", 5000, DEBUG);  // Answer the call
    if (response.indexOf("OK") != -1 || response.indexOf("CONNECT") != -1) {
        SerialUSB.println("Call answered successfully.");
        sendData("AT+CSDVC=3", 2000, DEBUG);  // Switch to speakerphone
        sendData("AT+CLVL=2", 2000, DEBUG);   // Set loudspeaker volume
    } else {
        SerialUSB.println("Failed to answer the call.");
    }
}

// Function to hang up a restricted call
void hangUpRestrictedCall() {
    String response = sendData("ATH", 5000, DEBUG);  
    delay(200);  
}

/*********************************************************************************************************************************************************************************************************************************************************************************************
   ___  _          __            ____     __          ___               
  / _ \(_)__ ___  / /__ ___ __  /  _/__  / /____ ____/ _/__ ________ ___
 / // / (_-</ _ \/ / _ `/ // / _/ // _ \/ __/ -_) __/ _/ _ `/ __/ -_|_-<
/____/_/___/ .__/_/\_,_/\_, / /___/_//_/\__/\__/_/ /_/ \_,_/\__/\__/___/
          /_/          /___/                                            
*********************************************************************************************************************************************************************************************************************************************************************************************/


// PAGE 0: Splash screen
void SplashscreenUI() {
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(SCREEN_WIDTH / 6, SCREEN_HEIGHT / 2 - 10);
    display.println("BLUEBAND  SPORTS");
}

// PAGE 1: Speed and Distance page 
void SpeedAndDistanceUI(String speed, String distance) {
    display.setCursor(5, 16);
    display.setTextSize(2);
    display.println("spd:" + (speed != "--.--" ? speed : "N/A"));
    display.setCursor(5, 36);
    display.println("dis:" + (distance != "--.--" ? distance : "N/A"));
}

// PAGE 2: Stopwatch and Elapsed Time page 
void StopwatchUI(String elapsedTime) {
    display.setTextSize(1);
    display.setCursor(2, 16);
    display.println("Stopwatch");
    display.setTextSize(2);
    display.setCursor(2, 30);
    display.println((elapsedTime != "00:00" ? elapsedTime : "N/A"));
}

// PAGE 3: Lap Timing page 
void LapTimingUI(String lapTime, String lastLap) {
    display.setCursor(5, 16);
    display.setTextSize(1);
    display.println("Lap Time: " + lapTime);
    display.setCursor(5, 26);
    display.println("Last Lap: " + lastLap);
}

// PAGE 4: Overall results page 
void OverallResultsUI(String avgSpeed, String totalTime, String penalties) {
    display.setCursor(5, 18);
    display.println("avg spd: " + (avgSpeed != "--.--" ? avgSpeed : "N/A"));
    display.setCursor(5, 28);
    display.println("total time: " + (totalTime != "00:00:00" ? totalTime : "N/A"));
    display.setCursor(5, 38);
    display.println("penalties: " + (penalties != "--" ? penalties : "N/A"));
}

// Function to draw Battery Status
void drawBatteryStatus(int batteryPercentage) {
    display.setCursor(SCREEN_WIDTH - 25, 3);
    if (batteryPercentage < 20 && blinkState) {
        display.println("LOW");
    } else {
        display.println(String(batteryPercentage) + "%");
    }
}

// Function to draw a line
void drawLine(int x0, int y0, int x1, int y1) {
    for (int x = x0; x < x1; x += 1) {
        display.drawPixel(x, y0, SSD1306_WHITE);
    }
}

// Function for Top Bar
void Topbar(const String& time, const String& carID, int batteryPercentage, bool blinkState) {
    drawLine(0, 0, SCREEN_WIDTH, 12); 
    display.setTextSize(1);
    display.setCursor(2, 3);
    display.print(time);
    display.setCursor(SCREEN_WIDTH / 2 - 15, 3);
    display.print(carID);
    drawBatteryStatus(batteryPercentage);
    drawLine(0, 12, SCREEN_WIDTH, 12);  
}

// Function for Bottom Bar
void Bottombar(const String& message) {
    drawLine(0, 52, SCREEN_WIDTH, 36);  // Always draw top divider line
    updateScrollingMessage(message);
    drawLine(0, 63, SCREEN_WIDTH, 12);  // Always draw bottom divider line
}

// Scrolling message function
void updateScrollingMessage(const String& message) {
    int messageWidth = message.length() * 6;  // Approximate message width
    display.setCursor(-scrollOffset, 55);
    display.setTextSize(1);
    display.print(message + " ");  // Append space for separation
    display.print(message);  // Repeat message to create loop

    scrollOffset++;
    if (scrollOffset > messageWidth) scrollOffset = 0;  
}

// Draw vertical dividers on both edges of the display
void drawVerticalDividers() {
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        display.drawPixel(0, i, SSD1306_WHITE);  
        display.drawPixel(SCREEN_WIDTH - 1, i, SSD1306_WHITE);  
            }
}

// Function to get the battery percentage
int getBatteryPercentage() {
    int analogValue = analogRead(BATTERY_PIN);
    float voltage = (analogValue * 3.3) / 1023.0;  // Convert analog reading to voltage
    float batteryVoltage = voltage * 2;  // Assuming voltage divider

    // Map the voltage to a percentage between MIN_VOLTAGE and MAX_VOLTAGE
    int percentage = map(batteryVoltage * 100, MIN_VOLTAGE * 100, MAX_VOLTAGE * 100, 0, 100);

    return constrain(percentage, 0, 100);  // Ensure the percentage is between 0 and 100
}

// Function to get the current time from GPS (mocked for now)
String gpsTime() {
    if (gps.hour == 0 && gps.minute == 0) {
        return "00:00";
    }
    String hour = String(gps.hour);
    String minute = String(gps.minute);
    if (hour.length() < 2) hour = "0" + hour;
    if (minute.length() < 2) minute = "0" + minute;
    return hour + ":" + minute;
}

// Function to format the elapsed time (MM:SS.M format)
String formatElapsedTime(unsigned long elapsed) {
    unsigned long milliseconds = (elapsed % 1000) / 100;  // Only show 1 digit of milliseconds
    unsigned long seconds = (elapsed / 1000) % 60;
    unsigned long minutes = (elapsed / (1000 * 60)) % 60;

    String formattedTime = "";
    if (minutes < 10) formattedTime += "0";
    formattedTime += String(minutes) + ":";
    if (seconds < 10) formattedTime += "0";
    formattedTime += String(seconds) + ".";
    formattedTime += String(milliseconds);  // Only 1 digit for milliseconds
    return formattedTime;
}

/*********************************************************************************************************************************************************************************************************************************************************************************************

   _____      _                          _                       
  / ____|    | |                 ___    | |                      
 | (___   ___| |_ _   _ _ __    ( _ )   | |     ___   ___  _ __  
  \___ \ / _ \ __| | | | '_ \   / _ \/\ | |    / _ \ / _ \| '_ \ 
  ____) |  __/ |_| |_| | |_) | | (_>  < | |___| (_) | (_) | |_) |
 |_____/ \___|\__|\__,_| .__/   \___/\/ |______\___/ \___/| .__/ 
                       | |                                | |    
                       |_|                                |_|    
*********************************************************************************************************************************************************************************************************************************************************************************************/

void setup(){
    SerialUSB.begin(115200);
    Serial1.begin(115200);
    gps.begin(9600);

    if (!display.begin(SSD1306_SWITCHCAPVCC)) {
        SerialUSB.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    display.clearDisplay();
    display.display();

    pinMode(MODE_BUTTON, INPUT_PULLUP);
    pinMode(STOPWATCH_BUTTON, INPUT_PULLUP);

    pinMode(BATTERY_PIN, INPUT);

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

    pinMode(STOPWATCH_BUTTON, INPUT_PULLUP);  
    pinMode(OK_BUTTON, INPUT_PULLUP); 

    attachInterrupt(digitalPinToInterrupt(STOPWATCH_BUTTON), handleSosPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(OK_BUTTON), handleOkPress, FALLING);

    while (!SerialUSB) {
        delay(10);
    }

    sosPressed = false;
    okPressed = false;
    SerialUSB.println("Switch test initialized.");
    //String response = sendData("AT+CFUN=1,1", 5000, DEBUG);
    String response = sendData("AT+CGATT=0", 2000, DEBUG);
    response = sendData("AT+CGATT=1", 2000, DEBUG);
    response = sendData("AT+CGACT=1,1", 2000, DEBUG);
    response = sendData("AT+CGPADDR=1", 1300, DEBUG);
    if (response.indexOf("OK") != -1 && response.indexOf(".") != -1) {
        SerialUSB.println("Internet connected.");
    } else {
        SerialUSB.println("Internet not connected.");
    }

    response = sendData("AT+CGPS=0", 3000, DEBUG);
    response = sendData("AT+CGPS=1", 3000, DEBUG);
}

void loop() {
    unsigned long currentTime = millis();

    // Page switch button logic
    if (digitalRead(PAGE_BUTTON) == LOW && currentTime - lastPageSwitchTime >= 300) {
        currentPage = (currentPage + 1) % 5;  // Cycle through 5 pages (0 to 4)
        lastPageSwitchTime = currentTime;
    }

    // SOS button logic (B2)
    if (digitalRead(SOS_BUTTON) == LOW && !sosCallInProgress && !callEnded) {
        sosPressed = true;
        sendingMessage = true;
        sosPressed = false;
        int max_sos_attempt = 0;

        // Send SOS HTTP Request and make the SOS call
        while (max_sos_attempt++ < 25) {
            if (!sendHTTPRequest("https://blueband-speed-zr7gm6w4cq-el.a.run.app/sos", "{\"carId\":2, \"message\": \"SOS\"}", true)) {
                SerialUSB.println("SOS DONE");
                makeVoiceCall("9944546840"); 
                sosCallStartTime = currentTime;
                sosCallInProgress = true;
                break;
            }
            SerialUSB.println("SOS NOT DONE");
        }
        sendingMessage = false;
    }

    // OK button logic (B3)
    if (digitalRead(OK_BUTTON) == LOW && broadcastMessage == "SOS alert pressed") {
        sendingMessage = true;
        okPressed = true;
        okPressed = false;
        int max_ok_attempt = 0;

        // Send OK HTTP Request
        while (max_ok_attempt++ < 25) {
            if (!sendHTTPRequest("https://blueband-speed-zr7gm6w4cq-el.a.run.app/ok", "{\"carId\":2, \"message\": \"OK\"}", true)) {
                SerialUSB.println("OK DONE");
                broadcastMessage = "";  // Clear the SOS alert when OK button is pressed
                callEnded = false;
                break;
            }
            SerialUSB.println("OK NOT DONE");
        }
        sendingMessage = false;
    }

    // Handle call progress and end
    if (sosCallInProgress) {
        sosCallElapsedTime = currentTime - sosCallStartTime;
        broadcastMessage = "Making call to 9944546840\nCall duration: " + formatElapsedTime(sosCallElapsedTime);
    }

    if (callEnded) {
        broadcastMessage = "Call ended\nSOS alert pressed";
    }

    // If no SOS or OK message is being sent, attempt to send track data
    if (!sendingMessage) {
        sendTrackData();  // Send track data only if no SOS or OK message is being sent
    }

    // splash screen 
    if (!splashScreenShown) {
        if (currentTime - splashScreenStartTime >= SPLASH_SCREEN_DURATION) {
            splashScreenShown = true;  // Splash screen displayed for 3 seconds
            currentPage = 1;  
            return;
        } else {
            if (currentTime - lastBlinkTime >= BLINK_INTERVAL) {
                blinkState = !blinkState;  
                lastBlinkTime = currentTime;
            }

            display.clearDisplay();
            if (blinkState) {
                SplashscreenUI();
            }
            display.display();
            return;
        }
    }

    display.clearDisplay();
    int batteryPercentage = getBatteryPercentage();
    Topbar(gpsTime(), carID, batteryPercentage, blinkState);

    switch (currentPage) {
        case 1:
            SpeedAndDistanceUI("--.--", "--.--");
            break;
        case 2:
            if (stopwatchState == 1) {
                stopwatchElapsed = millis() - stopwatchStart;
            }
            StopwatchUI(formatElapsedTime(stopwatchElapsed));
            break;
        case 3:
            LapTimingUI("00:00:00", "00:00:00");
            break;
        case 4:
            OverallResultsUI("--.--", "00:00:00", "--");
            break;
    }

    Bottombar(broadcastMessage);
    drawVerticalDividers();
    display.display();
    delay(10);
}
