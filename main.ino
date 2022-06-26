#include <Arduino.h>
#include <BleGamepad.h>
#include <avdweb_Switch.h>
#include <MPU6050_tockn.h>
#include <Wire.h>
#include "Adafruit_Keypad.h"

#define numOfButtons 17
#define numOfHatSwitches 0
#define enableX true
#define enableY true
#define enableZ false
#define enableRX false
#define enableRY false
#define enableRZ false
#define enableSlider1 false
#define enableSlider2 false
#define enableRudder false
#define enableThrottle false
#define enableAccelerator true
#define enableBrake true
#define enableSteering true
#define fullAngle 900
#define halfAngle 450
#define joystickXPin 25
#define joystickYPin 26

MPU6050 mpu6050(Wire);

BleGamepad bleGamepad("ESP32 Gamepad", "Baohuiming.top", 100);
// Shows how you can customise the device name,
// manufacturer name and initial battery level

float CurrentAngle;
float CurrentMPUAngle;
float DeltaAngle = 0.0;

// 4x4键盘
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {17, 18, 5, 7}, {12, 13, 4, 1}, {11, 14, 3, 2}, {10, 9, 6, 8}
};
byte rowPins[ROWS] = {17, 5, 18, 19};  //行的接口引脚
byte colPins[COLS] = {16, 4, 0, 2};    //列的接口引脚
Adafruit_Keypad customKeypad =  Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// 手动配置的按钮
Switch gearDrive = Switch(12);    // GPIO 12
Switch gearReverse = Switch(13);  // GPIO 13
Switch hallButton1 = Switch(14);
Switch joystickButton = Switch(27);

void gearDrivePushedCallbackFunction(void *s) {
  bleGamepad.press((char)(15));
}

void gearDriveReleasedCallbackFunction(void *s) {
  bleGamepad.release((char)(15));
}

void gearReversePushedCallbackFunction(void *s) {
  bleGamepad.press((char)(16));
}

void gearReverseReleasedCallbackFunction(void *s) {
  bleGamepad.release((char)(16));
}

void hallButton1PushedCallbackFunction(void *s) {
  // Set angle to 0, 360, -360 degree
  if (CurrentAngle > 300) { // 330 degree -> 360 degree
    DeltaAngle = 360 - CurrentMPUAngle;
  }
  else if (CurrentAngle < -300) { // -330 -> -360
    DeltaAngle = -360 - CurrentMPUAngle;
  }
  else {  // 30 -> 0
    DeltaAngle = 0 - CurrentMPUAngle;
  }
}

void joystickButtonPushedCallbackFunction(void *s) {
  bleGamepad.press((char)(17));
}

void joystickButtonReleasedCallbackFunction(void *s) {
  bleGamepad.release((char)(17));
}

void readjoystick() {
  float rawXValue = analogRead(joystickXPin);
  float rawYValue = analogRead(joystickYPin);
  // 右上： (0, 0)
  // 左下： (4096, 4096)
  // 上下：X
  // 左右：Y
  float XValue = (rawXValue - 2048) / 2048 * 32767;
  float YValue = (rawYValue - 2048) / 2048 * 32767;
  bleGamepad.setAxes(XValue, YValue, 0, 0, 0, 0, 0, 0);
}

void setup() {
  // Serial.begin(115200);
  pinMode(joystickXPin, INPUT);
  pinMode(joystickYPin, INPUT);

  // 4x4键盘
  customKeypad.begin();

  // 手动配置的按钮
  gearDrive.setPushedCallback(&gearDrivePushedCallbackFunction);
  gearDrive.setReleasedCallback(&gearDriveReleasedCallbackFunction);
  gearReverse.setPushedCallback(&gearReversePushedCallbackFunction);
  gearReverse.setReleasedCallback(&gearReverseReleasedCallbackFunction);
  hallButton1.setPushedCallback(&hallButton1PushedCallbackFunction);
  joystickButton.setPushedCallback(&joystickButtonPushedCallbackFunction);
  joystickButton.setReleasedCallback(&joystickButtonReleasedCallbackFunction);

  // 串口获取陀螺仪数据
  Wire.begin();

  // 陀螺仪自动初始化，不建议手动设置
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

  // 蓝牙键盘配置
  BleGamepadConfiguration bleGamepadConfig;
  bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD); // 蓝牙设备类型
  // CONTROLLER_TYPE_JOYSTICK,
  // CONTROLLER_TYPE_GAMEPAD (DEFAULT),
  // CONTROLLER_TYPE_MULTI_AXIS

  bleGamepadConfig.setButtonCount(numOfButtons);
  bleGamepadConfig.setWhichAxes(enableX, enableY, enableZ, enableRX, enableRY, enableRZ, enableSlider1, enableSlider2); // 轴
  bleGamepadConfig.setWhichSimulationControls(enableRudder, enableThrottle, enableAccelerator, enableBrake, enableSteering); // 控制器
  bleGamepadConfig.setHatSwitchCount(numOfHatSwitches);  // 帽子开关数量
  bleGamepadConfig.setVid(0x3b2b); // 厂商号
  bleGamepadConfig.setPid(0x2201); // 型号（版本号）

  bleGamepad.begin(&bleGamepadConfig);

  // changing bleGamepadConfig after the begin function has no effect, unless
  // you call the begin function again

  // Set accelerator and brake to min
  bleGamepad.setAccelerator(-32767);
  bleGamepad.setBrake(-32767);

  // Set Axis
  bleGamepad.setAxes(0, 0, 0, 0, 0, 0, 0, 0);

  // Set steering to center
  bleGamepad.setSteering(0);
}

void moveSteering(float angle) {
  // angle ∈ [-450, 450]
  // i > 0, 顺时针
  // angle = angle - DeltaAngle;
  // DeltaAngle = 0;

  if (angle > halfAngle) {
    angle = halfAngle;
  }
  else if (angle < -1 * halfAngle) {
    angle = -1 * halfAngle;
  }
  int i = angle / halfAngle * 32767;

  // 设置当前角度
  CurrentAngle = angle;
  bleGamepad.setSteering(i);
}

void loop() {
  customKeypad.tick();
  if (bleGamepad.isConnected()) {
    if (customKeypad.available()) {
      //读取键盘数据
      keypadEvent e = customKeypad.read();
      if (e.bit.EVENT == KEY_JUST_PRESSED) {
        // 按下
        switch (e.bit.KEY) {
          case 17: {
              // 刹车
              bleGamepad.setBrake(32767);
              break;
            }
          case 18: {
              // 油门
              bleGamepad.setAccelerator(32767);
              break;
            }
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
      }
      else if (e.bit.EVENT == KEY_JUST_RELEASED) {
        // 松开
        switch (e.bit.KEY) {
          case 17: {
              // 刹车
              bleGamepad.setBrake(-32767);
              break;
            }
          case 18: {
              // 油门
              bleGamepad.setAccelerator(-32767);
              break;
            }
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

    // 挂挡
    gearDrive.poll();
    gearReverse.poll();

    // 摇杆
    joystickButton.poll();
    readjoystick();

    // 识别和校正转动角度
    mpu6050.update();
    CurrentMPUAngle = -mpu6050.getAngleZ();
    hallButton1.poll();
    moveSteering(CurrentMPUAngle + DeltaAngle);
  }
}
