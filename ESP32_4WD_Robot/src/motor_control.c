#include "driver/ledc.h" // Включаем драйвер для управления ШИМ
#include <Arduino.h>       // 1. Библиотека с базовыми функциями (pinMode, digitalWrite)
#include "motor_control.h" // 2. Наш заголовочный файл

#include <stdlib.h> // 3. Подключаем стандартную библиотеку для функции abs()

/**
 * Функция инициализации конкретного мотора.
 * Принимает адрес структуры Motor_t.
 */
void motor_init(Motor_t* motor) {
    
    // --- 3. Настройка пинов направления ---
    
    // pinMode — это функция HAL (Hardware Abstraction Layer).
    // Она говорит контроллеру: "Переведи транзисторы внутри этой ножки 
    // в режим выхода (OUTPUT), чтобы мы могли выдавать туда напряжение".
    pinMode(motor->in1_pin, OUTPUT);
    pinMode(motor->in2_pin, OUTPUT);

    // Сразу подаем на них 0 (LOW), чтобы мотор не дернулся при включении.
    digitalWrite(motor->in1_pin, LOW);
    digitalWrite(motor->in2_pin, LOW);

    // --- 4. Настройка аппаратного ШИМ (LEDC) ---
    
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

void motor_set_speed(Motor_t* motor, int speed) {
    // ограничиваем диапазон.
    if (speed > 255)  speed = 255;
    if (speed < -255) speed = -255;

    // --- Шаг 2: Логика направления ---
    
    if (speed > 0) {
        // Вперед: IN1=1, IN2=0
        digitalWrite(motor->in1_pin, HIGH);
        digitalWrite(motor->in2_pin, LOW);
    } 
    else if (speed < 0) {
        // Назад: IN1=0, IN2=1
        digitalWrite(motor->in1_pin, LOW);
        digitalWrite(motor->in2_pin, HIGH);
    } 
    else {
        // Стоп: IN1=0, IN2=0
        digitalWrite(motor->in1_pin, LOW);
        digitalWrite(motor->in2_pin, LOW);
    }

    // --- Шаг 3: Установка мощности (ШИМ) ---
    ledcWrite(motor->ledc_channel, abs(speed));
}