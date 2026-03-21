#ifndef MOTOR_CONTROL_H // Защита от повторного включения 
#define MOTOR_CONTROL_H

#include "config.h" // Включаем конфигурационный файл для доступа к пинам и параметрам ШИМ
#include <stdint.h> // файл с типами данных, такими как uint8_t
// Структура для хранения информации о моторе
typedef struct {
    uint8_t pwm_pin;
    uint8_t in1_pin;
    uint8_t in2_pin;
    uint8_t ledc_channel;
} Motor_t;
// Функция для инициализации мотора
void motor_init(Motor_t* motor);

void motor_set_speed(Motor_t* motor, int speed);

#endif // конец MOTOR_CONTROL_H