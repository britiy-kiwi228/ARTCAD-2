#ifndef MOTOR_CONTROL_H // Защита от повторного включения 
#define MOTOR_CONTROL_H

#include "config.h" // Включаем конфигурационный файл для доступа к пинам и параметрам ШИМ
#include <stdint.h> // файл с типами данных, такими как uint8_t
#include <stdbool.h> // булевы типы (true/false)

// === ПАРАМЕТРЫ ЗАЩИТЫ ===
#define MOTOR_MAX_PWM 255           // Максимальное значение ШИМ (0-255)
#define MOTOR_SOFT_START_TIME_MS 150 // Время плавного разгона (150 мс)

// Структура для хранения информации о моторе
typedef struct {
    uint8_t pwm_pin;
    uint8_t in1_pin;
    uint8_t in2_pin;
    uint8_t ledc_channel;
    
    // === ПЕРЕМЕННЫЕ СОСТОЯНИЯ ПЛАВНОГО ПУСКА ===
    int target_speed;              // Целевая скорость
    int current_pwm;               // Текущее значение ШИМ (применяется к мотору)
    uint32_t soft_start_begin_ms;  // Начало плавного разгона
    bool is_soft_starting;         // Флаг: выполняется ли плавный разгон?
} Motor_t;

// Функция для инициализации мотора
void motor_init(Motor_t* motor);

// Функция для установки скорости с плавным разгоном
void motor_set_speed(Motor_t* motor, int speed);

// Функция для обновления плавного разгона (вызывается в loop)
void motor_update_soft_start(Motor_t* motor);

// Получить текущую мощность мотора (0-255)
int motor_get_current_pwm(Motor_t* motor);

// Получить целевую скорость мотора
int motor_get_target_speed(Motor_t* motor);

// Получить процент нагрузки мотора (0-100%)
uint8_t motor_get_load_percent(Motor_t* motor);

#endif // конец MOTOR_CONTROL_H