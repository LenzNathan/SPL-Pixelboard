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
  bool gedruecktGemeldet;                            // für nicht dauerhaft auslösung nachdem die bedingungen für den druck wahr sind
  static const unsigned long entprellZeit = 50;      // 50 ms Entprellzeit
  static const unsigned long langeDruckZeit = 1000;  // 1 Sekunde für langen Druck

public:
  EntprellterTaster(int tasterPin, int xpin, int ypin)                                                                                  //Konstruktor
    : pin(tasterPin), entprellterZustand(false), letzteAenderung(0), druckBeginn(0), gedruecktGemeldet(true), xpin(xpin), ypin(ypin) {  // setzt die default werte wie sonst standardmäßig untereinander
    pinMode(pin, INPUT_PULLUP);
  }

  void aktualisiere() {
    bool aktuellerZustand = !digitalRead(pin);  // Negation für negative Logik - ab hier positive logik
    unsigned long aktuelleZeit = millis();

    xwert = analogRead(xpin);
    ywert = analogRead(ypin);

    if (aktuellerZustand != entprellterZustand && (aktuelleZeit - letzteAenderung) > entprellZeit) {  // entprellen logik: nur bei zustandsänderung && prellzeit überschritten
      entprellterZustand = aktuellerZustand;                                                          // zustand aktualisieren (die änderung ist legitim - oben geprüft)
      letzteAenderung = aktuelleZeit;                                                                 // aktualisieren da geändert wurde

      if (entprellterZustand) {      // wenn der taster an is, dann
        druckBeginn = aktuelleZeit;  // is er seit jetzt an
        gedruecktGemeldet = false;   // und wir dürfen erneut melden
      }
    }
  }

  int getXwert() {
    return xwert;
  }

  int getYwert() {
    return ywert;
  }

  bool istGedrueckt() {
    return entprellterZustand;
  }

  bool wurdeGedrueckt() {
    if (!entprellterZustand && !gedruecktGemeldet) {  //erst wenn wieder losgelassen wird
      if (millis() - druckBeginn < langeDruckZeit) {
        gedruecktGemeldet = true;  // nur einmal melden (kommt in der bedingung ja auch vor)
        return true;
      }
    }  // else
    return false;
  }

  bool wurdeLangeGedrueckt() {
    if (entprellterZustand && !gedruecktGemeldet) {
      if (millis() - druckBeginn >= langeDruckZeit) {  // wie oben nur dass wir länger als langedruckzeit halten müssen
        gedruecktGemeldet = true;
        return true;
      }
    }
    return false;
  }
};  // sehr toll!

// Testprogramm
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
