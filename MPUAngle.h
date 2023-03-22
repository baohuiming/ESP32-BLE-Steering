#ifndef _MPU_ANGLE_H_
#define _MPU_ANGLE_H_

#include <Arduino.h>
#include "MPU6050.h" // https://github.com/ElectronicCats/mpu6050 (v0.6.0)
#include "Wire.h"

class MPUAngle  // 类的声明
{
 public:  // 公用函数、结构体的声明
  void init(); // 初始化函数
  float getRollAngle(); //获取角度
 private:
 MPU6050 accelgyro;

int16_t axo, ayo, azo, gxo, gyo, gzo;  //加速度计、陀螺仪原始数据
float ax, ay, az, gyrox, gyroy, gyroz, roll; // 绕x轴的滚动角roll
float AcceRatio = 16384.0;  //加速度计比例系数
float GyroRatio = 131.0;    //陀螺仪比例系数

float q[4] = {1.0f, 0.0f, 0.0f, 0.0f};

// parameters for 6 DoF sensor fusion calculations
float GyroMeasError =
    PI * (40.0f / 180.0f);  // gyroscope measurement error in rads/s (start at
                            // 60 deg/s), then reduce after ~10 s to 3
float beta = sqrt(3.0f / 4.0f) * GyroMeasError;  // compute beta
float GyroMeasDrift =
    PI *
    (2.0f /
     180.0f);  // gyroscope measurement drift in rad/s/s (start at 0.0 deg/s/s)
float zeta =
    sqrt(3.0f / 4.0f) *
    GyroMeasDrift;    // compute zeta, the other free parameter in the Madgwick
                      // scheme usually set to a small or zero value
float deltat = 0.0f;  // integration interval for both filter schemes
uint32_t lastUpdate = 0,
         firstUpdate = 0;  // used to calculate integration interval
uint32_t Now = 0;          // used to calculate integration interval
void MadgwickQuaternionUpdate(float ax, float ay, float az, float gyrox,
                              float gyroy, float gyroz);
};

#endif
