#include <FastLED.h>            // Bibliothek für die Ansteuerung von WS2812-LEDs
#include <LEDMatrix.h>          // Erweiterung für FastLED zur Arbeit mit 2D-Matrixen
#include <LEDText.h>            // Textausgabe auf LED-Matrix (Scroll, Anzeige, etc.)
#include <FontMatrise.h>        // Schriftart für LEDText
#include <DHT.h>                // Bibliothek zur Kommunikation mit DHT-Sensoren

// === DHT22 Sensor
#define DHTPIN 14               // GPIO-Pin, an dem der DHT22 angeschlossen ist
#define DHTTYPE DHT22           // Angabe des Sensortyps (DHT11, DHT22, etc.)
DHT dht(DHTPIN, DHTTYPE);       // Sensorobjekt erstellen

// === LED-Matrix 1: Temperaturanzeige
#define LED_PIN_TEMP 25         // Datenpin für obere Matrix (Temperatur)
#define MATRIX_WIDTH 32         // Matrixbreite (positive Zahl = linke Seite zuerst)
#define MATRIX_HEIGHT -8        // Negative Höhe = Matrix ist gespiegelt in Y-Richtung
#define MATRIX_TYPE VERTICAL_ZIGZAG_MATRIX // Verkabelung: vertikal, zickzackförmig

cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> ledsTemp;  // Matrixobjekt erstellen
cLEDText textTemp;               // Textobjekt für Temperaturanzeige
unsigned char tempText[] = { "     C" };  // Platzhalter-Text für Temperatur

// === LED-Matrix 2: Feuchtigkeitsanzeige
#define LED_PIN_HUM 26          // Datenpin für untere Matrix (Feuchtigkeit)
#define MATRIX_WIDTH2 -32       // Negativer Wert = X-Richtung gespiegelt
#define MATRIX_HEIGHT2 8        // Normale Höhe
#define MATRIX_TYPE2 VERTICAL_ZIGZAG_MATRIX // Auch vertikale Zickzack-Verkabelung

cLEDMatrix<MATRIX_WIDTH2, MATRIX_HEIGHT2, MATRIX_TYPE2> ledsHum;  // Matrixobjekt für Feuchtigkeit
cLEDText textHum;               // Textobjekt für Feuchtigkeit
unsigned char humText[] = { "     %" };  // Platzhalter-Text für Feuchtigkeit

void setup() {
  Serial.begin(9600);           // Serielle Schnittstelle starten für Debug-Ausgaben
  dht.begin();                  // DHT-Sensor initialisieren

  // LEDs für Temperaturmatrix initialisieren
  FastLED.addLeds<WS2812B, LED_PIN_TEMP, GRB>(ledsTemp[0], ledsTemp.Size());

  // LEDs für Feuchtigkeitsmatrix initialisieren
  FastLED.addLeds<WS2812B, LED_PIN_HUM, GRB>(ledsHum[0], ledsHum.Size());

  FastLED.setBrightness(30);   // Helligkeit der LEDs setzen (0–255)
  FastLED.clear(true);         // Alle LEDs auf schwarz setzen (initial)
  FastLED.show();              // Änderungen an LED-Streifen anzeigen

  // === Temperaturanzeige konfigurieren
  textTemp.SetFont(MatriseFontData);                              // Schriftart wählen
  textTemp.Init(&ledsTemp, ledsTemp.Width(), textTemp.FontHeight() + 1, 0, 0); // Position + Matrix binden
  textTemp.SetText((unsigned char*)tempText, sizeof(tempText) - 1);            // Anfangstext setzen
  textTemp.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xFF, 0x00, 0x00);       // Textfarbe: Rot

  // === Feuchtigkeitsanzeige konfigurieren
  textHum.SetFont(MatriseFontData);                               // Schriftart wählen
  textHum.Init(&ledsHum, ledsHum.Width(), textHum.FontHeight() + 1, 0, 0);     // Position + Matrix binden
  textHum.SetText((unsigned char*)humText, sizeof(humText) - 1);              // Anfangstext setzen
  textHum.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0x00, 0x00, 0xFF);       // Textfarbe: Blau
}

void loop() {
  float temperature = dht.readTemperature();  // Temperatur in °C vom Sensor lesen
  float humidity = dht.readHumidity();        // Luftfeuchtigkeit in % vom Sensor lesen

  if (isnan(temperature) || isnan(humidity)) { // Prüfen ob gültige Werte empfangen wurden
    Serial.println("Fehler beim Lesen vom DHT22!");
    return;             // Frühzeitiger Abbruch, wenn ungültige Daten
  }

  // Serielle Ausgabe zur Kontrolle
  Serial.print("Temperatur: ");
  Serial.print(temperature);
  Serial.print(" °C   |   Luftfeuchtigkeit: ");
  Serial.print(humidity);
  Serial.println(" %");

  // === Temperaturanzeige vorbereiten
  snprintf((char*)tempText, sizeof(tempText), "%5.1fC", temperature); // Text formatieren: z. B. " 23.4C"
  textTemp.SetText(tempText, strlen((char*)tempText));                // Textobjekt aktualisieren
  textTemp.UpdateText();                                              // Text einmal aktualisieren

  // === Feuchtigkeitsanzeige vorbereiten
  snprintf((char*)humText, sizeof(humText), "%5.1f%%", humidity);     // Text formatieren: z. B. " 45.6%"
  textHum.SetText(humText, strlen((char*)humText));                   // Textobjekt aktualisieren
  textHum.UpdateText();                                               // Text einmal aktualisieren

  // Text mehrfach "updaten", um zentriertes Erscheinen zu ermöglichen
  for (int i = 0; i < 5; i++) {
    textHum.UpdateText();
    textTemp.UpdateText();
  }

  FastLED.show();      // Alles auf den LEDs anzeigen
  delay(1000);         // 1 Sekunde warten bis zur nächsten Messung
}
