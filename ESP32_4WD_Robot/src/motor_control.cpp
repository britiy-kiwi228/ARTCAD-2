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
    
    // --- 1. ИНИЦИАЛИЗАЦИЯ ПЕРЕМЕННЫХ СОСТОЯНИЯ ---
    motor->target_speed = 0;
    motor->current_pwm = 0;
    motor->soft_start_begin_ms = 0;
    motor->is_soft_starting = false;
    
    // --- 2. Настройка пинов направления ---
    
    // pinMode — это функция HAL (Hardware Abstraction Layer).
    // Она говорит контроллеру: "Переведи транзисторы внутри этой ножки 
    // в режим выхода (OUTPUT), чтобы мы могли выдавать туда напряжение".
    pinMode(motor->in1_pin, OUTPUT);
    pinMode(motor->in2_pin, OUTPUT);

    // Сразу подаем на них 0 (LOW), чтобы мотор не дернулся при включении.
    digitalWrite(motor->in1_pin, LOW);
    digitalWrite(motor->in2_pin, LOW);

    // --- 3. Настройка аппаратного ШИМ (LEDC) ---
    
    // В ESP32 ШИМ настраивается в три шага через встроенные функции:

    // Шаг А: ledcSetup(канал, частота, разрешение)
    // Мы берем значения из нашего конфига. 
    // Эта функция настраивает внутренний таймер процессора.
    ledcSetup(motor->ledc_channel, PWM_FREQ, PWM_RES);

    // Шаг Б: ledcAttachPin(пин, канал)
    // Мы "привязываем" физическую ножку (pwm_pin) к настроенному каналу ШИМ.
    // Теперь всё, что мы отправим в канал, появится на этой ножке.
    ledcAttachPin(motor->pwm_pin, motor->ledc_channel);

    // Шаг В: Устанавливаем начальную скорость в 0.
    // ledcWrite(канал, значение_от_0_до_255)
    ledcWrite(motor->ledc_channel, 0);
}

/**
 * Установка скорости мотора С ПЛАВНЫМ РАЗГОНОМ (Soft Start protection).
 * 
 * Плавный разгон защищает:
 * - Аккумулятор от ударных токов пуска
 * - Механику от резких рывков
 * - Предохранитель от срабатывания
 */
void motor_set_speed(Motor_t* motor, int speed) {
    // Валидация: ограничиваем диапазон
    if (speed > 255)  speed = 255;
    if (speed < -255) speed = -255;

    // Сохраняем целевую скорость
    motor->target_speed = speed;

    // Если скорость 0 или меняется направление → сразу остановиться
    if (speed == 0) {
        motor->current_pwm = 0;
        motor->is_soft_starting = false;
        
        // Управление направлением
        digitalWrite(motor->in1_pin, LOW);
        digitalWrite(motor->in2_pin, LOW);
        
        // Останавливаем ШИМ
        ledcWrite(motor->ledc_channel, 0);
        return;
    }

    // === ПЛАВНЫЙ РАЗГОН (Soft Start) ===
    // Если текущая скорость != целевой и они не меняют направление → начнем плавный разгон
    if (abs(motor->current_pwm) < abs(speed)) {
        motor->is_soft_starting = true;
        motor->soft_start_begin_ms = millis();
    } else {
        motor->is_soft_starting = false;
    }

    // Установим направление вращения
    if (speed > 0) {
        // Вперед: IN1=1, IN2=0
        digitalWrite(motor->in1_pin, HIGH);
        digitalWrite(motor->in2_pin, LOW);
    } else {
        // Назад: IN1=0, IN2=1
        digitalWrite(motor->in1_pin, LOW);
        digitalWrite(motor->in2_pin, HIGH);
    }

    // Если плавный разгон активен → обновляем в motor_update_soft_start()
    // Иначе сразу применяем целевую скорость
    if (!motor->is_soft_starting) {
        motor->current_pwm = abs(speed);
        ledcWrite(motor->ledc_channel, motor->current_pwm);
    }
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