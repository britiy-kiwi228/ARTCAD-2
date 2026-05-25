#ifndef CONFIG_H 
#define CONFIG_H

#include "driver/ledc.h"
#include <Arduino.h>

// === ОТЛАДКА ===
#define DEBUG_ENABLE 

#ifdef DEBUG_ENABLE
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(...)
#endif


// --- Пины ходовой части (L298N) ---
#define MOTOR_L_PWM 32
#define MOTOR_L_IN1 25  // Возвращаем исходный пин
#define MOTOR_L_IN2 26  // Возвращаем исходный пин
#define MOTOR_R_PWM 33
#define MOTOR_R_IN1 27  // Возвращаем исходный пин
#define MOTOR_R_IN2 14  // Возвращаем исходный пин

// --- Пины системы вооружения (L298N) ---
#define WEAPON_EN  21  
#define WEAPON_IN1 19  
#define WEAPON_IN2 17  

// --- Пины Сервопривода ---
#define SERVO_SIG 15

// --- Пины Датчика HC-SR04 ---
#define ULTRA_TRIG 5   
#define ULTRA_ECHO 18  

// === НАСТРОЙКИ PWM (ШИМ) ДЛЯ МОТОРОВ ===
#define MOTOR_PWM_FREQ    1000    
#define MOTOR_PWM_RES     LEDC_TIMER_8_BIT
#define MOTOR_TIMER       LEDC_TIMER_0
#define MOTOR_SPEED_MODE  LEDC_LOW_SPEED_MODE  // Стабильно с WiFi
#define LEDC_CH_L         LEDC_CHANNEL_0
#define LEDC_CH_R         LEDC_CHANNEL_1
#define MOTOR_SOFT_START_TIME_MS 150

// === НАСТРОЙКИ PWM ДЛЯ КАТАПУЛЬТЫ ===
#define WEAPON_PWM_FREQ   1000   
#define WEAPON_PWM_RES    LEDC_TIMER_8_BIT      
#define WEAPON_TIMER      LEDC_TIMER_1
#define WEAPON_SPEED_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CH_WEAPON    LEDC_CHANNEL_2

// === НАСТРОЙКИ PWM ДЛЯ СЕРВО ===
#define SERVO_PWM_FREQ    50     
#define SERVO_PWM_RES     LEDC_TIMER_16_BIT     
#define SERVO_TIMER       LEDC_TIMER_2
#define SERVO_SPEED_MODE  LEDC_LOW_SPEED_MODE
#define LEDC_CH_SERVO     LEDC_CHANNEL_3
#define SERVO_MIN_DUTY    1638  // 0.5ms
#define SERVO_MAX_DUTY    7864  // 2.4ms

// --- Параметры оружия ---
#define WEAPON_MOTOR_RPM      271.0f           
#define WEAPON_GEAR_RATIO     1.0f         
#define WEAPON_MAX_PWM        255           
#define WEAPON_ROTATION_ANGLE 45.0f    
#define WEAPON_ROTATION_SPEED 200      
#define WEAPON_MOTOR_LOAD_THRESHOLD 50
// --- Настройки автовыстрела ---
#define WEAPON_AUTO_FIRE_SERVO_DELAY_MS  20  // Задержка (в мс) перед спуском сервопривода из 0 в 100 градусов

// --- Параметры движения ---
#define MOTOR_MAX_SPEED   255              
#define MOTOR_TURN_SPEED  160             

#endif