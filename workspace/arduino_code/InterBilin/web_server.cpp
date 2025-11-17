#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "state_machine.h"
#include "global_structs.h"
#include "storage.h"
#include "gps.h"
#include "eph_input_mode.h"
#include "manual_mode.h"

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
  <form action="/config_submit" method="get">
    <label>Latitude:</label> <input type="text" name="latitude"><br>
    <label>Longitude:</label> <input type="text" name="longitude"><br>
    <label>Pan:</label> <input type="text" name="pan"><br>
    <label>Tilt:</label> <input type="text" name="tilt"><br>
    <label>Tilt correction:</label>
    <input type="checkbox" name="tilt_correction" value="1"> Enable<br>
    <label for="country">Country:</label>
    <select id="country" name="country">
      <option value="">-- Select your country --</option>
      <option value="Spain">Spain</option>
      <option value="Spain_Canary">Spain (Canary Islands)</option>
      <option value="UK">United Kingdom</option>
      <option value="Poland">Poland</option>
      <option value="Argentina">Argentina</option>
    </select><br>

    <button type="button" onclick="window.location.href='/config_begin'">Begin</button>
    <input type="submit" value="Submit">
    <button type="button" onclick="window.location.href='/reset'">Reset</button>
	</form>

  <!-- SECTION 2: Ephemeris Input -->
  <h2>Ephemeris Input</h2>
  <form action="/eph_submit" method="get">
    <label>Azimuth:</label> <input type="text" name="azimuth"><br>
    <label>Elevation:</label> <input type="text" name="elevation"><br>
    <button type="button" onclick="window.location.href='/eph_begin'">Begin</button>
    <input type="submit" value="Submit">
    <button type="button" onclick="window.location.href='/end_eph'">End</button>
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
    <button class="manual-begin" type="button" onclick="window.location.href='/end_manual'">End</button>
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
bool auto_mode_on = false;
bool config_active = false;
void serverInit() {

  Serial.println("Setting up ESP32 Access Point...");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point IP: ");
  Serial.println(IP);

  // Página principal
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/html", index_html);
  });

  // ----- CONFIG BEGIN -----
  server.on("/config_begin", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (thisSt == AUTO_MODE) {
      request->send(200, "text/html",
        "<p style='color:red;'>AutoMode must be off.</p>");
      return;
    }

    config_active = true;
    changeState(fsmProcess(begin_config, false));
    request->send(200, "text/html",
      "<p style='color:green;'>Config mode started. Fill the gaps and press 'Submit'.</p>");
    Serial.println("[FSM] Config state");
  });

  // ----- CONFIG SUBMIT -----
  server.on("/config_submit", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!config_active || thisSt != CONFIG) {
      request->send(200, "text/html",
        "<p style='color:red;'>Error: Press Begin before sending parameters.</p>");
      return;
    }

    if (request->hasParam("latitude") && request->hasParam("longitude") &&
      request->hasParam("pan") && request->hasParam("tilt") && request->hasParam("country")) {

      g_SPAInputs.latitude  = request->getParam("latitude")->value().toDouble();
      g_SPAInputs.longitude = request->getParam("longitude")->value().toDouble();
      g_AOIInputs.pan       = request->getParam("pan")->value().toDouble();
      g_AOIInputs.tilt      = request->getParam("tilt")->value().toDouble();
      g_AOIInputs.tilt_correction = request->hasParam("tilt_correction");

      g_country = request->getParam("country")->value();

      bool ok = true;
      String errorMsg = "<p style='color:red;'>Wrong parameters:</p><ul>";

      if (g_SPAInputs.latitude < -90 || g_SPAInputs.latitude > 90) {
        ok = false; errorMsg += "<li>Latitude range: -90 y 90</li>";
      }
      if (g_SPAInputs.longitude < -180 || g_SPAInputs.longitude > 180) {
        ok = false; errorMsg += "<li>Longitude range: -180 y 180</li>";
      }

        errorMsg += "</ul>";

      if (!ok) {
        request->send(200, "text/html", errorMsg);
        Serial.println("[CONFIG] Invalid parameter. Try again.");
        return;
      }

      saveData();
      setLocalTime();
      Serial.println("[CONFIG] Data correctly saved in flash.");

        config_active = false;
        changeState(fsmProcess(end_config, auto_mode_on));

        request->send(200, "text/html",
          "<p style='color:green;'>Config saved. Back to standby.</p>");
    } else {
      request->send(200, "text/html",
        "<p style='color:red;'>Missing parameters.</p>");
    }
  });

  // ----- RESET -----
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", "<p>Restarting...</p>");
    Serial.println("[WEB] Reset");
    delay(500);
    ESP.restart();
  });


  // ----- EPH INPUT BEGIN -----
  
 server.on("/eph_begin", HTTP_GET, [](AsyncWebServerRequest *request) {

  if (thisSt == AUTO_MODE) {
    request->send(200, "text/html",
      "<p style='color:red;'>AutoMode must be off.</p>");
    return;
  }

  changeState(fsmProcess(begin_eph_input, false));
  Serial.println("[FSM] Begin EPH_INPUT");

  request->send(200, "text/html",
    "<p style='color:green;'>EPH input started.</p>");
});

  // ----- EPH INPUT SUBMIT -----

