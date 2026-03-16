#include <Arduino.h>
#include "config.h"
#include "motor_control.h"

// --- 1. Создание объектов моторов ---
// Мы выделяем память под две структуры типа Motor_t.
// Эти переменные глобальные, чтобы они были доступны во всём файле.

Motor_t motorL; // Левый борт
Motor_t motorR; // Правый борт

void setup() {
    // --- 2. Заполнение данных левого мотора ---
    motorL.pwm_pin = MOTOR_L_PWM;
    motorL.in1_pin = MOTOR_L_IN1;
    motorL.in2_pin = MOTOR_L_IN2;
    motorL.ledc_channel = LEDC_CH_L;

    // --- 3. Заполнение данных правого мотора ---
    motorR.pwm_pin = MOTOR_R_PWM;
    motorR.in1_pin = MOTOR_R_IN1;
    motorR.in2_pin = MOTOR_R_IN2;
    motorR.ledc_channel = LEDC_CH_R;

    // --- 4. Физическая инициализация ---
    // Вызываем нашу функцию, которая настроит пины и ШИМ внутри ESP32
    motor_init(&motorL);
    motor_init(&motorR);

    // Для отладки откроем последовательный порт (монитор порта)
    Serial.begin(115200);
    Serial.println("Robot 4WD Initialized!");
}

void loop() {
    // Пока оставим пустым, здесь будет Wi-Fi и FSM
}