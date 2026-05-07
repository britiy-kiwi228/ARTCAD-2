#include "servo_control.h"
#include "config.h"
#include <Arduino.h>

void servo_init(Servo_t* servo) {
    // Настройка пина управления как OUTPUT
    pinMode(servo->pin, OUTPUT);

    // ===== КРИТИЧНОЕ ИСПРАВЛЕНИЕ: Использовать правильную частоту для серво =====
    // Серво требует 50 Hz, а моторы работают на 5 kHz
    // Поэтому используем отдельный LEDC таймер (Timer 1) с 50 Hz частотой
    // Это предотвращает конфликты и обеспечивает стабильный сигнал для серво
    
    // ledcSetup(канал, частота_Hz, разрешение_бит)
    // Используем Channel 4 на Timer 1 с 50 Hz для серво
    // На 10-бит разрешении (1024 уровня) это дает периоды от 0.5мс до 2.5мс
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