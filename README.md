# KrishiPal
 Smart Plant Monitoring System

# 🌿 Smart Plant Monitoring System

A sleek, modern web dashboard to monitor plant health in real-time using sensors and IoT. Built for plant lovers, students, and techies who care about sustainability and smart automation.

## 📸 Overview

This project is designed to monitor and control the environment of a plant using:
- 🌡️ Temperature & Humidity Sensor (DHT11)
- 🌱 Soil Moisture Sensor
- 🕵️ PIR Motion Sensor (for security)
- 💧 Water Pump (auto/manual control)
- 🖥️ A beautifully styled HTML/CSS dashboard for display and toggles

---

## ⚙️ Features

- 📊 **Live Sensor Data**: View real-time temperature, humidity, and soil moisture stats.
- 🧠 **Smart Logic**: Alerts you when soil moisture is too low.
- 🖼️ **Glassmorphic UI**: Aesthetic dashboard with icons and animation.
- 🌗 **Dark Mode Toggle**: Switch themes based on your vibe.
- 🔐 **PIR Motion Sensor Toggle**: For optional security monitoring.
- 💦 **Pump Control Toggle**: Easily turn the water pump on/off.

---

## 🧰 Tech Stack

| Tech | Description |
|------|-------------|
| ESP8266 (Wemos D1) | Main IoT microcontroller board |
| HTML5/CSS3 | Frontend dashboard |
| JavaScript | Dark mode + toggles |
| Blynk (optional) | Remote control (if connected) |
| Sensors | DHT11, Soil Moisture, PIR |

---

## 📦 How to Run

1. Connect your ESP8266 to:
   - DHT11 sensor
   - Soil Moisture sensor
   - PIR sensor
   - Relay + Water pump
2. Flash the Arduino code with correct pins and logic.
3. Serve the `index.html` file locally or host it on your ESP as a web server.
4. Open it in a browser to monitor/control your plant!

---

## 🚀 Future Improvements

- Add Firebase or Blynk integration for remote monitoring.
- Use MQTT or Node-RED for scalable IoT workflows.
- Add real-time charts with Chart.js for sensor history.

---

## 👨‍💻 Author

**Shoaib**, Pre-Final Year B.Tech (CS) Student  

---