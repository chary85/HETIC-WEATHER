#include <WiFi.h>
#include <PubSubClient.h>

const unsigned long debounceDelay = 50;
const int BOUTON = 3; //PIN à changer par la suite
const int LED1 = 1; //PIN à changer par la suite
const int LED2 = 2; //PIN à changer par la suite

bool lastState = HIGH;
bool currentState = HIGH;
unsigned long lastDebounceTime = 0;
bool reading = HIGH;


// ========== WIFI ==========
const char* WIFI_SSID = "";
const char* WIFI_PASS = "azer1234";

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("[WIFI] Connexion");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n[WIFI] Connecté");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ========== MQTT ==========
const char* MQTT_HOST = "captain.dev0.pandor.cloud"; // À changer avec mon host
const uint16_t MQTT_PORT = 1884; // À changer avec mon port sur le raspberry pi
const char* MQTT_TOPIC = "temp/";
const char* MQTT_USER = "";
const char* MQTT_PASS = "";

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);


void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);

  while (!mqtt.connected()) {
    Serial.print("[MQTT] Connexion... ");

    if (mqtt.connect(MQTT_USER, MQTT_PASS)) {
      Serial.println("OK");
    } else {
      Serial.print("ECHEC, rc=");
      Serial.println(mqtt.state());
      delay(2000);
    }
  }
}

// ========== ENVOI MESSAGE ==========
void sendButtonEvent(char command) {
  String payload = "{\"esp32_id\":\"" + esp32_id + "\",\"temp\":\"" + String(command) + "\"}";
  mqtt.publish(MQTT_TOPIC, payload.c_str());
  Serial.println(payload);
}

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  delay(1000);

  esp32_id = "esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);

  pinMode(BOUTON, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  connectWiFi();
  connectMQTT();
}

// ========== LOOP ==========
void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();


  if (digitalRead(BOUTON) != lastState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (digitalRead(BOUTON) != currentState) {
      currentState = digitalRead(BOUTON);
      if (currentState == LOW) {
        digitalWrite(LED1, !digitalRead(LED1));
        digitalWrite(LED2, !digitalRead(LED2));
        if (digitalRead(LED1) == HIGH) sendButtonEvent('C');
        else sendButtonEvent('F');
      }
    }
    lastState = digitalRead(BOUTON);
  }
}