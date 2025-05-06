#include <Joystick.h>

// Pins für Joystick und Taster
#define TASTER_PIN 32
#define VRX_PIN 34
#define VRY_PIN 35

// Instanz der Joystick-Klasse
Joystick joystick(TASTER_PIN, 50, VRX_PIN, VRY_PIN);

void setup() {
    Serial.begin(9600);
    Serial.println("Joystick und Taster Test");
}

void loop() {
    // Aktualisierung der Zustände
    joystick.aktualisieren();

    // Ausgabe des Tasterzustands
    Serial.print("Taster ist ");
    Serial.println(joystick.istGedrueckt() ? "gedrückt" : "nicht gedrückt");

    // Ausgabe bei langem Klick
    if (joystick.LangerKlick()) {
        Serial.println("Langer Klick erkannt!");
    }

    // Ausgabe der Joystick-Koordinaten
    Serial.print("Joystick X: ");
    Serial.print(joystick.getX());
    Serial.print(" | Joystick Y: ");
    Serial.println(joystick.getY());

    delay(50);
}
