#include <WiFi.h>
#include <WebServer.h>
#include "secrets.h"         
#include "WebFrontend.h"
#include "SecuritySystem.h"
#include "RobotHardware.h"

// Global Objects
WebServer server(80);
SecuritySystem security;
RobotHardware robot;

// Helper: Validate Web Request
bool handleSecurityCheck() {  
    String token = server.hasArg("session") ? server.arg("session") : "";
    AuthStatus status = security.validateAccess(token);
    
    if (status == AuthStatus::RATE_LIMITED) {
        server.send(429, "text/plain", "Too Many Requests");
        return false;
    } else if (status != AuthStatus::OK) {
        server.send(403, "text/plain", "Forbidden");
        return false;
    }
    return true;
}


// HTTP ROUTE HANDLERS
void handleRoot() {
    server.send_P(200, "text/html", INDEX_HTML);
}

void handleStatus() {
    robot.updateTelemetry();
    String json = "{";
    json += "\"locked\":" + String(security.systemLocked ? "true" : "false") + ",";
    json += "\"distance\":" + String(robot.distance) + ",";
    json += "\"sessionActive\":" + String(security.sessionActive ? "true" : "false");
    json += "}";
    server.send(200, "application/json", json);
}

void handlePin() {
    if (!server.hasArg("digit")) {
        server.send(400, "application/json", "{\"status\":\"ERROR\"}");
        return;
    }
    int digit = server.arg("digit").toInt();
    Serial.printf("[SECURITY] PIN digit pressed: %d\n", digit);
    PinResult result = security.processPinDigit(digit);

    if (result == PinResult::LOCKED_OUT) {
        server.send(403, "application/json", "{\"status\":\"LOCKED_OUT\"}");
    } else if (result == PinResult::UNLOCKED) {
        String json = "{\"status\":\"UNLOCKED\",\"locked\":false,\"token\":\"" + security.getSessionToken() + "\"}";
        server.send(200, "application/json", json);
        Serial.println("[SECURITY] System Unlocked.");
    } else if (result == PinResult::PIN_INCORRECT) {
        robot.setMode(AutoMode::MANUAL); 
        server.send(200, "application/json", "{\"status\":\"PIN_INCORRECT\",\"locked\":true}");
        Serial.println("[SECURITY] Incorrect PIN.");
    } else {
        server.send(200, "application/json", "{\"status\":\"DIGIT_OK\"}");
    }
}

void handlePinClear() {
    Serial.println("[SECURITY] PIN input cleared.");
    security.clearPin();
    server.send(200, "text/plain", "PIN CLEARED");
}

void handleLock() {
    security.lockSystem();
    robot.setMode(AutoMode::MANUAL);
    Serial.println("[SECURITY] System locked via UI.");
    server.send(200, "text/plain", "LOCKED");
}

void handleMove() {
    if (!handleSecurityCheck()) return;

    String dir = server.arg("dir");
    Serial.print("[EXEC] Movement: "); Serial.println(dir);
    robot.moveDirection(dir);
    server.send(200, "text/plain", "OK");
}

void handleAuto() {
    if (!handleSecurityCheck()) return;

    String modeStr = server.arg("mode");
    if (modeStr == "follow") {
        robot.setMode(AutoMode::FOLLOW);
        Serial.println("[EXEC] Auto FOLLOW");
    } else if (modeStr == "avoid") {
        robot.setMode(AutoMode::AVOID);
        Serial.println("[EXEC] Auto AVOID");
    } else {
        robot.setMode(AutoMode::MANUAL);
        Serial.println("[EXEC] Auto HALTED");
    }
    server.send(200, "text/plain", "OK");
}

void handleArmSlider() {
    if (!handleSecurityCheck()) return;

    String joint = server.arg("joint");
    int angle = constrain(server.arg("angle").toInt(), 0, 180);
    
    Serial.printf("[EXEC] Arm [%s] -> %d\n", joint.c_str(), angle);
    robot.moveJoint(joint, angle);
    server.send(200, "text/plain", "SLIDER OK");
}

// SETUP + LOOP
void setup() {
    Serial.begin(115200);
    Serial.println("\n[BOOT] Starting ACEBOTT Secure Robot...");

    // Initialize subsystems
    robot.init();
    security.init(SECRET_PIN, SECRET_TOKEN_KEY);

    // Network setup
    Serial.println("[BOOT] Connecting to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(SECRET_SSID, SECRET_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500); Serial.print(".");
    }
    Serial.println("\n[NETWORK] Connected!");
    Serial.print("[NETWORK] IP: "); Serial.println(WiFi.localIP());

    // Routing definitions
    server.on("/", HTTP_GET, handleRoot);
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/pin", HTTP_GET, handlePin);
    server.on("/pin/clear", HTTP_GET, handlePinClear);
    server.on("/lock", HTTP_GET, handleLock);
    server.on("/move", HTTP_GET, handleMove);
    server.on("/auto", HTTP_GET, handleAuto);
    server.on("/arm/slider", HTTP_GET, handleArmSlider);

    server.begin();
    Serial.println("[SYSTEM] Web server started.");
}

void loop() {
    server.handleClient();

    // 1. Security auto-lock tick
    if (security.checkAutoLock()) {
        robot.setMode(AutoMode::MANUAL);
        Serial.println("[SECURITY] Auto-lock triggered.");
    }

    // 2. Hardware autonomous tick
    if (!security.systemLocked && robot.currentMode != AutoMode::MANUAL) {
        robot.handleAutonomousTasks();
    }
}
