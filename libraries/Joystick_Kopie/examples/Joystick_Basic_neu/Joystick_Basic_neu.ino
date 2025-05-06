#include <Joystick_neu.h>

// Pins für Joystick und Taster
#define TASTER_PIN 32
#define VRX_PIN 34
#define VRY_PIN 35

unsigned int last = 0;
int delta = 100;

// Instanz der Joystick-Klasse
Joystick joystick(TASTER_PIN, 50, VRX_PIN, VRY_PIN);

void setup() {
  Serial.begin(115200);
  delay(4000);
  Serial.println("Joystick und Taster Test");
}

void loop() {
  // Aktualisierung der Zustände
  joystick.aktualisieren();
  delay(5);

  if (millis() - last > delta) {
    last = millis();
    // Ausgabe des Tasterzustands
    Serial.print("Taster ist ");
    if (joystick.istGedrueckt()) {
      Serial.println("gedrückt");
    } else {
      Serial.println("nicht gedrückt");
    }

    //Serial.println(digitalRead(TASTER_PIN));

    // Ausgabe bei langem Klick
    if (joystick.LangerKlick()) {
      Serial.println("Langer Klick erkannt!");
    }

    // Ausgabe der Joystick-Koordinaten
    // Serial.print("Joystick X: ");
    // Serial.print(joystick.getX());
    // Serial.print(" | Joystick Y: ");
    // Serial.println(joystick.getY());
  }
}