#include "driver/ledc.h" // Включаем драйвер для управления ШИМ
#include <Arduino.h>       // 1. Библиотека с базовыми функциями (pinMode, digitalWrite)
#include "motor_control.h" // 2. Наш заголовочный файл
#include <stdlib.h>        // 3. Подключаем стандартную библиотеку для функции abs()
#include <math.h>          // Математические функции

/**
 * Функция инициализации конкретного мотора.
 * Принимает адрес структуры Motor_t.
 */
void motor_init(Motor_t* motor) {
    // Принудительно отключаем ШИМ перед настройкой
    ledcDetachPin(motor->pwm_pin);
    
    pinMode(motor->in1_pin, OUTPUT);
    pinMode(motor->in2_pin, OUTPUT);
    pinMode(motor->pwm_pin, OUTPUT); // Пин ENA/ENB тоже в OUTPUT
    
    digitalWrite(motor->in1_pin, LOW);
    digitalWrite(motor->in2_pin, LOW);
    digitalWrite(motor->pwm_pin, LOW);

    // Используем частоту 500 Гц и разрешение 8 бит
    ledcSetup(motor->ledc_channel, PWM_FREQ, PWM_RES);
    ledcAttachPin(motor->pwm_pin, motor->ledc_channel);
    ledcWrite(motor->ledc_channel, 0);
}

void motor_set_speed(Motor_t* motor, int speed) {
    if (motor->target_speed == speed) return;
    motor->target_speed = speed;

    if (speed == 0) {
        motor->current_pwm = 0;
        digitalWrite(motor->in1_pin, LOW);
        digitalWrite(motor->in2_pin, LOW);
        ledcWrite(motor->ledc_channel, 0);
        return;
    }

    // Направление
    digitalWrite(motor->in1_pin, (speed > 0) ? HIGH : LOW);
    digitalWrite(motor->in2_pin, (speed < 0) ? HIGH : LOW);

    // Скорость (сразу, без плавного пуска для теста)
    motor->current_pwm = abs(speed);
    ledcWrite(motor->ledc_channel, motor->current_pwm);
}
/**
 * Обновление плавного разгона.
 * ОБЯЗАТЕЛЬНО вызывать в loop() для всех моторов!
 * 
 * Формула: PWM = (elapsed_time / SOFT_START_TIME) * target_speed
 * Это дает линейный рост скорости от 0 до целевой за MOTOR_SOFT_START_TIME_MS мс
 */
void motor_update_soft_start(Motor_t* motor) {
    if (!motor->is_soft_starting) {
        return; // Плавный разгон не активен
    }

    uint32_t elapsed_ms = millis() - motor->soft_start_begin_ms;

    // Проверяем: завершился ли плавный разгон?
    if (elapsed_ms >= MOTOR_SOFT_START_TIME_MS) {
        // Плавный разгон завершен - применяем полную целевую скорость
        motor->current_pwm = abs(motor->target_speed);
        motor->is_soft_starting = false;
    } else {
        // Плавный разгон еще идет - интерполируем скорость
        // Формула: current_pwm = (elapsed / total) * target
        float progress = (float)elapsed_ms / (float)MOTOR_SOFT_START_TIME_MS;
        motor->current_pwm = (int)(progress * (float)abs(motor->target_speed));
    }

    // Применяем текущее значение ШИМ
    ledcWrite(motor->ledc_channel, motor->current_pwm);
}

/**
 * Получить текущую мощность, подаваемую на мотор (0-255).
 * Это может отличаться от целевой скорости если идет плавный разгон!
 */
int motor_get_current_pwm(Motor_t* motor) {
    if (!motor) return 0;
    return motor->current_pwm;
}

/**
 * Получить целевую скорость мотора (-255...255).
 */
int motor_get_target_speed(Motor_t* motor) {
    if (!motor) return 0;
    return motor->target_speed;
}

/**
 * Получить процент нагрузки мотора (0-100%).
 * 0% = остановлен, 100% = максимальная мощность
 */
uint8_t motor_get_load_percent(Motor_t* motor) {
    if (!motor) return 0;
    return (uint8_t)((motor->current_pwm * 100) / 255);
}