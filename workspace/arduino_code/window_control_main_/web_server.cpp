#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "state_machine.h"
#include "global_structs.h"
#include "storage.h"
#include "gps.h"
#include "eph_input_mode.h"
#include "manual_mode.h"
#include "movement_task.h"
#include "commonlib.h"
#include "gps.h"


//Paste this below Manual Movement Pad and uncomment functions in manual_mode and movement
  // for indepedendent motor control


  // <!-- PAD 1: MXLE / MXLI -->
  // <h3>MXL (External / Internal)</h3>
  // <div class="pad-grid">
  //   <div></div>
  //   <button id="mxle_plus"
  //     onmousedown="startMove('mxle_plus')" onmouseup="stopMove()"
  //     ontouchstart="startMove('mxle_plus')" ontouchend="stopMove()" ontouchcancel="stopMove()">MXLE+</button>
  //   <div></div>

  //   <button id="mxli_minus"
  //     onmousedown="startMove('mxli_minus')" onmouseup="stopMove()"
  //     ontouchstart="startMove('mxli_minus')" ontouchend="stopMove()" ontouchcancel="stopMove()">MXLI-</button>
  //   <div></div>

  //   <button id="mxli_plus"
  //     onmousedown="startMove('mxli_plus')" onmouseup="stopMove()"
  //     ontouchstart="startMove('mxli_plus')" ontouchend="stopMove()" ontouchcancel="stopMove()">MXLI+</button>
  //   <div></div>

  //   <button id="mxle_minus"
  //     onmousedown="startMove('mxle_minus')" onmouseup="stopMove()"
  //     ontouchstart="startMove('mxle_minus')" ontouchend="stopMove()" ontouchcancel="stopMove()">MXLE-</button>
  //   <div></div>
  // </div>
  // <!-- PAD 2: MXRE / MXRI -->
  // <h3>MXR (External / Internal)</h3>
  // <div class="pad-grid">
  //   <div></div>
  //   <button id="mxre_plus"
  //     onmousedown="startMove('mxre_plus')" onmouseup="stopMove()"
  //     ontouchstart="startMove('mxre_plus')" ontouchend="stopMove()" ontouchcancel="stopMove()">MXRE+</button>
  //   <div></div>

  //   <button id="mxri_minus"
  //     onmousedown="startMove('mxri_minus')" onmouseup="stopMove()"
  //     ontouchstart="startMove('mxri_minus')" ontouchend="stopMove()" ontouchcancel="stopMove()">MXRI-</button>
  //   <div></div>

  //   <button id="mxri_plus"
  //     onmousedown="startMove('mxri_plus')" onmouseup="stopMove()"
  //     ontouchstart="startMove('mxri_plus')" ontouchend="stopMove()" ontouchcancel="stopMove()">MXRI+</button>
  //   <div></div>

  //   <button id="mxre_minus"
  //     onmousedown="startMove('mxre_minus')" onmouseup="stopMove()"
  //     ontouchstart="startMove('mxre_minus')" ontouchend="stopMove()" ontouchcancel="stopMove()">MXRE-</button>
  //   <div></div>
  // </div>

  // <!-- PAD 3: MZL / MZR -->
  // <h3>MZ (Left / Right)</h3>
  // <div class="pad-grid">
  //   <div></div>
  //   <button id="mzl_plus"
  //     onmousedown="startMove('mzl_plus')" onmouseup="stopMove()"
  //     ontouchstart="startMove('mzl_plus')" ontouchend="stopMove()" ontouchcancel="stopMove()">MZL+</button>
  //   <div></div>

  //   <button id="mzr_minus"
  //     onmousedown="startMove('mzr_minus')" onmouseup="stopMove()"
  //     ontouchstart="startMove('mzr_minus')" ontouchend="stopMove()" ontouchcancel="stopMove()">MZR-</button>
  //   <div></div>

  //   <button id="mzr_plus"
  //     onmousedown="startMove('mzr_plus')" onmouseup="stopMove()"
  //     ontouchstart="startMove('mzr_plus')" ontouchend="stopMove()" ontouchcancel="stopMove()">MZR+</button>
  //   <div></div>

  //   <button id="mzl_minus"
  //     onmousedown="startMove('mzl_minus')" onmouseup="stopMove()"
  //     ontouchstart="startMove('mzl_minus')" ontouchend="stopMove()" ontouchcancel="stopMove()">MZL-</button>
  //   <div></div>
  // </div>



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
    form, .pad, .section, .status-box {
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
      user-select: none;
      touch-action: manipulation;
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
      width: 80px;
      height: 60px;
      font-size: 16px;
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
    .auto-btn {
      background-color: #28a745;
      font-size: 20px;
      padding: 15px 40px;
      margin-top: 25px;
    }
    .auto-btn.active {
      background-color: #dc3545 !important;
    }
  </style>
</head>

<body>

<!-- SECTION 1: Initial Configuration -->
<h2>Initial Configuration</h2>
<form action="/config_submit" method="get">
  Latitude: <input type="text" name="latitude"><br>
  Longitude: <input type="text" name="longitude"><br>
  Pan: <input type="text" name="pan"><br>
  Tilt: <input type="text" name="tilt"><br>
  Tilt correction:
  <input type="checkbox" name="tilt_correction" value="1"> Enable<br>

  Country:
  <select name="country">
    <option value="">-- Select --</option>
    <option value="Spain">Spain</option>
    <option value="Spain_Canary">Spain (Canary)</option>
    <option value="UK">United Kingdom</option>
    <option value="Poland">Poland</option>
    <option value="Argentina">Argentina</option>
  </select><br>

  <button type="button" onclick="location.href='/config_begin'">Begin</button>
  <input type="submit" value="Submit">
  <button type="button" onclick="location.href='/end_config'">End</button>
</form>

<!-- SECTION 2: Ephemeris Input -->
<h2>Ephemeris Input</h2>
<form action="/eph_submit" method="get">
  Azimuth: <input type="text" name="azimuth"><br>
  Elevation: <input type="text" name="elevation"><br>
  <button type="button" onclick="location.href='/eph_begin'">Begin</button>
  <input type="submit" value="Submit">
  <button type="button" onclick="location.href='/end_eph'">End</button>
</form>

<!-- SECTION 3: Manual Movement -->
<h2>Manual Movement</h2>
<div class="section">
  <h3>Global X / Z</h3>
  <div class="pad-grid">
    <div></div>
    <button id="x_plus"
      onmousedown="startMove('x_plus')" onmouseup="stopMove()"
      ontouchstart="startMove('x_plus')" ontouchend="stopMove()">X+</button>
    <div></div>

    <button id="z_minus"
      onmousedown="startMove('z_minus')" onmouseup="stopMove()"
      ontouchstart="startMove('z_minus')" ontouchend="stopMove()">Z-</button>
    <div></div>

    <button id="z_plus"
      onmousedown="startMove('z_plus')" onmouseup="stopMove()"
      ontouchstart="startMove('z_plus')" ontouchend="stopMove()">Z+</button>
    <div></div>

    <button id="x_minus"
      onmousedown="startMove('x_minus')" onmouseup="stopMove()"
      ontouchstart="startMove('x_minus')" ontouchend="stopMove()">X-</button>
    <div></div>
  </div>
  
  <form action="/manual_goto" method="get">
    X target: <input type="text" name="x"><br>
    Z target: <input type="text" name="z"><br>
    <input type="submit" value="Go to X / Z">
  </form>

  <button class="manual-begin" onclick="location.href='/manual_begin'">Begin</button>
  <button class="manual-begin" onclick="location.href='/end_manual'">End</button>
</div>

<!-- SECTION 4: Status -->
<h2>System Status</h2>
<div class="status-box">
  State: <b><span id="st">---</span></b><br>
  Latitude: <span id="lat">---</span><br>
  Longitude: <span id="lon">---</span><br>
  Pan: <span id="pan">---</span><br>
  Tilt: <span id="tilt">---</span><br>
  Tilt correction: <span id="tc">---</span><br>
  Date: <span id="date">---</span><br>
  Time: <span id="time">---</span><br>
  X position: <span id="x">---</span><br>
  Z position: <span id="z">---</span><br>
</div>

<!-- SECTION 5: Sun Angles -->
<h2>Sun-Surface Angles</h2>
<div class="status-box">
  Azimuth: <span id="az">---</span><br>
  Elevation: <span id="ele">---</span><br>
  AOIl: <span id="aoil">---</span><br>
  AOIt: <span id="aoit">---</span><br>
</div>

<! -- SECTION 6: AUTO MODE -->
<br><br>
<button id="autoBtn" class="auto-btn" onclick="toggleAutoMode()">AUTO MODE</button>


<! -- SECTION 7: SYSTEM_BUTTON -->
<hr>

<div style="display:flex; justify-content:space-between; gap:10px; margin-top:20px;">

  <form action="/reset" method="post" style="flex:1;">
    <button type="submit"
            style="width:100%; padding:12px; font-size:16px; background:#c00; color:white;">
      RESET
    </button>
  </form>

  <form action="/sleep" method="post" style="flex:1;">
    Sleep duration (minutes): <input type="number" name="minutes" min="1" max="1440">
    <button type="submit"
            style="width:100%; padding:12px; font-size:16px;">
      SLEEP
    </button>
  </form>

  <form action="/settime" method="post" style="flex:1;">
    <button type="submit"
            style="width:100%; padding:12px; font-size:16px;">
      SET TIME
    </button>
  </form>

</div>



<script>
let interval = null;
let autoMode = false;

setInterval(() => {
  fetch('/status')
    .then(r => r.json())
    .then(d => {
      st.innerText=d.state;
      lat.innerText=d.latitude;
      lon.innerText=d.longitude;
      pan.innerText=d.pan;
      tilt.innerText=d.tilt;
      tc.innerText=d.tilt_correction?"Enabled":"Disabled";
      date.innerText=d.date;
      time.innerText=d.time;
      x.innerText=d.x;
      z.innerText=d.z;
    });
}, 600);

setInterval(() => {
  fetch('/sunstatus')
    .then(r => r.json())
    .then(d => {
      az.innerText=d.azimuth;
      ele.innerText=d.elevation;
      aoil.innerText=d.aoil;
      aoit.innerText=d.aoit;
    });
}, 600);

function startMove(dir){
  document.getElementById(dir).classList.add("pressed");
  sendMove(dir);
  interval=setInterval(()=>sendMove(dir),500);
}
function stopMove(){
  clearInterval(interval);
  document.querySelectorAll(".pad-grid button")
    .forEach(b=>b.classList.remove("pressed"));
}
function sendMove(dir){
  fetch(`/manual?dir=${dir}`);
}

function toggleAutoMode(){
  autoMode=!autoMode;
  autoBtn.classList.toggle("active");
  autoBtn.innerText=autoMode?"STOP AUTO":"AUTO MODE";
  fetch(autoMode?"/auto_mode_on":"/auto_mode_off");
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
    if (thisSt != STDBY) {
      request->send(200, "text/html",
        "<p style='color:red;'>Go to STANDBY first.</p>");
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


  // ----- CONFIG END-----

  server.on("/end_config", HTTP_GET, [](AsyncWebServerRequest *request) {

  if (thisSt != CONFIG) {
    request->send(200, "text/html",
      "<p style='color:red;'>Not in Config mode.</p>");
    return;
  }

  changeState(fsmProcess(end_config, false));
  Serial.println("[FSM] End EPH_INPUT");

  request->send(200, "text/html",
    "<p style='color:green;'>Config ended.</p>");
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
    if (thisSt != MANUAL) return;
    if (!request->hasParam("dir")) {
      request->send(400, "text/plain", "Missing dir");
      return;
    }
    String dir = request->getParam("dir")->value();
    Serial.printf("[WEB] Recieved dir = %s\n", dir.c_str());
    manualMode(dir);
    request->send(200, "text/plain", "OK");
  });

  // ----- MANUAL GOTO X / Z -----
  server.on("/manual_goto", HTTP_GET, [](AsyncWebServerRequest *request) {

    if (thisSt != MANUAL) {
      request->send(200, "text/html",
        "<p style='color:red;'>Not in manual mode.</p>");
      return;
    }

    bool updated = false;

    // X
    if (request->hasParam("x")) {
      String xStr = request->getParam("x")->value();
      if (xStr.length() > 0) {    
        g_x_val = xStr.toDouble();
        updated = true;
      }
    }

    // Z
    if (request->hasParam("z")) {
      String zStr = request->getParam("z")->value();
      if (zStr.length() > 0) {            
        g_z_val = zStr.toDouble();
        updated = true;
      }
    }

    if (!updated) {
      request->send(200, "text/html",
        "<p style='color:orange;'>No values updated.</p>");
      return;
    }

    Serial.printf("[MANUAL_GOTO] Target X=%.3f Z=%.3f\n", g_x_val, g_z_val);
    requestMove();

    request->send(200, "text/html",
      "<p style='color:green;'>Manual target updated.</p>");
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
    changeState(fsmProcess(toggle_auto_mode, auto_on));
    request->send(200, "text/html",
      "<p style='color:green;'>Auto mode enabled.</p>");
    Serial.println("[FSM] Auto mode ON");
  });


  // ----- AUTO MODE OFF -----
  server.on("/auto_mode_off", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (thisSt != AUTO_MODE) { 
      request->send(200, "text/html",
        "<p style='color:red;'>Not in Auto mode.</p>");
      return;   
    }
    changeState(fsmProcess(toggle_auto_mode, auto_on));
    request->send(200, "text/html",
    "<p style='color:green;'>Auto mode disabled.</p>");
    Serial.println("[FSM] Auto mode OFF");
  });



  // ----- STATUS -----
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){

      time_t now;
      struct tm timeinfo;
      char date_str[11];
      char time_str[9];

      time(&now);
      localtime_r(&now, &timeinfo);

      strftime(date_str, sizeof(date_str), "%Y-%m-%d", &timeinfo);
      strftime(time_str, sizeof(time_str), "%H:%M:%S", &timeinfo);

      String json = "{";
      json += "\"state\":\"" + stateToText(thisSt) + "\",";
      json += "\"latitude\":" + String(g_SPAInputs.latitude, 2) + ",";
      json += "\"longitude\":" + String(g_SPAInputs.longitude, 2) + ",";
      json += "\"pan\":" + String(g_AOIInputs.pan, 2) + ",";
      json += "\"tilt\":" + String(g_AOIInputs.tilt, 2) + ",";
      json += "\"tilt_correction\":" + String(g_AOIInputs.tilt_correction ? "true" : "false") + ",";
      json += "\"date\":\"" + String(date_str) + "\",";
      json += "\"time\":\"" + String(time_str) + "\",";
      json += "\"x\":" + String(g_x_val, 6) + ",";
      json += "\"z\":" + String(g_z_val, 6);
      json += "}";

      request->send(200, "application/json", json);
  });



  // ----- SUN STATUS -----

  server.on("/sunstatus", HTTP_GET, [](AsyncWebServerRequest *request){

      String json = "{";
      json += "\"azimuth\":"   + String(g_AOIInputs.azimuth, 6) + ",";
      json += "\"elevation\":" + String(g_AOIInputs.elevation, 6) + ",";
      json += "\"aoil\":"      + String(g_InterpolInputs.AOIl, 6) + ",";
      json += "\"aoit\":"      + String(g_InterpolInputs.AOIt, 6);
      json += "}";

      request->send(200, "application/json", json);
  });

  // Not found
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
  });
 
  // ----- RESET -----
  server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", "<p>Restarting...</p>");
    Serial.println("[WEB] Reset");
    delay(500);
    ESP.restart();
  });

