#include <Arduino.h>

class EntprellterTaster {
private:
  int pin;
  bool tasterZustand;
  bool letzterZustand;
  unsigned long letzteEntprellZeit;
  unsigned long entprellVerzoegerung;
  bool wurdeGedruecktStatus;

public:
  EntprellterTaster(int pin, unsigned long entprellVerzoegerung = 50) {
    this->pin = pin;
    tasterZustand = false;  // Startzustand des Tasters (angenommen: nicht gedrückt)
    letzterZustand = true;  // Annahme: Taster ist beim Start nicht gedrückt
    letzteEntprellZeit = 0;
    this->entprellVerzoegerung = entprellVerzoegerung;  // Entprellzeit in Millisekunden
    pinMode(pin, INPUT_PULLUP);                         // Aktivierung des internen Pullup-Widerstands
    wurdeGedruecktStatus = false;                       // Initialer Status für "wurdeGedrueckt"
  }

  void aktualisieren() {
    bool aktuellerZustand = !digitalRead(pin);  // Einlesen mit Berücksichtigung der inversen Logik
    if (aktuellerZustand != letzterZustand) {
      letzteEntprellZeit = millis();  // Reset der Entprellzeit
    }
    if ((millis() - letzteEntprellZeit) > entprellVerzoegerung) {
      if (aktuellerZustand != letzterZustand) {
        letzterZustand = aktuellerZustand;
        tasterZustand = aktuellerZustand;  // Aktualisierung des entprellten Zustands

        // Wenn der Taster gedrückt wurde, den Status setzen
        if (tasterZustand == true) {
          wurdeGedruecktStatus = true;
        }
      }
    }
  }

  bool istGedrueckt() {
    return tasterZustand;
  }

  // Methode, die zurückgibt, ob der Taster seit dem letzten Aufruf gedrückt wurde
  bool wurdeGedrueckt() {
    if (wurdeGedruecktStatus) {
      wurdeGedruecktStatus = false;  // Zurücksetzen des Status nach dem ersten Abruf
      return true;                   // Rückgabe, dass der Taster gedrückt wurde
    }
    return false;  // Wenn der Taster nicht gedrückt wurde
  }

  // Setup-Methode für die Initialisierung
  void setup() {
    Serial.begin(9600);
    Serial.println("EntprellterTaster Test gestartet...");
  }

  // Loop-Methode für die Schleife
  void loop() {
    aktualisieren();  // Aktualisiere den Tasterzustand

    // Überprüfe, ob der Taster seit dem letzten Mal gedrückt wurde
    if (wurdeGedrueckt()) {
      Serial.println("Taster wurde gedrückt!");
    }

    delay(50);  // Kleine Verzögerung für die Entprellung
  }
};

// Pin für den Taster
#define tasterPin 32

// Instanz der entprellten Tasterklasse
EntprellterTaster taster(tasterPin, 50);

void setup() {
  taster.setup();  // Rufe die Setup-Methode der Klasse auf
}

void loop() {
  taster.loop();  // Rufe die Loop-Methode der Klasse auf
}