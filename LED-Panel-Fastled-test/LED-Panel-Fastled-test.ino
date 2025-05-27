#include <FastLED.h>
#include <LEDMatrix.h>
#include <DHT.h>

#define WIDTH        32
#define HEIGHT       8
#define CHIPSET      WS2812B
#define COLOR_ORDER  GRB
#define BRIGHTNESS   50

#define TEMP_PIN     26
#define HUM_PIN      25

#define DHTPIN       14
#define DHTTYPE      DHT22
DHT dht(DHTPIN, DHTTYPE);

// Matrix-Buffer
CRGB ledsTemp[WIDTH * HEIGHT];
CRGB ledsHum[WIDTH * HEIGHT];

// Zahlendarstellung 3x5 Pixel
const byte digits[10][5] = {
  {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
  {0b010, 0b110, 0b010, 0b010, 0b111}, // 1
  {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
  {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
  {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
  {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
  {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
  {0b111, 0b001, 0b010, 0b100, 0b100}, // 7
  {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
  {0b111, 0b101, 0b111, 0b001, 0b111}  // 9
};

// Zeichenfunktion: 1 Ziffer
void drawDigit(CRGB* leds, int digit, int xOffset, int yOffset, CRGB color) {
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 3; x++) {
      if (digits[digit][y] & (1 << (2 - x))) {
        leds[(y + yOffset) * WIDTH + (x + xOffset)] = color;
      }
    }
  }
}

// Löscht den Puffer
void clearLeds(CRGB* leds) {
  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    leds[i] = CRGB::Black;
  }
}

// Anzeige: Zwei Ziffern + ° oder %
void showValue(CRGB* leds, int value, bool isTemp) {
  clearLeds(leds);

  int v1 = value / 10;
  int v2 = value % 10;

  drawDigit(leds, v1, 1, 1, isTemp ? CRGB::Red : CRGB::Blue);
  drawDigit(leds, v2, 6, 1, isTemp ? CRGB::Red : CRGB::Blue);  // Abstand verbessert

  // Symbole setzen
  if (isTemp) {
    leds[1] = CRGB::Red;
    leds[WIDTH * 0 + 11] = CRGB::Red;  // ° weiter rechts
  } else {
    leds[WIDTH * 0 + 11] = CRGB::Blue; // % Punkt
    leds[WIDTH * 1 + 10] = CRGB::Blue;
  }
}


void setup() {
  Serial.begin(9600);
  dht.begin();

  FastLED.addLeds<CHIPSET, TEMP_PIN, COLOR_ORDER>(ledsTemp, WIDTH * HEIGHT);
  FastLED.addLeds<CHIPSET, HUM_PIN, COLOR_ORDER>(ledsHum, WIDTH * HEIGHT);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Sensorfehler!");
    return;
  }

  Serial.print("Temperatur: ");
  Serial.print(temp);
  Serial.print(" °C, Luftfeuchtigkeit: ");
  Serial.print(hum);
  Serial.println(" %");

  showValue(ledsTemp, (int)temp, true);
  showValue(ledsHum, (int)hum, false);

  FastLED.show();
  delay(2000);
}
