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
#include <EasyButton.h>


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

#define BUTTON_A  37  //          37 CENTRE
#define BUTTON_B  38 //          38 LEFT
#define BUTTON_C  39 //          39 RIGHT

unsigned long getDataTimer = 0;
unsigned long graphIntervalTimer = 0;
unsigned long uptime = millis();
int lastTemperature = 0;
int lastCO2PPM = 0;
int lastSecond = 0;

// bitmaps
extern uint8_t opens3[];

// Graphing Stuff
struct graphPoint {
    int CO2;
    int Temp;
    int ppm1_0;
    int ppm2_5;
    int ppm10;
    unsigned long time;
};
struct graphPoint gp[22];

int dataSetLength = 22;
int graphY[22] = {};
unsigned long graphX[22] = {};
int scale = 2;
int yMax = 160;
int xOffSet = 280;
int numYLabels = 8;


Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
EasyButton button(BUTTON_A);


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

    drawScales();
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(90, xOffSet + 20);
    tft.print("3 Hour Trend");
      button.begin();
  button.onPressed(onPressed);


}

void loop() {
    button.read();

    if (millis() - getDataTimer >= 100) {
        int curSecond = ((millis() - uptime) / 1000);

        // Update uptime first.
        tft.setTextColor(ILI9341_WHITE);
        if (lastSecond != curSecond) {
            tft.setTextSize(1);
            tft.fillRect(50, 307, 60, 15, CUSTOM_DARK);
            tft.setCursor(5, 307);
            tft.print("Uptime: ");
            tft.print(curSecond);
            tft.print("s");
        }
        int CO2 = 0;
        CO2 = myMHZ19.getCO2();
        int8_t Temp;
        Temp = myMHZ19.getTemperature();


        // Lazy update the CO2
        if (lastCO2PPM != CO2) {
            // CO2
            int color;
            if (CO2 <= 500) {
                color = ILI9341_BLUE;
            } else if (CO2 <= 1000) {
                color = ILI9341_GREEN;
            } else if (CO2 <= 1500) {
                color = ILI9341_YELLOW;
            } else if (CO2 <= 2000) {
                color = ILI9341_ORANGE;
            } else if (CO2 <= 2500) {
                color = ILI9341_RED;
            } else if (CO2 <= 5000) {
                color = ILI9341_PURPLE;
            }
            tft.setTextColor(color);
            tft.fillRect(110, 65, 80, 20, CUSTOM_DARK);
            tft.setCursor(5, 65);
            tft.setTextSize(2);
            tft.print("CO2 PPM: ");
            tft.setCursor(110, 65);
            tft.print(CO2);
            tft.setTextColor(ILI9341_WHITE);
        }
        // Lazy update the Temp
        if (lastTemperature != Temp) {
            // Temp
            tft.fillRect(110, 95, 80, 20, CUSTOM_DARK);
            tft.setCursor(5, 95);
            tft.setTextSize(2);
            tft.print("Temp: ");
            tft.setCursor(110, 95);
            tft.print(Temp);
        }

        // Add a graph data point every 8 mins.
        if ((millis() - graphIntervalTimer > 1000 * 500) || graphIntervalTimer == 0) {
            addMeasurement(millis(), CO2);
            drawGraph();
            graphIntervalTimer = millis();
        }


        lastTemperature = Temp;
        lastCO2PPM = CO2;
        lastSecond = curSecond;
        getDataTimer = millis();
    }

}

void addMeasurement(int x, unsigned long y) {
    for (int i = 0; i < dataSetLength; i++) {
        graphY[i] = graphY[i + 1];
    }
    graphY[dataSetLength - 1] = y;

    for (int i = 0; i < dataSetLength; i++) {
        gp[i] = gp[i + 1];
    }
    gp[dataSetLength - 1].CO2 = y;
    gp[dataSetLength - 1].CO2 = y;

}

