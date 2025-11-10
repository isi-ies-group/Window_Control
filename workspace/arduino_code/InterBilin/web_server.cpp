#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "web_server.h"

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP32 Control Interface</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      background-color: #f4f4f4;
    }
    h2 { color: #333; }
    form, .pad, .section {
      background: white;
      display: inline-block;
      padding: 20px;
      border-radius: 12px;
      margin: 10px;
      box-shadow: 0 0 10px rgba(0,0,0,0.1);
    }
    input[type="text"] {
      width: 150px;
      padding: 6px;
      margin: 6px;
      border-radius: 5px;
      border: 1px solid #ccc;
    }
    button, input[type="submit"] {
      background-color: #0078D7;
      color: white;
      border: none;
      padding: 10px 20px;
      margin: 5px;
      border-radius: 6px;
      cursor: pointer;
      font-size: 14px;
      transition: background-color 0.1s ease;
    }
    button:hover, input[type="submit"]:hover {
      background-color: #005fa3;
    }
    .pad-grid {
      display: grid;
      grid-template-columns: 60px 60px 60px;
      gap: 10px;
      justify-content: center;
      align-items: center;
      margin-bottom: 10px;
    }
    .pad-grid button {
      width: 60px;
      height: 60px;
      font-size: 24px;
    }
    .pressed {
      background-color: #004b80 !important;
    }
    .manual-begin {
      background-color: #0078D7;
      font-size: 14px;
      padding: 10px 25px;
      border-radius: 6px;
      margin-top: 10px;
    }
    .manual-begin:hover {
      background-color: #005fa3;
    }
    .auto-btn {
      background-color: #28a745;
      font-size: 20px;
      padding: 15px 40px;
      margin-top: 25px;
      transition: background-color 0.2s ease;
    }
    .auto-btn:hover {
      background-color: #218838;
    }
    .auto-btn.active {
      background-color: #dc3545 !important;
    }
    .auto-btn.active:hover {
      background-color: #b52a38 !important;
    }
  </style>
</head>
<body>

  <!-- SECTION 1: Initial Configuration -->
  <h2>Initial Configuration</h2>
  <form action="/begin_config" method="get">
    <label>Latitude:</label> <input type="text" name="latitude"><br>
    <label>Longitude:</label> <input type="text" name="longitude"><br>
    <label>Pan:</label> <input type="text" name="pan"><br>
    <label>Tilt:</label> <input type="text" name="tilt"><br>
    <label>Tilt_correction:</label>
    <input type="checkbox" id="tilt_correction" name="tilt_correction" value="1"> Enable<br>
    <label for="country">Country:</label>
    <select id="country" name="country">
      <option value="">-- Select your country --</option>
      <option value="Spain">Spain</option>
      <option value="Portugal">Portugal</option>
      <option value="France">France</option>
      <option value="Germany">Germany</option>
      <option value="Italy">Italy</option>
      <option value="United_Kingdom">United Kingdom</option>
      <option value="Mexico">Mexico</option>
      <option value="Argentina">Argentina</option>
      <option value="Chile">Chile</option>
      <option value="Colombia">Colombia</option>
      <option value="Japan">Japan</option>
      <option value="Australia">Australia</option>
  </select><br>
    <button type="button" onclick="window.location.href='/begin_config'">Begin</button>
	<input type="submit" value="Submit">
    <button type="button" onclick="window.location.href='/reset'">Reset</button>
  </form>

  <!-- SECTION 2: Ephemeris Input -->
  <h2>Ephemeris Input</h2>
  <form action="/begin_ephemeris" method="get">
    <label>Azimuth:</label> <input type="text" name="azimuth"><br>
    <label>Elevation:</label> <input type="text" name="elevation"><br>
    <button type="button" onclick="window.location.href='/begin_config'">Begin</button>
	<input type="submit" value="Submit">
  </form>

  <!-- SECTION 3: Manual Movement -->
  <h2>Manual Movement</h2>
  <div class="section">
  <div class="pad-grid">
    <div></div>
    <button id="up" onmousedown="startMove('x_plus')" onmouseup="stopMove()" 
            ontouchstart="startMove('x_plus')" ontouchend="stopMove()">X+</button>
    <div></div>

    <button id="left" onmousedown="startMove('z_minus')" onmouseup="stopMove()" 
            ontouchstart="startMove('z_minus')" ontouchend="stopMove()">Z-</button>
    <div></div>
    <button id="right" onmousedown="startMove('z_plus')" onmouseup="stopMove()" 
            ontouchstart="startMove('z_plus')" ontouchend="stopMove()">Z+</button>

    <div></div>
    <button id="down" onmousedown="startMove('x_minus')" onmouseup="stopMove()" 
            ontouchstart="startMove('x_minus')" ontouchend="stopMove()">X-</button>
    <div></div>
  </div>
    <button class="manual-begin" type="button" onclick="window.location.href='/manual_begin'">Begin</button>
  </div>

  <!-- AUTO MODE BUTTON -->
  <br><br>
  <button id="autoBtn" class="auto-btn" type="button" onclick="toggleAutoMode()">AUTO MODE</button>

  <script>
    let interval = null;
    let autoMode = false;

    function startMove(direction) {
      const btn = document.getElementById(direction);
      btn.classList.add("pressed");
      sendMove(direction);
      interval = setInterval(() => sendMove(direction), 200);
    }

    function stopMove() {
      clearInterval(interval);
      interval = null;
      document.querySelectorAll(".pad-grid button").forEach(btn => btn.classList.remove("pressed"));
    }

    function sendMove(direction) {
      fetch(`/manual?dir=${direction}`)
        .then(res => console.log("Move:", direction))
        .catch(err => console.error(err));
    }

    function toggleAutoMode() {
      autoMode = !autoMode;
      const btn = document.getElementById('autoBtn');
      btn.classList.toggle('active');
      btn.innerText = autoMode ? 'STOP AUTO' : 'AUTO MODE';
      const endpoint = autoMode ? '/auto_mode_on' : '/auto_mode_off';
      fetch(endpoint)
        .then(res => console.log("Auto mode:", autoMode))
        .catch(err => console.error(err));
    }
  </script>

</body>
</html>
)rawliteral";


//---------------------- END HTML -----------------------
AsyncWebServer server(80);
const char* ssid = "SW-Prototype";
const char* password = "solar";

void serverInit(){
	Serial.println("Setting up ESP32 Access Point...");
	WiFi.softAP(ssid, password);

	IPAddress IP = WiFi.softAPIP();
	Serial.print("Access Point IP: ");
	Serial.println(IP);

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
	request->send(200, "text/html", index_html);
	});

	server.onNotFound([](AsyncWebServerRequest *request){
	request->send(404, "text/plain", "Not found");
	});

	server.begin();
	Serial.println("Web server started.");
}