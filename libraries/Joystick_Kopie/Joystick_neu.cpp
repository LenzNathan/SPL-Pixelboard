#include "Joystick_neu.h"

Joystick::Joystick(int tasterPin, unsigned long entprellVerzoegerung, int vrxPin, int vryPin) {
    this->tasterPin = tasterPin;
    this->vrxPin = vrxPin;
    this->vryPin = vryPin;

    firstLoopLangerKlick = true;
    tasterZustand = false;
    letzterZustand = true;
    letzteEntprellZeit = 0;
    this->entprellVerzoegerung = entprellVerzoegerung;
    letzteKlickZeit = 0;
    langerKlickCounter = 0;
    langerKlickZustand = false;

    pinMode(tasterPin, INPUT_PULLUP);
    xKoordinate = 0;
    yKoordinate = 0;
}

void Joystick::aktualisieren() {
    bool aktuellerZustand = !digitalRead(tasterPin);
    if (aktuellerZustand != letzterZustand) {
        letzterZustand = aktuellerZustand;
        letzteEntprellZeit = millis();
    }
    if ((millis() - letzteEntprellZeit) > entprellVerzoegerung) {
        tasterZustand = aktuellerZustand;
        if (tasterZustand && ((millis() - letzteKlickZeit) > 1000)) {
            if(firstLoopLangerKlick){
                langerKlickCounter++;
                firstLoopLangerKlick = false;
            }
            langerKlickZustand = true;
        }else{
            firstLoopLangerKlick = true;
        }
        if (!tasterZustand) {
            letzteKlickZeit = millis();
        }
    }
    xKoordinate = analogRead(vrxPin);
    yKoordinate = analogRead(vryPin);
}

bool Joystick::istGedrueckt() {
    return tasterZustand;
}

bool Joystick::LangerKlick() {
    if (langerKlickZustand) {
        langerKlickZustand = false;
        return true;
    } else {
        return false;
    }
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
