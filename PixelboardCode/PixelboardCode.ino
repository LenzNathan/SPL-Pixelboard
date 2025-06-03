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

// === LED-Matrix 2: Feuchtigkeitsanzeige
#define LED_PIN_HUM 26                       // Datenpin für untere Matrix (Feuchtigkeit)
#define MATRIX_WIDTH2 -32                    // Negativer Wert = X-Richtung gespiegelt
#define MATRIX_HEIGHT2 8                     // Normale Höhe
#define MATRIX_TYPE2 VERTICAL_ZIGZAG_MATRIX  // Auch vertikale Zickzack-Verkabelung

cLEDMatrix<MATRIX_WIDTH2, MATRIX_HEIGHT2, MATRIX_TYPE2> ledsHum;  // Matrixobjekt für Feuchtigkeit
cLEDText textHum;                                                 // Textobjekt für Feuchtigkeit
unsigned char humText[] = { "     %" };                           // Platzhalter-Text für Feuchtigkeit





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

bool aTaskActive = true;

TaskHandle_t handle_a;
TaskHandle_t handle_DHT22;

void setup() {
  Serial.begin(9600);
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
    2048,
    NULL,
    1,
    NULL);
}

void loop() {
  // Entprellten Tasterzustand abfragen
  taster.aktualisieren();
  bool tasterGedrueckt = taster.istGedrueckt();

  if (taster.LangerKlick()) {
    Serial.println("Langer Klick erkannt!");
    aTaskActive = !aTaskActive;

    if (aTaskActive) {
      vTaskSuspend(handle_DHT22);  // Deaktiviere Task B
      vTaskResume(handle_a);       // Aktiviere Task A
    } else {
      vTaskSuspend(handle_a);     // Deaktiviere Task A
      vTaskResume(handle_DHT22);  // Aktiviere Task B
    }
  }

  delay(1);  // Kleinste Entprellverzögerung
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

    FastLED.show();
    vTaskDelay(pdMS_TO_TICKS(1000));  // 1 Sekunde warten
  }
}

