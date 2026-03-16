#include "servo_control.h"
#include "config.h"
#include <Arduino.h>

void servo_init(Servo_t* servo) {
    // Настройка пина управления как OUTPUT
    pinMode(servo->pin, OUTPUT);

    // Настройка аппаратного ШИМ (LEDC)
    ledcSetup(servo->ledc_channel, SERVO_FREQ, SERVO_RES);
    ledcAttachPin(servo->pin, servo->ledc_channel);

    // Устанавливаем начальный угол в 90 градусов (нейтральное положение)
    servo_set_angle(servo, 90);
}

void servo_set_angle(Servo_t* servo, int angle) {
    // Ограничиваем угол от 0 до 180 градусов
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    // Вычисляем значение ШИМ для заданного угла
    int duty = map(angle, 0, 180, SERVO_MIN_DUTY, SERVO_MAX_DUTY);

    // Устанавливаем ШИМ на соответствующий канал
    ledcWrite(servo->ledc_channel, duty);
}