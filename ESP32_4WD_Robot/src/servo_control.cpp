#include "servo_control.h"
#include "config.h"
#include <Arduino.h>
#include "driver/ledc.h"

// Глобальный флаг для инициализации таймера сервопривода (один раз)
static bool servo_timer_initialized = false;

/**
 * Инициализация таймера LEDC для сервопривода один раз
 * Таймер 2 @ 50 Hz для сервопривода MG995
 */
static void servo_ledc_timer_init() {
    if (servo_timer_initialized) return;
    
    // Настройка таймера 2 для сервопривода (50 Hz, HIGH SPEED, 16-bit)
    ledc_timer_config_t timer_conf = {
        .speed_mode = SERVO_SPEED_MODE,
        .duty_resolution = (ledc_timer_bit_t)SERVO_PWM_RES,
        .timer_num = SERVO_TIMER,
        .freq_hz = SERVO_PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);
    
    servo_timer_initialized = true;
}

/**
 * Инициализация сервопривода
 * @param servo Указатель на структуру Servo_t
 */
void servo_init(Servo_t* servo) {
    if (!servo) return;
    
    // Инициализируем таймер (один раз)
    servo_ledc_timer_init();
    
    // Устанавливаем пин в режим OUTPUT
    pinMode(servo->pin, OUTPUT);
    
    // Настройка канала LEDC для сервопривода
    ledc_channel_config_t channel_conf = {
        .gpio_num = servo->pin,
        .speed_mode = SERVO_SPEED_MODE,
        .channel = (ledc_channel_t)servo->ledc_channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = SERVO_TIMER,
        .duty = SERVO_MIN_DUTY,  // Начальная позиция: 0 градусов
        .hpoint = 0
    };
    ledc_channel_config(&channel_conf);
    
    Serial.print("[SERVO INIT] Pin: ");
    Serial.print(servo->pin);
    Serial.print(" | Freq: ");
    Serial.print(SERVO_PWM_FREQ);
    Serial.println(" Hz");
}

/**
 * Установка угла поворота сервопривода (0...180 градусов)
 * 
 * @param servo Указатель на структуру Servo_t
 * @param angle Угол в градусах (0...180)
 * 
 * Формула расчета duty cycle:
 * duty = MIN_DUTY + (angle / 180.0) * (MAX_DUTY - MIN_DUTY)
 * 
 * МГ995 требует:
 * - 0.5 мс (1% от 50мс) = 0 градусов
 * - 2.4 мс (4.8% от 50мс) = 180 градусов
 * 
 * При 16-бит разрешении (0-65535):
 * - 0.5 мс: 819 (0.5ms / 20ms * 65535)
 * - 2.4 мс: 3932 (2.4ms / 20ms * 65535)
 */
void servo_set_angle(Servo_t* servo, int angle) {
    if (!servo) return;
    
    // Клипируем угол в диапазон [0, 180]
    angle = constrain(angle, 0, 180);
    
    // Вычисляем duty cycle через линейную интерполяцию
    uint32_t duty = SERVO_MIN_DUTY + (angle * (SERVO_MAX_DUTY - SERVO_MIN_DUTY)) / 180;
    
    // Устанавливаем duty cycle для канала LEDC
    ledc_set_duty(SERVO_SPEED_MODE, (ledc_channel_t)servo->ledc_channel, duty);
    ledc_update_duty(SERVO_SPEED_MODE, (ledc_channel_t)servo->ledc_channel);
}