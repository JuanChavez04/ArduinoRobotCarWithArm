#include "RobotHardware.h"

RobotHardware::RobotHardware() {
    currentMode = AutoMode::MANUAL;
    distance = 0;
}

void RobotHardware::init() {
    carArm.ARM_init(25, 26, 27, 33, 4);
    carArm.Silde_ChassisCmd(chassisAngle);
    carArm.Silde_ShoulderCmd(shoulderAngle);
    carArm.Silde_ElbowCmd(elbowAngle);
    carArm.Silde_WristCmd(wristAngle);
    carArm.Silde_ClawsCmd(clawsAngle);

    acebottCar.Init();
    usSensor.Init(13, 14);
}

void RobotHardware::stopAll() {
    acebottCar.Move(Stop, 0);
}

void RobotHardware::setMode(AutoMode mode) {
    currentMode = mode;
    if (mode == AutoMode::MANUAL) {
        stopAll();
    }
}

void RobotHardware::moveDirection(const String& dir) {
    // Manual movement cancels autonomous modes
    if (currentMode != AutoMode::MANUAL) {
        currentMode = AutoMode::MANUAL;
        Serial.println("[EXEC] Manual override. Autonomous mode halted.");
    }

    if (dir == "stop") stopAll();
    else if (dir == "forward") acebottCar.Move(Forward, 255);
    else if (dir == "back") acebottCar.Move(Backward, 255);
    else if (dir == "left") acebottCar.Move(Move_Left, 255);
    else if (dir == "right") acebottCar.Move(Move_Right, 255);
    else if (dir == "cw") acebottCar.Move(Clockwise, 255);
    else if (dir == "ccw") acebottCar.Move(Contrarotate, 255);
}

void RobotHardware::moveJoint(const String& joint, int angle) {
    if (joint == "chassis") {
        chassisAngle = angle;
        carArm.Silde_ChassisCmd(angle);
    } else if (joint == "shoulder") {
        shoulderAngle = angle;
        carArm.Silde_ShoulderCmd(angle);
    } else if (joint == "elbow") {
        elbowAngle = angle;
        carArm.Silde_ElbowCmd(angle);
    } else if (joint == "wrist") {
        wristAngle = angle;
        carArm.Silde_WristCmd(angle);
    } else if (joint == "claws") {
        clawsAngle = angle;
        carArm.Silde_ClawsCmd(angle);
    }
}

void RobotHardware::updateTelemetry() {
    distance = usSensor.Ranging();
}

void RobotHardware::handleAutonomousTasks() {
    if (currentMode == AutoMode::FOLLOW) {
        processFollow();
    } else if (currentMode == AutoMode::AVOID) {
        processAvoid();
    }
}

void RobotHardware::processFollow() {
    updateTelemetry();
    if (distance < 15) {
        acebottCar.Move(Backward, 200);
    } else if (15 <= distance && distance <= 20) {
        acebottCar.Move(Stop, 0);
    } else if (20 <= distance && distance <= 25) {
        acebottCar.Move(Forward, 180);
    } else if (25 <= distance && distance <= 50) {
        acebottCar.Move(Forward, 220);
    } else {
        acebottCar.Move(Stop, 0);
    }
}

void RobotHardware::processAvoid() {
    updateTelemetry();
    if (distance <= 25) {
        acebottCar.Move(Stop, 0);
        delay(100); 
        
        int randNumber = random(1, 4);
        switch (randNumber) {
            case 1:
                acebottCar.Move(Backward, 180); delay(500);
                acebottCar.Move(Move_Left, 180); delay(500);
                break;
            case 2:
                acebottCar.Move(Clockwise, 180); delay(1000);
                break;
            case 3:
                acebottCar.Move(Contrarotate, 180); delay(1000);
                break;
            case 4:
                acebottCar.Move(Backward, 180); delay(500);
                acebottCar.Move(Move_Right, 180); delay(500);
                break;
        }
    } else {
        acebottCar.Move(Forward, 180);
    }
}
