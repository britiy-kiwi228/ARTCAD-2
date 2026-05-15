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
    motor->target_speed = 0;
    motor->current_pwm = 0;
    motor->is_soft_starting = false;
    
    pinMode(motor->in1_pin, OUTPUT);
    pinMode(motor->in2_pin, OUTPUT);
    digitalWrite(motor->in1_pin, LOW);
    digitalWrite(motor->in2_pin, LOW);

    // ДЛЯ ТВОЕЙ ВЕРСИИ (2.0.17) НУЖНО ТАК:
    ledcSetup(motor->ledc_channel, PWM_FREQ, PWM_RES);
    ledcAttachPin(motor->pwm_pin, motor->ledc_channel);
    ledcWrite(motor->ledc_channel, 0);

    Serial.printf("[MOTOR] Channel %d attached to Pin %d\n", motor->ledc_channel, motor->pwm_pin);
}

/**
 * Установка скорости мотора С ПЛАВНЫМ РАЗГОНОМ (Soft Start protection).
 * 
 * Плавный разгон защищает:
 * - Аккумулятор от ударных токов пуска
 * - Механику от резких рывков
 * - Предохранитель от срабатывания
 */
// Минимальный порог ШИМ для преодоления инерции L298N
#define MOTOR_MIN_PWM_THRESHOLD 120 

void motor_set_speed(Motor_t* motor, int speed) {
    // 1. Ограничение входящего значения (защита от мусора)
    if (speed > 255)  speed = 255;
    if (speed < -255) speed = -255;

    // 2. КРИТИЧЕСКАЯ ПРОВЕРКА: Если целевая скорость уже такая же, ничего не делаем.
    // Это исключает постоянный сброс таймеров при получении пакетов от браузера.
    if (motor->target_speed == speed) return;

    motor->target_speed = speed;

    // 3. Обработка полной остановки
    if (speed == 0) {
        motor->current_pwm = 0;
        motor->is_soft_starting = false;
        digitalWrite(motor->in1_pin, LOW);
        digitalWrite(motor->in2_pin, LOW);
        ledcWrite(motor->ledc_channel, 0); // Используем КАНАЛ для версии 2.0.17
        return;
    }

    // 4. Установка направления (IN пины)
    // Сначала ставим оба в LOW для безопасности, потом нужное направление
    digitalWrite(motor->in1_pin, LOW);
    digitalWrite(motor->in2_pin, LOW);
    
    if (speed > 0) {
        digitalWrite(motor->in1_pin, HIGH);
        digitalWrite(motor->in2_pin, LOW);
    } else {
        digitalWrite(motor->in1_pin, LOW);
        digitalWrite(motor->in2_pin, HIGH);
    }

    // 5. Расчет мощности ШИМ
    int effective_speed = abs(speed);

    // Минимальный порог, чтобы мотор вообще сдвинулся (для L298N)
    // Если скорость слишком мала, драйвер просто будет греться, но не крутить.
    if (effective_speed < MOTOR_MIN_PWM_THRESHOLD) {
        effective_speed = MOTOR_MIN_PWM_THRESHOLD;
    }

    // 6. ПРИМЕНЕНИЕ (Временно отключаем плавный пуск для теста)
    // Мы сразу подаем нужную скорость, чтобы исключить ошибки в loop()
    motor->current_pwm = effective_speed;
    motor->is_soft_starting = false; // Отключаем логику разгона в loop
    
    // ВАЖНО: В твоей версии ядра (2.0.17) ledcWrite работает ТОЛЬКО с КАНАЛОМ
    ledcWrite(motor->ledc_channel, motor->current_pwm);

    // Отладка в сериал (если когда-то подключишь)
    // Serial.printf("Motor set: Target %d, PWM %d, Channel %d\n", speed, motor->current_pwm, motor->ledc_channel);
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