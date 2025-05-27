#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <FastLED.h>
#include <Joystick_neu.h>
#include <vector>

#define LED_PIN_UPPER 25
#define LED_PIN_LOWER 26

#define COLOR_ORDER GRB
#define CHIPSET WS2811

#define BRIGHTNESS 50

const uint8_t kMatrixWidth = 32;
const uint8_t kMatrixHeight = 16;

const bool kMatrixSerpentineLayout = true;
const bool kMatrixVertical = true;


#define TOTAL_LED_COUNT (kMatrixWidth * kMatrixHeight)

CRGB ledsRawUpper[(TOTAL_LED_COUNT / 2) + 2];
CRGB ledsRawLower[(TOTAL_LED_COUNT / 2) + 2];

CRGB* const ledsUpper(ledsRawUpper + 1);
CRGB* const ledsLower(ledsRawLower + 1);

// Pins für den Joystick
#define tasterPin 32
#define xPin 34
#define yPin 35
#define joystickInverted false

// Instanz der entprellten Tasterklasse
Joystick taster(tasterPin, 50, xPin, yPin, joystickInverted);

// WLAN-Zugangsdaten
const char* ssid = "iPhone_13 Pro_AEW";
const char* password = "Gmylelqbln05+";

// HiveMQ Cloud (TLS, Auth)
const char* mqtt_server = "7a5f0484e9d74e10a592ae63a46e1a48.s1.eu.hivemq.cloud";  // z. B. abc123.hivemq.cloud
const int mqtt_port = 8883;                                                       // TLS
const char* mqtt_user = "Lenz_Wensink_ESP";
const char* mqtt_pass = "kaxgAv-baqma5-cybdup";

const char* mqtt_topic = "PingPong";
const char* client_id = "esp32-secure-client";

WiFiClientSecure secureClient;
PubSubClient client(secureClient);

void connectToWiFi() {
  Serial.print("Verbinde mit WLAN");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWLAN verbunden. IP: " + WiFi.localIP().toString());
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Verbinde mit MQTT...");
    if (client.connect(client_id, mqtt_user, mqtt_pass)) {
      Serial.println(" verbunden.");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Fehler: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

/*
length: laenge in bytes
*/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Nachricht empfangen [");
  Serial.print(topic);
  Serial.print("]: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  // connectToWiFi();

  // // UNSICHER (zum Testen): Keine Zertifikatsprüfung
  // secureClient.setInsecure();

  // // Alternative (sicher): eigenes Root-CA-Zertifikat setzen
  // // secureClient.setCACert(yourRootCA);  // z. B. als PEM-String

  // client.setServer(mqtt_server, mqtt_port);
  // client.setCallback(callback);

  randomSeed(analogRead(0));
  Serial.println("BEGINNING");
  FastLED.addLeds<CHIPSET, LED_PIN_UPPER, COLOR_ORDER>(ledsUpper, TOTAL_LED_COUNT / 2).setCorrection(TypicalSMD5050);
  FastLED.addLeds<CHIPSET, LED_PIN_LOWER, COLOR_ORDER>(ledsLower, TOTAL_LED_COUNT / 2).setCorrection(TypicalSMD5050);
  //FastLED.setMaxPowerInVoltsAndMilliamps(5, 2000);  // 5V, 2A
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  // if (!client.connected()) {
  //   reconnectMQTT();
  // }
  // client.loop();

  // static unsigned long lastMsg = 0;
  // if (millis() - lastMsg > 10000) {
  //   lastMsg = millis();
  //   int position = 2;
  //   String msg = WiFi.macAddress() + " " + position;
  //   Serial.println(msg);
  //   client.publish(mqtt_topic, msg.c_str());
  // }


}

struct Color {
  int H;
  int S;
  int V;
};

void reSetBorder(Color color) {
  for (int x = 1; x < 31; x++) {
    setLed(x, 15, color);
  }
  for (int x = 1; x < 31; x++) {
    setLed(x, 0, color);
  }
}

void clear() {
  for (int x = 0; x < 32; x++) {
    for (int y = 0; y < 16; y++) {
      setLed(x, y, black);
    }
  }
}