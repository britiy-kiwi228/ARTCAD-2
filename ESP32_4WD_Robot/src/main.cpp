#include <Arduino.h>
#include "config.h"
#include "motor_control.h"
#include "servo_control.h"
#include "wifi_handler.h"
#include "ultrasonic.h"
#include "weapon_system.h"

// ===== РЕАЛЬНОЕ СОЗДАНИЕ ОБЪЕКТОВ (Linker будет доволен) =====
Motor_t motorL;
Motor_t motorR;
Servo_t servoWeapon;
WeaponMotor_t weapon_motor;
Ultrasonic_t distanceSensor;

// Глобальные переменные команд
volatile int cmdSpeedL = 0, cmdSpeedR = 0;
volatile bool cmdChanged = false;
volatile int cmdServoAngle = 90;
volatile bool cmdServoChanged = false;
volatile int cmdWeaponSpeed = 0, cmdWeaponAngle = 0;
volatile bool cmdWeaponFire = false, cmdWeaponChanged = false;

// Состояние системы
volatile uint32_t lastUpdateTime = 0;
bool isFailsafeActive = false;
uint32_t lastFailsafeCheck = 0;
uint32_t lastUltraScan = 0;

// Массив для FreeRTOS
Motor_t* motors_ptr[2];

void setup() {
    #ifdef DEBUG_ENABLE
    Serial.begin(115200);
    #endif
    
    DEBUG_PRINTLN("\n=== ROBOT BOOTING ===");

    // 1. Моторы
    motorL = {MOTOR_L_PWM, MOTOR_L_IN1, MOTOR_L_IN2, (uint8_t)LEDC_CH_L, 0, 0, 0, false};
    motorR = {MOTOR_R_PWM, MOTOR_R_IN1, MOTOR_R_IN2, (uint8_t)LEDC_CH_R, 0, 0, 0, false};
    motor_init(&motorL);
    motor_init(&motorR);

    // 2. Серво
    servoWeapon = {SERVO_SIG, (uint8_t)LEDC_CH_SERVO};
    servo_init(&servoWeapon);
    servo_set_angle(&servoWeapon, 90);

    // 3. Катапульта
    weapon_motor.en_pin = WEAPON_EN;
    weapon_motor.in1_pin = WEAPON_IN1;
    weapon_motor.in2_pin = WEAPON_IN2;
    weapon_motor.ledc_channel = LEDC_CH_WEAPON;
    weapon_motor.motor_rpm = WEAPON_MOTOR_RPM;
    weapon_motor.gear_ratio = WEAPON_GEAR_RATIO;
    weapon_init(&weapon_motor);

    // 4. Датчик
    distanceSensor = {ULTRA_TRIG, ULTRA_ECHO, -1.0, 0};
    ultrasonic_init(&distanceSensor);

    // 5. WiFi
    wifi_init();

    // 6. Запуск фоновой задачи разгона
    motors_ptr[0] = &motorL;
    motors_ptr[1] = &motorR;
    xTaskCreatePinnedToCore(motor_control_task, "MotorTask", 4096, (void*)motors_ptr, 10, NULL, 1);
    
    lastUpdateTime = millis();
    DEBUG_PRINTLN("=== SYSTEM READY ===");
}

void loop() {
    uint32_t now = millis();

    // FAILSAFE ПРОВЕРКА (раз в 100мс)
    if (now - lastFailsafeCheck > 100) {
        lastFailsafeCheck = now;
        bool timeout = (now - lastUpdateTime > 2000); // 2 секунды без связи
        
        if (timeout && !isFailsafeActive) {
            isFailsafeActive = true;
            motor_set_speed(&motorL, 0);
            motor_set_speed(&motorR, 0);
            weapon_stop(&weapon_motor);
            DEBUG_PRINTLN("⚠️ FAILSAFE ON");
        } else if (!timeout && isFailsafeActive) {
            isFailsafeActive = false;
            DEBUG_PRINTLN("✅ FAILSAFE OFF");
        }
    }

    if (!isFailsafeActive) {
        // Применяем команды движения
        if (cmdChanged) {
            motor_set_speed(&motorL, cmdSpeedL);
            motor_set_speed(&motorR, cmdSpeedR);
            cmdChanged = false;
        }

        // Применяем команды серво
        if (cmdServoChanged) {
            servo_set_angle(&servoWeapon, cmdServoAngle);
            cmdServoChanged = false;
        }

        // Применяем команды катапульты
        if (cmdWeaponChanged) {
            if (cmdWeaponFire) {
                weapon_rotate_to_angle(&weapon_motor, (float)cmdWeaponAngle, cmdWeaponSpeed, 
                                       motor_get_load_percent(&motorL), motor_get_load_percent(&motorR));
                cmdWeaponFire = false;
            }
            cmdWeaponChanged = false;
        }
    }

    // Эти функции должны работать всегда (для плавного завершения или замера дистанции)
    weapon_update_rotation(&weapon_motor);

    if (now - lastUltraScan > 150) { // Опрос датчика раз в 150мс
        ultrasonic_start_measurement(&distanceSensor);
        ultrasonic_get_distance_cm(&distanceSensor); 
        lastUltraScan = now;
    }
    
    delay(1); // Даем время WiFi стеку
}