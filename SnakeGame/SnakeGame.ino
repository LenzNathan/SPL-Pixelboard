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
#define joystickInverted true

// Instanz der entprellten Tasterklasse
Joystick taster(tasterPin, 50, xPin, yPin, joystickInverted);

// ---------------------------------- Game Vars ---------------------------------- //

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

struct Color {
  int H;
  int S;
  int V;
};


Position apple = Position{ random(1, 15), random(1, 30) };
Direction snakeDir = Direction::none;
Direction newDir = Direction::none;
int snakeMoveDelay = 250;
unsigned long lastSnakeMove = 0;
std::vector<Position> snake;
Color snakeHead = Color{
  150, 255, 255
};
Color snakeBody = Color{
  125, 255, 255
};
//das Spielfeld wurde auf:
//Y: 1-14, X: 1-30 eingeschränkt

//für das Punktesystem:
int movementPoints = 0;
Color movementPointColor = Color{
  60, 255, 255
};
int movementSkillPoints = 0;
Color movementSkillPointColor = Color{
  60, 255, 150
};
int applePoints = 0;
Color applePointColor = Color{
  105, 255, 255
};
int appleSkillPoints = 0;
Color appleSkillPointColor = Color{
  105, 255, 150
};

int legendaryPoints = 0;
Color legendary = Color{ // updaten --> Regenbogen
                         0, 255, 255
};


