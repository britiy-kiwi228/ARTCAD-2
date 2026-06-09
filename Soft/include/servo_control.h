#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <stdint.h>
#include "config.h"

typedef struct {
    uint8_t pin;           // Пин управления (SERVO_SIG)
    uint8_t ledc_channel;  // Канал (LEDC_CH_SERVO)
} Servo_t;

// Функция инициализации сервопривода
void servo_init(Servo_t* servo);

// Функция установки угла (0...180 градусов)
void servo_set_angle(Servo_t* servo, int angle);

#endif // SERVO_CONTROL_H