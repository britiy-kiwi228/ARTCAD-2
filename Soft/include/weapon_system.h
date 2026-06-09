#ifndef WEAPON_SYSTEM_H
#define WEAPON_SYSTEM_H

#include "config.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t en_pin;
    uint8_t in1_pin;
    uint8_t in2_pin;
    uint8_t ledc_channel;
    float motor_rpm;
    float gear_ratio;
    int current_speed;
    int current_pwm;
    uint32_t rotation_start_ms;
    float target_time_ms;
    bool is_rotating;
} WeaponMotor_t;

void weapon_init(WeaponMotor_t* weapon);
void weapon_set_speed(WeaponMotor_t* weapon, int speed);
void weapon_stop(WeaponMotor_t* weapon);
bool weapon_rotate_to_angle(WeaponMotor_t* weapon, float target_angle, int speed, uint8_t motor_left_load, uint8_t motor_right_load);
bool weapon_update_rotation(WeaponMotor_t* weapon);
bool weapon_fire_simple(WeaponMotor_t* weapon, uint8_t motor_left_load, uint8_t motor_right_load);
bool weapon_is_rotating(WeaponMotor_t* weapon);
int weapon_get_current_pwm(WeaponMotor_t* weapon);

#endif