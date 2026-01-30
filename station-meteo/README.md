# Station Météo IoT

Projet de station météo connectée : ESP32, broker MQTT, bridge et interface web.

## Architecture

```
station-meteo/
├── esp32/          → Firmware Arduino (capteurs, envoi MQTT)
├── bridge/         → Service Node.js (MQTT ↔ API / WebSocket)
├── frontend/       → Interface web temps réel
├── docs/           → Documentation, schémas, photos
└── config/         → Configuration Mosquitto, HostAPd, tuto (Parcours B)
```

## Prérequis

- Carte ESP32
- Capteurs (DHT22/AM2302 ou BME280 pour T° + humidité)
- Broker MQTT (Mosquitto)
- Node.js 18+ (pour le bridge)
- Option Parcours B : Raspberry Pi, HostAPd, Mosquitto

## Comment lancer le projet

### 1. Installer les dépendances (une seule fois)

À la racine du projet (`station-meteo`) :

```bash
npm run install:bridge
npm run install:frontend
```

Ou manuellement :
```bash
cd bridge && npm install
cd ../frontend && npm install
```

### 2. Lancer le bridge (MQTT → API / WebSocket)

Dans un terminal, à la racine :

```bash
npm run bridge
```

Le bridge écoute sur **http://localhost:3000** et se connecte au broker MQTT (voir `config/SETUP.md` pour Mosquitto).

### 3. Lancer le frontend (interface web)

Dans un **autre** terminal, à la racine :

```bash
npm run frontend
```

Ouvre ensuite l’URL affichée (souvent **http://localhost:5173**) dans le navigateur.

### 4. (Optionnel) ESP32 et broker MQTT

- **Broker MQTT** : Mosquitto doit tourner et être accessible (voir `config/SETUP.md`).
- **ESP32** : flasher `esp32/station.ino` dans l’IDE Arduino après avoir configuré WiFi et MQTT dans le code.

## Documentation

- Montage : `docs/photos/montage.jpg`
- Simulation Tinkercad : `docs/tinkercad/simulation.png`
- Schéma de câblage : `docs/schema-cablage.png`

## Licence

Projet pédagogique HETIC.

## Participant

Charles GROSSIN
Baptiste BONNICHON-JACQUES
Hugo MARTINS

## PS

Du fait de la volonté de déployer en utilisant la méthode B, le déploiement de la méthode A a été retardée en dernière minute