void drawGraph() {
    bool virginScale = true;
    tft.fillRect(28, 120, 240, 170, CUSTOM_DARK);
    tft.drawLine(30, 120, 30, xOffSet + 10, ILI9341_WHITE);
    tft.drawLine(0, xOffSet + 10, 240, xOffSet + 10, ILI9341_WHITE);
    int lastX = 0;
    int lastY = 0;
    for (int i = 0; i < dataSetLength; i++) {
        if (graphY[i] <= 0) {
            continue;
        }

        int scaled = (graphY[i] / scale);
        int dotYLocation = xOffSet - scaled;
        int currentX = (i * (240 / dataSetLength)) + 30;

        if ((dotYLocation > (xOffSet + 30)) && virginScale) {
            scale--;
            virginScale = false;
            drawScales();
            continue;
        }
        if ((dotYLocation < 120) && virginScale) {
            int oldScale = scale;
            scale = (graphY[i] / 160) + 1;
            virginScale = false;
            if (oldScale != scale) {
                drawScales();
            }
            continue;
        }
        Serial.print(graphY[i]);
        Serial.print(",");
        int color;
        int CO2 = graphY[i];
        if (CO2 <= 500) {
            color = ILI9341_BLUE;
        } else if (CO2 <= 1000) {
            color = ILI9341_GREEN;
        } else if (CO2 <= 1500) {
            color = ILI9341_YELLOW;
        } else if (CO2 <= 2000) {
            color = ILI9341_ORANGE;
        } else if (CO2 <= 2500) {
            color = ILI9341_RED;
        } else if (CO2 <= 5000) {
            color = ILI9341_PURPLE;
        }
        tft.fillCircle(currentX, dotYLocation, 2, color);
        if (lastX > 0 && lastY > 0) {
            tft.drawLine(currentX, dotYLocation, lastX, lastY, color);
        }
        Serial.print("Plotting at (");
        Serial.print(scaled);
        Serial.print(",");
        Serial.print(dotYLocation - 30);
        Serial.print("): ");
        Serial.println(graphY[i]);

        lastX = currentX;
        lastY = dotYLocation;

    }
    for (int i = 1; i < 11; i++) {
        if (i < numYLabels) {
            tft.drawLine(30, (xOffSet - ((i * (yMax / numYLabels)))), 240, (xOffSet - ((i * (yMax / numYLabels)))),
                         0x8C71);
        }
        tft.drawLine((i * 20) + 30, xOffSet + 10, (i * 20) + 30, 120, 0x8C71);

    }

    Serial.println();
}

void drawScales() {
    if (scale < 1) {
        scale = 1;
    }
    if (scale >= 32) {
        scale = 31;
    }
    tft.setTextSize(1);
    tft.setCursor(0, xOffSet + 20);
    Serial.print("Y Scale: ");
    Serial.println(scale);
    tft.fillRect(0, 115, 240, (xOffSet - 115), CUSTOM_DARK);
    tft.drawLine(30, 120, 30, xOffSet + 10, ILI9341_WHITE);
    tft.drawLine(0, xOffSet + 10, 240, xOffSet + 10, ILI9341_WHITE);
    for (int i = 0; i < numYLabels; i++) {
        int color;
        int CO2 = (i * (yMax / numYLabels) * scale);
        if (CO2 <= 500) {
            color = ILI9341_BLUE;
        } else if (CO2 <= 1000) {
            color = ILI9341_GREEN;
        } else if (CO2 <= 1500) {
            color = ILI9341_YELLOW;
        } else if (CO2 <= 2000) {
            color = ILI9341_ORANGE;
        } else if (CO2 <= 2500) {
            color = ILI9341_RED;
        } else if (CO2 <= 5000) {
            color = ILI9341_PURPLE;
        }
        tft.setTextColor(color);
        tft.setCursor(0, (xOffSet - ((i * (yMax / numYLabels)))));
        tft.print(i * (yMax / numYLabels) * scale);
    }
}
void onPressed() {
  Serial.println("Button has been pressed!");
}