// ----- SLEEP -----
server.on("/sleep", HTTP_POST, [](AsyncWebServerRequest *request) {

    if (thisSt != STDBY && thisSt != AUTO_MODE) {
        request->send(403, "text/html",
            "<p style='color:red;'>Sleep only allowed from STDBY</p>");
        return;
    }

    if (!request->hasParam("minutes", true)) {
      request->send(400, "text/html", "Missing duration");
      return;
    }

    int mins = request->getParam("minutes", true)->value().toInt();
    if (mins <= 0) mins = 1;

    Serial.printf("[WEB] Deep sleep requested for %d minutes\n", mins);

    time_t now;
    time(&now);
    g_sunrise_epoch = now + mins * 60;
    
    changeState(fsmProcess(go_sleep, auto_on));

    request->send(200, "text/html",
        "<p>Entering deep sleep...</p>");
});

// ----- set_time -----
server.on("/settime", HTTP_POST, [](AsyncWebServerRequest *request) {
  manual_time = false;
  request->send(200, "text/html", "<p>Setting time...</p>");
  Serial.println("[WEB] Reset");
  delay(500);
  setLocalTime();
});

  server.begin();
  Serial.println("Web server started.");

//-------- Date-time ---------
server.on("/set_datetime", HTTP_POST, [](AsyncWebServerRequest *request) {
  manual_time = true;
  if (!request->hasParam("date", true) || !request->hasParam("time", true)) {
    request->send(400, "text/html", "<p>Missing date or time</p>");
    return;
  }

  String dateStr = request->getParam("date", true)->value();
  String timeStr = request->getParam("time", true)->value();

  int year, month, day;
  int hour, min, sec;

  if (sscanf(dateStr.c_str(), "%d-%d-%d", &year, &month, &day) != 3 ||
      sscanf(timeStr.c_str(), "%d:%d:%d", &hour, &min, &sec) != 3) {
    request->send(400, "text/html", "<p>Invalid date/time format</p>");
    return;
  }

  struct tm t = {};
  t.tm_year = year - 1900;
  t.tm_mon  = month - 1;
  t.tm_mday = day;
  t.tm_hour = hour;
  t.tm_min  = min;
  t.tm_sec  = sec;
  t.tm_isdst = -1;

  // IMPORTANTE: interpretamos lo que mete el usuario como LOCAL TIME
  time_t local_epoch = mktime(&t);

  struct timeval now = {
   
  setSystemTimeManualLocal(year, month, day, hour, min, sec);

  printLocalTime();

  request->send(200, "text/html",
    "<p style='color:green;'>Date and time set manually.</p>");
});

}



