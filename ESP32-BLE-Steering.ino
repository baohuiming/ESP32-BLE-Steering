#include "Adafruit_Keypad.h" // https://github.com/adafruit/Adafruit_Keypad
#include "MPUAngle.h"
#include <Arduino.h>
#include <BleGamepad.h> // https://github.com/lemmingDev/ESP32-BLE-Gamepad (v0.5.3)

#define numOfButtons 17
#define numOfHatSwitches 0
#define enableX false
#define enableY true
#define enableZ true
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
#define vibrationMotorPin 15
#define brakePin 33
#define acceleratorPin 32
#define gearDrivePin 12
#define gearReversePin 13
#define joystickButtonPin 27
#define minAxValue -32767
#define maxAxValue 32767
#define midAxValue 0
#define minSmValue 0
#define maxSmValue 32767
#define midSmValue 16383

// 蓝牙设备名称、厂商名称、电池电量
BleGamepad bleGamepad("ESP32 Gamepad", "Baohuiming.net", 100);

float currentAngle;
float currentMPUAngle;
int currentJoystickXValue = midAxValue;
int currentJoystickYValue = midAxValue;
int gearDrive = 1;
int gearReverse = 1;
int joystickButton = 1;
MPUAngle mpuAngle;

// 4x4键盘
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {18, 19, 5, 7},
  {12, 13, 4, 1},
  {11, 14, 3, 2},
  {10, 9, 6, 8}};
byte rowPins[ROWS] = {17, 5, 18, 19}; // 行的接口引脚
byte colPins[COLS] = {16, 4, 0, 2};   // 列的接口引脚
Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void readJoystick() {
  // 设置死区
  float neutralZone = 380;
  float rawXValue = analogRead(joystickXPin);
  float rawYValue = analogRead(joystickYPin);

  // 右上： (0, 0)， 左下： (4095, 4095)
  // 上下：X， 左右：Y
  if (abs(rawXValue - 2047.5) < neutralZone)
    currentJoystickXValue = midAxValue;
  else
    currentJoystickXValue = (rawXValue - 2047.5) / 2047.5 * (maxAxValue - midAxValue);
  if (abs(rawYValue - 2047.5) < neutralZone)
    currentJoystickYValue = midAxValue;
  else
    currentJoystickYValue = (rawYValue - 2047.5) / 2047.5 * (minAxValue - midAxValue);
}

void readPedal() {
  // max = 2800, min = 850
  // 创建一个从(850, 2800)到(minSmValue, maxSmValue)的映射
  float maxmV = 2800;
  float minmV = 900; // 850 + 50死区
  float k = (maxSmValue - minSmValue) / (maxmV - minmV);
  float b = maxSmValue - k * maxmV;

  float rawBrakeValue = analogRead(brakePin);
  float rawAcceleratorValue = analogRead(acceleratorPin);
  float brakeValue = rawBrakeValue * k + b;
  float acceleratorValue = rawAcceleratorValue * k + b;

  if (brakeValue < minSmValue)
    brakeValue = minSmValue;
  else if (brakeValue > maxSmValue)
    brakeValue = maxSmValue;
  if (acceleratorValue < minSmValue)
    acceleratorValue = minSmValue;
  else if (acceleratorValue > maxSmValue)
    acceleratorValue = maxSmValue;

  bleGamepad.setBrake(brakeValue);
  bleGamepad.setAccelerator(acceleratorValue);
}

void readCustomButton() {
  // 读取前进挡(15)、后退挡(16)、摇杆按钮(17)
  // 0：按下，1：松开
  // 15、16、17表示按键的编号
  int newGearDrive = digitalRead(gearDrivePin);
  int newGearReverse = digitalRead(gearReversePin);
  int newJoystickButton = digitalRead(joystickButtonPin);
  if (gearDrive == 1 && newGearDrive == 0)
    bleGamepad.press((char)(15));
  else if (gearDrive == 0 && newGearDrive == 1)
    bleGamepad.release((char)(15));

  if (gearReverse == 1 && newGearReverse == 0)
    bleGamepad.press((char)(16));
  else if (gearReverse == 0 && newGearReverse == 1)
    bleGamepad.release((char)(16));

  if (joystickButton == 1 && newJoystickButton == 0)
    bleGamepad.press((char)(17));
  else if (joystickButton == 0 && newJoystickButton == 1)
    bleGamepad.release((char)(17));

  gearDrive = newGearDrive;
  gearReverse = newGearReverse;
  joystickButton = newJoystickButton;
}

void moveSteering(float angle) {
  // angle ∈ [-450, 450]，超出范围则取边界值
  // i > 0, 顺时针
  // i < 0, 逆时针
  if (angle > halfAngle)
    angle = halfAngle;
  else if (angle < -halfAngle)
    angle = -halfAngle;

  int angleAx = (angle - -halfAngle) / (halfAngle - -halfAngle) * (maxSmValue - minSmValue);

  // 设置当前角度
  currentAngle = angle;
  bleGamepad.setSteering(angleAx);
}

void setVibration() {
  // 角度大于900度时振动
  if (currentAngle >= halfAngle || currentAngle <= -halfAngle)
    digitalWrite(vibrationMotorPin, HIGH);
  else
    digitalWrite(vibrationMotorPin, LOW);
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
  bleGamepadConfig.setWhichAxes(enableX, enableY, enableZ, enableRX, enableRY, enableRZ, enableSlider1, enableSlider2);
  bleGamepadConfig.setWhichSimulationControls(enableRudder, enableThrottle, enableAccelerator, enableBrake, enableSteering);
  bleGamepadConfig.setHatSwitchCount(numOfHatSwitches);
  bleGamepadConfig.setVid(0x3b2b);     // 厂商号
  bleGamepadConfig.setPid(0x2802);     // 型号（版本号）
  bleGamepadConfig.setAxesMin(0x8001); // -32767，只针对摇杆，对刹车等无效
  bleGamepadConfig.setAxesMax(0x7FFF); // 32767，同上

  bleGamepad.begin(&bleGamepadConfig);

  // Set accelerator and brake to min
  bleGamepad.setAccelerator(minSmValue);
  bleGamepad.setBrake(minSmValue);

  // Set steering to center
  bleGamepad.setSteering(midSmValue);

  // bleGamepad.setAxes(currentJoystickXValue, currentJoystickYValue, 0, 0, 0, 0, 0, 0);
  bleGamepad.setAxes(0, midAxValue, midAxValue, 0, 0, 0, 0, 0);

  // start MPU6050
  mpuAngle.init();
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
        default: {
          bleGamepad.press((char)(e.bit.KEY));
          break;
        }
        }
      } else if (e.bit.EVENT == KEY_JUST_RELEASED) {
        // 松开
        switch (e.bit.KEY) {
        default: {
          bleGamepad.release((char)(e.bit.KEY));
          break;
        }
        }
      }
    }

    // 获取陀螺仪的角度MPUAngle∈[-∞, +∞]
    currentMPUAngle = mpuAngle.getRollAngle();
    moveSteering(currentMPUAngle);

    // 摇杆
    readJoystick();

    // 踏板
    readPedal();

    // 读取前进挡(15)、后退挡(16)、摇杆按钮(17)
    readCustomButton();

    // 设置摇杆
    bleGamepad.setAxes(0, currentJoystickXValue, currentJoystickYValue, 0, 0, 0, 0, 0);

    // 振动
    setVibration();
  }
}
