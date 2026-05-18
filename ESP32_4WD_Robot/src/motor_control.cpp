#include "driver/ledc.h"
#include <Arduino.h>
#include "motor_control.h"
#include "config.h"
#include <stdlib.h>
#include <math.h>

// Глобальный флаг для инициализации таймеров (выполняется один раз)
static bool ledc_timers_initialized = false;

/**
 * Инициализация таймеров LEDC один раз
 * Таймер 0 @ 1000 Hz для ходовых моторов
 */
static void motor_ledc_timers_init() {
    if (ledc_timers_initialized) return;
    
    // Настройка таймера 0 для ходовых моторов (1000 Hz, HIGH SPEED)
    ledc_timer_config_t timer_conf = {
        .speed_mode = MOTOR_SPEED_MODE,
        .duty_resolution = (ledc_timer_bit_t)MOTOR_PWM_RES,
        .timer_num = MOTOR_TIMER,
        .freq_hz = MOTOR_PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);
    
    ledc_timers_initialized = true;
}

/**
 * Функция инициализации конкретного мотора.
 * Принимает адрес структуры Motor_t.
 */
void motor_init(Motor_t* motor) {
    // Инициализируем таймеры (один раз)
    motor_ledc_timers_init();
    
    // Отключаем ШИМ перед настройкой
    pinMode(motor->in1_pin, OUTPUT);
    pinMode(motor->in2_pin, OUTPUT);
    pinMode(motor->pwm_pin, OUTPUT);
    
    digitalWrite(motor->in1_pin, LOW);
    digitalWrite(motor->in2_pin, LOW);
    digitalWrite(motor->pwm_pin, LOW);

    // Настройка канала LEDC
    ledc_channel_config_t channel_conf = {
        .gpio_num = motor->pwm_pin,
        .speed_mode = MOTOR_SPEED_MODE,
        .channel = (ledc_channel_t)motor->ledc_channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = MOTOR_TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel_conf);
}

void motor_set_speed(Motor_t* motor, int speed) {
    // Клипируем скорость в диапазон [-255, 255]
    speed = constrain(speed, -255, 255);
    
    if (motor->target_speed == speed) return;
    
    motor->target_speed = speed;

    if (speed == 0) {
        motor->current_pwm = 0;
        motor->is_soft_starting = false;
        digitalWrite(motor->in1_pin, LOW);
        digitalWrite(motor->in2_pin, LOW);
        ledc_set_duty(MOTOR_SPEED_MODE, (ledc_channel_t)motor->ledc_channel, 0);
        ledc_update_duty(MOTOR_SPEED_MODE, (ledc_channel_t)motor->ledc_channel);
        return;
    }

    // Устанавливаем направление вращения
    if (speed > 0) {
        digitalWrite(motor->in1_pin, HIGH);
        digitalWrite(motor->in2_pin, LOW);
    } else {
        digitalWrite(motor->in1_pin, LOW);
        digitalWrite(motor->in2_pin, HIGH);
    }

    // Инициализируем плавный пуск
    motor->is_soft_starting = true;
    motor->soft_start_begin_ms = millis();
}

/**
 * Обновление плавного разгона.
 * ОБЯЗАТЕЛЬНО вызывать в loop() или в FreeRTOS task для всех моторов!
 */
void motor_update_soft_start(Motor_t* motor) {
    if (!motor->is_soft_starting) {
        return;
    }

    uint32_t elapsed_ms = millis() - motor->soft_start_begin_ms;

    if (elapsed_ms >= MOTOR_SOFT_START_TIME_MS) {
        // Плавный разгон завершен
        motor->current_pwm = abs(motor->target_speed);
        motor->is_soft_starting = false;
    } else {
        // Плавный разгон идет
        float progress = (float)elapsed_ms / (float)MOTOR_SOFT_START_TIME_MS;
        motor->current_pwm = (int)(progress * (float)abs(motor->target_speed));
    }

    // Применяем значение ШИМ через LEDC API (0-255 для 8-бит)
    ledc_set_duty(MOTOR_SPEED_MODE, (ledc_channel_t)motor->ledc_channel, motor->current_pwm);
    ledc_update_duty(MOTOR_SPEED_MODE, (ledc_channel_t)motor->ledc_channel);
}

int motor_get_current_pwm(Motor_t* motor) {
    if (!motor) return 0;
    return motor->current_pwm;
}

/**
 * Получить целевую скорость мотора (-255...255).
 */
int motor_get_target_speed(Motor_t* motor) {
    if (!motor) return 0;
    return motor->target_speed;
}

/**
 * Получить процент нагрузки мотора (0-100%).
 * 0% = остановлен, 100% = максимальная мощность
 */
uint8_t motor_get_load_percent(Motor_t* motor) {
    if (!motor) return 0;
    return (uint8_t)((motor->current_pwm * 100) / 255);
}

/**
 * FreeRTOS задача для управления плавным разгоном моторов.
 * Вызывается периодически из loop() или как отдельный Task.
 */
void motor_control_task(void* pvParameters) {
    Motor_t** motors = (Motor_t**)pvParameters;
    const int num_motors = 2;

    while (true) {
        for (int i = 0; i < num_motors; i++) {
            motor_update_soft_start(motors[i]);
        }
        // Небольшая задержка для предотвращения перегрузки CPU
        // 10-20 мс достаточно для плавности при 150 мс разгоне
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}