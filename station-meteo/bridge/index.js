/**
 * Bridge Station Météo
 * - S'abonne au topic MQTT station-meteo/+/data
 * - Expose une API REST (dernière valeur) et WebSocket (temps réel)
 */

const mqtt = require('mqtt');
const express = require('express');
const { WebSocketServer } = require('ws');
const cors = require('cors');
const http = require('http');

const MQTT_BROKER = process.env.MQTT_BROKER || 'mqtt://localhost:1883';
const MQTT_TOPIC = 'station-meteo/+/data';
const PORT = process.env.PORT || 3000;

// Dernière donnée reçue par station (id => payload)
const lastData = new Map();
// Clients WebSocket
const wssClients = new Set();

const app = express();
app.use(cors());
app.use(express.json());

const server = http.createServer(app);
const wss = new WebSocketServer({ server, path: '/ws' });

// --- MQTT ---
const client = mqtt.connect(MQTT_BROKER);

client.on('connect', () => {
  console.log('Bridge connected to MQTT');
  client.subscribe(MQTT_TOPIC, (err) => {
    if (err) console.error('Subscribe error:', err);
  });
});

client.on('message', (topic, message) => {
  const match = topic.match(/station-meteo\/([^/]+)\/data/);
  const stationId = match ? match[1] : 'unknown';
  let payload;
  try {
    payload = JSON.parse(message.toString());
  } catch {
    payload = { raw: message.toString() };
  }
  lastData.set(stationId, { ...payload, stationId, topic, at: new Date().toISOString() });
  // Broadcast aux clients WebSocket
  const msg = JSON.stringify({ stationId, ...payload });
  wssClients.forEach((ws) => {
    if (ws.readyState === 1) ws.send(msg);
  });
});

// --- API REST ---
app.get('/api/health', (_, res) => res.json({ ok: true, service: 'station-meteo-bridge' }));

app.get('/api/stations', (_, res) => {
  const stations = Array.from(lastData.entries()).map(([id, data]) => ({ id, ...data }));
  res.json(stations);
});

app.get('/api/stations/:id', (req, res) => {
  const data = lastData.get(req.params.id);
  if (!data) return res.status(404).json({ error: 'Station not found' });
  res.json(data);
});

// --- WebSocket ---
wss.on('connection', (ws) => {
  wssClients.add(ws);
  // Envoyer immédiatement les dernières valeurs
  lastData.forEach((data, id) => {
    ws.send(JSON.stringify({ stationId: id, ...data }));
  });
  ws.on('close', () => wssClients.delete(ws));
});

// --- Static (optionnel : servir le frontend) ---
// app.use(express.static(path.join(__dirname, '../frontend/dist')));

server.listen(PORT, () => {
  console.log(`Bridge running at http://localhost:${PORT}`);
  console.log(`WebSocket: ws://localhost:${PORT}/ws`);
});
