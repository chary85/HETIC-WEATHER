# SETUP — Reproduction de la station météo (Parcours B)

Ce guide permet de reproduire l’environnement de la station météo (broker MQTT, point d’accès WiFi optionnel).

## 1. Matériel

- **ESP32** (avec WiFi)
- **Capteur DHT22** (ou BME280) pour température et humidité
- **Broker MQTT** : PC avec Mosquitto ou **Raspberry Pi**
- Câbles, breadboard, résistance 10 kΩ si besoin pour le DHT

## 2. Schéma de câblage

Voir `../docs/schema-cablage.png` (à ajouter) :

- **DHT22**  
  - VCC → 3.3 V  
  - GND → GND  
  - DATA → GPIO 4 (ESP32) avec pull-up 10 kΩ vers 3.3 V  

Adapter si vous utilisez un BME280 (I2C : SDA/SCL).

## 3. Installer Mosquitto (broker MQTT)

### Sur Raspberry Pi / Linux

```bash
sudo apt update
sudo apt install mosquitto mosquitto-clients
```

Copier `mosquitto.conf` dans `/etc/mosquitto/conf.d/` ou lancer :

```bash
mosquitto -c config/mosquitto.conf
```

### Sur Windows

Télécharger depuis [Eclipse Mosquitto](https://mosquitto.org/download/), installer, puis lancer avec le fichier `config/mosquitto.conf`.

## 4. Configurer l’ESP32

1. Ouvrir `../esp32/station.ino` dans l’IDE Arduino.
2. Installer les librairies : **PubSubClient**, **DHT sensor library**.
3. Dans le code, modifier :
   - `WIFI_SSID` / `WIFI_PASSWORD` : votre réseau WiFi
   - `MQTT_BROKER` : IP du PC ou du Raspberry où tourne Mosquitto (ex. `192.168.1.10`)
4. Flasher l’ESP32.

## 5. Lancer le bridge et le frontend

```bash
# Terminal 1 - Bridge
cd bridge
npm install
npm start

# Terminal 2 - Frontend
cd frontend
npm install
npm run dev
```

Ouvrir l’URL affichée par Vite (ex. http://localhost:5173).

## 6. (Optionnel) Point d’accès WiFi (Raspberry Pi)

Pour que l’ESP32 se connecte via un AP hébergé sur le Pi :

1. Installer HostAPd et dnsmasq.
2. Copier `hostapd.conf` et adapter :
   - `interface` (ex. `wlan0`)
   - `ssid`, `wpa_passphrase`
3. Configurer le Pi en mode routeur/NAT si besoin pour que le bridge soit accessible.

## 7. Vérifications

- **MQTT** : `mosquitto_sub -t "station-meteo/#" -v` → vous devez voir les messages de l’ESP32.
- **Bridge** : `curl http://localhost:3000/api/stations` → liste des stations et dernières valeurs.
- **Frontend** : la page doit afficher les stations et se mettre à jour en temps réel.

## Dépannage

- **ESP32 ne se connecte pas au WiFi** : vérifier SSID/mot de passe et portée.
- **Pas de messages MQTT** : vérifier que Mosquitto tourne et que `MQTT_BROKER` pointe vers la bonne IP.
- **Frontend « Bridge indisponible »** : vérifier que le bridge tourne sur le port 3000 et que le proxy Vite pointe vers `http://localhost:3000`.
