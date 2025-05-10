#include <FastLED.h>

#define LED_PIN_UPPER 25
#define LED_PIN_LOWER 26

#define COLOR_ORDER GRB
#define CHIPSET WS2811

#define BRIGHTNESS 50

const uint8_t kMatrixWidth = 32;
const uint8_t kMatrixHeight = 16;

const bool kMatrixSerpentineLayout = true;
const bool kMatrixVertical = true;


#define TOTAL_LED_COUNT (kMatrixWidth * kMatrixHeight)
CRGB leds_plus_safety_pixel[TOTAL_LED_COUNT + 2];
CRGB* const ledsUpper(leds_plus_safety_pixel + 1);
CRGB* const ledsLower(leds_plus_safety_pixel + 1 + TOTAL_LED_COUNT / 2);



void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("BEGINNING");
  FastLED.addLeds<CHIPSET, LED_PIN_UPPER, COLOR_ORDER>(ledsUpper, TOTAL_LED_COUNT / 2).setCorrection(TypicalSMD5050);
  FastLED.addLeds<CHIPSET, LED_PIN_LOWER, COLOR_ORDER>(ledsLower, TOTAL_LED_COUNT / 2).setCorrection(TypicalSMD5050);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  for (int x = 0; x < 32; x++) {
    for (int y = 0; y < 16; y++) {
      setLed(x, y, x * 8, 255 - y * 16, 255);
      FastLED.show();
      delay(25);
    }
  }
  delay(5000);
  FastLED.clear();
  FastLED.show();
  for (int x = 0; x < 32; x++) {
    for (int y = 0; y < 16; y++) {
      setLed(x, y, x * 8, 255, 255 - y * 16);
      FastLED.show();
      delay(25);
    }
  }
  delay(5000);
  FastLED.clear();
  FastLED.show();
}

/*
 0|0 is at the lower left corner of the device (when the DHT looks up and the joystick is on the upper right)
 y is the axis pointing up
 x is the axis pointing to the right
 btw: with 8-pack i mean the panel's 8 pixels which are in the same column 
    - we don't mind if they originally initialized upwards or downwards - in the end it is always 8 pixels more (or less) of the original index
*/
void setLed(int x, int y, int H, int S, int V) {
  if (y >= 16 || x >= 32) {  //if we are out of range
    Serial.println("--- INDEX OUT OF RANGE ---");
    return;
  }

  int index = 0;
  bool upper = false;  //wether we need to use the upper / lower panel for setting the pixel

  if (y <= 7) {
    upper = false;         //lower Panel
    index = (31 - x) * 8;  //value for the 8 pixel packs
  } else {
    upper = true;   //we're on the upper panel
    index = x * 8;  //so we can directly multiply the eight packs into the index
    y = y - 8;      //we have to subtract that from y because we use the same Logic for the upper and the lower Panel in the next part
  }

  if (x % 2 == 0) {  //when it is zero, the original pixel index counts down, so we need to invert the y value. (This is true for both panels)
    index += 7 - y;
  } else {  // otherwise we may just add it up, thus the original pixel count direction looks the coordinate system's way (up)
    index += y;
  }

  // now we got the pixel index.

  if (upper) {
    ledsUpper[index] = CHSV(H, S, V);
  } else {
    ledsLower[index] = CHSV(H, S, V);
  }
}
