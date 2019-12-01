/*
 * Copyright Wilyarti Howard - 2019
 */


#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <TimeLib.h>
#include <Arduino.h>
#include "MHZ19.h"
#include <SoftwareSerial.h>

MHZ19 myMHZ19;
HardwareSerial mySerial(1);

#define RX_PIN 35
#define TX_PIN 33
#define BAUDRATE 9600
#define CUSTOM_DARK 0x3186
#define TFT_CS   27
#define TFT_DC   26
#define TFT_MOSI 23
#define TFT_CLK  18
#define TFT_RST  5
#define TFT_MISO 12

unsigned long getDataTimer = 0;
unsigned long uptime = millis();
int lastTemperature = 0;
int lastCO2PPM = 0;
int lastSecond = 0;

// bitmaps
extern uint8_t opens3[];

// Graphing Stuff
int dataSetLength = 22;
int graphY[22] = {};
unsigned long graphX[22] = {};

int xOffSet = 280;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

void setup() {
    Serial.begin(9600);
    mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);
    myMHZ19.begin(mySerial);
    myMHZ19.autoCalibration();
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(CUSTOM_DARK);
    tft.setTextColor(ILI9341_ORANGE);
    tft.setCursor(40, 20);
    tft.setTextSize(2);
    tft.drawBitmap(5, 5, opens3, 28, 32, ILI9341_YELLOW);
    tft.println("Air Monitor");
    tft.drawLine(40, 10, 240, 10, ILI9341_WHITE);
    tft.drawLine(0, 40, 240, 40, ILI9341_WHITE);
    tft.drawLine(30, 120, 30, xOffSet + 10, ILI9341_WHITE);
    tft.drawLine(0, xOffSet + 10, 240, xOffSet + 10, ILI9341_WHITE);
    tft.setTextSize(1);
    for (int i = 0; i < 8; i++) {
        tft.setCursor(4, (xOffSet - ((i * 20))));
        tft.print(i * 200);
    }
    tft.setCursor(90, xOffSet + 20);
    tft.print("3 Hour Trend");
    tft.setTextColor(ILI9341_WHITE);

}

void loop() {
    if (millis() - getDataTimer >= 100) {
        int curSecond = ((millis() - uptime) / 1000);
        if (true) {
            int CO2 = 0;
            CO2 = myMHZ19.getCO2();
            Serial.print("CO2 (ppm): ");
            Serial.println(CO2);
            int8_t Temp;
            Temp = myMHZ19.getTemperature();
            Serial.print("Temperature (C): ");
            Serial.println(Temp);

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
            addMeasurement(millis(), CO2);
            drawGraph();
            lastTemperature = Temp;
            lastCO2PPM = CO2;
        }

        tft.setTextColor(ILI9341_WHITE);
        if (lastSecond != curSecond) {
            tft.setTextSize(1);
            tft.fillRect(50, 307, 60, 15, CUSTOM_DARK);
            tft.setCursor(5, 307);
            tft.print("Uptime: ");
            tft.print(curSecond);
            tft.print("s");
        }

        lastSecond = curSecond;
        getDataTimer = millis();
    }
}

void addMeasurement(int x, unsigned long y) {
    Serial.println(y);
    for (int i = (dataSetLength - 1); i > 0; i--) {
        graphY[i] = graphY[i - 1];
        // mess with it for testing
        graphY[i] = graphY[i] - 10;
    }
    graphY[0] = y;
}

void drawGraph() {
    tft.fillRect(32, 120, 240, 170, CUSTOM_DARK);
    int lastX = 0;
    int lastY = 0;
    for (int i = 0; i < dataSetLength; i++) {
        if (graphY[i] <= 0) {
            continue;
        }
        int dotYLocation = xOffSet - (graphY[i] / 10);
        int currentX = (i * (240 / dataSetLength) + 30);
        //Serial.print(graphY[i]);
        //Serial.print(",");
        tft.fillCircle(currentX, dotYLocation, 2, ILI9341_RED);
        if (lastX > 0 && lastY > 0) {
            tft.drawLine(currentX, dotYLocation, lastX, lastY, ILI9341_RED);
        }
        Serial.print("Plotting at (");
        Serial.print(currentX);
        Serial.print(",");
        Serial.print(dotYLocation);
        Serial.print("): ");
        Serial.println(graphY[i]);

        lastX = currentX;
        lastY = dotYLocation;
    }
    for (int i = 1; i < 11; i++) {
        if (i  < 8) {
            tft.drawLine(30, (xOffSet - ((i * 20))), 240, (xOffSet - ((i * 20))), 0xA9A9);
        }
        tft.drawLine((i * 20)+ 30, xOffSet +10 ,(i * 20)+ 30 , 120, 0XA9A9);

    }

    Serial.println();
}