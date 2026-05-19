#include "motor_control.h"
#include <Arduino.h>
#include "driver/ledc.h"
#include "esp_rom_gpio.h"
#include "soc/gpio_sig_map.h"
#include "driver/mcpwm.h"
#include "soc/rtc_cntl_reg.h"

static int soft_pwm_l = 0;
static int soft_pwm_r = 0;

void motor_task(void* pv) {
    while(1) {
        // Простейший программный ШИМ на 100 Гц (хватит для моторов)
        // Период 10мс
        for (int i = 0; i < 255; i++) {
            digitalWrite(MOTOR_L_PWM, (i < soft_pwm_l) ? HIGH : LOW);
            digitalWrite(MOTOR_R_PWM, (i < soft_pwm_r) ? HIGH : LOW);
            delayMicroseconds(30); // Очень быстрый цикл
        }
        vTaskDelay(1); // Даем системе подышать
    }
}

void motor_init(Motor_t* motor) {
    pinMode(motor->pwm_pin, OUTPUT);
    pinMode(motor->in1_pin, OUTPUT);
    pinMode(motor->in2_pin, OUTPUT);
    
    // Запускаем задачу управления один раз
    static bool task_started = false;
    if (!task_started) {
        xTaskCreatePinnedToCore(motor_task, "MotorTask", 2048, NULL, 10, NULL, 1);
        task_started = true;
    }
}

void motor_set_speed(Motor_t* motor, int speed) {
    int pwm = constrain(abs(speed), 0, 255);
    if (pwm > 0 && pwm < 80) pwm = 80; // Порог старта

    // Установка направления
    digitalWrite(motor->in1_pin, (speed > 0) ? HIGH : LOW);
    digitalWrite(motor->in2_pin, (speed < 0) ? HIGH : LOW);

    // Передаем значение в программный ШИМ
    if (motor->pwm_pin == 32) soft_pwm_l = pwm;
    else soft_pwm_r = pwm;
    
    motor->current_pwm = pwm;
}
// Заглушки для компиляции
void motor_refresh_pwm(Motor_t* motor) {}
void motor_update_soft_start(Motor_t* motor) {}
void motor_control_task(void* pvParameters) { while(1) vTaskDelay(100); }
int motor_get_current_pwm(Motor_t* motor) { return motor->current_pwm; }
int motor_get_target_speed(Motor_t* motor) { return motor->target_speed; }
uint8_t motor_get_load_percent(Motor_t* motor) { return (motor->current_pwm * 100) / 255; }