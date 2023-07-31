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

const char* ssid = "";
const char* password = "";

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

unsigned long testText() {
  tft.fillScreen(HX8357_BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(HX8357_WHITE);  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(HX8357_YELLOW); tft.setTextSize(2);
  tft.println(1234.56);
  tft.setTextColor(HX8357_RED);    tft.setTextSize(3);
  tft.println(0xDEADBEEF, HEX);
  tft.println();
  tft.setTextColor(HX8357_GREEN);
  tft.setTextSize(5);
  tft.println("Groop");
  tft.setTextSize(2);
  tft.println("I implore thee,");
  tft.setTextSize(1);
  tft.println("my foonting turlingdromes.");
  tft.println("And hooptiously drangle me");
  tft.println("with crinkly bindlewurdles,");
  tft.println("Or I will rend thee");
  tft.println("in the gobberwarts");
  tft.println("with my blurglecruncheon,");
  tft.println("see if I don't!");
  
  tft.setTextColor(HX8357_WHITE);
  tft.println(F("Alice was beginning to get very tired of sitting by her sister on the bank, and of having nothing to do: once or twice she had peeped into the book her sister was reading, but it had no pictures or conversations in it, 'and what is the use of a book,' thought Alice 'without pictures or conversations?'"));

  tft.println(F("So she was considering in her own mind (as well as she could, for the hot day made her feel very sleepy and stupid), whether the pleasure of making a daisy-chain would be worth the trouble of getting up and picking the daisies, when suddenly a White Rabbit with pink eyes ran close by her."));

  tft.println(F("There was nothing so very remarkable in that; nor did Alice think it so very much out of the way to hear the Rabbit say to itself, 'Oh dear! Oh dear! I shall be late!' (when she thought it over afterwards, it occurred to her that she ought to have wondered at this, but at the time it all seemed quite natural); but when the Rabbit actually took a watch out of its waistcoat-pocket, and looked at it, and then hurried on, Alice started to her feet, for it flashed across her mind that she had never before seen a rabbit with either a waistcoat-pocket, or a watch to take out of it, and burning with curiosity, she ran across the field after it, and fortunately was just in time to see it pop down a large rabbit-hole under the hedge."));

  tft.println(F("In another moment down went Alice after it, never once considering how in the world she was to get out again."));

  tft.println(F("The rabbit-hole went straight on like a tunnel for some way, and then dipped suddenly down, so suddenly that Alice had not a moment to think about stopping herself before she found herself falling down a very deep well."));

  tft.println(F("Either the well was very deep, or she fell very slowly, for she had plenty of time as she went down to look about her and to wonder what was going to happen next. First, she tried to look down and make out what she was coming to, but it was too dark to see anything; then she looked at the sides of the well, and noticed that they were filled with cupboards and book-shelves; here and there she saw maps and pictures hung upon pegs. She took down a jar from one of the shelves as she passed; it was labelled 'ORANGE MARMALADE', but to her great disappointment it was empty: she did not like to drop the jar for fear of killing somebody, so managed to put it into one of the cupboards as she fell past it."));
    
  return micros() - start;
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
  tft.setTextSize(5);
 
  tft.println(">>> Connecting to: ");
  tft.println(" ");
  tft.println(ssid);
  Serial.println("start");

  delay(5000);

  Serial.println("Beginning WIFI!!");

  WiFi.begin(ssid, password);

  Serial.println("Wifi BEGUN");
  delay(10000);

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
  tft.print(estimatedCall->frontText);
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
    "quay(id: \"NSR:Quay:11814\") {"
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
      stopLabel = (const char*)(root["data"]["quay"]["name"]);
      for (int i=0; i < root["data"]["quay"]["estimatedCalls"].size(); i++)
      {
        Serial.println(i);
        Serial.print("Retrieved departure time: ");
        Serial.println((const char*)root["data"]["quay"]["estimatedCalls"][i]["expectedDepartureTime"]);
        estimatedCalls[i] = (EstimatedCall) {
          (uint_fast8_t)root["data"]["quay"]["estimatedCalls"][i]["serviceJourney"]["line"]["publicCode"],
          (const char*)root["data"]["quay"]["estimatedCalls"][i]["destinationDisplay"]["frontText"],
          (const char*)root["data"]["quay"]["estimatedCalls"][i]["expectedDepartureTime"]
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


