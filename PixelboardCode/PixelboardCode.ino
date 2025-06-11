// ╔══════════════════════════════════════════════════════════════╗
// ║                      BIBLIOTHEKS-EINBINDUNGEN                ║
// ║    FastLED, LEDMatrix, LEDText, DHT-Sensor etc.              ║
// ╚══════════════════════════════════════════════════════════════╝

#include <Joystick_neu.h>
#include <FastLED.h>      // Bibliothek für die Ansteuerung von WS2812-LEDs
#include <LEDMatrix.h>    // Erweiterung für FastLED zur Arbeit mit 2D-Matrixen
#include <LEDText.h>      // Textausgabe auf LED-Matrix (Scroll, Anzeige, etc.)
#include <FontMatrise.h>  // Schriftart für LEDText
#include <DHT.h>          // Bibliothek zur Kommunikation mit DHT-Sensoren
#include <WiFi.h>
#include <vector>

// ╔══════════════════════════════════════════════════════════════╗
// ║                       Structs, Gamevars                      ║
// ║                           und Farben                         ║
// ╚══════════════════════════════════════════════════════════════╝

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

// ╔══════════════════════════════════════════════════════════════╗
// ║                      DHT22 SENSOR-AUSGABE                    ║
// ║         Temperatur (°C) und Luftfeuchtigkeit (%) anzeigen    ║
// ╚══════════════════════════════════════════════════════════════╝

// === DHT22 Sensor
#define DHTPIN 14          // GPIO-Pin, an dem der DHT22 angeschlossen ist
#define DHTTYPE DHT22      // Angabe des Sensortyps (DHT11, DHT22, etc.)
DHT dht(DHTPIN, DHTTYPE);  // Sensorobjekt erstellen


// === LED-Matrix 1: Temperaturanzeige
#define LED_PIN_TEMP 25                     // Datenpin für obere Matrix (Temperatur)
#define MATRIX_WIDTH 32                     // Matrixbreite (positive Zahl = linke Seite zuerst)
#define MATRIX_HEIGHT -8                    // Negative Höhe = Matrix ist gespiegelt in Y-Richtung
#define MATRIX_TYPE VERTICAL_ZIGZAG_MATRIX  // Verkabelung: vertikal, zickzackförmig

cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> ledsTemp;  // Matrixobjekt erstellen
cLEDText textTemp;                                              // Textobjekt für Temperaturanzeige
unsigned char tempText[] = { "     C" };                        // Platzhalter-Text für Temperatur

int h = 0;
int m = 0;
cLEDText ClockTime;
unsigned char TxtDemo[] = { "   :  " };

// === LED-Matrix 2: Feuchtigkeitsanzeige
#define LED_PIN_HUM 26                       // Datenpin für untere Matrix (Feuchtigkeit)
#define MATRIX_WIDTH2 -32                    // Negativer Wert = X-Richtung gespiegelt
#define MATRIX_HEIGHT2 8                     // Normale Höhe
#define MATRIX_TYPE2 VERTICAL_ZIGZAG_MATRIX  // Auch vertikale Zickzack-Verkabelung

cLEDMatrix<MATRIX_WIDTH2, MATRIX_HEIGHT2, MATRIX_TYPE2> ledsHum;  // Matrixobjekt für Feuchtigkeit
cLEDText textHum;                                                 // Textobjekt für Feuchtigkeit
unsigned char humText[] = { "     %" };                           // Platzhalter-Text für Feuchtigkeit

int day = 0;
int month = 0;
cLEDText Date;
unsigned char DateText[] = { "   .  " };


unsigned long newPanelDataTime = 0;  //--- LED PANEL STEUERUNG
#define TOTAL_LED_COUNT (32 * 16)


// ╔══════════════════════════════════════════════════════════════╗
// ║                         TASTER-EINGABE                       ║
// ║   Umschalten zwischen Tasks durch langen Tastendruck (Hold)  ║
// ╚══════════════════════════════════════════════════════════════╝


// Pin für den Taster
#define tasterPin 32
// Instanz der entprellten Tasterklasse
Joystick taster(tasterPin, 50);

void TaskA(void *pvParameters);
void TaskDHT22(void *pvParameters);
void TaskTime(void *pvParameters);
void TaskSnake(void *pvParameters);

int aTaskActive = 1;

TaskHandle_t handle_a;
TaskHandle_t handle_DHT22;
TaskHandle_t handle_Time;
TaskHandle_t handle_Snake;


// ╔══════════════════════════════════════════════════════════════╗
// ║                         Urhzeit                              ║
// ║                     Uhrzeit anzeigen, WLAN, ...              ║
// ╚══════════════════════════════════════════════════════════════╝
const char *wlanName = "iPhone_13 Pro_AEW";
const char *wlanPasswort = "Gmylelqbln05+";

