#define SW_pin 32 // Der SW pin auf derm Joystick
#define X_pin 34 // x Koordinate - pin
#define Y_pin 35 // y Koordinate - pin

void setup() {
  pinMode(SW_pin, INPUT_PULLUP);// Den Taster über einen Pullup Wiederstand als input konfigurieren
  digitalWrite(SW_pin, HIGH);
  Serial.begin(115200);// Starten der Seriellen verbindung 
}

void loop() {
  Serial.print("Switch:  ");
  Serial.print(digitalRead(SW_pin));
  Serial.print("\n");

  Serial.print("X-axis: ");
  Serial.print(analogRead(X_pin));
  Serial.print("\n");

  Serial.print("Y-axis: ");
  Serial.println(analogRead(Y_pin));
  Serial.print("\n\n");
  
  delay(500);
}


