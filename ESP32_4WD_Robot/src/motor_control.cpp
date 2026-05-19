#include "motor_control.h"
#include <Arduino.h>
#include "driver/ledc.h"
#include "esp_rom_gpio.h"
#include "soc/gpio_sig_map.h"
#include "driver/mcpwm.h"
#include "soc/rtc_cntl_reg.h"

static bool motor_timer_initialized = false;

#define MOTOR_FREQ_FIX    1000
#define MOTOR_RES_FIX     LEDC_TIMER_8_BIT
#define MOTOR_TIMER_FIX   LEDC_TIMER_1  // ПЕРЕКЛЮЧАЕМ НА ТАЙМЕР ОРУЖИЯ

void motor_init(Motor_t* motor) {
    // 1. Настройка GPIO для MCPWM
    if (motor->pwm_pin == 32) {
        mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, motor->pwm_pin);
    } else {
        mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, motor->pwm_pin);
    }

    // 2. Конфигурация MCPWM
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 1000; // 1кГц
    pwm_config.cmpr_a = 0;       // Начальный цикл 0%
    pwm_config.cmpr_b = 0;
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    
    mcpwm_init(MCPWM_UNIT_0, (motor->pwm_pin == 32) ? MCPWM_TIMER_0 : MCPWM_TIMER_1, &pwm_config);

    // Настройка пинов направления
    pinMode(motor->in1_pin, OUTPUT);
    pinMode(motor->in2_pin, OUTPUT);
    digitalWrite(motor->in1_pin, LOW);
    digitalWrite(motor->in2_pin, LOW);
    
    // Форсируем максимальный ток на пинах (Drive Strength)
    gpio_set_drive_capability((gpio_num_t)motor->pwm_pin, GPIO_DRIVE_CAP_3);
}

void motor_set_speed(Motor_t* motor, int speed) {
    float duty = (float)abs(speed);
    if (duty > 255.0) duty = 255.0;
    
    // Переводим 0-255 в 0-100% для MCPWM
    float duty_percent = (duty / 255.0) * 100.0;

    motor->current_pwm = (int)duty;
    motor->target_speed = speed;

    // Установка направления
    if (speed > 0) {
        digitalWrite(motor->in1_pin, HIGH);
        digitalWrite(motor->in2_pin, LOW);
    } else if (speed < 0) {
        digitalWrite(motor->in1_pin, LOW);
        digitalWrite(motor->in2_pin, HIGH);
    } else {
        digitalWrite(motor->in1_pin, LOW);
        digitalWrite(motor->in2_pin, LOW);
    }

    // Прямая запись в регистр MCPWM (минуя систему LEDC)
    if (motor->pwm_pin == 32) {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_percent);
    } else {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, duty_percent);
    }
}
void motor_refresh_pwm(Motor_t* motor) {
    if (motor->current_pwm > 0) {
        ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)motor->ledc_channel);
    }
}

// Заглушки для компиляции
void motor_update_soft_start(Motor_t* motor) {}
void motor_control_task(void* pvParameters) { while(1) vTaskDelay(100); }
int motor_get_current_pwm(Motor_t* motor) { return motor->current_pwm; }
int motor_get_target_speed(Motor_t* motor) { return motor->target_speed; }
uint8_t motor_get_load_percent(Motor_t* motor) { return (motor->current_pwm * 100) / 255; }