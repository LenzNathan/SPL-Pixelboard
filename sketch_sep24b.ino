#include <Arduino.h>

class EntprellterTaster {
private:
    int pin;
    bool tasterZustand;
    bool letzterZustand;
    unsigned long letzteEntprellZeit;
    unsigned long entprellVerzoegerung;
    unsigned long letzteKlickZeit;
    bool warteAufZweitenKlick;
    bool doppelKlickErkannt;
    int doppelKlickCounter;
    int langerKlickCounter;

public:
    EntprellterTaster(int pin, unsigned long entprellVerzoegerung = 50) {
        this->pin = pin;
        tasterZustand = false; // Startzustand des Tasters (angenommen: nicht gedrückt)
        letzterZustand = true; // Annahme: Taster ist beim Start nicht gedrückt
        letzteEntprellZeit = 0;
        this->entprellVerzoegerung = entprellVerzoegerung; // Entprellzeit in Millisekunden
        letzteKlickZeit = 0;
        warteAufZweitenKlick = false;
        doppelKlickErkannt = false;
        doppelKlickCounter = 0;
        langerKlickCounter = 0;
        pinMode(pin, INPUT_PULLUP); // Aktivierung des internen Pullup-Widerstands
    }

    void aktualisieren() {
        bool aktuellerZustand = digitalRead(pin); // Einlesen ohne Berücksichtigung der inversen Logik
        if (aktuellerZustand != letzterZustand) {
            letzteEntprellZeit = millis(); // Reset der Entprellzeit
        }
        if ((millis() - letzteEntprellZeit) > entprellVerzoegerung) {
            if (aktuellerZustand != letzterZustand) {
                letzterZustand = aktuellerZustand;
                tasterZustand = aktuellerZustand; // Aktualisierung des entprellten Zustands
                // Prüfen auf Doppelklick
                if (aktuellerZustand && !warteAufZweitenKlick) {
                    warteAufZweitenKlick = true;
                    letzteKlickZeit = millis();
                } else if (aktuellerZustand && warteAufZweitenKlick) {
                    if ((millis() - letzteKlickZeit) < 500) {
                        doppelKlickErkannt = true;
                        doppelKlickCounter++; // Zähler für Doppelklick erhöhen
                        warteAufZweitenKlick = false;
                    } else {
                        warteAufZweitenKlick = false;
                    }
                }
                // Prüfen auf langen Klick
                if (tasterZustand && ((millis() - letzteKlickZeit) > 1000)) {
                    langerKlickCounter++; // Zähler für langen Klick erhöhen
                }
            }
        }
    }

    bool istGedrueckt() {
        return tasterZustand;
    }

    bool DoppelKlick() {
        if (doppelKlickErkannt) {
            doppelKlickErkannt = false;
            return true;
        }
        return false;
    }

    bool LangerKlick() {
        if (tasterZustand && ((millis() - letzteKlickZeit) > 1000)) {
            return true;
        }
        return false;
    }

    int getDoppelKlickCounter() {
        return doppelKlickCounter;
    }

    int getLangerKlickCounter() {
        return langerKlickCounter;
    }
};

// Pin für den Taster
#define tasterPin 32

// Instanz der entprellten Tasterklasse mit einer Entprellzeit von 2000ms
EntprellterTaster taster(tasterPin, 2000);

void setup() {
    Serial.begin(9600);
    Serial.println("Starte Taster Test...");
}

void loop() {
    // Aktualisiere den Zustand des entprellten Tasters
    taster.aktualisieren();

    // Lese den entprellten Zustand des Tasters aus und gebe ihn über die serielle Schnittstelle aus
    bool tasterGedrueckt = taster.istGedrueckt();
    Serial.print("Taster ist ");
    Serial.println(tasterGedrueckt ? "gedrückt" : "nicht gedrückt");

    // Überprüfe auf Doppelklick
    if (taster.DoppelKlick()) {
        Serial.println("Doppelklick erkannt!");
        Serial.print("Anzahl der Doppelklicks: ");
        Serial.println(taster.getDoppelKlickCounter());
    }

    // Überprüfe auf langen Klick
    if (taster.LangerKlick()) {
        Serial.println("Langer Klick erkannt!");
        Serial.print("Anzahl der langen Klicks: ");
        Serial.println(taster.getLangerKlickCounter());
    }

    delay(50); // Kleine Verzögerung für die Entprellung
}
