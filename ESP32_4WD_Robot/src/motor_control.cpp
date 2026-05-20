#include "motor_control.h"
#include <Arduino.h>
#include "driver/ledc.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"
#include "esp_rom_gpio.h"

void motor_init(Motor_t* motor) {
    pinMode(motor->pwm_pin, OUTPUT);
    pinMode(motor->in1_pin, OUTPUT);
    pinMode(motor->in2_pin, OUTPUT);

    // Настройка LEDC как обычно
    static bool timer_init = false;
    if (!timer_init) {
        ledc_timer_config_t timer_conf = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = LEDC_TIMER_8_BIT,
            .timer_num = LEDC_TIMER_0,
            .freq_hz = 1000, // Поднимем частоту для стабильности
            .clk_cfg = LEDC_AUTO_CLK
        };
        ledc_timer_config(&timer_conf);
        timer_init = true;
    }

    ledc_channel_config_t ch_conf = {
        .gpio_num = motor->pwm_pin,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = (ledc_channel_t)motor->ledc_channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&ch_conf);

    // ПРИНУДИТЕЛЬНАЯ ПРИВЯЗКА (Спасет пины 32 и 33)
    // Мы берем сигнал LEDC_PWM_OUT_IDX и силой кидаем его на пин
    int sig_idx = (motor->ledc_channel < 8) ? (LEDC_LS_SIG_OUT0_IDX + motor->ledc_channel) : (LEDC_HS_SIG_OUT0_IDX + (motor->ledc_channel - 8));
    esp_rom_gpio_connect_out_signal(motor->pwm_pin, sig_idx, false, false);
}

void motor_set_speed(Motor_t* motor, int speed) {
    int target_pwm = constrain(abs(speed), 0, 255);
    
    // Снижаем минимальный порог старта для L298N до 50
    if (target_pwm > 0 && target_pwm < 50) {
        target_pwm = 50;
    }

    digitalWrite(motor->in1_pin, (speed > 0) ? HIGH : LOW);
    digitalWrite(motor->in2_pin, (speed < 0) ? HIGH : LOW);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)motor->ledc_channel, target_pwm);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)motor->ledc_channel);
}

// Заглушки для остального функционала
void motor_refresh_pwm(Motor_t* motor) {}
void motor_update_soft_start(Motor_t* motor) {}
void motor_control_task(void* pvParameters) { vTaskDelete(NULL); }
int motor_get_current_pwm(Motor_t* motor) { return motor->current_pwm; }
int motor_get_target_speed(Motor_t* motor) { return motor->target_speed; }
uint8_t motor_get_load_percent(Motor_t* motor) { return (motor->current_pwm * 100) / 255; }