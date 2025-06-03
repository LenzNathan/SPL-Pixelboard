#include <Joystick_neu.h>

// Pin für den Taster
#define tasterPin 32

// Instanz der entprellten Tasterklasse 
Joystick taster(tasterPin, 50);

void TaskA(void *pvParameters);
void TaskB(void *pvParameters);

bool aTaskActive = true;

TaskHandle_t handle_a;
TaskHandle_t handle_b;

void setup() {
  Serial.begin(9600);
  Serial.println("Starte FreeRTOS A-B Test");
  xTaskCreate(
    TaskA, "TaskA"  //name
    ,
    2048  // The stack size can be checked by calling `uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);`
    ,
    NULL // Task parameter which can modify the task behavior. This must be passed as pointer to void.
    ,
    2  // Priority
    ,
    &handle_a
  );
  xTaskCreate(
    TaskB, "TaskB"  //name
    ,
    2048  // The stack size can be checked by calling `uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);`
    ,
    NULL // Task parameter which can modify the task behavior. This must be passed as pointer to void.
    ,
    2  // Priority
    ,
    &handle_b
  );
  vTaskSuspend(handle_b);
}

void loop() {
  // Aktualisiere den Zustand des entprellten Tasters
  taster.aktualisieren();

  // Lese den entprellten Zustand des Tasters aus und gebe ihn über die serielle Schnittstelle aus
  bool tasterGedrueckt = taster.istGedrueckt();
  //Serial.print("Taster ist ");
  //Serial.println(tasterGedrueckt ? "gedrückt" : "nicht gedrückt");
  
  // Überprüfe auf langen Klick
  if (taster.LangerKlick()) {
    Serial.println("Langer Klick erkannt!");
    //Serial.print("Anzahl der langen Klicks: ");
    //Serial.println(taster.getLangerKlickCounter());
    aTaskActive = !aTaskActive;
    if(aTaskActive){
      vTaskSuspend(handle_b);
      vTaskResume(handle_a);
    }else{
      vTaskSuspend(handle_a);
      vTaskResume(handle_b);
    }
  }

  delay(1);  // Kleine Verzögerung für die Entprellung
}

void TaskA(void *pvParameters) {
  delay(100);
  for(;;){
    delay(3000);
    Serial.println("A");
  }
}

void TaskB(void *pvParameters) {
  delay(100);
  for(;;){
    delay(3000);
    Serial.println("B");
  }
}


void TaskDHT22(){
  
}
