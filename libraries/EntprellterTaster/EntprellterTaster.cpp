#include "EntprellterTaster.h"

EntprellterTaster::EntprellterTaster(int tasterPin, int xpin, int ypin)
    : pin(tasterPin), entprellterZustand(false), letzteAenderung(0), druckBeginn(0), gedruecktGemeldet(true), xpin(xpin), ypin(ypin) {
    pinMode(pin, INPUT_PULLUP);
}

void EntprellterTaster::aktualisiere() {
    bool aktuellerZustand = !digitalRead(pin);
    unsigned long aktuelleZeit = millis();

    xwert = analogRead(xpin);
    ywert = analogRead(ypin);

    if (aktuellerZustand != entprellterZustand && (aktuelleZeit - letzteAenderung) > entprellZeit) {
        entprellterZustand = aktuellerZustand;
        letzteAenderung = aktuelleZeit;

        if (entprellterZustand) {
            druckBeginn = aktuelleZeit;
            gedruecktGemeldet = false;
        }
    }
}

int EntprellterTaster::getXwert() {
    return xwert;
}

int EntprellterTaster::getYwert() {
    return ywert;
}

bool EntprellterTaster::istGedrueckt() {
    return entprellterZustand;
}

bool EntprellterTaster::wurdeGedrueckt() {
    if (!entprellterZustand && !gedruecktGemeldet) {
        if (millis() - druckBeginn < langeDruckZeit) {
            gedruecktGemeldet = true;
            return true;
        }
    }
    return false;
}

bool EntprellterTaster::wurdeLangeGedrueckt() {
    if (entprellterZustand && !gedruecktGemeldet) {
        if (millis() - druckBeginn >= langeDruckZeit) {
            gedruecktGemeldet = true;
            return true;
        }
    }
    return false;
}
