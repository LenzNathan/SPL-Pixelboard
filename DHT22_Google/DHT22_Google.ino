#include <WiFi.h>               // Für ESP32 (bei ESP8266: <ESP8266WiFi.h>)
#include <HTTPClient.h>
#include <FastLED.h>
#include <LEDMatrix.h>
#include <LEDText.h>
#include <FontMatrise.h>
#include <DHT.h>

// === WLAN-Zugangsdaten ===
const char* ssid = "iPhone_13_Pro_AEW";
const char* password = "Gmylelqbln05+";

// === Google Apps Script Web App URL ===
const char* scriptURL = "https://script.google.com/macros/s/AKfycbwZG59QOl3csuZ0E7lQc5xKED7dNksxxAN_HI9BNRobZjp86vhyjmDzgvIME4ulo_dT/exec";

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
unsigned char tempText[] = { "     C" };

// === LED-Matrix 2: Feuchtigkeitsanzeige
#define LED_PIN_HUM 26
#define MATRIX_WIDTH2 -32
#define MATRIX_HEIGHT2 8
#define MATRIX_TYPE2 VERTICAL_ZIGZAG_MATRIX
cLEDMatrix<MATRIX_WIDTH2, MATRIX_HEIGHT2, MATRIX_TYPE2> ledsHum;
cLEDText textHum;
unsigned char humText[] = { "     %" };

// === WLAN verbinden
void connectToWiFi() {
  Serial.print("Verbinde mit WLAN");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWLAN verbunden!");
}

// === Daten an Google Sheet senden
void sendToGoogleSheet(float temperature, float humidity) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(scriptURL);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"temperature\":" + String(temperature, 1) + 
                     ",\"humidity\":" + String(humidity, 1) + "}";

    int httpResponseCode = http.POST(payload);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    http.end();
  } else {
    Serial.println("Keine WLAN-Verbindung.");
  }
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  connectToWiFi();

  FastLED.addLeds<WS2812B, LED_PIN_TEMP, GRB>(ledsTemp[0], ledsTemp.Size());
  FastLED.addLeds<WS2812B, LED_PIN_HUM, GRB>(ledsHum[0], ledsHum.Size());

  FastLED.setBrightness(30);
  FastLED.clear(true);
  FastLED.show();

  textTemp.SetFont(MatriseFontData);
  textTemp.Init(&ledsTemp, ledsTemp.Width(), textTemp.FontHeight() + 1, 0, 0);
  textTemp.SetText((unsigned char*)tempText, sizeof(tempText) - 1);
  textTemp.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xFF, 0x00, 0x00); // Rot

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

  snprintf((char*)tempText, sizeof(tempText), "%5.1fC", temperature);
  textTemp.SetText(tempText, strlen((char*)tempText));
  textTemp.UpdateText();

  snprintf((char*)humText, sizeof(humText), "%5.1f%%", humidity);
  textHum.SetText(humText, strlen((char*)humText));
  textHum.UpdateText();

  for (int i = 0; i < 5; i++) {
    textHum.UpdateText();
    textTemp.UpdateText();
  }

  FastLED.show();

  sendToGoogleSheet(temperature, humidity);  // <-- Daten senden

  delay(1000);
}
