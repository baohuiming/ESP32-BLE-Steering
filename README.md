# ESP32-BLE-Steering
自制ESP32蓝牙方向盘

DIY ESP32 Bluetooth Steering

<hr/>

准备工作：
* ESP32开发板，足够多的GPIO、ADC接口
* MPU6050陀螺仪
* 若干电位器（摇杆、踏板）
* 按钮、拨动开关
* 小型振动电机
* Arduino（选择ESP32开发板）
* esptool

Arduino库：
* 蓝牙功能：[NimBLE](https://github.com/h2zero/NimBLE-Arduino)
* 游戏板控制：[BleGamepad](https://github.com/lemmingDev/ESP32-BLE-Gamepad/)
* 4x4矩阵键盘：[Adafruit Keypad](https://github.com/adafruit/Adafruit_Keypad)
* MPU6050角度识别：[MPU6050_Madgwick](https://github.com/bjtu-bhm/madgwick_steer)
* I2C通信：Wire（Arduino自带）

❗注意事项：
* 电位器需接在3.3v
* 陀螺仪需接在5v
* 11/23/2022更新：修改角度识别库，须将陀螺仪的x轴作为方向盘的转动轴，移除了霍尔传感器
* 3/9/2023更新：移除了avdweb_Switch库，采用读电平的方式替代


<hr/>

Previous work:
* ESP32-Devkit (Enough GPIO/ADC pins)
* MPU6050 gyroscope
* Some potentiometer (rocker and pedal)
* Buttons and switches
* Small vibration motor
* Arduino（choose ESP32 board）
* esptool

Arduino libraries:
* Bluetooth：[NimBLE](https://github.com/h2zero/NimBLE-Arduino)
* Gamepad control：[BleGamepad](https://github.com/lemmingDev/ESP32-BLE-Gamepad/)
* 4x4 matrix keypad：[Adafruit Keypad](https://github.com/adafruit/Adafruit_Keypad)
* MPU6050 angle detect：[MPU6050_Madgwick](https://github.com/bjtu-bhm/madgwick_steer)
* I2C communication：Wire(Arduino built-in)

❗Attention：
* Connect potentiometer to +3.3v
* Connect gyroscope to +5v
* 11/23/2022 updated：Modify MPU6050 library. The x-axis of the gyroscope must be used as the rotation axis of the steering wheel. The Hall sensor is removed.
* 3/9/2023 updated：The module avdweb_Switch is removed.