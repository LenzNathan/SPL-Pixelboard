#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>

class Joystick {
private:
    int tasterPin;
    bool tasterZustand;
    bool letzterZustand;
    bool langerKlickZustand;
    unsigned long letzteEntprellZeit;
    unsigned long entprellVerzoegerung;
    unsigned long letzteKlickZeit;
    int langerKlickCounter;
    int firstLoopLangerKlick;

    int vrxPin;
    int vryPin;
    int xKoordinate;
    int yKoordinate;

public:
    Joystick(int tasterPin, unsigned long entprellVerzoegerung = 50, int vrxPin = 34, int vryPin = 35);
    void aktualisieren();
    bool istGedrueckt();
    bool LangerKlick();
    int getLangerKlickCounter();
    int getX();
    int getY();
};

#endif
