const WS_URL = 'ws://localhost:3000/ws';
const stationsEl = document.getElementById('stations');
const statusEl = document.getElementById('status');


let mockStations = [
  { id: 'S-01', temp: 22.0, humidity: 45, at: new Date().toISOString() },
  { id: 'S-02', temp: 15.4, humidity: 60, at: new Date().toISOString() }
];

function setStatus(text, ok = true, isMock = false) {
  statusEl.textContent = text + (isMock ? ' (SIMULÉ)' : '');
  statusEl.className = 'status ' + (ok ? 'connected' : 'disconnected');
  if (isMock) statusEl.style.backgroundColor = '#ff9800'; 
}

function renderStation(stationId, data) {
  let card = document.getElementById(station-${stationId});
  if (!card) {
    card = document.createElement('article');
    card.id = station-${stationId};
    card.className = 'station-card';
    stationsEl.appendChild(card);
  }


  const tempClass = data.temp > 20 ? 'hot' : 'cold';
  card.innerHTML = 
    <h2 class="${tempClass}">Station ${stationId}</h2>
    <div class="values">
      <div class="value">
        <span class="label">Température</span>
        <span class="number">${data.temp} °C</span>
      </div>
      <div class="value">
        <span class="label">Humidité</span>
        <span class="number">${data.humidity} %</span>
      </div>
    </div>
    <p class="updated">Dernière mesure : ${new Date(data.at).toLocaleTimeString('fr-FR')}</p>
  ;
}
function startMocking() {
  setStatus('Serveur absent', true, true);

  mockStations.forEach(s => renderStation(s.id, s));

  setInterval(() => {
    const index = Math.floor(Math.random() * mockStations.length);
    const s = mockStations[index];


    s.temp = (parseFloat(s.temp) + (Math.random() - 0.5)).toFixed(1);
    s.humidity = Math.min(100, Math.max(0, s.humidity + Math.floor(Math.random() * 3) - 1));
    s.at = new Date().toISOString();

    renderStation(s.id, s);
  }, 1500);
}

function connectWebSocket() {
  const socket = new WebSocket(WS_URL);
  let isConnected = false;

  socket.onopen = () => {
    isConnected = true;
    setStatus('Connecté au serveur', true);
  };

  socket.onmessage = (event) => {
    const data = JSON.parse(event.data);
    renderStation(data.stationId || data.id, data);
  };

  socket.onerror = () => {
    if (!isConnected) {
      console.warn("Serveur injoignable, activation du multi-mock...");
      startMocking();
    }
  };
}

// Lancement
connectWebSocket();