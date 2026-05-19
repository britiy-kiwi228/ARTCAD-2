#include "motor_control.h"
#include <Arduino.h>
#include "driver/ledc.h"
#include "esp_rom_gpio.h"
#include "soc/gpio_sig_map.h"

static bool motor_timer_initialized = false;

#define MOTOR_FREQ_FIX    1000
#define MOTOR_RES_FIX     LEDC_TIMER_8_BIT
#define MOTOR_TIMER_FIX   LEDC_TIMER_1  // ПЕРЕКЛЮЧАЕМ НА ТАЙМЕР ОРУЖИЯ

void motor_init(Motor_t* motor) {
    // Настраиваем таймер 1 (как в оружии)
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = MOTOR_RES_FIX,
        .timer_num = MOTOR_TIMER_FIX,
        .freq_hz = MOTOR_FREQ_FIX,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);

    pinMode(motor->in1_pin, OUTPUT);
    pinMode(motor->in2_pin, OUTPUT);
    pinMode(motor->pwm_pin, OUTPUT);
    
    // СТРОГАЯ логика инициализации канала
    ledc_channel_config_t ch_conf = {
        .gpio_num = motor->pwm_pin,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = (ledc_channel_t)motor->ledc_channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = MOTOR_TIMER_FIX,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&ch_conf);
    
    // Прямая команда драйверу L298N занять пин (Strong Drive)
   
     int signal_idx = (motor->ledc_channel < 8) ? (LEDC_LS_SIG_OUT0_IDX + motor->ledc_channel) : (LEDC_HS_SIG_OUT0_IDX + (motor->ledc_channel - 8));
    esp_rom_gpio_connect_out_signal(motor->pwm_pin, signal_idx, false, false);
}

void motor_set_speed(Motor_t* motor, int speed) {
    // 1. Копируем constrain из weapon_system
    speed = constrain(speed, -255, 255);
    int pwm = abs(speed);
    
    // 2. Ограничение как в оружии
    if (pwm > 0 && pwm < 80) pwm = 80; 
    
   
    motor->current_pwm = pwm;
    motor->target_speed = speed;

    // 4. Установка направления (как в оружии)
    digitalWrite(motor->in1_pin, (speed > 0) ? HIGH : LOW);
    digitalWrite(motor->in2_pin, (speed < 0) ? HIGH : LOW);

    // 5. Запись ШИМ
    ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)motor->ledc_channel, pwm);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)motor->ledc_channel);
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