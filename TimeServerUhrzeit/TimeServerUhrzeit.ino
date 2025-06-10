#include <FastLED.h>
#include <LEDMatrix.h>
#include <LEDText.h>
#include <FontMatrise.h>
#include <WiFi.h>
#include <time.h>

//WIFI zeugs
const char* wlanName = "SSID";
const char* wlanPasswort = "PWD";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset = 3600;      // MEZ
const int daylightOffset = 3600;  // Sommerzeit

//LED zeugs

#define LED_PIN 25
#define COLOR_ORDER GRB
#define CHIPSET WS2812B

#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT -8
#define MATRIX_TYPE VERTICAL_ZIGZAG_MATRIX

int h = 0;
int m = 0;

cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;

cLEDText ClockTime;

unsigned char TxtDemo[] = { "   :  " };

//LED2 Zeugs:

#define LED_PIN2 26
#define COLOR_ORDER2 GRB
#define CHIPSET2 WS2812B

#define MATRIX_WIDTH2 -32
#define MATRIX_HEIGHT2 8
#define MATRIX_TYPE2 VERTICAL_ZIGZAG_MATRIX

int day = 0;
int month = 0;

cLEDMatrix<MATRIX_WIDTH2, MATRIX_HEIGHT2, MATRIX_TYPE2> leds2;

cLEDText Date;

unsigned char DateText[] = { "   .  " };


void setup() {
  Serial.begin(9600);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
  FastLED.addLeds<CHIPSET2, LED_PIN2, COLOR_ORDER2>(leds2[0], leds2.Size());
  FastLED.setBrightness(10);
  FastLED.clear(true);
  delay(500);
  FastLED.show();

  verbindeMitWLAN();
  konfiguriereZeit();

  ClockTime.SetFont(MatriseFontData);
  ClockTime.Init(&leds, leds.Width(), ClockTime.FontHeight() + 1, 0, 0);
  ClockTime.SetText((unsigned char*)TxtDemo, sizeof(TxtDemo) - 1);
  ClockTime.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);

  Date.SetFont(MatriseFontData);
  Date.Init(&leds2, leds2.Width(), Date.FontHeight() + 1, 0, 0);
  Date.SetText((unsigned char*)DateText, sizeof(DateText) - 1);
  Date.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
}


void loop() {
  struct tm zeitinfo;
  if (!getLocalTime(&zeitinfo)) {
    Serial.println("Zeitabruf fehlgeschlagen.");
    return;
  }

  Date.SetText((unsigned char*)DateText, sizeof(DateText) - 1);
  ClockTime.SetText((unsigned char*)TxtDemo, sizeof(TxtDemo) - 1);

  ClockTime.UpdateText();  //Anzeigen
  Date.UpdateText();       //Anzeigen
  for (int i = 0; i < 5; i++) {
    ClockTime.UpdateText();  //um eins verschieben, weil 6*5 = 30 pixel, von 32 Pixel macht 1 Pixel Abstand / Seite
    Date.UpdateText();
  }

  String hours = String(h);
  if (h < 10) {
    hours = "0" + hours;
  }
  String minutes = String(m);
  if (m < 10) {
    minutes = "0" + minutes;
  }

  String dayString = String(day);
  if (day < 10) {
    dayString = "0" + dayString;
  }
  String monthString = String(month);
  if (month < 10) {
    monthString = "0" + monthString;
  }

  for (int i = 0; i < 2; i++) {
    TxtDemo[i + 1] = hours[i];
  }
  for (int i = 0; i < 2; i++) {
    TxtDemo[i + 4] = minutes[i];
  }

  for (int i = 0; i < 2; i++) {
    DateText[i + 1] = dayString[i];
  }
  for (int i = 0; i < 2; i++) {
    DateText[i + 4] = monthString[i];
  }

  FastLED.show();
  delay(1000);
  h = zeitinfo.tm_hour;
  m = zeitinfo.tm_min;
  day = zeitinfo.tm_mday;
  month = zeitinfo.tm_mon + 1;
}

void verbindeMitWLAN() {
  WiFi.begin(wlanName, wlanPasswort);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWLAN verbunden.");
}

void konfiguriereZeit() {
  configTime(gmtOffset, daylightOffset, ntpServer);
}
