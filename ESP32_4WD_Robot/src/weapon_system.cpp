#include "weapon_system.h"
#include <Arduino.h>
#include <driver/ledc.h>
#include <stdlib.h>
/**
 * Initialize weapon system (TC1508 driver + motor).
 * 
 * Steps:
 * 1. Save motor parameters to structure
 * 2. Configure direction pins as outputs (INA, INB)
 * 3. Configure PWM channel (LEDC) for speed control
 * 4. Bring motor to rest state
 */
void weapon_init(WeaponMotor_t* weapon) {
    if (!weapon) {
        return;
    }

    // Initialize parameters if not already set
    if (weapon->motor_rpm == 0) {
        weapon->motor_rpm = WEAPON_MOTOR_RPM;
    }
    if (weapon->gear_ratio == 0) {
        weapon->gear_ratio = WEAPON_GEAR_RATIO;
    }
    
    // Initialize state variables
    weapon->current_speed = 0;
    weapon->current_pwm = 0;
    weapon->is_rotating = false;
    weapon->rotation_start_ms = 0;
    weapon->target_time_ms = 0.0f;

    // Configure direction control pins as outputs
    pinMode(weapon->ina_pin, OUTPUT);
    pinMode(weapon->inb_pin, OUTPUT);
    
    // Set pins to LOW (motor disabled)
    digitalWrite(weapon->ina_pin, LOW);
    digitalWrite(weapon->inb_pin, LOW);

    // Configure PWM via LEDC
    ledcSetup(weapon->ledc_channel, PWM_FREQ, PWM_RES);
    ledcAttachPin(weapon->pwm_pin, weapon->ledc_channel);
    ledcWrite(weapon->ledc_channel, 0);
}

/**
 * Set motor speed with TC1508 driver control.
 * 
 * Logic:
 *   speed > 0:  INA=HIGH, INB=LOW  (forward rotation)
 *   speed < 0:  INA=LOW,  INB=HIGH (reverse rotation)
 *   speed = 0:  INA=LOW,  INB=LOW  (full stop)
 * 
 * SAFETY: PWM is capped at WEAPON_MAX_PWM (200/255 = 78% = 9.8V)
 * This protects TC1508 from thermal shutdown at 12.6V input
 */
void weapon_set_speed(WeaponMotor_t* weapon, int speed) {
    if (!weapon) {
        return;
    }

    // Limit speed to [-255, 255]
    if (speed > 255)  speed = 255;
    if (speed < -255) speed = -255;

    // Save current speed
    weapon->current_speed = speed;

    // Control direction via INA/INB logic
    if (speed > 0) {
        digitalWrite(weapon->ina_pin, HIGH);
        digitalWrite(weapon->inb_pin, LOW);
    } 
    else if (speed < 0) {
        digitalWrite(weapon->ina_pin, LOW);
        digitalWrite(weapon->inb_pin, HIGH);
    } 
    else {
        digitalWrite(weapon->ina_pin, LOW);
        digitalWrite(weapon->inb_pin, LOW);
    }

    // SAFETY: PWM LIMITING
    // Max WEAPON_MAX_PWM (200 instead of 255) to protect TC1508
    // 200/255 = ~78% = 12.6V * 0.78 = 9.8V (below TC1508 thermal shutdown)
    int pwm_value = abs(speed);
    if (pwm_value > WEAPON_MAX_PWM) {
        pwm_value = WEAPON_MAX_PWM;
    }
    
    weapon->current_pwm = pwm_value;
    ledcWrite(weapon->ledc_channel, pwm_value);
}

/**
 * Safe motor stop.
 */
void weapon_stop(WeaponMotor_t* weapon) {
    if (!weapon) {
        return;
    }
    weapon_set_speed(weapon, 0);
    weapon->is_rotating = false;
}

/**
 * Initiate rotation to target angle WITH load protection.
 * 
 * SAFETY: Checks drive motor loads.
 * If load > WEAPON_MOTOR_LOAD_THRESHOLD (50%) -> blocks fire
 * This protects 7.5A fuse from overload when catapult + drive motors run together
 * 
 * @param weapon Pointer to weapon motor structure
 * @param target_angle Target rotation angle (0-360 degrees)
 * @param speed Motor speed (0-255)
 * @param motor_left_load Drive motor left load percentage (0-100)
 * @param motor_right_load Drive motor right load percentage (0-100)
 * @return true if fire initiated successfully, false if blocked
 */
bool weapon_rotate_to_angle(WeaponMotor_t* weapon, float target_angle, int speed,
                            uint8_t motor_left_load, uint8_t motor_right_load) {
    if (!weapon) {
        return false;
    }

    // SAFETY: CHECK DRIVE MOTOR LOADS
    // If either motor load > 50% -> block catapult
    // Protects fuse (max 7.5A) from overload
    uint8_t max_motor_load = (motor_left_load > motor_right_load) ? motor_left_load : motor_right_load;
    
    if (max_motor_load > WEAPON_MOTOR_LOAD_THRESHOLD) {
        Serial.printf("[WEAPON BLOCKED] Motor load too high: L=%d%%, R=%d%% (threshold: %d%%)\n", 
                      motor_left_load, motor_right_load, WEAPON_MOTOR_LOAD_THRESHOLD);
        return false;
    }

    // Normalize angle to [0, 360)
    while (target_angle >= 360.0f) {
        target_angle -= 360.0f;
    }
    while (target_angle < 0.0f) {
        target_angle += 360.0f;
    }

    // Calculate rotation time for 360 degrees in milliseconds
    // T_360_ms = 60000 / RPM
    float time_for_360_ms = 60000.0f / weapon->motor_rpm;

    // Calculate rotation time for target angle
    // T_angle_ms = (angle / 360) * T_360
    float required_time_ms = (target_angle / 360.0f) * time_for_360_ms;

    // Apply gear ratio (if > 1.0, it slows rotation)
    required_time_ms = required_time_ms / weapon->gear_ratio;

    // Save rotation parameters
    weapon->target_time_ms = required_time_ms;
    weapon->rotation_start_ms = millis();
    weapon->is_rotating = true;

    Serial.printf("[WEAPON FIRE] Angle: %.1f degrees, Speed: %d, Load: L=%d%% R=%d%%\n",
                  target_angle, speed, motor_left_load, motor_right_load);

    // Start motor at specified speed
    weapon_set_speed(weapon, speed);
    return true;
}

/**
 * Update rotation state - call regularly in main loop.
 * 
 * Logic:
 * 1. Check if rotation is active
 * 2. If target time reached -> stop motor and return true
 * 3. Otherwise return false
 * 
 * @return true if rotation completed, false if still rotating
 */
bool weapon_update_rotation(WeaponMotor_t* weapon) {
    if (!weapon || !weapon->is_rotating) {
        return false;
    }

    uint32_t elapsed_ms = millis() - weapon->rotation_start_ms;

    if (elapsed_ms >= weapon->target_time_ms) {
        weapon_stop(weapon);
        weapon->is_rotating = false;
        return true;
    }

    return false;
}

/**
 * Get current rotation state.
 * 
 * @return true if motor is rotating to angle, false otherwise
 */
bool weapon_is_rotating(WeaponMotor_t* weapon) {
    if (!weapon) {
        return false;
    }
    return weapon->is_rotating;
}

/**
 * Get current PWM value (after safety limits).
 * 
 * @return Current PWM value (0 to WEAPON_MAX_PWM)
 */
int weapon_get_current_pwm(WeaponMotor_t* weapon) {
    if (!weapon) {
        return 0;
    }
    return weapon->current_pwm;
}
