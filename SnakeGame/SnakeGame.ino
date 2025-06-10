#include <FastLED.h>
#include <Joystick_neu.h>
#include <vector>

#define LED_PIN_UPPER 25
#define LED_PIN_LOWER 26

#define COLOR_ORDER GRB
#define CHIPSET WS2811

#define BRIGHTNESS 50

const uint8_t kMatrixWidth = -32;
const uint8_t kMatrixHeight = 16;

const bool kMatrixSerpentineLayout = true;
const bool kMatrixVertical = true;


#define TOTAL_LED_COUNT (kMatrixWidth * kMatrixHeight)


CRGB ledsRawUpper[(TOTAL_LED_COUNT / 2) + 2];
CRGB ledsRawLower[(TOTAL_LED_COUNT / 2) + 2];

CRGB* const ledsUpper(ledsRawUpper + 1);
CRGB* const ledsLower(ledsRawLower + 1);

// Pins für den Joystick
#define tasterPin 32
#define xPin 34
#define yPin 35
#define joystickInverted false

// Instanz der entprellten Tasterklasse
Joystick taster(tasterPin, 50, xPin, yPin, joystickInverted);

// ---------------------------------- Game Vars ---------------------------------- //

bool gameover = false;
int gameoverBlinkPointDelay = 3000;
unsigned long gameoverBlinkTimer = 0;
bool gameoverPointScreen = true;
bool snakeOnSelf = false;

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
int snakeMoveDelay = 500;
int minSnakeMoveDelay = 100;
int stageCounter = 0;
unsigned long lastSnakeMove = 0;
std::vector<Position> snake;

//btw: das Spielfeld wurde auf:
//Y: 1-14, X: 1-30 eingeschränkt

//für das Punktesystem: (werte sind zimlich egal da sie eh zu begin in der ResetGame Funktion gelöscht werden)
int movementPoints = 0;
int movementSkillPoints = 0;
int applePoints = 0;
int appleSkillPoints = 0;
int legendaryPoints = 0;

//Farben
Color snakeHead = Color{ 150, 255, 255 };
Color snakeBody = Color{ 125, 255, 255 };
Color movementPointColor = Color{ 60, 255, 255 };
Color movementSkillPointColor = Color{ 60, 255, 150 };
Color applePointColor = Color{ 105, 255, 255 };
Color appleSkillPointColor = Color{ 105, 255, 150 };
Color legendaryColor = Color{ 0, 255, 255 };  // updaten --> Regenbogen
Color white = Color{ 0, 0, 255 };
Color red = Color{ 255, 255, 255 };
Color black = Color{ 0, 0, 0 };

int legendaryColorUpdateTime = 50;
unsigned long legendaryColorUpdateTimer = 0;

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
  resetGame(false);
}