server.on("/eph_submit", HTTP_GET, [](AsyncWebServerRequest *request) {

  if (thisSt != EPH_INPUT) {
      request->send(200, "text/html",
          "<p style='color:red;'>Error: Press Begin first.</p>");
      return;
  }

  if (!request->hasParam("azimuth") || !request->hasParam("elevation")) {
    request->send(200, "text/html",
      "<p style='color:red;'>Missing parameters.</p>");
    return;
  }

  g_AOIInputs.azimuth   = request->getParam("azimuth")->value().toDouble();
  g_AOIInputs.elevation = request->getParam("elevation")->value().toDouble();

  Serial.printf("[EPH] Received az=%.2f el=%.2f\n", g_AOIInputs.azimuth, g_AOIInputs.elevation);
  ephInputMode();
  request->send(200, "text/html",
    "<p style='color:green;'>EPH submitted.</p>");
});


  // ----- END EPH INPUT -----

  server.on("/end_eph", HTTP_GET, [](AsyncWebServerRequest *request) {

  if (thisSt != EPH_INPUT) {
    request->send(200, "text/html",
      "<p style='color:red;'>Not in ephemeris mode.</p>");
    return;
  }

  changeState(fsmProcess(end_eph_input, false));
  Serial.println("[FSM] End EPH_INPUT");

  request->send(200, "text/html",
    "<p style='color:green;'>EPH input ended.</p>");
});


  // ----- MANUAL BEGIN -----

  server.on("/manual_begin", HTTP_GET, [](AsyncWebServerRequest *request) {

    if (thisSt == AUTO_MODE) {
      request->send(200, "text/html",
        "<p style='color:red;'>AutoMode must be off.</p>");
      return;
    }

    changeState(fsmProcess(begin_manual, false));

    request->send(200, "text/html",
      "<p style='color:green;'>Manual mode started.</p>");
    Serial.println("[FSM] Manual begin");
  });


  // ----- MANUAL MOVEMENT -----

  server.on("/manual", HTTP_GET, [](AsyncWebServerRequest *request) {

    if (request->hasParam("dir")) {
      String dir = request->getParam("dir")->value();
      manualUpdate(dir);
    }
  });


  // ----- END MANUAL -----
  server.on("/end_manual", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(thisSt != MANUAL){
      request->send(200, "text/html",
        "<p style='color:red;'>Not in manual mode.</p>");
      return;
    }

    changeState(fsmProcess(end_manual, false));

    request->send(200, "text/html",
      "<p style='color:green;'>Manual mode finished.</p>");
    Serial.println("[FSM] Manual end");
  });

    // ----- AUTO MODE ON -----
  server.on("/auto_mode_on", HTTP_GET, [](AsyncWebServerRequest *request) {
  
    if (thisSt != STDBY) {
      request->send(200, "text/html",
        "<p style='color:red;'>AutoMode can only start from STDBY.</p>");
      return;
    }
    auto_mode_on = true;
    changeState(fsmProcess(toggle_auto_mode, false));
    request->send(200, "text/html",
      "<p style='color:green;'>Auto mode enabled.</p>");
    Serial.println("[FSM] Auto mode ON");
  });


  // ----- AUTO MODE OFF -----
  server.on("/auto_mode_off", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!auto_mode_on) { 
      request->send(200, "text/html",
        "<p style='color:red;'>Not in Auto mode.</p>");
      return;   
    }
    changeState(fsmProcess(toggle_auto_mode, true));
    request->send(200, "text/html",
    "<p style='color:green;'>Auto mode disabled.</p>");
    Serial.println("[FSM] Auto mode OFF");
  });


  // Not found
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
  });

  server.begin();
  Serial.println("Web server started.");
}