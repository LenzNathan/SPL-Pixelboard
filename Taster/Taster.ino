#include <Arduino.h>

class Taster {
private:
  int pin;
  unsigned long entprellZeit = 50;
  unsigned long letzteFlanke = 0;
  bool letzterZustand = false;
  bool entprellterZustand = false;
public:
  Taster(int pin, unsigned long entprellZeit) {
    this->pin = pin;
    this->entprellZeit = entprellZeit;
    pinMode(pin, INPUT_PULLUP);
    letzterZustand = !digitalRead(pin);  // initiales Lesen des inversen Zustands
  }

  void aktualisieren() {
    unsigned long jetzt = millis();
    bool zustand = !digitalRead(pin);  // inverse Logik des Tasters

    if (jetzt - letzteFlanke > entprellZeit) {
      entprellterZustand = zustand;
      if (letzterZustand != zustand) {
        letzteFlanke = jetzt;
      }
    }
    letzterZustand = zustand;
  }

  bool
  istGedrueckt() {
    return entprellterZustand;
  }
};



//----------------------------------------------------------------------------------------------------------------


const int TASTER_PIN = 8;  // Beispiel-Pin

Taster meinTaster(TASTER_PIN, 50);  // Beispiel: Pin-Nummer und Entprellzeit angeben

void setup() {
  Serial.begin(115200);
  Serial.println("Starte Programm...");
}

void loop() {
  meinTaster.aktualisieren();
  bool zustand = meinTaster.istGedrueckt();

  static int anzahlKlick = 0;

  Serial.println(zustand);

  delay(10);  // Kleine Verzögerung für Stabilität der Schleife
}