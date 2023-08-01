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

#include "config.h"

#include <ArduinoJson.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include <SPI.h>
#include <Adafruit_GFX.h>

#include <TimeLib.h>
#include <time.h>

struct tm local_ts;


/***** This is for HX8357 *****/
#define BLACK HX8357_BLACK
#define WHITE HX8357_WHITE
#define BLUE HX8357_BLUE
#define RED HX8357_RED

#include "Adafruit_HX8357.h"
#define TFT_CS 5
#define TFT_DC 21
#define TFT_RST 22

Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);
/*******************************/


typedef struct
{
  uint publicCode;
  const char* frontText;
  const char* expectedDepartureTime;
} EstimatedCall;

EstimatedCall estimatedCalls[5] = {
  {0, "", ""},
  {0, "", ""},
  {0, "", ""},
  {0, "", ""},
  {0, "", ""},
};

const char host[] = "api.entur.io";

// Colors
// int ILI9341_COLOR;

// Bitmap_WiFi
extern uint8_t wifi_1[];
extern uint8_t wifi_2[];
extern uint8_t wifi_3[];

unsigned long previousMillis = 0;
long interval = 0;

void InitNTPTime()
{
  //Configure NTP
  const char* ntpServer = "no.pool.ntp.org";
  const long  gmtOffset_sec = 3600;
  const int   daylightOffset_sec = 3600;

  //Config time using NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  //Get RTC time
  getLocalTime(&local_ts);
  Serial.print("Current time obtained from RTC after NTP config is: ");
  Serial.println(&local_ts, "%A, %B %d %Y %H:%M:%S");
}

void setup() {

  Serial.begin(115200);
  
#ifdef SCREEN
  if (SCREEN == ILI9341)
  {
    pinMode(TFT_LED, OUTPUT);
    digitalWrite(TFT_LED, HIGH);
  }
#endif

  tft.begin();

  tft.setRotation(3);

  tft.fillScreen(BLACK);

  tft.setTextColor(WHITE);
  tft.setTextWrap(false);
  tft.setCursor(0, 0);
  tft.setTextSize(3);
 
  tft.println("Connecting to: ");
  tft.println(" ");
  tft.println(ssid);
  Serial.println("start");

  Serial.println("Beginning WIFI!!");

  WiFi.begin(ssid, password);

  Serial.println("Wifi BEGUN");

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      tft.print(".");
  }

  tft.fillScreen(BLACK); // Clear Screen
  tft.setTextColor(BLUE);
  tft.setCursor(0, 150);
  tft.setTextSize(2);
  tft.println("Entur Tracker v0.1");
  // tft.drawLine(0,130,240,130, ILI9341_WHITE);
  tft.drawLine(0,130,240,130, WHITE);
  // tft.drawLine(0,185,240,185, ILI9341_WHITE);
  tft.drawLine(0,185,240,185, WHITE);

  tft.setTextSize(1);
  tft.setCursor(5, 230);
  tft.setTextColor(WHITE);
  tft.println("Data from: api.entur.io. Initialising Time");

  InitNTPTime();
}

void DisplayEstimatedCall(int x, int y, EstimatedCall *estimatedCall)
{
  struct tm ts = {0};
  strptime(estimatedCall->expectedDepartureTime,"%Y-%m-%dT%H:%M:%SZ",&ts);
  Serial.print("Expected Time: ");
  Serial.println(&ts, "%A, %B %d %Y %H:%M:%S");
  ts.tm_isdst = local_ts.tm_isdst;
  time_t expectedTime = mktime(&ts);
  time_t localTime = mktime(&local_ts);
  double timeDifference = difftime(expectedTime, localTime);
  int timeDiffRounded = floor(timeDifference/60);
  
  tft.fillRoundRect(x+10, y, 60, 45, 4, RED);
  tft.setTextSize(3);
  tft.setCursor(x+23, y+10);
  tft.print(estimatedCall->publicCode);
  tft.setTextSize(4);
  tft.setCursor(x+100, y+6);
  String frontText = String(estimatedCall->frontText);
  frontText.replace("å", "a");
  tft.print(frontText);
  tft.setCursor(x+380, y+6);
  tft.print(timeDiffRounded);
}

