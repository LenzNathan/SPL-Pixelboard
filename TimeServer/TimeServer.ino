#include <WiFi.h>
#include <time.h>
#include <FastLED.h>
#include <Joystick_neu.h>

// =========================
// WiFi-Zugangsdaten
// =========================
const char* ssid = "HUAWEI P9";
const char* password = "@htlanich@";

// =========================
// WS2812 Panel Konfiguration
// =========================
#define LED_PIN_OBEN 25
#define PANEL_WIDTH 32
#define PANEL_HEIGHT 16
#define NUM_LEDS PANEL_WIDTH * PANEL_HEIGHT

CRGB leds[NUM_LEDS];

// =========================
// Taster
// =========================
#define tasterPin 32
Joystick taster(tasterPin, 50);

// =========================
// FreeRTOS Tasks
// =========================
void TaskA(void *pvParameters);
void TaskB(void *pvParameters);

bool aTaskActive = true;
TaskHandle_t handle_a;
TaskHandle_t handle_b;

// =========================
// Setup
// =========================
void setup() {
  Serial.begin(9600);
  delay(100);

  // WLAN verbinden
  WiFi.begin(ssid, password);
  Serial.println("Verbinde mit WLAN...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }
  Serial.println("\nWLAN verbunden!");

  // Zeit abrufen
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  while (!getLocalTime(nullptr)) {
    Serial.print(".");
    delay(500);
  }

  // LEDs initialisieren
  FastLED.addLeds<NEOPIXEL, LED_PIN_OBEN>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  // Tasks
  xTaskCreate(TaskA, "TaskA", 4096, NULL, 2, &handle_a);
  xTaskCreate(TaskB, "TaskB", 2048, NULL, 2, &handle_b);

  vTaskSuspend(handle_a);
  vTaskSuspend(handle_b);
}

// =========================
// loop()
// =========================
void loop() {
  taster.aktualisieren();

  if (taster.LangerKlick()) {
    aTaskActive = !aTaskActive;
    if (aTaskActive) {
      vTaskSuspend(handle_b);
      vTaskResume(handle_a);
    } else {
      vTaskSuspend(handle_a);
      vTaskResume(handle_b);
    }
  }

  delay(1);
}

// =========================
// Hilfsfunktionen
// =========================

// Gibt die Uhrzeit als String zurück
String getTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "00:00";

  char buffer[6];
  strftime(buffer, sizeof(buffer), "%H:%M", &timeinfo);
  return String(buffer);
}

// Zeichnet eine einzelne Zahl (3x5 Pixel)
const uint8_t zahlen[10][5] = {
  {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
  {0b010, 0b110, 0b010, 0b010, 0b111}, // 1
  {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
  {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
  {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
  {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
  {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
  {0b111, 0b001, 0b010, 0b010, 0b010}, // 7
  {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
  {0b111, 0b101, 0b111, 0b001, 0b111}  // 9
};

// Setzt ein Pixel auf dem Panel
void setPixel(int x, int y, CRGB color) {
  if (x < 0 || x >= PANEL_WIDTH || y < 0 || y >= PANEL_HEIGHT) return;
  int index = y * PANEL_WIDTH + x;
  leds[index] = color;
}

// Zeichnet eine einzelne Zahl an x,y
void drawDigit(int digit, int offsetX, int offsetY, CRGB color) {
  for (int row = 0; row < 5; row++) {
    for (int col = 0; col < 3; col++) {
      if (zahlen[digit][row] & (1 << (2 - col))) {
        setPixel(offsetX + col, offsetY + row, color);
      }
    }
  }
}

// Zeichnet die Uhrzeit "HH:MM"
void drawTime(String timeStr) {
  FastLED.clear();
  int x = 1;

  for (int i = 0; i < timeStr.length(); i++) {
    char c = timeStr.charAt(i);
    if (c == ':') {
      setPixel(x, 1, CRGB::White);
      setPixel(x, 3, CRGB::White);
      x += 2;
    } else {
      drawDigit(c - '0', x, 0, CRGB::Blue);
      x += 4;
    }
  }

  FastLED.show();
}

// =========================
// Task A – Zeigt Uhrzeit
// =========================
void TaskA(void *pvParameters) {
  delay(100);
  for (;;) {
    String zeit = getTimeString();
    Serial.println("Zeit: " + zeit);

    Serial.println("Zeit: " + zeit);
    drawTime(zeit);
    delay(1000);
  }
}

// =========================
// Task B – Nur Textausgabe
// =========================
void TaskB(void *pvParameters) {
  delay(100);
  for (;;) {
    Serial.println("Task B läuft...");
    delay(3000);
  }
}
