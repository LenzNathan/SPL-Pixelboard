#include "Joystick.h"

Joystick::Joystick(int tasterPin, unsigned long entprellVerzoegerung, int vrxPin, int vryPin) {
    this->tasterPin = tasterPin;
    this->vrxPin = vrxPin;
    this->vryPin = vryPin;

    tasterZustand = false;
    letzterZustand = true;
    letzteEntprellZeit = 0;
    this->entprellVerzoegerung = entprellVerzoegerung;
    letzteKlickZeit = 0;
    langerKlickCounter = 0;

    pinMode(tasterPin, INPUT_PULLUP);
    xKoordinate = 0;
    yKoordinate = 0;
}

void Joystick::aktualisieren() {
    bool aktuellerZustand = digitalRead(tasterPin);
    if (aktuellerZustand != letzterZustand) {
        letzteEntprellZeit = millis();
    }
    if ((millis() - letzteEntprellZeit) > entprellVerzoegerung) {
        if (aktuellerZustand != letzterZustand) {
            letzterZustand = aktuellerZustand;
            tasterZustand = aktuellerZustand;

            if (tasterZustand && ((millis() - letzteKlickZeit) > 1000)) {
                langerKlickCounter++;
            }
            if (tasterZustand) {
                letzteKlickZeit = millis();
            }
        }
    }

    xKoordinate = analogRead(vrxPin);
    yKoordinate = analogRead(vryPin);
}

bool Joystick::istGedrueckt() {
    return tasterZustand;
}

bool Joystick::LangerKlick() {
    if (tasterZustand && ((millis() - letzteKlickZeit) > 1000)) {
        return true;
    }
    return false;
}

int Joystick::getLangerKlickCounter() {
    return langerKlickCounter;
}

int Joystick::getX() {
    return xKoordinate;
}

int Joystick::getY() {
    return yKoordinate;
}
