#pragma once
#include <Arduino.h>

// Embedded HTML + CSS + JavaScript
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>ACEBOTT Car + Arm Dashboard</title>
  <style>
    body { font-family: Arial, sans-serif; background:#0d1117; color:#c9d1d9; margin:0; padding:20px; }
    h1 { margin-top:0; }
    .section { border:1px solid #30363d; padding:15px; margin-bottom:15px; border-radius:8px; background:#161b22; }
    button { margin:4px; padding:8px 12px; border:none; border-radius:4px; cursor:pointer; font-size:14px; transition: all 0.1s ease-in-out; }
    button:active { transform: scale(0.95); box-shadow: inset 0 4px 8px rgba(0, 0, 0, 0.4); filter: brightness(85%); } 
    .pin-btn { background:#21262d; color:#c9d1d9; }
    .action-btn { background:#238636; color:#fff; }
    .danger-btn { background:#f85149; color:#fff; }
    .move-btn { background:#21262d; color:#c9d1d9; min-width:80px; }
    .auto-btn { background:#8957e5; color:#fff; min-width:120px; padding:10px; font-weight:bold; }
    label { font-size:13px; margin-right:4px; }
    input[type="range"] { width:260px; }
    .status { font-size:13px; color:#8b949e; margin-bottom:10px; }
  </style>
</head>
<body>
  <h1>ACEBOTT Car + Arm Secure Dashboard</h1>
  <div class="status">
    <span id="lockStatus">Lock status: UNKNOWN</span> |
    <span id="distanceStatus">Distance: -- cm</span> |
    <span id="sessionStatus">Session: NONE</span>
  </div>

  <div class="section">
    <h2>PIN Unlock</h2>
    <div>
      <button class="pin-btn" onclick="sendPinDigit(1)">1</button>
      <button class="pin-btn" onclick="sendPinDigit(2)">2</button>
      <button class="pin-btn" onclick="sendPinDigit(3)">3</button>
      <button class="pin-btn" onclick="sendPinDigit(4)">4</button>
      <button class="pin-btn" onclick="sendPinDigit(5)">5</button>
      <button class="pin-btn" onclick="sendPinDigit(6)">6</button>
      <button class="pin-btn" onclick="sendPinDigit(7)">7</button>
      <button class="pin-btn" onclick="sendPinDigit(8)">8</button>
      <button class="pin-btn" onclick="sendPinDigit(9)">9</button>
      <button class="pin-btn" onclick="sendPinDigit(0)">0</button>
    </div>
    <button class="action-btn" onclick="clearPin()">Clear PIN</button>
    <button class="danger-btn" onclick="lockSystem()">Lock</button>
  </div>

  <div class="section">
    <h2>Autonomous Systems</h2>
    <button class="auto-btn" onclick="setAutoMode('follow')">Follow Mode</button>
    <button class="auto-btn" onclick="setAutoMode('avoid')">Avoid Mode</button>
    <button class="danger-btn" onclick="setAutoMode('manual')" style="padding:10px; font-weight:bold;">Halt Autonomous</button>
  </div>

  <div class="section">
    <h2>Car Movement (Hold to move. After a few seconds, cruise control is activates)</h2>
    <div>
      <button class="move-btn" onmousedown="startMove('forward')" onmouseup="stopMove()" ontouchstart="startMove('forward')" ontouchend="stopMove()">Forward</button>
    </div>
    <div>
      <button class="move-btn" onmousedown="startMove('left')" onmouseup="stopMove()" ontouchstart="startMove('left')" ontouchend="stopMove()">Left</button>
      <button class="move-btn" onclick="stopMove()">Stop</button>
      <button class="move-btn" onmousedown="startMove('right')" onmouseup="stopMove()" ontouchstart="startMove('right')" ontouchend="stopMove()">Right</button>
    </div>
    <div>
      <button class="move-btn" onmousedown="startMove('back')" onmouseup="stopMove()" ontouchstart="startMove('back')" ontouchend="stopMove()">Backward</button>
    </div>
    <div>
      <button class="move-btn" onmousedown="startMove('cw')" onmouseup="stopMove()" ontouchstart="startMove('cw')" ontouchend="stopMove()">Rotate CW</button>
      <button class="move-btn" onmousedown="startMove('ccw')" onmouseup="stopMove()" ontouchstart="startMove('ccw')" ontouchend="stopMove()">Rotate CCW</button>
    </div>
  </div>

  <div class="section">
    <h2>Arm Control</h2>
    <h3>Chassis</h3>
    <input type="range" min="0" max="180" value="90" id="chassisSlider" oninput="sendSlider('chassis', this.value)">
    <h3>Shoulder</h3>
    <input type="range" min="0" max="180" value="40" id="shoulderSlider" oninput="sendSlider('shoulder', this.value)">
    <h3>Elbow</h3>
    <input type="range" min="0" max="180" value="50" id="elbowSlider" oninput="sendSlider('elbow', this.value)">
    <h3>Wrist</h3>
    <input type="range" min="0" max="180" value="90" id="wristSlider" oninput="sendSlider('wrist', this.value)">
    <h3>Claw</h3>
    <input type="range" min="0" max="180" value="90" id="clawsSlider" oninput="sendSlider('claws', this.value)">
  </div>

  <script>
    let sessionToken = null;

    function sendPinDigit(digit) {
      fetch('/pin?digit=' + digit)
        .then(r => r.json())
        .then(data => {
          if (data.token) {
            sessionToken = data.token;
            document.getElementById('sessionStatus').textContent = 'Session: ACTIVE';
          }
          if (data.locked !== undefined) {
            document.getElementById('lockStatus').textContent = 'Lock status: ' + (data.locked ? 'LOCKED' : 'UNLOCKED');
          }
        });
    }

    function clearPin() {
      fetch('/pin/clear').then(r => r.text()).then(_ => {
        sessionToken = null;
        document.getElementById('sessionStatus').textContent = 'Session: NONE';
      });
    }

    function lockSystem() {
      fetch('/lock').then(r => r.text()).then(_ => {
        sessionToken = null;
        document.getElementById('sessionStatus').textContent = 'Session: NONE';
      });
    }

    function sendMove(dir) {
      let url = '/move?dir=' + dir;
      if (sessionToken) url += '&session=' + encodeURIComponent(sessionToken);
      fetch(url);
    }

    function setAutoMode(mode) {
      let url = '/auto?mode=' + mode;
      if (sessionToken) url += '&session=' + encodeURIComponent(sessionToken);
      fetch(url);
    }

    function startMove(dir) { sendMove(dir); }
    function stopMove() { sendMove('stop'); }

    function sendSlider(joint, angle) {
      let url = '/arm/slider?joint=' + joint + '&angle=' + angle;
      if (sessionToken) url += '&session=' + encodeURIComponent(sessionToken);
      fetch(url);
    }

    function refreshStatus() {
      fetch('/status')
        .then(r => r.json())
        .then(data => {
          document.getElementById('lockStatus').textContent = 'Lock status: ' + (data.locked ? 'LOCKED' : 'UNLOCKED');
          document.getElementById('distanceStatus').textContent = 'Distance: ' + data.distance + ' cm';
          document.getElementById('sessionStatus').textContent = 'Session: ' + (data.sessionActive ? 'ACTIVE' : 'NONE');
        });
    }

    setInterval(refreshStatus, 2000);
    refreshStatus();
  </script>
</body>
</html>
)rawliteral";
