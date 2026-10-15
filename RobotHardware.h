#pragma once
#include <Arduino.h>
#include <ACB_CAR_ARM.h>
#include <ACB_SmartCar_V2.h>
#include <ultrasonic.h>

enum class AutoMode { MANUAL, FOLLOW, AVOID };

class RobotHardware {
private:
    ACB_CAR_ARM carArm;
    ACB_SmartCar_V2 acebottCar;
    ultrasonic usSensor;

    int chassisAngle = 90;
    int shoulderAngle = 40;
    int elbowAngle = 50;
    int wristAngle = 90;
    int clawsAngle = 90;

    void processFollow();
    void processAvoid();

public:
    AutoMode currentMode;
    int distance;

    RobotHardware();
    void init();
    
    void stopAll();
    void moveDirection(const String& dir);
    void moveJoint(const String& joint, int angle);
    void setMode(AutoMode mode);
    
    void updateTelemetry();
    void handleAutonomousTasks();
};
