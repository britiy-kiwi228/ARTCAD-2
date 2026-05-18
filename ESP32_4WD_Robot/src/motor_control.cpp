#include "motor_control.h"
#include <Arduino.h>
#include "driver/ledc.h"

void motor_init(Motor_t* motor) {
    // Настраиваем таймер принудительно для каждого вызова
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);

    pinMode(motor->in1_pin, OUTPUT);
    pinMode(motor->in2_pin, OUTPUT);

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
    
    Serial.printf("[Motor Init] Pin %d, Channel %d\n", motor->pwm_pin, motor->ledc_channel);
}

void motor_set_speed(Motor_t* motor, int speed) {
    int pwm = abs(speed);
    if (pwm > 255) pwm = 255;
    
    motor->current_pwm = pwm;

    if (speed == 0) {
        digitalWrite(motor->in1_pin, LOW);
        digitalWrite(motor->in2_pin, LOW);
    } else if (speed > 0) {
        digitalWrite(motor->in1_pin, HIGH);
        digitalWrite(motor->in2_pin, LOW);
    } else {
        digitalWrite(motor->in1_pin, LOW);
        digitalWrite(motor->in2_pin, HIGH);
    }

    // Пишем напрямую в регистры ШИМ
    ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)motor->ledc_channel, pwm);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)motor->ledc_channel);
    
    Serial.printf("  -> Motor Pin %d: PWM=%d, Dir=%s\n", 
                  motor->pwm_pin, pwm, (speed > 0 ? "FWD" : (speed < 0 ? "REV" : "STOP")));
}

// Заглушки для компиляции
void motor_update_soft_start(Motor_t* motor) {}
void motor_control_task(void* pvParameters) { while(1) vTaskDelay(100); }
int motor_get_current_pwm(Motor_t* motor) { return motor->current_pwm; }
int motor_get_target_speed(Motor_t* motor) { return motor->target_speed; }
uint8_t motor_get_load_percent(Motor_t* motor) { return (motor->current_pwm * 100) / 255; }