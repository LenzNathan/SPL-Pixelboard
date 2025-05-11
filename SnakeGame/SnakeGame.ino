#include <FastLED.h>
#include <Joystick_neu.h>
#include <vector>

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
// CRGB leds_plus_safety_pixel[TOTAL_LED_COUNT + 2];
// CRGB* const ledsUpper(leds_plus_safety_pixel + 1);
// CRGB* const ledsLower(leds_plus_safety_pixel + 1 + TOTAL_LED_COUNT / 2);

CRGB ledsRawUpper[(TOTAL_LED_COUNT / 2) + 2];
CRGB ledsRawLower[(TOTAL_LED_COUNT / 2) + 2];

CRGB* const ledsUpper(ledsRawUpper + 1);
CRGB* const ledsLower(ledsRawLower + 1);

// Pins für den Joystick
#define tasterPin 32
#define xPin 34
#define yPin 35

// Instanz der entprellten Tasterklasse
Joystick taster(tasterPin, 50, xPin, yPin);

//Game Vars

bool gameover = false;

enum Direction {
  none,
  up,
  right,
  down,
  left
};

struct Position {
  int x;
  int y;
};

Position apple = Position{ random(1, 15), random(1, 30) };
Direction snakeDir = Direction::none;
int snakeMoveDelay = 500;
unsigned long lastSnakeMove = 0;
std::vector<Position> snake;
//das Spielfeld wurde auf:
//Y: 1-14, X: 1-30 eingeschränkt


void setup() {
  randomSeed(analogRead(0));
  Serial.begin(9600);
  delay(1000);
  Serial.println("BEGINNING");
  FastLED.addLeds<CHIPSET, LED_PIN_UPPER, COLOR_ORDER>(ledsUpper, TOTAL_LED_COUNT / 2).setCorrection(TypicalSMD5050);
  FastLED.addLeds<CHIPSET, LED_PIN_LOWER, COLOR_ORDER>(ledsLower, TOTAL_LED_COUNT / 2).setCorrection(TypicalSMD5050);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 2000);  // 5V, 2A
  FastLED.setBrightness(BRIGHTNESS);
  //init Snake
  resetGame();
}

void loop() {
  taster.aktualisieren();
  if (!gameover) {                                            //make the Game run
    if (taster.getX() > 4094 && snakeDir != Direction::up) {  //the x - Axis of the joystick is like the Matrix inverted y axis
      snakeDir = Direction::down;
    }
    if (taster.getX() <= 1 && snakeDir != Direction::down) {  //this is why we have that logic here
      snakeDir = Direction::up;
    }
    if (taster.getY() > 4094 && snakeDir != Direction::right) {
      snakeDir = Direction::left;
    }
    if (taster.getY() <= 1 && snakeDir != Direction::left) {
      snakeDir = Direction::right;
    }
    if (snakeDir != Direction::none && millis() - lastSnakeMove > snakeMoveDelay) {
      lastSnakeMove = millis();
      switch (snakeDir) {
        case Direction::up:
          snake.insert(snake.begin(), { snake[0].x, snake[0].y + 1 });
          setLed(snake[snake.size() - 1].x, snake[snake.size() - 1].y, 0, 0, 0);  //setze das letzte Pixel der Schlange auf Schwarz
          snake.pop_back();
          break;
        case Direction::down:
          snake.insert(snake.begin(), { snake[0].x, snake[0].y - 1 });
          setLed(snake[snake.size() - 1].x, snake[snake.size() - 1].y, 0, 0, 0);
          snake.pop_back();
          break;
        case Direction::right:
          snake.insert(snake.begin(), { snake[0].x + 1, snake[0].y });
          setLed(snake[snake.size() - 1].x, snake[snake.size() - 1].y, 0, 0, 0);
          snake.pop_back();
          break;
        case Direction::left:
          snake.insert(snake.begin(), { snake[0].x - 1, snake[0].y });
          setLed(snake[snake.size() - 1].x, snake[snake.size() - 1].y, 0, 0, 0);
          snake.pop_back();
          break;
      }
    }
    //draw Snake
    setLed(snake[0].x, snake[0].y, 150, 255, 255);
    for (int i = 1; i < snake.size(); i++) {
      setLed(snake[i].x, snake[i].y, 125, 255, 255);
    }
    //draw apple
    setLed(apple.x, apple.y, 200, 255, 255);
  }

  if (snake[0].x > 30 || snake[0].x < 1 || snake[0].y > 14 || snake[0].y < 1) {
    setBorder(255, 255, 255);  //red
    gameover = true;
  }

  FastLED.show();  //DAS DELAY IST WICHTIG! - ansonsten gibts probleme mit Taster.aktualisieren
  delay(10);       //da er den AD wandler benötigt und Fastled.show unterbrechen / stören könnte (glitches entstehen bei der Bildausgabe)

  if (taster.istGedrueckt()) {
    resetGame();
    gameover = false;
  }
}

/*
 0|0 is at the lower left corner of the device (when the DHT looks up and the joystick is on the upper right)
 y is the axis pointing up
 x is the axis pointing to the right
 btw: with 8-pack i mean the panel's 8 pixels which are in the same column 
    - we don't mind if they originally initialized upwards or downwards - in the end it is always 8 pixels more (or less) of the original index
*/
void setLed(int x, int y, int H, int S, int V) {
  if (y >= 16 || x >= 32 || x < 0 || y < 0) {  //if we are out of range
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
  if (index < 0 || index >= TOTAL_LED_COUNT / 2) {
    Serial.print("Invalid index!");
    Serial.println(index);
    return;
  }

  if (upper) {
    ledsUpper[index] = CHSV(H, S, V);
  } else {
    ledsLower[index] = CHSV(H, S, V);
  }
}

void setBorder(int H, int S, int V) {
  for (int y = 0; y < 16; y++) {
    setLed(0, y, H, S, V);
  }
  for (int y = 0; y < 16; y++) {
    setLed(31, y, H, S, V);
  }
  for (int x = 1; x < 31; x++) {
    setLed(x, 15, H, S, V);
  }
  for (int x = 1; x < 31; x++) {
    setLed(x, 0, H, S, V);
  }
}

void resetGame() {
  FastLED.clear();
  setBorder(0, 0, 255);
  snake.clear();
  snake.push_back({ 14, 7 });
  snake.push_back({ 15, 7 });
  snake.push_back({ 16, 7 });
  snakeMoveDelay = 500;
  snakeDir = Direction::none;
  apple = Position{ random(1, 30), random(1, 15) };
}

void newApple() {
  bool granted = false;
   while (!granted) {
    for (int i = 0; i < snake.size(); i++) {
      if(apple.x != snake[i].x && apple.y != snake[i].y){

      }
    }
  }
}