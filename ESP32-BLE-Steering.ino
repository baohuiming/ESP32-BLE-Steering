#include <Arduino.h>
#include <BleGamepad.h>       // https://github.com/lemmingDev/ESP32-BLE-Gamepad
#include "Adafruit_Keypad.h"  // https://github.com/adafruit/Adafruit_Keypad
#include "MPUAngle.h"

#define numOfButtons 17
#define numOfHatSwitches 0
#define enableX true
#define enableY true
#define enableZ true
#define enableRX false
#define enableRY false
#define enableRZ false
#define enableSlider1 true
#define enableSlider2 false
#define enableRudder false
#define enableThrottle false
#define enableAccelerator false
#define enableBrake true
#define enableSteering false
#define fullAngle 900
#define halfAngle 450
#define joystickXPin 25
#define joystickYPin 26
#define vibrationMotorPin 15
#define brakePin 33
#define acceleratorPin 32
#define gearDrivePin 12
#define gearReversePin 13
#define joystickButtonPin 27

// 蓝牙设备名称、厂商名称、电池电量
BleGamepad bleGamepad("ESP32 Gamepad", "Baohuiming.net", 100);

float CurrentAngle;
float CurrentMPUAngle;
int CurrentAngleAxis = 0;
float CurrentJoystickXValue = 0.0;
float CurrentJoystickYValue = 0.0;
int CurrentAcceleratorValue = -32767;
int gearDrive = 1;
int gearReverse = 1;
int joystickButton = 1;
MPUAngle mpuAngle;

// 4x4键盘
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {{17, 18, 5, 7},
                         {12, 13, 4, 1},
                         {11, 14, 3, 2},
                         {10, 9, 6, 8}};
byte rowPins[ROWS] = {17, 5, 18, 19};  // 行的接口引脚
byte colPins[COLS] = {16, 4, 0, 2};    // 列的接口引脚
Adafruit_Keypad customKeypad =
    Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void readJoystick() {
    // 设置死区
    float neutralZone = 350;
    float rawXValue = analogRead(joystickXPin);
    float rawYValue = analogRead(joystickYPin);
    // 右上： (0, 0)， 左下： (4096, 4096)
    // 上下：X， 左右：Y
    if (abs(rawXValue - 2048) < neutralZone)
        CurrentJoystickXValue = 0;
    else
        CurrentJoystickXValue = (rawXValue - 2048) / 2048 * 32767;
    if (abs(rawYValue - 2048) < neutralZone)
        CurrentJoystickYValue = 0;
    else
        CurrentJoystickYValue = (rawYValue - 2048) / 2048 * -32767;
}

void readPedal() {
    // max = 2800, min = 850
    // 创建一个从(850, 2800)到(-32767, 32767)的映射
    float maxmV = 2800;
    float minmV = 850;
    float k = (32767 - (-32767)) / (maxmV - minmV);
    float b = 32767 - k * maxmV;

    float rawBrakeValue = analogRead(brakePin);
    float rawAcceleratorValue = analogRead(acceleratorPin);
    float brakeValue = rawBrakeValue * k + b;
    float acceleratorValue = rawAcceleratorValue * k + b;

    if (brakeValue < -32767)
        brakeValue = -32767;
    else if (brakeValue > 32767)
        brakeValue = 32767;
    if (acceleratorValue < -32767)
        acceleratorValue = -32767;
    else if (acceleratorValue > 32767)
        acceleratorValue = 32767;

    bleGamepad.setBrake(brakeValue);
    CurrentAcceleratorValue = acceleratorValue;
    // bleGamepad.setAccelerator(acceleratorValue);
}

void readCustomButton() {
    // 读取前进挡(15)、后退挡(16)、摇杆按钮(17)
    // 0：按下，1：松开
    // 15、16、17表示按键的编号
    int newGearDrive = digitalRead(gearDrivePin);
    int newGearReverse = digitalRead(gearReversePin);
    int newJoystickButton = digitalRead(joystickButtonPin);
    if (gearDrive == 1 && newGearDrive == 0) {
        bleGamepad.press((char)(15));
    } else if (gearDrive == 0 && newGearDrive == 1) {
        bleGamepad.release((char)(15));
    }
    if (gearReverse == 1 && newGearReverse == 0) {
        bleGamepad.press((char)(16));
    } else if (gearReverse == 0 && newGearReverse == 1) {
        bleGamepad.release((char)(16));
    }
    if (joystickButton == 1 && newJoystickButton == 0) {
        bleGamepad.press((char)(17));
    } else if (joystickButton == 0 && newJoystickButton == 1) {
        bleGamepad.release((char)(17));
    }
    gearDrive = newGearDrive;
    gearReverse = newGearReverse;
    joystickButton = newJoystickButton;
}

