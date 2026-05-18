#include "weapon_system.h"
#include <Arduino.h>

static bool weapon_timer_initialized = false;

void weapon_init(WeaponMotor_t* weapon) {
    if (!weapon_timer_initialized) {
        ledc_timer_config_t timer_conf = {
            .speed_mode = WEAPON_SPEED_MODE,
            .duty_resolution = WEAPON_PWM_RES,
            .timer_num = WEAPON_TIMER,
            .freq_hz = WEAPON_PWM_FREQ,
            .clk_cfg = LEDC_AUTO_CLK
        };
        ledc_timer_config(&timer_conf);
        weapon_timer_initialized = true;
    }

    pinMode(weapon->in1_pin, OUTPUT);
    pinMode(weapon->in2_pin, OUTPUT);
    digitalWrite(weapon->in1_pin, LOW);
    digitalWrite(weapon->in2_pin, LOW);

    ledc_channel_config_t ch_conf = {
        .gpio_num = weapon->en_pin,
        .speed_mode = WEAPON_SPEED_MODE,
        .channel = (ledc_channel_t)weapon->ledc_channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = WEAPON_TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&ch_conf);
}

void weapon_set_speed(WeaponMotor_t* weapon, int speed) {
    speed = constrain(speed, -255, 255);
    int pwm = abs(speed);
    if (pwm > WEAPON_MAX_PWM) pwm = WEAPON_MAX_PWM;
    
    weapon->current_pwm = pwm;
    digitalWrite(weapon->in1_pin, (speed > 0) ? HIGH : LOW);
    digitalWrite(weapon->in2_pin, (speed < 0) ? HIGH : LOW);

    ledc_set_duty(WEAPON_SPEED_MODE, (ledc_channel_t)weapon->ledc_channel, pwm);
    ledc_update_duty(WEAPON_SPEED_MODE, (ledc_channel_t)weapon->ledc_channel);
}

void weapon_stop(WeaponMotor_t* weapon) {
    weapon_set_speed(weapon, 0);
    weapon->is_rotating = false;
}

bool weapon_rotate_to_angle(WeaponMotor_t* weapon, float angle, int speed, uint8_t loadL, uint8_t loadR) {
    if (loadL > WEAPON_MOTOR_LOAD_THRESHOLD || loadR > WEAPON_MOTOR_LOAD_THRESHOLD) return false;

    float t_360 = 60000.0f / weapon->motor_rpm;
    weapon->target_time_ms = (angle / 360.0f) * t_360 / weapon->gear_ratio;
    weapon->rotation_start_ms = millis();
    weapon->is_rotating = true;
    weapon_set_speed(weapon, speed);
    return true;
}

bool weapon_update_rotation(WeaponMotor_t* weapon) {
    if (!weapon->is_rotating) return false;
    if (millis() - weapon->rotation_start_ms >= weapon->target_time_ms) {
        weapon_stop(weapon);
        return true;
    }
    return false;
}

bool weapon_fire_simple(WeaponMotor_t* weapon, uint8_t loadL, uint8_t loadR) {
    return weapon_rotate_to_angle(weapon, WEAPON_ROTATION_ANGLE, WEAPON_ROTATION_SPEED, loadL, loadR);
}

bool weapon_is_rotating(WeaponMotor_t* weapon) { return weapon->is_rotating; }
int weapon_get_current_pwm(WeaponMotor_t* weapon) { return weapon->current_pwm; }