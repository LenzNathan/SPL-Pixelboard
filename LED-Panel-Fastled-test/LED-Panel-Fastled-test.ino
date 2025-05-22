#include <FastLED.h>

#include <LEDMatrix.h>
#include <LEDText.h>
#include <FontMatrise.h>

// Change the next 6 defines to match your matrix type and size

#define LED_PIN 25
#define COLOR_ORDER GRB
#define CHIPSET WS2812B

#define MATRIX_WIDTH -32
#define MATRIX_HEIGHT -8
#define MATRIX_TYPE VERTICAL_ZIGZAG_MATRIX

int h = 0;
int m = 0;

cLEDMatrix<-MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;

cLEDText ClockTime;

unsigned char TxtDemo[] = { "   :  " };


void setup() {
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
  FastLED.setBrightness(10);
  FastLED.clear(true);
  delay(500);
  FastLED.show();

  ClockTime.SetFont(MatriseFontData);
  ClockTime.Init(&leds, leds.Width(), ClockTime.FontHeight() + 1, 0, 0);
  ClockTime.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  ClockTime.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
}


void loop() {
  ClockTime.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);

  ClockTime.UpdateText();  //Anzeigen
  for (int i = 0; i < 5; i++) {
    ClockTime.UpdateText();  //um eins verschieben, weil 6*5 = 30 pixel, von 32 Pixel macht 1 Pixel Abstand / Seite
  }

  String hours = String(h);
  if (h < 10) {
    hours = "0" + hours;
  }
  String minutes = String(m);
  if (m < 10) {
    minutes = "0" + minutes;
  }
  for (int i = 0; i < 2; i++) {
    TxtDemo[i + 1] = hours[i];
  }
  for (int i = 0; i < 2; i++) {
    TxtDemo[i + 4] = minutes[i];
  }

  FastLED.show();
  delay(1000);
  h++;
  m++;
}