Color white = Color{
  0, 0, 255
};

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
      newDir = Direction::down;
    }
    if (taster.getX() <= 1 && snakeDir != Direction::down) {  //this is why we have that logic here
      newDir = Direction::up;
    }
    if (taster.getY() > 4094 && snakeDir != Direction::right) {
      newDir = Direction::left;
    }
    if (taster.getY() <= 1 && snakeDir != Direction::left && snakeDir != Direction::none) {  //after reset --> Status none, we may not move right (because the snake looks left)
      newDir = Direction::right;
    }
    //move Snake
    if (millis() - lastSnakeMove > snakeMoveDelay) {
      snakeDir = newDir;
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
      if (snakeDir != Direction::none) {  //nur wenn wir wirklich gefahren sind
        setBorderPoints(true, false);     //einen movement point geben und anzeigen
      }
    }
    //draw Snake
    setLed(snake[0].x, snake[0].y, snakeHead.H, snakeHead.S, snakeHead.V);
    for (int i = 1; i < snake.size(); i++) {
      setLed(snake[i].x, snake[i].y, snakeBody.H, snakeBody.S, snakeBody.V);
    }
    //draw apple
    setLed(apple.x, apple.y, applePointColor.H, applePointColor.S, applePointColor.V);
    //apfel essen
    if (snake[0].x == apple.x && snake[0].y == apple.y) {
      snake.push_back(snake[snake.size() - 1]);  //den Letzten Schlangenpixel ein weiteres mal hinzufügen
      newApple();
      setBorderPoints(false, true);
    }
  }

  if (snake[0].x > 30 || snake[0].x < 1 || snake[0].y > 14 || snake[0].y < 1) {  // check if the Snake crashed into the borders
    gameover = true;
  }

  for (int i = 1; i < snake.size(); i++) {  // check wether the snake crashed into itself
    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
      setLed(snake[0].x, snake[0].y, 255, 255, 255);
      gameover = true;
    }
  }

  if (gameover) {                //action on gameover
    reSetBorder(255, 255, 255);  //red
  }

  FastLED.show();  //DAS DELAY IST WICHTIG! - ansonsten gibts probleme mit Taster.aktualisieren
  delay(10);       //da er den AD wandler benötigt und Fastled.show unterbrechen / stören könnte (glitches entstehen bei der Bildausgabe)

  if (gameover && taster.istGedrueckt()) {
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

void setBorderPoints(bool incrementMovementPoints, bool incrementApplePoints) {
  if (incrementMovementPoints) {
    movementPoints++;
  }
  if (incrementApplePoints) {
    applePoints++;
  }
  if (movementPoints >= 23) {
    movementPoints = 0;
    movementSkillPoints++;
    applePoints++;
  }
  if (applePoints >= 23) {
    applePoints = 0;
    appleSkillPoints++;
  }
  if (movementSkillPoints >= 23) {
    movementSkillPoints = 0;
    appleSkillPoints++;
  }
  if (appleSkillPoints >= 23) {
    appleSkillPoints = 0;

    legendaryPoints++;
    //reset stuff and give snake upgrade
  }


  //left upper (apple)
  for (int y = 8; y < 16; y++) {
    if (applePoints > y - 8) {
      setLed(0, y, applePointColor.H, applePointColor.S, applePointColor.V);
    } else {
      setLed(0, y, white.H, white.S, white.V);
    }
  }
  for (int x = 1; x < 15; x++) {
    if (applePoints > x + 7) {
      setLed(x, 15, applePointColor.H, applePointColor.S, applePointColor.V);
    } else {
      setLed(x, 15, white.H, white.S, white.V);
    }
  }

  // left lower (movement)
  for (int y = 7; y >= 0; y--) {
    if (movementPoints > 7 - y) {
      setLed(0, y, movementPointColor.H, movementPointColor.S, movementPointColor.V);
    } else {
      setLed(0, y, white.H, white.S, white.V);
    }
  }
  for (int x = 1; x < 15; x++) {
    if (movementPoints > x + 7) {
      setLed(x, 0, movementPointColor.H, movementPointColor.S, movementPointColor.V);
    } else {
      setLed(x, 0, white.H, white.S, white.V);
    }
  }

  //right upper (apple skill)
  for (int x = 15; x < 31; x++) {
    if (appleSkillPoints > x - 15) {
      setLed(x, 15, appleSkillPointColor.H, appleSkillPointColor.S, appleSkillPointColor.V);
    } else {
      setLed(x, 15, white.H, white.S, white.V);
    }
  }
  for (int y = 15; y >= 8; y--) {
    if (appleSkillPoints > 31 - y) {
      setLed(31, y, appleSkillPointColor.H, appleSkillPointColor.S, appleSkillPointColor.V);
    } else {
      setLed(31, y, white.H, white.S, white.V);
    }
  }

  //right lower (movement skill)
  for (int x = 15; x < 31; x++) {
    if (movementSkillPoints > x - 15) {
      setLed(x, 0, movementSkillPointColor.H, movementSkillPointColor.S, movementSkillPointColor.V);
    } else {
      setLed(x, 0, white.H, white.S, white.V);
    }
  }
  for (int y = 0; y < 8; y++) {
    if (movementSkillPoints > y + 15) {
      setLed(31, y, movementSkillPointColor.H, movementSkillPointColor.S, movementSkillPointColor.V);
    } else {
      setLed(31, y, white.H, white.S, white.V);
    }
  }
}

void reSetBorder(int H, int S, int V) {
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
  reSetBorder(0, 0, 255);
  snake.clear();
  snake.push_back({ 14, 7 });
  snake.push_back({ 15, 7 });
  snake.push_back({ 16, 7 });
  snakeMoveDelay = 250;
  snakeDir = Direction::none;
  newDir = Direction::none;
  movementPoints = 0;
  movementSkillPoints = 15;
  applePoints = 0;
  appleSkillPoints = 15;
  newApple();
}

void newApple() {
  bool granted = false;
  while (!granted) {
    apple = Position{ random(1, 30), random(1, 15) };
    granted = true;
    for (int i = 0; i < snake.size(); i++) {
      if (apple.x == snake[i].x && apple.y == snake[i].y) {
        granted = false;
      }
    }
  }
}

void whiteUp(int startheight) {
  for (int x = 0; x < 32; x++) {
    for (int y = startheight; y + startheight < 16; y++) {
      setLed(x, y, x * 8, 255 - y * 16, 255);
    }
  }
}