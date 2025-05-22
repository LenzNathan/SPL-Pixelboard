#include <WiFi.h>
#include <time.h>
#include <FastLED.h>
#include <LEDMatrix.h>
#include <LEDText.h>

// WLAN-Zugangsdaten

const char* wlanName = "iPhone_13 Pro_AEW";
const char* wlanPasswort = "Gmylelqbln05+";

// NTP-Konfiguration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset = 3600;      // MEZ
const int daylightOffset = 3600;  // Sommerzeit

// Panel-Größe (einzeln)
#define PANEL_WIDTH 32
#define PANEL_HEIGHT 8

// Gesamtmatrix
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 16

// Anzahl LEDs insgesamt
#define NUM_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)

// Pins
#define DATA_PIN 25  // Du kannst beide Panels an einem Pin mit entsprechendem Layout verkabeln

// LED-Array
CRGB leds[NUM_LEDS];

// Matrix-Layout
// 2 Panels à 32x8 übereinander => 32x16 Gesamtmatrix
// Mapping: Serpentin-Zickzack, horizontal, first panel at top
cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, HORIZONTAL_ZIGZAG_MATRIX> ledMatrix(leds);

// LEDText-Objekt
cLEDText scrollText;

// Setup
void setup() {
  Serial.begin(115200);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(20);

  verbindeMitWLAN();
  konfiguriereZeit();

  scrollText.SetFont(ScrollFont5x7);  // Verwende die eingebaute 5x7-Schrift
  scrollText.Init(&ledMatrix, ledMatrix.Width(), ledMatrix.Height(), 0, 0);
  scrollText.SetTextDirection(CHAR_DIRECTION_LEFT);  // statisch mittig setzen wir später
}

// Loop
void loop() {
  zeigeZeit();
  delay(1000);
}

// WLAN verbinden
void verbindeMitWLAN() {
  WiFi.begin(wlanName, wlanPasswort);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWLAN verbunden.");
}

// Zeit konfigurieren
void konfiguriereZeit() {
  configTime(gmtOffset, daylightOffset, ntpServer);
}

// Zeit anzeigen
void zeigeZeit() {
  struct tm zeitinfo;
  if (!getLocalTime(&zeitinfo)) {
    Serial.println("Zeitabruf fehlgeschlagen.");
    return;
  }

  char uhrzeit[6];
  snprintf(uhrzeit, sizeof(uhrzeit), "%02d:%02d", zeitinfo.tm_hour, zeitinfo.tm_min);
  Serial.println(uhrzeit);

  // Bildschirm löschen
  ledMatrix.clear();

  // Text vorbereiten
  scrollText.setText((unsigned char*)uhrzeit, scrollText.CenterText((unsigned char*)uhrzeit));
  scrollText.setTextColor(CRGB::Green);

  // Text zeichnen
  scrollText.UpdateText();
  FastLED.show();
}