void loop() {
  bool tasterLastState = taster.istGedrueckt();
  taster.aktualisieren();
  if (millis() - legendaryColorUpdateTimer > legendaryColorUpdateTime) {  //Farbe updaten
    legendaryColorUpdateTimer = millis();
    legendaryColor.H = legendaryColor.H + 5;
    if (legendaryColor.H > 255) {
      legendaryColor.H -= 255;
    }
  }
  if (!gameover) {                                            //make the Game run and check on death
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
    int actualSnakeDelay = snakeMoveDelay;
    if (taster.istGedrueckt()) {
      actualSnakeDelay = actualSnakeDelay * 2 / 3;
      if (actualSnakeDelay > minSnakeMoveDelay) {
        actualSnakeDelay = minSnakeMoveDelay;
      }
    }
    if (millis() - lastSnakeMove > actualSnakeDelay) {
      snakeDir = newDir;
      lastSnakeMove = millis();
      switch (snakeDir) {
        case Direction::up:
          snake.insert(snake.begin(), { snake[0].x, snake[0].y + 1 });
          setLed(snake[snake.size() - 1].x, snake[snake.size() - 1].y, black);  //setze das letzte Pixel der Schlange auf Schwarz
          snake.pop_back();
          break;
        case Direction::down:
          snake.insert(snake.begin(), { snake[0].x, snake[0].y - 1 });
          setLed(snake[snake.size() - 1].x, snake[snake.size() - 1].y, black);
          snake.pop_back();
          break;
        case Direction::right:
          snake.insert(snake.begin(), { snake[0].x + 1, snake[0].y });
          setLed(snake[snake.size() - 1].x, snake[snake.size() - 1].y, black);
          snake.pop_back();
          break;
        case Direction::left:
          snake.insert(snake.begin(), { snake[0].x - 1, snake[0].y });
          setLed(snake[snake.size() - 1].x, snake[snake.size() - 1].y, black);
          snake.pop_back();
          break;
      }
      if (snakeDir != Direction::none) {  //nur wenn wir wirklich gefahren sind
        setBorderPoints(true, false);     //einen movement point geben und anzeigen
      }
    }
    drawScreen();
    //apfel essen
    if (snake[0].x == apple.x && snake[0].y == apple.y) {
      snake.push_back(snake[snake.size() - 1]);  //den Letzten Schlangenpixel ein weiteres mal hinzufügen
      newApple();
      setBorderPoints(false, true);
    }

    if (snake[0].x > 30 || snake[0].x < 1 || snake[0].y > 14 || snake[0].y < 1) {  // check if the Snake crashed into the borders
      gameover = true;
      printScore();
    }

    for (int i = 1; i < snake.size(); i++) {  // check wether the snake crashed into itself
      if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
        setLed(snake[0].x, snake[0].y, red);
        gameover = true;
        printScore();
        snakeOnSelf = true;
      }
    }

  } else {  //action on gameover
    if (gameoverPointScreen) {
      if (millis() - gameoverBlinkTimer > gameoverBlinkPointDelay) {
        gameoverBlinkTimer = millis();
        gameoverPointScreen = !gameoverPointScreen;  // wir wollen ja das nächste mal den anderen Status Zeigen :)
      }
    } else {
      if (millis() - gameoverBlinkTimer > gameoverBlinkPointDelay / 3) {
        gameoverBlinkTimer = millis();
        gameoverPointScreen = !gameoverPointScreen;  // wir wollen ja das nächste mal den anderen Status Zeigen :)
      }
    }

    drawScreen();
    if (gameoverPointScreen) {
      setBorderPoints(false, false);
      if (snakeOnSelf) {
        setLed(snake[0].x, snake[0].y, red);
      }
    } else {
      reSetBorder(red);  // der Kopf soll schon auch angezeigt werden, nicht nur der Rest des Körpers
      if (legendaryPoints > 0) {
        setLed(snake[0].x, snake[0].y, Color{ legendaryColor.H + 20, legendaryColor.S, legendaryColor.V });
      } else {
        setLed(snake[0].x, snake[0].y, snakeHead);
      }
    }
  }

  FastLED.show();  //DAS DELAY IST WICHTIG! - ansonsten gibts probleme mit Taster.aktualisieren
  delay(10);       //da er den AD wandler benötigt und Fastled.show unterbrechen / stören könnte (glitches entstehen bei der Bildausgabe)

  if (gameover && !tasterLastState && taster.istGedrueckt()) {
    resetGame(false);
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
void setLed(int x, int y, Color color) {
  if (y >= 16 || x >= 32 || x < 0 || y < 0) {  //if we are out of range
    //Serial.println("--- INDEX OUT OF RANGE ---");
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
    ledsUpper[index] = CHSV(color.H, color.S, color.V);
  } else {
    ledsLower[index] = CHSV(color.H, color.S, color.V);
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
    int prevdelay = snakeMoveDelay;
    snakeMoveDelay = snakeMoveDelay * 4 / 5;  //10 stages bis
    if (snakeMoveDelay < minSnakeMoveDelay) {
      snakeMoveDelay = minSnakeMoveDelay;
    }
    if (!(prevdelay == snakeMoveDelay)) {
      stageCounter++;
      Serial.print("Stage: ");
      Serial.print(stageCounter);
      Serial.print(" with delay: ");
      Serial.println(snakeMoveDelay);
    }
  }
  if (movementSkillPoints >= 23) {
    movementSkillPoints = 0;
    appleSkillPoints++;
  }
  if (appleSkillPoints >= 23) {
    appleSkillPoints = 0;

    legendaryPoints++;
    resetGame(true);
    newDir = Direction::left;  //u may not rest, there are Monsters nearby :D
    rainbowWhiteTransition();
  }


  //left upper (apple)
  for (int y = 8; y < 16; y++) {
    if (applePoints > y - 8) {
      setLed(0, y, applePointColor);
    } else {
      setLed(0, y, white);
    }
  }
  for (int x = 1; x < 15; x++) {
    if (applePoints > x + 7) {
      setLed(x, 15, applePointColor);
    } else {
      setLed(x, 15, white);
    }
  }

  // left lower (movement)
  for (int y = 7; y >= 0; y--) {
    if (movementPoints > 7 - y) {
      setLed(0, y, movementPointColor);
    } else {
      setLed(0, y, white);
    }
  }
  for (int x = 1; x < 15; x++) {
    if (movementPoints > x + 7) {
      setLed(x, 0, movementPointColor);
    } else {
      setLed(x, 0, white);
    }
  }

  //right upper (apple skill)
  for (int x = 15; x < 31; x++) {
    if (appleSkillPoints > x - 15) {
      setLed(x, 15, appleSkillPointColor);
    } else {
      setLed(x, 15, white);
    }
  }
  for (int y = 15; y >= 8; y--) {
    if (appleSkillPoints > 31 - y) {
      setLed(31, y, appleSkillPointColor);
    } else {
      setLed(31, y, white);
    }
  }

  //right lower (movement skill)
  for (int x = 15; x < 31; x++) {
    if (movementSkillPoints > x - 15) {
      setLed(x, 0, movementSkillPointColor);
    } else {
      setLed(x, 0, white);
    }
  }
  for (int y = 0; y < 8; y++) {
    if (movementSkillPoints > y + 15) {
      setLed(31, y, movementSkillPointColor);
    } else {
      setLed(31, y, white);
    }
  }
}

void reSetBorder(Color color) {
  for (int y = 0; y < 16; y++) {
    setLed(0, y, color);
  }
  for (int y = 0; y < 16; y++) {
    setLed(31, y, color);
  }
  for (int x = 1; x < 31; x++) {
    setLed(x, 15, color);
  }
  for (int x = 1; x < 31; x++) {
    setLed(x, 0, color);
  }
}

void resetGame(bool inGameReset) {
  if (!inGameReset) {
    legendaryPoints = 0;
    snakeMoveDelay = 500;
    stageCounter = 0;
  }

  FastLED.clear();
  reSetBorder(white);
  resetSnake();
  snakeOnSelf = false;
  snakeDir = Direction::none;
  newDir = Direction::none;
  movementPoints = 0;
  movementSkillPoints = 0;
  applePoints = 0;
  appleSkillPoints = 0;
  newApple();
  Serial.println("Resetted");
}

void resetSnake() {
  snake.clear();
  snake.push_back({ 14, 7 });
  snake.push_back({ 15, 7 });
  snake.push_back({ 16, 7 });
  if (legendaryPoints > 3) {
    for (int i = 3; i < legendaryPoints; i++) {
      snake.push_back({ 16, 7 });  //snake staut sich an dem Punkt
    }
  }
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

void drawScreen() {
  setBorderPoints(false, false);
  //draw Snake
  if (legendaryPoints <= 0) {
    setLed(snake[0].x, snake[0].y, snakeHead);
    for (int i = 1; i < snake.size(); i++) {
      setLed(snake[i].x, snake[i].y, snakeBody);
    }
  } else {                                                                                               //der erste legPixel benötigt einen offset, --> erkennen wohin die Schlange fährt
    setLed(snake[0].x, snake[0].y, Color{ legendaryColor.H + 20, legendaryColor.S, legendaryColor.V });  //wir dürfen 125 einf. dazuaddieren, CHSV rechnet für uns auf einen gültigen Wert um
    for (int i = 1; i < legendaryPoints; i++) {
      setLed(snake[i].x, snake[i].y, legendaryColor);
    }
    for (int i = legendaryPoints; i < snake.size(); i++) {  //wenn legendaryPoints < Snakesize (die Snake wird ja im Laufe der Zeit länger) muss der Körper gezeichnet werden
      setLed(snake[i].x, snake[i].y, snakeBody);
    }
  }
  //draw apple
  setLed(apple.x, apple.y, applePointColor);
}

void rainbowWhiteTransition() {
  delay(500);  // delays before and after for better orientation
  for (int i = 16; i >= -33; i--) {
    if (i < 0) {  // just when the Animation already has overwritten the prev. State
      clear();
      drawScreen();  //draw next screen (again)
    }
    whiteUp(i);
    for (int x = 0; x < 32; x++) {
      setLed(x, i + 16, white);
    }
    whiteUpInverted(i + 17);
    FastLED.show();
    delay(50);
  }
  delay(500);
}

void whiteUp(int startheight) {
  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 32; x++) {
      setLed(x, y + startheight, Color{ x * 8, 255 - y * 16, 255 });
    }
  }
}

void whiteUpInverted(int startheight) {
  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 32; x++) {
      setLed(x, y + startheight, Color{ x * 8, y * 16, 255 });  // we dont invert the hue for the same Colors to appear on the same position after the Transition
    }
  }
}

void clear() {
  for (int x = 0; x < 32; x++) {
    for (int y = 0; y < 16; y++) {
      setLed(x, y, black);
    }
  }
}

void printScore() {
  String begin = (String)(calculateScore());
  int dotsNeeded = begin.length() / 3;
  if (begin.length() % 3 == 0) {
    dotsNeeded--;
  }

  String result = "";
  while (dotsNeeded > 0) {
    result = " " + begin.substring(begin.length() - 3) + "" + result;
    begin = begin.substring(0, begin.length() - 3);
    dotsNeeded--;
  }

  result = begin + "" + result;
  Serial.println("+-------------------------------+");
  Serial.print("| Score: \t");
  Serial.println(result);
  Serial.println("+-------------------------------+");
}

int calculateScore() {
  return (5 * movementPoints + 10 * 20 * movementSkillPoints + 50 * 20 * applePoints + 100 * 20 * 20 * appleSkillPoints + 500 * 20 * 20 * 20 * legendaryPoints);
}
