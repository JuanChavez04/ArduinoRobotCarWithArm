#include "SecuritySystem.h"

SecuritySystem::SecuritySystem() {
    systemLocked = true;
    sessionActive = false;
    pinIndex = 0;
    pinFailCount = 0;
    pinLockUntil = 0;
    failedAttemptCount = 0;
    lastFailedAttempt = 0;
    lastCommandTime = 0;
}

void SecuritySystem::init(const int* secretPin, const char* tokenKey) {
    for (int i = 0; i < PIN_LENGTH; i++) {
        correctPin[i] = secretPin[i];
        enteredPin[i] = 0;
    }
    secretTokenKey = String(tokenKey);
}

String SecuritySystem::fnv1aHash(const String &data, const String &key) {
    uint32_t h = 2166136261UL;
    String combined = data + key;
    for (size_t i = 0; i < combined.length(); i++) {
        h ^= (uint8_t)combined[i];
        h *= 16777619UL;
    }
    char buf[9];
    snprintf(buf, sizeof(buf), "%08X", h);
    return String(buf);
}

String SecuritySystem::generateSessionToken() {
    uint32_t id = (uint32_t)millis() ^ (uint32_t)random(0xFFFFFFFF);
    String idStr = String(id, HEX);
    String sig = fnv1aHash(idStr, secretTokenKey);
    return idStr + "." + sig;
}

void SecuritySystem::resetEnteredPin() {
    for (int i = 0; i < PIN_LENGTH; i++) enteredPin[i] = 0;
    pinIndex = 0;
}

bool SecuritySystem::checkPin() {
    for (int i = 0; i < PIN_LENGTH; i++)
        if (enteredPin[i] != correctPin[i]) return false;
    return true;
}

bool SecuritySystem::isPinLockedOut() {
    // Check if we have hit the failure threshold
    if (pinFailCount >= MAX_PIN_FAIL) {
        
        // If we are still inside the cooldown window, stay locked
        if (millis() < pinLockUntil) {
            Serial.print("[IPS ALERT] Brute Force Detected");
            Serial.println("[IPS ALERT] LOCKED OUT FOR 30s");
            return true; 
        } else {
            // The 30 seconds have passed, reset the counter and unlock
            pinFailCount = 0; 
            return false;
        }
    }
    
    // If we haven't hit the threshold yet, don't lock out
    return false;
}

PinResult SecuritySystem::processPinDigit(int digit) {
    if (isPinLockedOut()) return PinResult::LOCKED_OUT;

    if (pinIndex < PIN_LENGTH) {
        enteredPin[pinIndex++] = digit;
    }

    if (pinIndex == PIN_LENGTH) {
        if (checkPin()) {
            systemLocked = false;
            pinFailCount = 0;
            sessionToken = generateSessionToken();
            sessionActive = true;
            lastCommandTime = millis();
            resetEnteredPin();
            return PinResult::UNLOCKED;
        } else {
            systemLocked = true;
            pinFailCount++;
            resetEnteredPin();
            if (pinFailCount >= MAX_PIN_FAIL) {
                pinLockUntil = millis() + PIN_COOLDOWN_MS;
            }
            return PinResult::PIN_INCORRECT;
        }
    }
    return PinResult::DIGIT_OK;
}

void SecuritySystem::lockSystem() {
    systemLocked = true;
    sessionActive = false;
    sessionToken = "";
    resetEnteredPin();
}

void SecuritySystem::clearPin() {
    resetEnteredPin();
    sessionActive = false;
    sessionToken = "";
}

bool SecuritySystem::checkAutoLock() {
    if (sessionActive && !systemLocked) {
        if (millis() - lastCommandTime > AUTO_LOCK_MS) {
            lockSystem();
            return true; // Auto-lock was triggered
        }
    }
    return false;
}

void SecuritySystem::triggerIDS(const String& reason) {
    unsigned long now = millis();
    if (now - lastFailedAttempt > IDS_WINDOW_MS) {
        failedAttemptCount = 0;
    }
    lastFailedAttempt = now;
    failedAttemptCount++;

    if (failedAttemptCount <= 2) {
        Serial.print("[IDS ALERT] ");
        Serial.println(reason);
    }
}

AuthStatus SecuritySystem::validateAccess(const String& token) {
    // Check if the whole system is locked
    if (systemLocked) {
        triggerIDS("Command blocked — system locked.");
        if (failedAttemptCount > RATE_LIMIT_THRESHOLD) return AuthStatus::RATE_LIMITED;
        return AuthStatus::NO_SESSION; 
    }

    // Check if a session exists
    if (!sessionActive) {
        triggerIDS("No active session.");
        if (failedAttemptCount > RATE_LIMIT_THRESHOLD) return AuthStatus::RATE_LIMITED;
        return AuthStatus::NO_SESSION;
    }
    
    // Check token formatting
    int dotIndex = token.indexOf('.');
    if (dotIndex <= 0) {
        triggerIDS("Invalid token format.");
        if (failedAttemptCount > RATE_LIMIT_THRESHOLD) return AuthStatus::RATE_LIMITED;
        return AuthStatus::INVALID_TOKEN;
    }
    
    // Cryptographic signature check
    String idStr = token.substring(0, dotIndex);
    String sigStr = token.substring(dotIndex + 1);
    String expectedSig = fnv1aHash(idStr, secretTokenKey);
    
    if (sigStr != expectedSig || token != sessionToken) {
        triggerIDS("Invalid session token.");
        if (failedAttemptCount > RATE_LIMIT_THRESHOLD) return AuthStatus::RATE_LIMITED;
        return AuthStatus::INVALID_TOKEN;
    }
    
    // Valid command
    failedAttemptCount = 0; 
    lastCommandTime = millis();
    return AuthStatus::OK;
}
