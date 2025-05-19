#ifndef JOYSTICK_NEU_H
#define JOYSTICK_NEU_H

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
    bool invertJoystickCoordinates;

public:
    Joystick(int tasterPin, unsigned long entprellVerzoegerung = 50, int vrxPin = 34, int vryPin = 35, bool invertJoystickCoordinates = false);
    void aktualisieren();
    bool istGedrueckt();
    bool LangerKlick();
    int getLangerKlickCounter();
    int getX();
    int getY();
};

#endif
