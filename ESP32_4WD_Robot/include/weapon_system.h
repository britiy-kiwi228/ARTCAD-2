#ifndef WEAPON_SYSTEM_H
#define WEAPON_SYSTEM_H

#include "config.h"
#include <stdint.h>
#include <stdbool.h>

// Safety parameters for L298N motor driver
#define WEAPON_MAX_PWM 200
#define WEAPON_MOTOR_LOAD_THRESHOLD 50
#define WEAPON_SOFT_START_TIME_MS 100

/**
 * Weapon motor control structure.
 * Encapsulates motor parameters and state for L298N driver control.
 */
typedef struct {
    uint8_t en_pin;       // Enable pin (for PWM speed control)
    uint8_t in1_pin;      // Input 1 (direction control)
    uint8_t in2_pin;      // Input 2 (direction control)
    uint8_t ledc_channel;
    
    float motor_rpm;
    float gear_ratio;
    
    int current_speed;
    int current_pwm;
    uint32_t rotation_start_ms;
    float target_time_ms;
    bool is_rotating;
    
} WeaponMotor_t;

/**
 * Initialize weapon system.
 * Configures GPIO pins, LEDC PWM channels, and brings motor to rest state.
 * 
 * @param weapon Pointer to WeaponMotor_t structure
 */
void weapon_init(WeaponMotor_t* weapon);

/**
 * Set motor speed with thermal protection.
 * 
 * @param weapon Pointer to WeaponMotor_t structure
 * @param speed Speed from -255 (max reverse) to 255 (max forward), 0 = stop
 * 
 * SAFETY: PWM limited to WEAPON_MAX_PWM to prevent L298N thermal shutdown
 */
void weapon_set_speed(WeaponMotor_t* weapon, int speed);

/**
 * Safe motor stop (speed = 0).
 * 
 * @param weapon Pointer to WeaponMotor_t structure
 */
void weapon_stop(WeaponMotor_t* weapon);

/**
 * Initiate rotation to target angle WITH load protection.
 * 
 * SAFETY: Checks drive motor loads.
 * If load > WEAPON_MOTOR_LOAD_THRESHOLD -> blocks fire
 * 
 * @param weapon Pointer to WeaponMotor_t structure
 * @param target_angle Target rotation angle (0-360 degrees)
 * @param speed Motor speed (0-255)
 * @param motor_left_load Drive motor left load percentage (0-100)
 * @param motor_right_load Drive motor right load percentage (0-100)
 * @return true if fire initiated, false if blocked by protection
 */
bool weapon_rotate_to_angle(WeaponMotor_t* weapon, float target_angle, int speed, 
                            uint8_t motor_left_load, uint8_t motor_right_load);

/**
 * Update rotation state - call regularly in main loop().
 * 
 * @param weapon Pointer to WeaponMotor_t structure
 * @return true if rotation completed, false if still rotating
 */
bool weapon_update_rotation(WeaponMotor_t* weapon);

/**
 * Get current rotation state.
 * 
 * @param weapon Pointer to WeaponMotor_t structure
 * @return true if motor is rotating to angle, false otherwise
 */
bool weapon_is_rotating(WeaponMotor_t* weapon);

/**
 * Get current PWM value (after safety limits).
 * 
 * @param weapon Pointer to WeaponMotor_t structure
 * @return Current PWM value (0 to WEAPON_MAX_PWM)
 */
int weapon_get_current_pwm(WeaponMotor_t* weapon);

#endif
