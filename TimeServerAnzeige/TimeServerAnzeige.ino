#include <WiFi.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

// WLAN-Zugangsdaten
const char* wlanName = "DEIN_WIFI_NAME";
const char* wlanPasswort = "DEIN_WIFI_PASSWORT";

// NTP-Server und Zeitzone
const char* ntpServer = "pool.ntp.org";
const long gmtOffset = 3600;       // GMT+1 (MEZ)
const int daylightOffset = 3600;   // Sommerzeit

// LED-Matrix Setup (16 hoch, 32 breit)
#define PIN_MATRIX 25

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(
  32, 16, PIN_MATRIX,
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB + NEO_KHZ800
);

void setup() {
  Serial.begin(115200);
  delay(1000);

  verbindeMitWLAN();
  konfiguriereZeit();

  matrix.begin();
  matrix.setBrightness(20);  // Helligkeit (0–255)
  matrix.setTextWrap(false);
  matrix.setTextColor(matrix.Color(0, 255, 0));  // Grün
  matrix.fillScreen(0);
  matrix.show();
}

void loop() {
  zeigeZeitAufMatrix();
  delay(1000);  // Jede Sekunde aktualisieren
}

void verbindeMitWLAN() {
  Serial.print("Verbinde mit WLAN: ");
  Serial.println(wlanName);

  WiFi.begin(wlanName, wlanPasswort);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWLAN verbunden.");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void konfiguriereZeit() {
  configTime(gmtOffset, daylightOffset, ntpServer);
  Serial.println("Zeitkonfiguration abgeschlossen.");
}

void zeigeZeitAufMatrix() {
  struct tm zeitinfo;

  if (!getLocalTime(&zeitinfo)) {
    Serial.println("Zeitabruf fehlgeschlagen.");
    return;
  }

  // Zeit formatieren (HH:MM)
  char uhrzeit[6];
  snprintf(uhrzeit, sizeof(uhrzeit), "%02d:%02d",
           zeitinfo.tm_hour, zeitinfo.tm_min);

  // Ausgabe im Seriellen Monitor
  Serial.print("Uhrzeit: ");
  Serial.println(uhrzeit);

  // Anzeige auf Matrix
  matrix.fillScreen(0);
  matrix.setCursor(1, 4);  // X,Y-Position für zentrierten Text
  matrix.print(uhrzeit);
  matrix.show();
}
