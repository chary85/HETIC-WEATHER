/**
 * Frontend Station Météo - Connexion au bridge (API + WebSocket)
 */

const BRIDGE_URL = import.meta.env.VITE_BRIDGE_URL || '';
const API = BRIDGE_URL ? `${BRIDGE_URL}/api` : '/api';
const WS_URL = BRIDGE_URL ? BRIDGE_URL.replace(/^http/, 'ws') : `${location.protocol === 'https:' ? 'wss:' : 'ws:'}//${location.host}`;

const stationsEl = document.getElementById('stations');
const statusEl = document.getElementById('status');

let ws = null;

function setStatus(text, ok = true) {
  statusEl.textContent = text;
  statusEl.className = 'status ' + (ok ? 'connected' : 'disconnected');
}

function renderStation(stationId, data) {
  let card = document.getElementById(`station-${stationId}`);
  if (!card) {
    card = document.createElement('article');
    card.id = `station-${stationId}`;
    card.className = 'station-card';
    stationsEl.appendChild(card);
  }
  const temp = data.temp != null ? data.temp : '—';
  const humidity = data.humidity != null ? data.humidity : '—';
  const at = data.at || '';
  card.innerHTML = `
    <h2>Station ${stationId}</h2>
    <div class="values">
      <div class="value">
        <span class="label">Température</span>
        <span class="number">${temp} °C</span>
      </div>
      <div class="value">
        <span class="label">Humidité</span>
        <span class="number">${humidity} %</span>
      </div>
    </div>
    ${at ? `<p class="updated">Dernière mise à jour : ${new Date(at).toLocaleString('fr-FR')}</p>` : ''}
  `;
}

function updateFromMessage(data) {
  const id = data.stationId || data.id || '01';
  renderStation(id, data);
}

function connectWebSocket() {
  const url = `${WS_URL.replace(/\/$/, '')}/ws`;
  ws = new WebSocket(url);
  ws.onopen = () => setStatus('Connecté (temps réel)', true);
  ws.onclose = () => {
    setStatus('Déconnecté', false);
    setTimeout(connectWebSocket, 3000);
  };
  ws.onerror = () => setStatus('Erreur WebSocket', false);
  ws.onmessage = (e) => {
    try {
      updateFromMessage(JSON.parse(e.data));
    } catch (_) {}
  };
}

async function fetchStations() {
  try {
    const res = await fetch(`${API}/stations`);
    if (!res.ok) throw new Error(res.statusText);
    const stations = await res.json();
    setStatus('Connecté', true);
    stations.forEach((s) => renderStation(s.id, s));
  } catch (err) {
    setStatus('Bridge indisponible', false);
    stationsEl.innerHTML = '<p class="error">Impossible de joindre le bridge. Vérifiez qu’il tourne sur le port 3000.</p>';
  }
}

// Chargement initial + WebSocket
fetchStations();
connectWebSocket();
