#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

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
  Serial.begin(115200);
  connectToWiFi();

  // UNSICHER (zum Testen): Keine Zertifikatsprüfung
  secureClient.setInsecure();

  // Alternative (sicher): eigenes Root-CA-Zertifikat setzen
  // secureClient.setCACert(yourRootCA);  // z. B. als PEM-String

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 10000) {
    lastMsg = millis();
    int position = 2;
    String msg = WiFi.macAddress() + " " + position;
    Serial.println(msg);
    client.publish(mqtt_topic, msg.c_str());
  }
}


