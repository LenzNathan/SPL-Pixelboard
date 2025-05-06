#include <Arduino.h>
#include <Joystick_neu.h>

// Pin für den Taster
#define tasterPin 32

// Instanz der entprellten Tasterklasse 
Joystick taster(tasterPin, 50);

void setup() {
  Serial.begin(9600);
  Serial.println("Starte Taster Test...");
}

void loop() {
  // Aktualisiere den Zustand des entprellten Tasters
  Serial.println(digitalRead(32));
  taster.aktualisieren();

  // Lese den entprellten Zustand des Tasters aus und gebe ihn über die serielle Schnittstelle aus
  bool tasterGedrueckt = taster.istGedrueckt();
  Serial.print("Taster ist ");
  Serial.println(tasterGedrueckt ? "gedrückt" : "nicht gedrückt");

  // Überprüfe auf Doppelklick
  // if (taster.DoppelKlick()) {
  //   Serial.println("Doppelklick erkannt!");
  //   Serial.print("Anzahl der Doppelklicks: ");
  //   Serial.println(taster.getDoppelKlickCounter());
  // }

  // Überprüfe auf langen Klick
  if (taster.LangerKlick()) {
    Serial.println("Langer Klick erkannt!");
    Serial.print("Anzahl der langen Klicks: ");
    Serial.println(taster.getLangerKlickCounter());
  }

  delay(50);  // Kleine Verzögerung für die Entprellung
}
