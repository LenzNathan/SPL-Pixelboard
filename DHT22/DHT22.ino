#include <FastLED.h>
#include <LEDMatrix.h>
#include <LEDText.h>
#include <FontMatrise.h>
#include <DHT.h>

// === DHT22 Sensor
#define DHTPIN 14
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// === LED-Matrix 1: Temperaturanzeige
#define LED_PIN_TEMP 25
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT -8
#define MATRIX_TYPE VERTICAL_ZIGZAG_MATRIX

cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> ledsTemp;
cLEDText textTemp;
unsigned char tempText[] = { "     " };  // z. B. "23.1C"

// === LED-Matrix 2: Feuchtigkeitsanzeige
#define LED_PIN_HUM 26
#define MATRIX_WIDTH2 -32
#define MATRIX_HEIGHT2 8
#define MATRIX_TYPE2 VERTICAL_ZIGZAG_MATRIX

cLEDMatrix<MATRIX_WIDTH2, MATRIX_HEIGHT2, MATRIX_TYPE2> ledsHum;
cLEDText textHum;
unsigned char humText[] = { "     " };  // z. B. "45.3%"

void setup() {
  Serial.begin(9600);
  dht.begin();

  // LED-Streifen initialisieren
  FastLED.addLeds<WS2812B, LED_PIN_TEMP, GRB>(ledsTemp[0], ledsTemp.Size());
  FastLED.addLeds<WS2812B, LED_PIN_HUM, GRB>(ledsHum[0], ledsHum.Size());
  FastLED.setBrightness(30);
  FastLED.clear(true);
  FastLED.show();

  // Temperaturanzeige konfigurieren
  textTemp.SetFont(MatriseFontData);
  textTemp.Init(&ledsTemp, ledsTemp.Width(), textTemp.FontHeight() + 1, 0, 0);
  textTemp.SetText((unsigned char*)tempText, sizeof(tempText) - 1);
  textTemp.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xFF, 0x00, 0x00); // Rot

  // Luftfeuchtigkeit konfigurieren
  textHum.SetFont(MatriseFontData);
  textHum.Init(&ledsHum, ledsHum.Width(), textHum.FontHeight() + 1, 0, 0);
  textHum.SetText((unsigned char*)humText, sizeof(humText) - 1);
  textHum.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0x00, 0x00, 0xFF); // Blau
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Fehler beim Lesen vom DHT22!");
    return;
  }

  Serial.print("Temperatur: ");
  Serial.print(temperature);
  Serial.print(" °C   |   Luftfeuchtigkeit: ");
  Serial.print(humidity);
  Serial.println(" %");

  // Temperatur-Text vorbereiten
  snprintf((char*)tempText, sizeof(tempText), "%.1fC", temperature);
  textTemp.SetText(tempText, strlen((char*)tempText));
  textTemp.UpdateText();

  // Feuchtigkeits-Text vorbereiten
  snprintf((char*)humText, sizeof(humText), "%.1f%%", humidity);
  textHum.SetText(humText, strlen((char*)humText));
  textHum.UpdateText();

  FastLED.show();
  delay(2000);
}
