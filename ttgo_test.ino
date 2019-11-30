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
//SoftwareSerial mySerial(RX_PIN, TX_PIN);                   // Uno example
HardwareSerial mySerial(1);                              // ESP32 Example

unsigned long getDataTimer = 0;
unsigned long uptime = millis();
int lastTemperature = 0;
int lastCO2PPM = 0;

#define TFT_CS   27
#define TFT_DC   26
#define TFT_MOSI 23
#define TFT_CLK  18
#define TFT_RST  5
#define TFT_MISO 12

// bitmaps
extern uint8_t opens3[];
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

void setup() {
    Serial.begin(9600);                                     // For ESP32 baudarte is 115200 etc.
    mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN); // ESP32 Example
    myMHZ19.begin(mySerial);                                // *Important, Pass your Stream reference
    myMHZ19.autoCalibration();
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextWrap(true);
    tft.setCursor(0, 170);
    tft.setTextSize(2);


    delay(1500);
    tft.drawBitmap(70, 50, opens3, 100, 100, ILI9341_WHITE);
    tft.fillScreen(ILI9341_BLACK); // Clear Screen
    tft.setTextColor(ILI9341_ORANGE);
    tft.setCursor(5, 20);
    tft.setTextSize(2);
    tft.println("Air Quality Monitor");
    tft.drawLine(0, 10, 240, 10, ILI9341_WHITE);
    tft.drawLine(0, 40, 240, 40, ILI9341_WHITE);
    int h = 50, w = 50, row, col, buffidx = 0;
    for (row=5; row < h; row++) { // For each scanline...
        for (col = 5; col < w; col++) { // For each pixel...
            tft.drawPixel(10, 10, pgm_read_word(opens3 + buffidx));
            buffidx++;
        }
    }

}

void loop() {
    if (millis() - getDataTimer >=
        500)                    // Check if interval has elapsed (non-blocking delay() equivilant)
    {
        int CO2;                                            // Buffer for CO2
        CO2 = myMHZ19.getCO2();                             // Request CO2 (as ppm)

        Serial.print("CO2 (ppm): ");
        Serial.println(CO2);

        int8_t Temp;                                         // Buffer for temperature
        Temp = myMHZ19.getTemperature();                     // Request Temperature (as Celsius)
        Serial.print("Temperature (C): ");
        Serial.println(Temp);

        getDataTimer = millis();                            // Update interval

        if (lastCO2PPM != CO2) {
            // CO2
            tft.fillRect(110, 65, 80, 20, ILI9341_ORANGE);
            tft.setCursor(5, 65);
            tft.setTextSize(2);
            tft.print("CO2 PPM: ");
            tft.setCursor(110, 65);
            tft.print(CO2);
        }

        if (lastTemperature != Temp) {
            // Temp
            tft.fillRect(110, 95, 80, 20, ILI9341_GREEN);
            tft.setCursor(5, 95);
            tft.setTextSize(2);
            tft.print("Temp: ");
            tft.setCursor(110, 95);
            tft.print(Temp);
        }


        tft.setTextSize(1);
        tft.fillRect(50, 300, 60, 15, ILI9341_BLUE);
        tft.setCursor(5, 307);
        tft.setTextColor(ILI9341_WHITE);
        tft.print("Uptime: ");
        tft.print((millis() - uptime) /1000);
        tft.print("s");

        lastTemperature = Temp;
        lastCO2PPM = CO2;

    }


}
