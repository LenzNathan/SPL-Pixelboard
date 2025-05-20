#include <WiFi.h>
#include <time.h>

// WLAN-Zugangsdaten
const char* wlanName = "**************************";
const char* wlanPasswort = "******************";

// NTP-Server und Zeitzone
const char* ntpServer = "time.google.com";
const long gmtOffsetInSeconds = 3600;      // GMT+1 (MEZ)
const int daylightOffsetInSeconds = 3600;  // Sommerzeit

// Initialisierung
void setup() {
  Serial.begin(9600);
  delay(1000);

  verbindeMitWLAN();
  konfiguriereZeit();

  Serial.println("Initialisierung abgeschlossen.");
}

// Hauptloop
void loop() {
  zeigeAktuelleZeit();
  delay(10000);  // Alle 10 Sekunden aktualisieren
}

// Verbindung mit dem WLAN herstellen
void verbindeMitWLAN() {
  Serial.print("Verbinde mit WLAN: ");
  Serial.println(wlanName);

  WiFi.begin(wlanName, wlanPasswort);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWLAN verbunden.");
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());
}

// Zeit vom NTP-Server konfigurieren
void konfiguriereZeit() {
  configTime(gmtOffsetInSeconds, daylightOffsetInSeconds, ntpServer);
  Serial.println("Zeitkonfiguration abgeschlossen.");
}

// Aktuelle Zeit ausgeben
void zeigeAktuelleZeit() {
  struct tm zeitinfo;

  if (!getLocalTime(&zeitinfo)) {
    Serial.println("Fehler beim Abrufen der Zeit.");
    return;
  }

  char zeichenkette[64];
  strftime(zeichenkette, sizeof(zeichenkette),
           "%A, %d.%m.%Y %H:%M:%S", &zeitinfo);

  Serial.print("Aktuelle Zeit: ");
  Serial.println(zeichenkette);
}