#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// ===== LEDs =====
#define LED_C_PIN 12
#define LED_F_PIN 13

// ===== Boutons =====
#define BUTTON_UNIT_PIN 4
#define BUTTON_MODE_PIN 5

// ===== DHT22 =====
#define DHT_PIN 14
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// ===== États =====
bool isCelsius = true;
bool simulationMode = true;

// ===== Anti-rebond bouton unité =====
bool lastUnitButtonState = HIGH;
bool unitButtonState = HIGH;
unsigned long lastUnitDebounceTime = 0;

// ===== Anti-rebond bouton mode =====
bool lastModeButtonState = HIGH;
bool modeButtonState = HIGH;
unsigned long lastModeDebounceTime = 0;

const unsigned long debounceDelay = 50;

// ===== Données capteur (température toujours en °C en interne) =====
float temperature = 0.0;
float humidity = 0.0;

// ===== Identifiant ESP32 (pour MQTT) =====
String esp32_id;

// ========== WIFI ==========
const char* WIFI_SSID = "";   // À REMPLIR : nom de ton réseau WiFi
const char* WIFI_PASS = "azer1234";
const unsigned long WIFI_TIMEOUT_MS = 15000;  // 15 s max, puis on continue (bouton/LED marchent sans WiFi)

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  if (strlen(WIFI_SSID) == 0) {
    Serial.println("[WIFI] SSID vide ! Renseigne WIFI_SSID dans le code. Poursuite sans WiFi.");
    return;
  }

  Serial.print("[WIFI] Connexion à ");
  Serial.print(WIFI_SSID);
  Serial.print(" ");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < WIFI_TIMEOUT_MS) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WIFI] Connecté");
    Serial.print("[WIFI] IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[WIFI] Échec (timeout). Vérifier SSID/mot de passe. Poursuite sans WiFi.");
  }
}

// ========== MQTT ==========
const char* MQTT_HOST = "captain.dev0.pandor.cloud";
const uint16_t MQTT_PORT = 1884;
const char* MQTT_TOPIC = "temp/";
const char* MQTT_USER = "";
const char* MQTT_PASS = "";

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);

  while (!mqtt.connected()) {
    Serial.print("[MQTT] Connexion... ");

    if (mqtt.connect(esp32_id.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println("OK");
    } else {
      Serial.print("ECHEC, rc=");
      Serial.println(mqtt.state());
      delay(2000);
    }
  }
}

void sendMqttData() {
  if (!mqtt.connected()) return;

  float tSend = temperature;
  if (!isCelsius) {
    tSend = temperature * 9.0f / 5.0f + 32.0f;
  }
  String payload = "{\"esp32_id\":\"" + esp32_id + "\",\"temp\":" + String(tSend, 1) +
                   ",\"humidity\":" + String(humidity, 1) +
                   ",\"unit\":\"" + (isCelsius ? "C" : "F") + "\"" +
                   ",\"simulation\":" + (simulationMode ? "true" : "false") + "}";

  if (mqtt.publish(MQTT_TOPIC, payload.c_str())) {
    Serial.println("[MQTT] Envoyé: " + payload);
  } else {
    Serial.println("[MQTT] Echec envoi");
  }
}

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("[INIT] Démarrage station météo");

  esp32_id = "esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  Serial.print("[INIT] ID: ");
  Serial.println(esp32_id);

  pinMode(LED_C_PIN, OUTPUT);
  pinMode(LED_F_PIN, OUTPUT);
  pinMode(BUTTON_UNIT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_MODE_PIN, INPUT_PULLUP);

  digitalWrite(LED_C_PIN, HIGH);
  digitalWrite(LED_F_PIN, LOW);

  dht.begin();
  randomSeed(analogRead(0));

  connectWiFi();
  if (WiFi.status() == WL_CONNECTED) {
    connectMQTT();
  }

  Serial.println("[INIT] Prêt. Unité: °C, Mode: Simulation");
  Serial.println("[INIT] Bouton unité = GPIO 4, bouton mode = GPIO 5 (un côté du bouton vers GND)");
}

// ========== LOOP ==========
unsigned long lastPublishTime = 0;
const unsigned long publishIntervalMs = 3000;

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (WiFi.status() == WL_CONNECTED) {
    if (!mqtt.connected()) connectMQTT();
    mqtt.loop();
  }

  handleUnitButton();
  handleModeButton();
  readSensorData();

  if (millis() - lastPublishTime >= publishIntervalMs) {
    lastPublishTime = millis();
    float tDisplay = isCelsius ? temperature : (temperature * 9.0f / 5.0f + 32.0f);
    Serial.print("[SENSOR] T=");
    Serial.print(tDisplay, 1);
    Serial.print(isCelsius ? " °C" : " °F");
    Serial.print(" H=");
    Serial.print(humidity, 1);
    Serial.println(" %");
    if (WiFi.status() == WL_CONNECTED && mqtt.connected()) sendMqttData();
  }
}

void handleUnitButton() {
  bool reading = digitalRead(BUTTON_UNIT_PIN);

  if (reading != lastUnitButtonState) {
    lastUnitDebounceTime = millis();
  }

  if (millis() - lastUnitDebounceTime > debounceDelay) {
    if (reading != unitButtonState) {
      unitButtonState = reading;

      if (unitButtonState == LOW) {
        isCelsius = !isCelsius;
        updateLeds();
        Serial.println(isCelsius ? "[UNIT] Affichage: Celsius" : "[UNIT] Affichage: Fahrenheit");
      }
    }
  }

  lastUnitButtonState = reading;
}

void handleModeButton() {
  bool reading = digitalRead(BUTTON_MODE_PIN);

  if (reading != lastModeButtonState) {
    lastModeDebounceTime = millis();
  }

  if (millis() - lastModeDebounceTime > debounceDelay) {
    if (reading != modeButtonState) {
      modeButtonState = reading;

      if (modeButtonState == LOW) {
        simulationMode = !simulationMode;
        Serial.println(simulationMode ? "[MODE] Simulation" : "[MODE] Capteur réel (DHT22)");
      }
    }
  }

  lastModeButtonState = reading;
}

void readSensorData() {
  if (simulationMode) {
    temperature = random(180, 300) / 10.0f;
    humidity = random(400, 700) / 10.0f;
  } else {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t)) temperature = t;
    if (!isnan(h)) humidity = h;
  }
}

void updateLeds() {
  digitalWrite(LED_C_PIN, isCelsius ? HIGH : LOW);
  digitalWrite(LED_F_PIN, isCelsius ? LOW : HIGH);
}
