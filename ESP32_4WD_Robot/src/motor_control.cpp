#include "motor_control.h"
#include <Arduino.h>
#include "driver/ledc.h"
#include "esp_rom_gpio.h"

// Виртуальные переменные для хранения чистой математической скорости моторов [-255...255].
// Это исключает любые ошибки округления и конфликты с аппаратными ШИМ-регистрами.
static int virtual_speed[2] = {0, 0};

void motor_init(Motor_t* motor) {
    pinMode(motor->pwm_pin, OUTPUT);
    pinMode(motor->in1_pin, OUTPUT);
    pinMode(motor->in2_pin, OUTPUT);

    // Сбрасываем виртуальную скорость для данного мотора
    uint8_t ch = (motor->ledc_channel < 2) ? motor->ledc_channel : 0;
    virtual_speed[ch] = 0;
    motor->current_pwm = 0;
    motor->target_speed = 0;
    motor->is_soft_starting = false;

    // Настройка LEDC
    static bool timer_init = false;
    if (!timer_init) {
        ledc_timer_config_t timer_conf = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = LEDC_TIMER_8_BIT,
            .timer_num = LEDC_TIMER_0,
            .freq_hz = 1000, 
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

    // Аппаратный обход матрицы GPIO для стабильности вывода ШИМ
    int sig_idx = (motor->ledc_channel < 8) ? (LEDC_LS_SIG_OUT0_IDX + motor->ledc_channel) : (LEDC_HS_SIG_OUT0_IDX + (motor->ledc_channel - 8));
    esp_rom_gpio_connect_out_signal(motor->pwm_pin, sig_idx, false, false);
}

void motor_set_speed(Motor_t* motor, int speed) {
    motor->target_speed = speed;
    motor->is_soft_starting = true;
    motor->soft_start_begin_ms = millis();
}

void motor_update_soft_start(Motor_t* motor) {
    static uint32_t last_update_time[2] = {0, 0};
    uint8_t ch = (motor->ledc_channel < 2) ? motor->ledc_channel : 0;

    uint32_t now = millis();
    // Частота обновления разгона — каждые 15 миллисекунд
    if (now - last_update_time[ch] < 15) {
        return; 
    }
    last_update_time[ch] = now;

    // Считываем скорость из независимой виртуальной переменной
    int current = virtual_speed[ch];
    int target = motor->target_speed;

    if (current == target) {
        motor->is_soft_starting = false;
        return;
    }

    // Шаг изменения скорости за один такт (15 мс)
    int step = 25; 
    int next_speed = current;

    if (current < target) {
        next_speed += step;
        if (next_speed > target) next_speed = target;
    } else {
        next_speed -= step;
        if (next_speed < target) next_speed = target;
    }

    // Сохраняем новое точное значение скорости в виртуальную память
    virtual_speed[ch] = next_speed;

    // Переводим виртуальную скорость в физический ШИМ
    int abs_pwm = abs(next_speed);

    // Накладываем аппаратный порог трогания L298N
    if (abs_pwm > 0 && abs_pwm < 50) {
        abs_pwm = 50; 
    }

    motor->current_pwm = abs_pwm;

    // Управление пинами направления
    if (next_speed > 0) {
        digitalWrite(motor->in1_pin, HIGH);
        digitalWrite(motor->in2_pin, LOW);
    } else if (next_speed < 0) {
        digitalWrite(motor->in1_pin, LOW);
        digitalWrite(motor->in2_pin, HIGH);
    } else {
        digitalWrite(motor->in1_pin, LOW);
        digitalWrite(motor->in2_pin, LOW);
    }

    // Обновляем ШИМ на ножке ESP32
    ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)motor->ledc_channel, abs_pwm);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)motor->ledc_channel);
}

// Заглушки для совместимости с заголовочным файлом
void motor_refresh_pwm(Motor_t* motor) {}
int motor_get_current_pwm(Motor_t* motor) { return motor->current_pwm; }
int motor_get_target_speed(Motor_t* motor) { return motor->target_speed; }
uint8_t motor_get_load_percent(Motor_t* motor) { return (motor->current_pwm * 100) / 255; }
void motor_control_task(void* pvParameters) { vTaskDelete(NULL); }