void DisplayInfoBar(int y)
{
  tft.fillRect(0, y, 480, y+10, WHITE);
  DisplayClock(60, y+2);
  DisplayDate(320, y+2);
}

void DisplayClock(int x, int y)
{
  tft.setTextColor(BLACK);
  tft.setTextSize(4);
  tft.setCursor(x, y);
  tft.printf("%02d:%02d", local_ts.tm_hour, local_ts.tm_min);
}

void DisplayDate(int x, int y)
{
  tft.setTextColor(BLACK);
  tft.setTextSize(4);
  tft.setCursor(x, y);
  tft.printf("%02d/%02d", local_ts.tm_mday, local_ts.tm_mon);
}

void DisplayEstimatedCalls(int y)
{
  tft.setTextColor(WHITE);

  tft.setTextSize(3);
  tft.setCursor(2, y);
  tft.print("Linje");
  tft.setCursor(120, y);
  tft.print("Destinasjon");
  tft.setCursor(350, y);
  tft.print("Avgang");

  int topBannerSpacing = 40;
  int estimatedCallSpacing = 47;
  for (int i=0; i<5; i++)
  {
    if (strcmp(estimatedCalls[i].expectedDepartureTime, "") == 0)
      return;

    DisplayEstimatedCall(0, y + topBannerSpacing + (estimatedCallSpacing * i), &estimatedCalls[i]);
  }
}

//#########################################################################################
void MakeRuterHttpRequest()
{
  HTTPClient http;  // Declare an object of class HTTPClient
  // specify request destination
  http.begin("https://api.entur.io/journey-planner/v3/graphql");  // !!
  http.addHeader("Content-Type", "application/json");

  const int capacity = JSON_OBJECT_SIZE(3);
  StaticJsonBuffer<capacity> jb;

  // Create a JsonObject
  JsonObject& requestPayloadObject = jb.createObject();
  requestPayloadObject["query"] =
    "{"
    "stopPlace(id: \"NSR:StopPlace:4336\") {"
    "  name"
    "  id"
    "  estimatedCalls(timeRange: 58800, numberOfDepartures: 5) {"
    "    expectedDepartureTime"
    "    destinationDisplay {"
    "      frontText"
    "    }"
    "    serviceJourney {"
    "      line {"
    "        publicCode"
    "      }"
    "    }"
    "  }"
    "}"
  "}";

  String payload = "";
  requestPayloadObject.printTo(payload);
  int httpCode = http.POST(payload);  // Sending the request

  if (httpCode > 0)  // Checking the returning code
  {
    const char* stopLabel="";
    String payload = http.getString();   // Getting the request response payload
    DynamicJsonBuffer jsonBuffer(512);

    // Parse JSON object
    JsonObject& root = jsonBuffer.parseObject(payload);
    if (!root.success())
    {
      // gfx.print("Could not parse JSON!"); // not initialized
      stopLabel = "No Label";
    }
    else
    {
      stopLabel = (const char*)(root["data"]["stopPlace"]["name"]);
      for (int i=0; i < root["data"]["stopPlace"]["estimatedCalls"].size(); i++)
      {
        Serial.println(i);
        Serial.print("Retrieved departure time: ");
        Serial.println((const char*)root["data"]["stopPlace"]["estimatedCalls"][i]["expectedDepartureTime"]);
        estimatedCalls[i] = (EstimatedCall) {
          (uint_fast8_t)root["data"]["stopPlace"]["estimatedCalls"][i]["serviceJourney"]["line"]["publicCode"],
          (const char*)root["data"]["stopPlace"]["estimatedCalls"][i]["destinationDisplay"]["frontText"],
          (const char*)root["data"]["stopPlace"]["estimatedCalls"][i]["expectedDepartureTime"]
        };

      }
    }
    Serial.println(stopLabel);
    
  }
  else
  {
    tft.println("HTTP Code is less than or equal to 0?");
  }
  http.end();   // Close connection
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {

    previousMillis = currentMillis;
    interval = 15000;

    MakeRuterHttpRequest();
    // Update the time so the DisplayEstimatedCall has the right local time.
    getLocalTime(&local_ts);
    tft.fillScreen(BLACK);
    DisplayInfoBar(285);
    DisplayEstimatedCalls(2);
  }
}