// NTP-Konfiguration
const char *ntpServer = "pool.ntp.org";
const long gmtOffset = 3600;      // MEZ
const int daylightOffset = 3600;  // Sommerzeit




void setup() {
  randomSeed(analogRead(0));
  Serial.begin(9600);
  dht.begin();  // DHT-Sensor initialisieren

  // LEDs für Temperaturmatrix initialisieren
  FastLED.addLeds<WS2812B, LED_PIN_TEMP, GRB>(ledsTemp[0], ledsTemp.Size());

  // LEDs für Feuchtigkeitsmatrix initialisieren
  FastLED.addLeds<WS2812B, LED_PIN_HUM, GRB>(ledsHum[0], ledsHum.Size());

  FastLED.setBrightness(30);  // Helligkeit der LEDs setzen (0–255)
  FastLED.clear(true);        // Alle LEDs auf schwarz setzen (initial)
  FastLED.show();             // Änderungen an LED-Streifen anzeigen

  // === Temperaturanzeige konfigurieren
  textTemp.SetFont(MatriseFontData);                                            // Schriftart wählen
  textTemp.Init(&ledsTemp, ledsTemp.Width(), textTemp.FontHeight() + 1, 0, 0);  // Position + Matrix binden
  textTemp.SetText((unsigned char *)tempText, sizeof(tempText) - 1);            // Anfangstext setzen
  textTemp.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xFF, 0x00, 0x00);        // Textfarbe: Rot

  // === Feuchtigkeitsanzeige konfigurieren
  textHum.SetFont(MatriseFontData);                                         // Schriftart wählen
  textHum.Init(&ledsHum, ledsHum.Width(), textHum.FontHeight() + 1, 0, 0);  // Position + Matrix binden
  textHum.SetText((unsigned char *)humText, sizeof(humText) - 1);           // Anfangstext setzen
  textHum.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0x00, 0x00, 0xFF);     // Textfarbe: Blau

  ClockTime.SetFont(MatriseFontData);
  ClockTime.Init(&ledsTemp, ledsTemp.Width(), ClockTime.FontHeight() + 1, 0, 0);
  ClockTime.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  ClockTime.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);

  Date.SetFont(MatriseFontData);
  Date.Init(&ledsHum, ledsHum.Width(), Date.FontHeight() + 1, 0, 0);
  Date.SetText((unsigned char *)DateText, sizeof(DateText) - 1);
  Date.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);

  //------ FREE RTOS TASKS DEFINITION ------//
  Serial.println("Starte FreeRTOS Tasks");
  xTaskCreate(
    TaskA, "TaskA"  //name
    ,
    2048  // The stack size can be checked by calling `uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);`
    ,
    NULL  // Task parameter which can modify the task behavior. This must be passed as pointer to void.
    ,
    2  // Priority
    ,
    &handle_a);
  xTaskCreate(
    TaskDHT22,
    "TaskDHT22",
    4096,
    NULL,
    1,
    &handle_DHT22);
  xTaskCreate(
    TaskTime,
    "TaskTime",
    4096,
    NULL,
    1,
    &handle_Time);
  xTaskCreate(
    TaskSnake,
    "TaskSnake",
    12288,
    NULL,
    1,
    &handle_Snake);
  vTaskSuspend(handle_DHT22);
  vTaskSuspend(handle_Time);
  vTaskSuspend(handle_Snake);
  //vTaskSuspend(&handle_a);
}

void loop() {
  // Entprellten Tasterzustand abfragen
  if (aTaskActive != 0) {
    taster.aktualisieren();
  }
  
  if (taster.LangerKlick()) {
    Serial.println("Langer Klick erkannt!");
    aTaskActive++;

    switch (aTaskActive) {
      case 1:
        vTaskSuspend(handle_DHT22);  // Deaktiviere Task B
        vTaskSuspend(handle_Time);
        vTaskSuspend(handle_Snake);
        clear();
        delay(20);
        FastLED.show();
        vTaskResume(handle_a);  // Aktiviere Task A
        break;
      case 2:
        vTaskSuspend(handle_a);  // Deaktiviere Task A
        vTaskSuspend(handle_Time);
        vTaskSuspend(handle_Snake);
        clear();
        delay(20);
        FastLED.show();
        vTaskResume(handle_DHT22);  // Aktiviere Task B
        break;
      case 3:
        vTaskSuspend(handle_a);
        vTaskSuspend(handle_DHT22);
        vTaskSuspend(handle_Snake);
        clear();
        delay(20);
        FastLED.show();
        vTaskResume(handle_Time);
        break;
      case 4:
        vTaskSuspend(handle_Time);
        vTaskSuspend(handle_a);
        vTaskSuspend(handle_DHT22);
        clear();
        delay(20);
        FastLED.show();
        vTaskResume(handle_Snake);
        aTaskActive = 0;
        break;
    }
    FastLED.clear();
    newPanelDataTime = millis();
  }
  if (aTaskActive != 0 && millis() - newPanelDataTime > 20) {
    FastLED.show();
  }
  delay(1);
}

