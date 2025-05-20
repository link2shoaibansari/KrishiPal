#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define DHTPIN D5
#define DHTTYPE DHT11
#define IRSENSOR D6
#define MOTOR_IN1 D7
#define MOTOR_IN2 D8

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80);

unsigned long previousMillis = 0;
const long interval = 4000;
bool showSoil = false;
bool motionAlert = false;
unsigned long motionDisplayUntil = 0;

bool pumpOn = false;
unsigned long pumpStartTime = 0;
const unsigned long minPumpDuration = 10000;

float tempC = 0.0;
float humidity = 0.0;
int soilValue = 0;
int moisturePercent = 0;

const char* ssid = "Plant Monitor";
const char* password = "password";

void handleRoot() {
  String html = R"====(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>Smart Plant Monitoring</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <script src="https://www.gstatic.com/firebasejs/8.0.0/firebase-app.js"></script>
  <script src="https://www.gstatic.com/firebasejs/8.0.0/firebase-database.js"></script>
  <style>
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }
    body {
      font-family: 'Segoe UI', sans-serif;
      background: #121212;
      color: #eee;
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: 2rem;
    }
    h1 {
      font-size: 2.5rem;
      color: #baf7c4;
      margin-bottom: 2rem;
    }
    .status-bar {
      background: #1e1e1e;
      padding: 10px 20px;
      border-radius: 20px;
      margin-bottom: 2rem;
      display: flex;
      align-items: center;
      font-size: 0.9rem;
      gap: 1rem;
    }
    .dashboard {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
      gap: 1.5rem;
      width: 100%;
      max-width: 1000px;
    }
    .card {
      background: #1e1e1e;
      border-radius: 20px;
      padding: 1.5rem;
      text-align: center;
      box-shadow: 0 0 10px #00000055;
    }
    .dial {
      height: 100px;
      width: 100px;
      border-radius: 50%;
      border: 8px solid #444;
      margin: 0 auto 1rem;
      position: relative;
      transition: border-top-color 0.3s ease;
    }
    .dial-value {
      position: absolute;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      font-size: 1.2rem;
      color: #fff;
    }
    .label {
      margin-top: 0.5rem;
      color: #9fefac;
      font-weight: 600;
    }
    .controls {
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    .switch-group {
      display: flex;
      justify-content: space-between;
      width: 100%;
      margin-top: 1rem;
    }
    .switch-label {
      font-size: 0.9rem;
    }
    .switch {
      position: relative;
      display: inline-block;
      width: 40px;
      height: 20px;
    }
    .switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }
    .slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #444;
      transition: .4s;
      border-radius: 34px;
    }
    .slider:before {
      position: absolute;
      content: "";
      height: 16px;
      width: 16px;
      left: 2px;
      bottom: 2px;
      background-color: white;
      transition: .4s;
      border-radius: 50%;
    }
    input:checked + .slider {
      background-color: #4caf50;
    }
    input:checked + .slider:before {
      transform: translateX(20px);
    }
    .footer-box {
      margin-top: 2rem;
      background: #1e1e1e;
      border-radius: 30px;
      padding: 1.5rem;
      display: flex;
      justify-content: space-around;
      gap: 2rem;
      color: #c5f3c5;
      flex-wrap: wrap;
    }
    .icon-text {
      display: flex;
      align-items: center;
      gap: 0.5rem;
      font-size: 0.95rem;
    }
    .floating-btn {
      position: fixed;
      bottom: 30px;
      right: 30px;
      background: #4caf50;
      color: white;
      font-size: 2rem;
      border-radius: 50%;
      width: 50px;
      height: 50px;
      text-align: center;
      line-height: 50px;
      cursor: pointer;
      box-shadow: 0 0 10px #4caf50aa;
      transition: transform 0.3s ease;
      z-index: 100;
      user-select: none;
    }
    .floating-btn:hover {
      transform: scale(1.1);
    }
    .floating-btn.expanded {
      transform: rotate(45deg);
    }
    .chart-overlay {
      position: fixed;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background: rgba(0, 0, 0, 0.9);
      z-index: 90;
      display: none;
      padding: 2rem;
      overflow-y: auto;
      animation: fadeIn 0.3s ease;
    }
    .chart-container {
      width: 100%;
      max-width: 1000px;
      margin: 0 auto;
      background: #1e1e1e;
      border-radius: 20px;
      padding: 1.5rem;
      box-shadow: 0 0 20px #000000aa;
    }
    .chart-title {
      color: #9fefac;
      text-align: center;
      margin-bottom: 1.5rem;
      font-size: 1.5rem;
    }
    .chart-row {
      display: flex;
      flex-wrap: wrap;
      gap: 1.5rem;
      margin-bottom: 1.5rem;
    }
    .chart-wrapper {
      flex: 1;
      min-width: 300px;
      background: #252525;
      border-radius: 15px;
      padding: 1rem;
    }
    canvas {
      width: 100% !important;
      height: 250px !important;
    }
    .close-btn {
      position: absolute;
      top: 20px;
      right: 20px;
      background: #f44336;
      color: white;
      border: none;
      border-radius: 50%;
      width: 40px;
      height: 40px;
      font-size: 1.2rem;
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: center;
    }
    /* Enhanced Motion Detection Styles */
    .motion-indicator {
      width: 80px;
      height: 80px;
      margin: 0 auto 1rem;
      border-radius: 50%;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 0.9rem;
      font-weight: 500;
      color: #fff;
      text-transform: uppercase;
      transition: all 0.3s ease;
    }
    .motion-indicator.inactive {
      background: linear-gradient(135deg, #4caf50, #388e3c);
      box-shadow: 0 0 15px rgba(76, 175, 80, 0.5);
    }
    .motion-indicator.active {
      background: linear-gradient(135deg, #ff4d4d, #d32f2f);
      box-shadow: 0 0 20px rgba(255, 77, 77, 0.7);
      animation: pulse 1.5s infinite ease-in-out;
    }
    @keyframes pulse {
      0% { transform: scale(1); }
      50% { transform: scale(1.05); }
      100% { transform: scale(1); }
    }
    .motion-card {
      position: relative;
      overflow: hidden;
    }
    .motion-card::before {
      content: '';
      position: absolute;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      background: radial-gradient(circle, rgba(255, 255, 255, 0.05) 0%, transparent 70%);
      opacity: 0;
      transition: opacity 0.3s ease;
    }
    .motion-card.active::before {
      opacity: 1;
    }
    .motion-label {
      font-size: 1rem;
      letter-spacing: 1px;
      color: #9fefac;
      margin-bottom: 1rem;
    }
    .motion-toggle {
      display: flex;
      align-items: center;
      gap: 0.5rem;
      margin-top: 1rem;
      font-size: 0.85rem;
      color: #ccc;
    }
    @keyframes fadeIn {
      from { opacity: 0; }
      to { opacity: 1; }
    }
    @media (max-width: 500px) {
      h1 { font-size: 1.8rem; }
      .card { padding: 1rem; }
      .chart-wrapper {
        min-width: 100%;
      }
      .chart-overlay {
        padding: 1rem;
      }
      .motion-indicator {
        width: 60px;
        height: 60px;
        font-size: 0.8rem;
      }
    }
  </style>
</head>
<body>
  <h1>Smart Plant Monitoring</h1>
  <div class="status-bar">
    <span><span class="status-dot" id="statusDot" style="background-color: #5bff9b;"></span><span id="statusText">System Online</span></span>
  </div>

  <div class="dashboard">
    <div class="card">
      <div class="dial" id="tempDial"><div class="dial-value" id="temp">--°</div></div>
      <div class="label">TEMPERATURE</div>
    </div>
    <div class="card">
      <div class="dial" id="humidityDial"><div class="dial-value" id="humidity">--%</div></div>
      <div class="label">HUMIDITY</div>
    </div>
    <div class="card">
      <div class="dial" id="moistureDial"><div class="dial-value" id="moisture">--%</div></div>
      <div class="label">SOIL MOISTURE</div>
    </div>
    <div class="card motion-card" id="motionCard">
      <div class="motion-indicator inactive" id="motionIndicator">No Motion</div>
      <div class="motion-label">MOTION DETECTION</div>
      <div class="motion-toggle">
        <span>Alarm Sound</span>
        <label class="switch">
          <input type="checkbox" id="alarmToggle" checked>
          <span class="slider"></span>
        </label>
      </div>
    </div>
    <div class="card controls">
      <div class="label">SYSTEM CONTROLS</div>
      <div class="switch-group">
        <span class="switch-label">Auto Mode</span>
        <label class="switch">
          <input type="checkbox" id="autoToggle" checked>
          <span class="slider"></span>
        </label>
      </div>
      <div class="switch-group">
        <span class="switch-label">PIR Sensor</span>
        <label class="switch">
          <input type="checkbox" id="pirToggle" checked>
          <span class="slider"></span>
        </label>
      </div>
      <div class="switch-group">
        <span class="switch-label">Pump Control</span>
        <label class="switch">
          <input type="checkbox" id="pumpToggle">
          <span class="slider"></span>
        </label>
      </div>
      <div class="switch-group">
        <span class="switch-label">Light Control</span>
        <label class="switch">
          <input type="checkbox" id="lightToggle">
          <span class="slider"></span>
        </label>
      </div>
    </div>
  </div>

  <div class="footer-box">
    <div class="icon-text">🌡️ <b>Temperature</b>: <span id="footerTemp">--°C</span></div>
    <div class="icon-text">💧 <b>Humidity</b>: <span id="footerHumidity">--%</span></div>
    <div class="icon-text">🪴 <b>Soil Moisture</b>: <span id="footerMoisture">--%</span></div>
    <div class="icon-text">🔐 <b>Security</b>: <span id="footerSecurity">Inactive</span></div>
    <div class="icon-text">💡 <b>Light</b>: <span id="footerLight">OFF</span></div>
  </div>

  <!-- Floating Action Button -->
  <div class="floating-btn" id="fabButton" onclick="toggleChartOverlay()">+</div>

  <!-- Chart Overlay (Hidden by default) -->
  <div class="chart-overlay" id="chartOverlay">
    <button class="close-btn" id="closeChartsBtn">×</button>
    <div class="chart-container">
      <h2 class="chart-title">Historical Data Trends</h2>
      <div class="chart-row">
        <div class="chart-wrapper">
          <canvas id="tempChart"></canvas>
        </div>
        <div class="chart-wrapper">
          <canvas id="humidityChart"></canvas>
        </div>
      </div>
      <div class="chart-row">
        <div class="chart-wrapper">
          <canvas id="moistureChart"></canvas>
        </div>
        <div class="chart-wrapper">
          <canvas id="pumpChart"></canvas>
        </div>
      </div>
    </div>
  </div>

  <script>
    // Firebase Configuration
    const firebaseConfig = {
      apiKey: "YOUR_API_KEY",
      authDomain: "your-project.firebaseapp.com",
      databaseURL: "https://your-project.firebaseio.com",
      projectId: "your-project",
      storageBucket: "your-project.appspot.com",
      messagingSenderId: "123456789",
      appId: "1:123456789:web:abcdef123456"
    };

    // Initialize Firebase
    firebase.initializeApp(firebaseConfig);
    const database = firebase.database();

    // DOM Elements
    const statusText = document.getElementById('statusText');
    const statusDot = document.getElementById('statusDot');
    const alarmToggle = document.getElementById('alarmToggle');
    const motionIndicator = document.getElementById('motionIndicator');
    const motionCard = document.getElementById('motionCard');
    const fabButton = document.getElementById('fabButton');
    const chartOverlay = document.getElementById('chartOverlay');
    const closeChartsBtn = document.getElementById('closeChartsBtn');
    const warningSound = new Audio('https://assets.mixkit.co/sfx/download/mixkit-alert-alarm-1005.mp3');
    warningSound.loop = true;

    // Chart Initialization
    const tempChart = createChart('tempChart', 'Temperature (°C)', 'rgba(255, 99, 132, 0.8)');
    const humidityChart = createChart('humidityChart', 'Humidity (%)', 'rgba(54, 162, 235, 0.8)');
    const moistureChart = createChart('moistureChart', 'Soil Moisture (%)', 'rgba(75, 192, 192, 0.8)');
    const pumpChart = createChart('pumpChart', 'Pump Activity', 'rgba(255, 159, 64, 0.8)', true);

    function createChart(id, label, color, isStepped = false) {
      const ctx = document.getElementById(id).getContext('2d');
      return new Chart(ctx, {
        type: 'line',
        data: {
          datasets: [{
            label: label,
            borderColor: color,
            backgroundColor: color.replace('0.8', '0.2'),
            borderWidth: 2,
            pointRadius: 2,
            fill: true,
            stepped: isStepped ? 'after' : false,
            tension: 0.1
          }]
        },
        options: {
          responsive: true,
          maintainAspectRatio: false,
          scales: {
            x: {
              type: 'time',
              time: {
                unit: 'hour',
                displayFormats: {
                  hour: 'HH:mm'
                }
              },
              grid: {
                color: 'rgba(255, 255, 255, 0.1)'
              },
              ticks: {
                color: '#ccc'
              }
            },
            y: {
              beginAtZero: true,
              grid: {
                color: 'rgba(255, 255, 255, 0.1)'
              },
              ticks: {
                color: '#ccc'
              }
            }
          },
          plugins: {
            legend: {
              labels: {
                color: '#eee'
              }
            }
          }
        }
      });
    }

    // Toggle Chart Overlay
    function toggleChartOverlay() {
      chartOverlay.style.display = chartOverlay.style.display === 'block' ? 'none' : 'block';
      fabButton.classList.toggle('expanded');
      if (chartOverlay.style.display === 'block') {
        fetchHistoricalData();
      }
    }

    closeChartsBtn.addEventListener('click', () => {
      chartOverlay.style.display = 'none';
      fabButton.classList.remove('expanded');
    });

    // Fetch and display historical data
    function fetchHistoricalData() {
      const now = new Date();
      const twentyFourHoursAgo = new Date(now.getTime() - (24 * 60 * 60 * 1000));
      
      database.ref('sensor_logs')
        .orderByChild('timestamp')
        .startAt(twentyFourHoursAgo.getTime().toString())
        .once('value')
        .then(snapshot => {
          const data = snapshot.val();
          const timestamps = [];
          const temps = [];
          const humids = [];
          const moistures = [];
          const pumps = [];

          if (data) {
            Object.keys(data).forEach(key => {
              const entry = data[key];
              timestamps.push(new Date(parseInt(entry.timestamp)));
              temps.push(entry.temperature);
              humids.push(entry.humidity);
              moistures.push(entry.moisture);
              pumps.push(entry.pump ? 100 : 0);
            });

            updateChart(tempChart, timestamps, temps);
            updateChart(humidityChart, timestamps, humids);
            updateChart(moistureChart, timestamps, moistures);
            updateChart(pumpChart, timestamps, pumps);
          }
        });
    }

    function updateChart(chart, labels, data) {
      chart.data.labels = labels;
      chart.data.datasets[0].data = data;
      chart.update();
    }

    // Real-time data updates
    function updateDialColor(el, value, low, high) {
      if (value < low) el.style.borderTopColor = '#ffa726';
      else if (value > high) el.style.borderTopColor = '#f44336';
      else el.style.borderTopColor = '#7cf0a4';
    }

    function fetchData() {
      fetch('/data')
        .then(res => res.json())
        .then(data => {
          statusText.textContent = 'System Online';
          statusDot.style.backgroundColor = '#5bff9b';

          document.getElementById('temp').textContent = data.temp + '°';
          document.getElementById('humidity').textContent = data.humidity + '%';
          document.getElementById('moisture').textContent = data.moisture + '%';

          document.getElementById('footerTemp').textContent = data.temp + '°C';
          document.getElementById('footerHumidity').textContent = data.humidity + '%';
          document.getElementById('footerMoisture').textContent = data.moisture + '%';
          document.getElementById('footerLight').textContent = data.light ? 'ON' : 'OFF';

          updateDialColor(document.getElementById('tempDial'), data.temp, 15, 35);
          updateDialColor(document.getElementById('humidityDial'), data.humidity, 30, 70);
          updateDialColor(document.getElementById('moistureDial'), data.moisture, 40, 80);

          document.getElementById('autoToggle').checked = data.autoMode;
          document.getElementById('pirToggle').checked = data.pirEnabled;
          document.getElementById('pumpToggle').checked = data.pump;
          document.getElementById('lightToggle').checked = data.light;

          if (data.pirEnabled && data.motion) {
            motionIndicator.textContent = 'Motion Detected';
            motionIndicator.classList.remove('inactive');
            motionIndicator.classList.add('active');
            motionCard.classList.add('active');
            document.getElementById('footerSecurity').textContent = 'Active';
            if (alarmToggle.checked) warningSound.play();
          } else {
            motionIndicator.textContent = 'No Motion';
            motionIndicator.classList.remove('active');
            motionIndicator.classList.add('inactive');
            motionCard.classList.remove('active');
            document.getElementById('footerSecurity').textContent = data.pirEnabled ? 'Inactive' : 'Disabled';
            warningSound.pause();
            warningSound.currentTime = 0;
          }
        })
        .catch(() => {
          statusText.textContent = 'System Offline';
          statusDot.style.backgroundColor = '#f44336';
        });
    }

    // Event Listeners
    alarmToggle.addEventListener('change', () => {
      if (!alarmToggle.checked) {
        warningSound.pause();
        warningSound.currentTime = 0;
      }
    });

    document.getElementById('pumpToggle').addEventListener('change', (e) => {
      fetch('/control', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ pump: e.target.checked })
      });
    });

    document.getElementById('pirToggle').addEventListener('change', (e) => {
      fetch('/control', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ pir: e.target.checked })
      });
    });

    document.getElementById('lightToggle').addEventListener('change', (e) => {
      fetch('/control', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ light: e.target.checked })
      });
    });

    document.getElementById('autoToggle').addEventListener('change', (e) => {
      fetch('/control', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ auto: e.target.checked })
      });
    });

    // Initialize
    fetchData();
    setInterval(fetchData, 2000);
  </script>