void moveSteering(float angle) {
    // angle ∈ [-450, 450]，超出范围则取边界值
    // i > 0, 顺时针
    // i < 0, 逆时针
    if (angle > halfAngle) {
        angle = halfAngle;
    } else if (angle < -1 * halfAngle) {
        angle = -1 * halfAngle;
    }
    CurrentAngleAxis = angle / halfAngle * 32767;

    // 设置当前角度
    CurrentAngle = angle;
    // bleGamepad.setSteering(i);
}

void setup() {
    // Serial.begin(115200);
    pinMode(joystickXPin, INPUT);
    pinMode(joystickYPin, INPUT);
    pinMode(brakePin, INPUT);
    pinMode(acceleratorPin, INPUT);
    pinMode(vibrationMotorPin, OUTPUT);
    pinMode(gearDrivePin, INPUT_PULLUP);
    pinMode(gearReversePin, INPUT_PULLUP);
    pinMode(joystickButtonPin, INPUT_PULLUP);

    // 4x4键盘
    customKeypad.begin();

    // 蓝牙键盘配置
    BleGamepadConfiguration bleGamepadConfig;
    // 蓝牙设备类型：CONTROLLER_TYPE_JOYSTICK,CONTROLLER_TYPE_GAMEPAD,CONTROLLER_TYPE_MULTI_AXIS
    bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);

    bleGamepadConfig.setButtonCount(numOfButtons);
    bleGamepadConfig.setWhichAxes(enableX, enableY, enableZ, enableRX, enableRY,
                                  enableRZ, enableSlider1, enableSlider2);
    bleGamepadConfig.setWhichSimulationControls(enableRudder, enableThrottle,
                                                enableAccelerator, enableBrake,
                                                enableSteering);
    bleGamepadConfig.setHatSwitchCount(numOfHatSwitches);
    bleGamepadConfig.setVid(0x3b2b);  // 厂商号
    bleGamepadConfig.setPid(0x2601);  // 型号（版本号）

    bleGamepad.begin(&bleGamepadConfig);

    // Set accelerator and brake to min
    // bleGamepad.setAccelerator(-32767);
    bleGamepad.setBrake(-32767);

    // Set Axis
    bleGamepad.setAxes(0, 0, 0, 0, 0, 0, -32767, 0);

    // start MPU6050
    mpuAngle.init();

    // Set steering to center
    // bleGamepad.setSteering(0);
}

void loop() {
    customKeypad.tick();
    if (bleGamepad.isConnected()) {
        if (customKeypad.available()) {
            // 读取键盘数据
            keypadEvent e = customKeypad.read();
            if (e.bit.EVENT == KEY_JUST_PRESSED) {
                // 按下
                switch (e.bit.KEY) {
                    case 14: {
                        // 手刹
                        bleGamepad.press((char)(e.bit.KEY));
                        bleGamepad.release((char)(e.bit.KEY));
                        break;
                    }
                    default: {
                        bleGamepad.press((char)(e.bit.KEY));
                        break;
                    }
                }
            } else if (e.bit.EVENT == KEY_JUST_RELEASED) {
                // 松开
                switch (e.bit.KEY) {
                    case 14: {
                        // 手刹
                        bleGamepad.press((char)(e.bit.KEY));
                        bleGamepad.release((char)(e.bit.KEY));
                        break;
                    }
                    default: {
                        bleGamepad.release((char)(e.bit.KEY));
                        break;
                    }
                }
            }
        }

        // 获取陀螺仪的角度MPUAngle∈[-∞, +∞]
        CurrentMPUAngle = mpuAngle.getRollAngle();
        moveSteering(CurrentMPUAngle);

        // 摇杆
        readJoystick();

        // 踏板
        readPedal();

        // 读取前进挡(15)、后退挡(16)、摇杆按钮(17)
        readCustomButton();

        bleGamepad.setAxes(CurrentAngleAxis, CurrentJoystickXValue,
                           CurrentJoystickYValue, 0, 0, 0,
                           CurrentAcceleratorValue, 0);

        // 角度大于900度时振动
        if (CurrentAngle >= 450.f || CurrentAngle <= -450.f) {
            digitalWrite(vibrationMotorPin, HIGH);
        } else {
            digitalWrite(vibrationMotorPin, LOW);
        }
    }
}