//Tasks

void TaskA(void *pvParameters) {
  delay(100);
  for (;;) {
    delay(3000);
    Serial.println("A");
  }
}

void TaskDHT22(void *pvParameters) {
  for (;;) {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Fehler beim Lesen vom DHT22!");
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    // Serielle Ausgabe
    Serial.print("Temperatur: ");
    Serial.print(temperature);
    Serial.print(" °C   |   Luftfeuchtigkeit: ");
    Serial.print(humidity);
    Serial.println(" %");

    // Temperaturanzeige vorbereiten
    newPanelDataTime = millis();
    snprintf((char *)tempText, sizeof(tempText), "%5.1fC", temperature);
    textTemp.SetText(tempText, strlen((char *)tempText));
    textTemp.UpdateText();

    // Feuchtigkeitsanzeige vorbereiten
    snprintf((char *)humText, sizeof(humText), "%5.1f%%", humidity);
    textHum.SetText(humText, strlen((char *)humText));
    textHum.UpdateText();

    // Animation für zentrierte Darstellung
    for (int i = 0; i < 5; i++) {
      textHum.UpdateText();
      textTemp.UpdateText();
    }
    vTaskDelay(pdMS_TO_TICKS(1000));  // 1 Sekunde warten
  }
}

void TaskTime(void *pvParameters) {
  verbindeMitWLAN(false);
  konfiguriereZeit();
  for (;;) {
    struct tm zeitinfo;
    if (!getLocalTime(&zeitinfo)) {
      Serial.println("Zeitabruf fehlgeschlagen.");
    }

    newPanelDataTime = millis();
    Date.SetText((unsigned char *)DateText, sizeof(DateText) - 1);
    ClockTime.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);

    ClockTime.UpdateText();  //Anzeigen
    Date.UpdateText();       //Anzeigen
    for (int i = 0; i < 5; i++) {
      ClockTime.UpdateText();  //um eins verschieben, weil 6*5 = 30 pixel, von 32 Pixel macht 1 Pixel Abstand / Seite
      Date.UpdateText();
    }

    String hours = String(h);
    if (h < 10) {
      hours = "0" + hours;
    }
    String minutes = String(m);
    if (m < 10) {
      minutes = "0" + minutes;
    }

    String dayString = String(day);
    if (day < 10) {
      dayString = "0" + dayString;
    }
    String monthString = String(month);
    if (month < 10) {
      monthString = "0" + monthString;
    }

    for (int i = 0; i < 2; i++) {
      TxtDemo[i + 1] = hours[i];
    }
    for (int i = 0; i < 2; i++) {
      TxtDemo[i + 4] = minutes[i];
    }

    for (int i = 0; i < 2; i++) {
      DateText[i + 1] = dayString[i];
    }
    for (int i = 0; i < 2; i++) {
      DateText[i + 4] = monthString[i];
    }
    delay(1000);
    h = zeitinfo.tm_hour;
    m = zeitinfo.tm_min;
    day = zeitinfo.tm_mday;
    month = zeitinfo.tm_mon + 1;
  }
}

void TaskSnake(void *pvParameters) {
  resetGame(false);
  for (;;) {
    taster.aktualisieren();
    bool tasterLastState = taster.istGedrueckt();
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

    //newPanelDataTime = millis();  //DAS DELAY IST WICHTIG! - ansonsten gibts probleme mit Taster.aktualisieren
    FastLED.show();
    delay(10);  //da die Methode den AD wandler benötigt und Fastled.show unterbrechen / stören könnte (glitches entstehen bei der Bildausgabe)

    if (gameover && !tasterLastState && taster.istGedrueckt()) {
      resetGame(false);
      gameover = false;
    }
  }
}


void verbindeMitWLAN(bool quiet) {
  WiFi.begin(wlanName, wlanPasswort);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (!quiet) {
      Serial.print(".");
    }
  }
  if (!quiet) {
    Serial.println("\nWLAN verbunden.");
  }
}

void konfiguriereZeit() {
  configTime(gmtOffset, daylightOffset, ntpServer);
}

//Game funcs
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
    ledsTemp(index) = CHSV(color.H, color.S, color.V);
  } else {
    ledsHum(index) = CHSV(color.H, color.S, color.V);
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