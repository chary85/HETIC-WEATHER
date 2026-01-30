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

## Démarrage rapide

1. **Configurer le broker MQTT** : voir `config/SETUP.md`
2. **Flasher l’ESP32** : ouvrir `esp32/station.ino` dans l’IDE Arduino
3. **Lancer le bridge** : `cd bridge && npm install && npm start`
4. **Ouvrir le frontend** : `cd frontend && npm install && npm run dev` puis ouvrir l’URL indiquée

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