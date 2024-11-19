#ifndef ENTPRELLTER_TASTER_H
#define ENTPRELLTER_TASTER_H

#include <Arduino.h>

class EntprellterTaster {
private:
    int pin;
    int xpin;
    int xwert;
    int ypin;
    int ywert;
    bool entprellterZustand;
    unsigned long letzteAenderung;
    unsigned long druckBeginn;
    bool gedruecktGemeldet;

    static const unsigned long entprellZeit = 50;      // 50 ms Entprellzeit
    static const unsigned long langeDruckZeit = 1000;  // 1 Sekunde für langen Druck

public:
    EntprellterTaster(int tasterPin, int xpin, int ypin);

    void aktualisiere();

    int getXwert();
    int getYwert();
    bool istGedrueckt();
    bool wurdeGedrueckt();
    bool wurdeLangeGedrueckt();
};

#endif
