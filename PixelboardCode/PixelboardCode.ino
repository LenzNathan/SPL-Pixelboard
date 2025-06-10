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

// ╔══════════════════════════════════════════════════════════════╗
// ║                            Structs                           ║
// ║                           und Farben                         ║
// ╚══════════════════════════════════════════════════════════════╝

struct Color {
  int H;
  int S;
  int V;
};

struct Position {
  int x;
  int y;
};

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

int aTaskActive = 1;

TaskHandle_t handle_a;
TaskHandle_t handle_DHT22;
TaskHandle_t handle_Time;


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
  Serial.println("Starte FreeRTOS A-B Test");
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

  vTaskSuspend(handle_DHT22);
  vTaskSuspend(handle_Time);
  //vTaskSuspend(&handle_a);
}

void loop() {
  // Entprellten Tasterzustand abfragen
  taster.aktualisieren();
  bool tasterGedrueckt = taster.istGedrueckt();

  if (taster.LangerKlick()) {
    Serial.println("Langer Klick erkannt!");
    aTaskActive++;

    switch (aTaskActive) {
      case 1:
        vTaskSuspend(handle_DHT22);  // Deaktiviere Task B
        vTaskSuspend(handle_Time);
        vTaskResume(handle_a);  // Aktiviere Task A
        break;
      case 2:
        vTaskSuspend(handle_a);  // Deaktiviere Task A
        vTaskSuspend(handle_Time);
        vTaskResume(handle_DHT22);  // Aktiviere Task B
        break;
      case 3:
        vTaskSuspend(handle_a);
        vTaskSuspend(handle_DHT22);
        clear();
        delay(20);
        FastLED.show();
        verbindeMitWLAN();
        konfiguriereZeit();
        vTaskResume(handle_Time);
        aTaskActive = 0;
        break;
    }
    FastLED.clear();
    newPanelDataTime = millis();
  }
  if (millis() - newPanelDataTime > 20) {
    FastLED.show();
  }
  delay(20);
}


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

void verbindeMitWLAN() {
  WiFi.begin(wlanName, wlanPasswort);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWLAN verbunden.");
}

void konfiguriereZeit() {
  configTime(gmtOffset, daylightOffset, ntpServer);
}

void clear() {
  for (int x = 0; x < 32; x++) {
    for (int y = 0; y < 16; y++) {
      setLed(x, y, black);
    }
  }
}

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