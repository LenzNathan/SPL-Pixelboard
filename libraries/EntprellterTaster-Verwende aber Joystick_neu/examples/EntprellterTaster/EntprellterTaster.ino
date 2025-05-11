#include <Arduino.h>
#include "EntprellterTaster.h"

const int TASTER_PIN = 32;
const int xwert = 34;
const int ywert = 35;
unsigned long timer = 0;
int del = 500;

EntprellterTaster taster(TASTER_PIN, xwert, ywert);

void setup() {
    Serial.begin(9600);
    Serial.println("Taster-Test gestartet");
}

void loop() {
    taster.aktualisiere();

    if (taster.wurdeGedrueckt()) {
        Serial.println("Taster wurde kurz gedrückt");
    }

    if (taster.wurdeLangeGedrueckt()) {
        Serial.println("Taster wurde lange gedrückt");
    }

    if (millis() > timer + del) {
        Serial.print("X: ");
        Serial.println(taster.getXwert());
        Serial.print("Y: ");
        Serial.println(taster.getYwert());
        Serial.println();

        timer = millis();
    }
}
