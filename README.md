# ArduinoRobotCarWithArm
Secure object-oriented ESP32 web server for an autonomous robot, featuring a custom Intrusion Detection System (IDS), DoS mitigation, and cryptographic access control.

# Secure ESP32 Robotics Platform & Custom Web IDS

A custom object-oriented embedded web server for an ESP32-based autonomous robot. This project focuses heavily on **Application Security (AppSec)** and **Defensive Engineering**, featuring a modular C++ architecture, a custom Intrusion Detection System (IDS), and Denial of Service (DoS) mitigation.

## Security Features & Architecture

This project was built to demonstrate secure coding practices in resource-constrained IoT/embedded environments. The original monolithic codebase was refactored into a scalable Object-Oriented structure.

* **Custom IAM & Access Control:** 
  * Implemented a secure PIN-based authentication system with brute-force lockouts.
  * Utilizes cryptographic session token signing (FNV-1a hash algorithm) to prevent session hijacking.
  * State-machine driven 5-minute inactivity auto-lock.
* **Fail-Fast Intrusion Detection System (IDS):** 
  * Actively monitors incoming web requests for malformed tokens, unauthorized access, and brute-force attempts.
  * **DoS Mitigation:** Features a strict rate limiter that drops high-speed unauthenticated floods and immediately throttles the attacking IP with HTTP `429 Too Many Requests`.
* **Vulnerability Management (Patched):** 
  * Discovered and patched a state-dependent logic bypass where the system's "locked" state was short-circuiting the web router before reaching the IDS. The patch ensures the IDS successfully logs and throttles attacks regardless of the robot's lock state.

## Hardware & Functional Capabilities

* **Microcontroller:** ESP32 (Wi-Fi Enabled)
* **Chassis:** ACEBOTT 4WD Smart Car Kit
* **Robotics:** 5 Degrees of Freedom (5DOF) Robotic Arm
* **Sensors:** Ultrasonic sensor for autonomous collision avoidance
* **User Interface:** Custom interactive HTML/CSS Web Dashboard with tactile CSS animations and real-time telemetry updates.

## Repository Structure

The codebase is modularized for maintainability and security separation of concerns:

* `main.ino`: Web routing and main loop execution.
* `SecuritySystem.cpp / .h`: Core IDS, rate-limiting, and cryptography logic.
* `RobotHardware.cpp / .h`: Motor drivers, ultrasonic sensing, and arm kinematics.
* `WebFrontend.h`: Minified HTML, CSS, and JavaScript for the control dashboard.
* `dos_attack.py`: Custom Python penetration testing script used to audit the web server's rate limiter.

## Penetration Testing

This repository includes a custom Python script (`dos_attack.py`) used to simulate a high-speed unauthenticated request flood against the ESP32. 

**Attack Simulation Output:**
```text
[+] Target locked: [http://(IP Address)/move?dir=forward](http://(IP Address)/move?dir=forward)
[+] Launching high-speed unauthenticated request flood...

[1] HTTP 403 (Forbidden) -> Request blocked. IDS is logging.
[2] HTTP 403 (Forbidden) -> Request blocked. IDS is logging.
...
[6] HTTP 429 (Too Many Requests) -> FAIL-FAST TRIGGERED! IP throttled.
[7] HTTP 429 (Too Many Requests) -> FAIL-FAST TRIGGERED! IP throttled.
