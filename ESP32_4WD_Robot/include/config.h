#ifndef CONFIG_H 
#define CONFIG_H

#include "driver/ledc.h"
#include <Arduino.h>

// === ОТЛАДКА ===
//#define DEBUG_ENABLE 

#ifdef DEBUG_ENABLE
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(...)
#endif

// --- Пины ходовой части ---
#define MOTOR_L_PWM 32
#define MOTOR_L_IN1 25
#define MOTOR_L_IN2 26
#define MOTOR_R_PWM 33
#define MOTOR_R_IN1 27
#define MOTOR_R_IN2 14

// --- Пины катапульты ---
#define WEAPON_EN  21  
#define WEAPON_IN1 19  
#define WEAPON_IN2 17  

// --- Пины Сервопривода ---
#define SERVO_SIG 15

// --- Датчик HC-SR04 ---
#define ULTRA_TRIG 5   
#define ULTRA_ECHO 18  

// === НАСТРОЙКИ PWM (ШИМ) ДЛЯ МОТОРОВ ===
#define MOTOR_PWM_FREQ   1000    
#define MOTOR_PWM_RES    8       // 8 бит (0-255)
#define MOTOR_TIMER      LEDC_TIMER_0
#define MOTOR_SPEED_MODE LEDC_HIGH_SPEED_MODE
#define LEDC_CH_L        LEDC_CHANNEL_0
#define LEDC_CH_R        LEDC_CHANNEL_1

// === НАСТРОЙКИ PWM ДЛЯ КАТАПУЛЬТЫ ===
#define WEAPON_PWM_FREQ   1000   
#define WEAPON_PWM_RES    8      
#define WEAPON_TIMER      LEDC_TIMER_1
#define WEAPON_SPEED_MODE LEDC_HIGH_SPEED_MODE
#define LEDC_CH_WEAPON    LEDC_CHANNEL_2

// === НАСТРОЙКИ PWM ДЛЯ СЕРВО ===
#define SERVO_PWM_FREQ    50     // 50 Гц
#define SERVO_PWM_RES     16     // 16 бит
#define SERVO_RES         16     // Дубликат для совместимости
#define SERVO_FREQ        50     // Дубликат для совместимости
#define SERVO_TIMER       LEDC_TIMER_2
#define SERVO_SPEED_MODE  LEDC_HIGH_SPEED_MODE
#define LEDC_CH_SERVO     LEDC_CHANNEL_3

// Параметры серво MG995 (16-бит: 0.5мс = 1638, 2.4мс = 7864)
#define SERVO_MIN_DUTY 1638    
#define SERVO_MAX_DUTY 7864    

// --- Параметры оружия ---
#define WEAPON_MOTOR_RPM 205           
#define WEAPON_GEAR_RATIO 1.0f         
#define WEAPON_MAX_PWM 200             
#define WEAPON_ROTATION_ANGLE 45.0f    
#define WEAPON_ROTATION_SPEED 200      

// --- Скорости движения ---
#define MOTOR_MAX_SPEED 255              
#define MOTOR_TURN_SPEED 150             

#endif