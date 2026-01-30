/*
 * Station Météo - Firmware ESP32
 * Envoie température et humidité vers un broker MQTT.
 * Capteur : DHT22 (GPIO 4) ou adapter pour BME280.
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// --- Configuration WiFi ---
#define WIFI_SSID     "VOTRE_SSID"
#define WIFI_PASSWORD "VOTRE_MOT_DE_PASSE"

// --- Configuration MQTT ---
#define MQTT_BROKER   "192.168.1.10"   // IP du broker (Raspberry ou PC)
#define MQTT_PORT     1883
#define MQTT_CLIENT   "station-meteo-01"
#define MQTT_TOPIC    "station-meteo/01/data"

// --- Capteur DHT ---
#define DHT_PIN       4
#define DHT_TYPE      DHT22

WiFiClient espClient;
PubSubClient mqtt(espClient);
DHT dht(DHT_PIN, DHT_TYPE);

unsigned long lastRead = 0;
const unsigned long READ_INTERVAL_MS = 10000; // 10 s

void setup() {
  Serial.begin(115200);
  dht.begin();
  connectWiFi();
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
}

void loop() {
  if (!mqtt.connected()) {
    reconnectMQTT();
  }
  mqtt.loop();

  if (millis() - lastRead >= READ_INTERVAL_MS) {
    lastRead = millis();
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
      char payload[64];
      snprintf(payload, sizeof(payload),
               "{\"temp\":%.1f,\"humidity\":%.1f,\"ts\":%lu}",
               t, h, millis());
      if (mqtt.publish(MQTT_TOPIC, payload)) {
        Serial.println("Published: " + String(payload));
      } else {
        Serial.println("Publish failed");
      }
    } else {
      Serial.println("DHT read failed");
    }
  }
}

void connectWiFi() {
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected, IP: " + WiFi.localIP().toString());
}

void reconnectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Connecting MQTT... ");
    if (mqtt.connect(MQTT_CLIENT)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.println(mqtt.state());
      delay(3000);
    }
  }
}
