/*
The MIT License (MIT)

Copyright © 2018 Médéric NETTO

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <TimeLib.h>
#include <Arduino.h>
#include "MHZ19.h"                                         // include main library
#include <SoftwareSerial.h>                                // Remove if using HardwareSerial or non-uno library compatable device

#define RX_PIN 35                                   // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN 33                                          // Tx pin which the MHZ19 Rx pin is attached to
#define BAUDRATE 9600                                      // Native to the sensor (do not change)

MHZ19 myMHZ19;                                             // Constructor for MH-Z19 class
HardwareSerial mySerial(1);                              // ESP32 Example

#define CUSTOM_DARK 0x3186 // Background color

unsigned long getDataTimer = 0;
unsigned long uptime = millis();
int lastTemperature = 0;
int lastCO2PPM = 0;
int lastSecond = 0;

#define TFT_CS   27
#define TFT_DC   26
#define TFT_MOSI 23
#define TFT_CLK  18
#define TFT_RST  5
#define TFT_MISO 12

// bitmaps
extern uint8_t wifi_1[];
extern uint8_t opens3[];
extern uint8_t shroom[];

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

void setup() {
    Serial.begin(9600);                                     // For ESP32 baudarte is 115200 etc.
    mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN); // ESP32 Example
    myMHZ19.begin(mySerial);                                // *Important, Pass your Stream reference
    myMHZ19.autoCalibration();
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(CUSTOM_DARK); // Clear Screen
    tft.setTextColor(ILI9341_ORANGE);
    tft.setCursor(40, 20);
    tft.setTextSize(2);
    tft.drawBitmap(5, 5, opens3, 28, 32, ILI9341_YELLOW);
    tft.println("Air Monitor");
    tft.drawLine(40, 10, 240, 10, ILI9341_WHITE);
    tft.drawLine(0, 40, 240, 40, ILI9341_WHITE);
    tft.drawBitmap(175, 250, shroom, 66, 64, ILI9341_RED);

}

void loop() {
    if (millis() - getDataTimer >= 100) {
        int curSecond = ((millis() - uptime) / 1000);
        int CO2;                                            // Buffer for CO2
        CO2 = myMHZ19.getCO2();                             // Request CO2 (as ppm)

        Serial.print("CO2 (ppm): ");
        Serial.println(CO2);

        int8_t Temp;                                         // Buffer for temperature
        Temp = myMHZ19.getTemperature();                     // Request Temperature (as Celsius)
        Serial.print("Temperature (C): ");
        Serial.println(Temp);

        getDataTimer = millis();                            // Update interval

        tft.setTextColor(ILI9341_WHITE);
        if (lastCO2PPM != CO2) {
            // CO2
            tft.fillRect(110, 65, 80, 20, CUSTOM_DARK);
            tft.setCursor(5, 65);
            tft.setTextSize(2);
            tft.print("CO2 PPM: ");
            tft.setCursor(110, 65);
            tft.print(CO2);
        }

        if (lastTemperature != Temp) {
            // Temp
            tft.fillRect(110, 95, 80, 20, CUSTOM_DARK);
            tft.setCursor(5, 95);
            tft.setTextSize(2);
            tft.print("Temp: ");
            tft.setCursor(110, 95);
            tft.print(Temp);
        }

        if (lastSecond != curSecond) {
            tft.setTextSize(1);
            tft.fillRect(50, 300, 60, 15, CUSTOM_DARK);
            tft.setCursor(5, 307);
            tft.print("Uptime: ");
            tft.print(curSecond);
            tft.print("s");
        }

        lastTemperature = Temp;
        lastCO2PPM = CO2;
        lastSecond = curSecond;
    }


}