</body>
</html>
)====";
  server.send(200, "text/html", html);
}

void handleData() {
  String json = "{";
  json += "\"temp\":" + String(tempC, 1) + ",";
  json += "\"humidity\":" + String(humidity, 1) + ",";
  json += "\"moisture\":" + String(moisturePercent) + ",";
  json += "\"motion\":" + String(motionAlert ? "true" : "false") + ",";
  json += "\"pump\":" + String(pumpOn ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void handleTogglePump() {
  if (pumpOn) {
    pumpOn = false;
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW);
    Serial.println("Pump OFF (manual)");
  } else {
    pumpOn = true;
    pumpStartTime = millis();
    digitalWrite(MOTOR_IN1, HIGH);
    digitalWrite(MOTOR_IN2, LOW);
    Serial.println("Pump ON (manual)");
  }
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP8266 DHT + Soil + IR + Motor Pump");

  dht.begin();
  pinMode(IRSENSOR, INPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Env Monitor Init");
  lcd.setCursor(0, 1);
  lcd.print("Please wait...");
  delay(2000);

  WiFi.softAP(ssid, password);
  Serial.print("WiFi AP started: ");
  Serial.println(ssid);

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/toggle", handleTogglePump);
  server.begin();
}

void loop() {
  server.handleClient();
  unsigned long currentMillis = millis();

  if (digitalRead(IRSENSOR) == LOW) {
    motionAlert = true;
    motionDisplayUntil = currentMillis + 2000;
    Serial.println("Motion Detected!");
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    tempC = dht.readTemperature();
    humidity = dht.readHumidity();
    soilValue = analogRead(A0);
    moisturePercent = map(soilValue, 1024, 600, 0, 100);
    moisturePercent = constrain(moisturePercent, 0, 100);

    if (moisturePercent < 30 && !pumpOn) {
      pumpOn = true;
      pumpStartTime = currentMillis;
      digitalWrite(MOTOR_IN1, HIGH);
      digitalWrite(MOTOR_IN2, LOW);
      Serial.println("Pump ON due to low moisture");
    }

    if (pumpOn) {
      if (moisturePercent >= 40 && (currentMillis - pumpStartTime >= minPumpDuration)) {
        pumpOn = false;
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, LOW);
        Serial.println("Pump OFF: Moisture restored");
      } else {
        digitalWrite(MOTOR_IN1, HIGH);
        digitalWrite(MOTOR_IN2, LOW);
      }
    }

    Serial.print("Temp: ");
    Serial.print(tempC);
    Serial.print("°C | Humidity: ");
    Serial.print(humidity);
    Serial.print("% | Soil: ");
    Serial.print(soilValue);
    Serial.print(" -> ");
    Serial.print(moisturePercent);
    Serial.println("%");

    lcd.clear();
    if (isnan(tempC) || isnan(humidity)) {
      lcd.setCursor(0, 0);
      lcd.print("DHT Error!");
    } else if (motionAlert && currentMillis < motionDisplayUntil) {
      lcd.setCursor(0, 0);
      lcd.print("Motion Detected!");
      lcd.setCursor(0, 1);
      lcd.print("Animal nearby!");
    } else {
      if (showSoil) {
        lcd.setCursor(0, 0);
        lcd.print("Soil: ");
        lcd.print(moisturePercent);
        lcd.print("%");
        lcd.setCursor(0, 1);
        if (pumpOn) {
          lcd.print("Auto Pump ON");
        } else {
          lcd.print("Soil OK");
        }
      } else {
        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(tempC);
        lcd.print((char)223);
        lcd.print("C");
        lcd.setCursor(0, 1);
        lcd.print("Humidity: ");
        lcd.print(humidity);
        lcd.print("%");
      }
      showSoil = !showSoil;
      motionAlert = false;
    }
  }
}