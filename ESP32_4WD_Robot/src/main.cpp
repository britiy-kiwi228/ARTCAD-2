#include <Arduino.h>
#include "config.h"
#include "motor_control.h"
#include "servo_control.h"
#include "wifi_handler.h"
#include "ultrasonic.h"
#include "weapon_system.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"


Motor_t motorL, motorR;
Servo_t servoWeapon;
WeaponMotor_t weapon_motor;
Ultrasonic_t distanceSensor;

volatile int cmdSpeedL = 0, cmdSpeedR = 0;
volatile bool cmdChanged = false;
volatile int cmdServoAngle = 90;
volatile bool cmdServoChanged = false;
volatile int cmdWeaponSpeed = 0, cmdWeaponAngle = 0;
volatile bool cmdWeaponFire = false, cmdWeaponChanged = false;

volatile uint32_t lastUpdateTime = 0;
uint32_t lastUltraScan = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n=== ROBOT POWER ON ===");

    // Моторы
    motorL.pwm_pin = MOTOR_L_PWM; motorL.in1_pin = MOTOR_L_IN1; motorL.in2_pin = MOTOR_L_IN2;
    motorL.ledc_channel = LEDC_CH_L; motor_init(&motorL);

    motorR.pwm_pin = MOTOR_R_PWM; motorR.in1_pin = MOTOR_R_IN1; motorR.in2_pin = MOTOR_R_IN2;
    motorR.ledc_channel = LEDC_CH_R; motor_init(&motorR);

    // Оружие
    weapon_motor.en_pin = WEAPON_EN; weapon_motor.in1_pin = WEAPON_IN1; weapon_motor.in2_pin = WEAPON_IN2;
    weapon_motor.ledc_channel = LEDC_CH_WEAPON; weapon_motor.motor_rpm = WEAPON_MOTOR_RPM;
    weapon_motor.gear_ratio = WEAPON_GEAR_RATIO;
    weapon_init(&weapon_motor);

    // Серво и Датчик
    servoWeapon.pin = SERVO_SIG; servoWeapon.ledc_channel = LEDC_CH_SERVO;
    servo_init(&servoWeapon);
    distanceSensor.trig_pin = ULTRA_TRIG; distanceSensor.echo_pin = ULTRA_ECHO;
    ultrasonic_init(&distanceSensor);

    wifi_init();
    lastUpdateTime = millis();
    Serial.println("--- SYSTEM READY TO MOVE ---");
}

void loop() {
    static uint32_t lastRefresh = 0;
    uint32_t now = millis();
    if (now - lastUpdateTime > 1500) { // Если нет запросов от веб-интерфейса больше 1.5 сек
        motor_set_speed(&motorL, 0);
        motor_set_speed(&motorR, 0);
    }

    
    if (cmdChanged) {
        // Создаем локальную копию ПЕРЕД использованием
        // Это гарантирует, что пока мотор крутится, WiFi не изменит значение в середине такта
        noInterrupts(); 
        int localL = cmdSpeedL;
        int localR = cmdSpeedR;
        cmdChanged = false;
        interrupts();

        motor_set_speed(&motorL, localL);
        motor_set_speed(&motorR, localR);
    }
    motor_update_soft_start(&motorL);
    motor_update_soft_start(&motorR);


    if (cmdServoChanged) {
        servo_set_angle(&servoWeapon, cmdServoAngle);
        cmdServoChanged = false;
    }

    if (cmdWeaponChanged) {
        if (cmdWeaponFire) {
            
            weapon_rotate_to_angle(&weapon_motor, (float)cmdWeaponAngle, cmdWeaponSpeed, 0, 0);
            cmdWeaponFire = false;
        }   
        cmdWeaponChanged = false;
    }
    if (weapon_motor.is_rotating) {
        weapon_update_rotation(&weapon_motor);
    }
    

    if (now - lastUltraScan > 300) {
        ultrasonic_start_measurement(&distanceSensor);
        
        // Считываем расстояние и сразу транслируем его по WebSocket всем подключенным клиентам
        float d = ultrasonic_get_distance_cm(&distanceSensor);
        wifi_broadcast_distance(d); 
        
        lastUltraScan = now;
    }
    ultrasonic_get_distance_cm(&distanceSensor);
    
    yield(); 
}