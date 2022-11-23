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
* Arduino 1.8.19

Arduino库：
* 蓝牙功能：[BleGamepad](https://github.com/lemmingDev/ESP32-BLE-Gamepad/)
* 4x4矩阵键盘：[Adafruit Keypad](https://github.com/adafruit/Adafruit_Keypad)
* MPU6050角度识别：[MPU6050_tockn](https://github.com/bjtu-bhm/madgwick_steer)
* 普通独立按钮：[avdweb_Switch](https://github.com/avandalen/avdweb_Switch)
* I2C通信：Wire

❗注意事项：
* 电位器需接在3.3v
* 11/23/2022更新：修改角度识别库，须将陀螺仪的x轴作为方向盘的转动轴，移除了霍尔传感器


<hr/>

Previous work:
* ESP32-Devkit (Enough GPIO/ADC pins)
* MPU6050 gyroscope
* Some potentiometer (rocker and pedal)
* Buttons and switches
* Small vibration motor
* Arduino 1.8.19

Arduino libraries:
* Bluetooth：[BleGamepad](https://github.com/lemmingDev/ESP32-BLE-Gamepad/)
* 4x4 matrix keypad：[Adafruit Keypad](https://github.com/adafruit/Adafruit_Keypad)
* MPU6050 angle detect：[MPU6050_tockn](https://github.com/bjtu-bhm/madgwick_steer)
* Normal independent button：[avdweb_Switch](https://github.com/avandalen/avdweb_Switch)
* I2C communication：Wire

❗Attention：
* Connect potentiometer to +3.3v
* 11/23/2022 updated：Modify MPU6050 library. The x-axis of the gyroscope must be used as the rotation axis of the steering wheel. The Hall sensor is removed.
