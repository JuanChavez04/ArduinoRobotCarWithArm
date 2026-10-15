#pragma once
#include <Arduino.h>

enum class PinResult { DIGIT_OK, UNLOCKED, PIN_INCORRECT, LOCKED_OUT };
enum class AuthStatus { OK, NO_SESSION, INVALID_TOKEN, RATE_LIMITED };

class SecuritySystem {
private:
    static const int PIN_LENGTH = 4;
    int correctPin[PIN_LENGTH];
    int enteredPin[PIN_LENGTH];
    int pinIndex;

    int pinFailCount;
    static const int MAX_PIN_FAIL = 5;
    unsigned long pinLockUntil;
    static const unsigned long PIN_COOLDOWN_MS = 30000;

    String sessionToken;
    String secretTokenKey;
    unsigned long lastCommandTime;
    static const unsigned long AUTO_LOCK_MS = 300000; // 5 mins

    unsigned long lastFailedAttempt;
    int failedAttemptCount;
    static const unsigned long IDS_WINDOW_MS = 2000;
    static const int RATE_LIMIT_THRESHOLD = 5;

    String fnv1aHash(const String &data, const String &key);
    String generateSessionToken();
    bool checkPin();
    bool isPinLockedOut();
    void triggerIDS(const String& reason);

public:
    bool systemLocked;
    bool sessionActive;

    SecuritySystem();
    void init(const int* secretPin, const char* tokenKey);
    
    void resetEnteredPin();
    PinResult processPinDigit(int digit);
    
    void lockSystem();
    void clearPin();
    
    bool checkAutoLock();
    
    AuthStatus validateAccess(const String& token);
    String getSessionToken() { return sessionToken; }
};
