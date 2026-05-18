#include <Arduino.h>
#include "config.h"
#include "motor_control.h"
#include "servo_control.h"
#include "WiFi.h"
#include "wifi_handler.h"
#include "ultrasonic.h"
#include "weapon_system.h"

// ===== ГЛОБАЛЬНЫЕ ОБЪЕКТЫ УСТРОЙСТВ =====
Motor_t motorL;        // Левый борт
Motor_t motorR;        // Правый борт
Servo_t servoWeapon;   // Сервопривод катапульты
WeaponMotor_t weapon_motor;  // Мотор катапульты
Ultrasonic_t distanceSensor; // HC-SR04

// ===== ВНЕШНИЕ ПЕРЕМЕННЫЕ FAILSAFE =====
volatile uint32_t lastUpdateTime = 0;
bool isFailsafeActive = false;
volatile uint32_t lastUltrasonicTime = 0;
volatile uint32_t setupCompleteTime = 0;

// ===== АРХИТЕКТУРА "ФЛАГИ И ПЕРЕМЕННЫЕ" =====
// Эти переменные используются для асинхронной передачи команд из WiFi в loop
volatile int cmdSpeedL = 0;      // Целевая скорость левого мотора
volatile int cmdSpeedR = 0;      // Целевая скорость правого мотора
volatile bool cmdChanged = false; // Флаг: пришла новая команда для моторов

volatile int cmdServoAngle = 90;    // Целевой угол сервопривода
volatile bool cmdServoChanged = false; // Флаг для сервопривода

volatile int cmdWeaponSpeed = 0; // Целевая скорость катапульты
volatile int cmdWeaponAngle = 0; // Целевой угол катапульты
volatile bool cmdWeaponFire = false;  // Флаг: выстрелить
volatile bool cmdWeaponChanged = false; // Флаг для катапульты

// Вспомогательные переменные для failsafe
enum {
    STATE_IDLE,
    STATE_DRIVING,
    STATE_EMERGENCY
} currentState = STATE_IDLE;

// Константы failsafe
const uint32_t FAILSAFE_GRACE_PERIOD_MS = 3000;  // 3 сек после старта
const uint32_t FAILSAFE_TIMEOUT_MS = 3000;        // 3 сек без команд = failsafe

// === ГЛОБАЛЬНЫЙ МАССИВ МОТОРОВ ДЛЯ FreeRTOS ЗАДАЧИ ===
// КРИТИЧНО: Массив должен быть ГЛОБАЛЬНЫМ (не локальным в setup)
// иначе FreeRTOS задача будет обращаться к удалённой памяти и вызовет крах
Motor_t* motors[2];

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n\n=== ESP32 4WD Robot Initialization ===\n");

    // === Инициализация левого мотора ===
    motorL.pwm_pin = MOTOR_L_PWM;
    motorL.in1_pin = MOTOR_L_IN1;
    motorL.in2_pin = MOTOR_L_IN2;
    motorL.ledc_channel = LEDC_CH_L;
    motorL.target_speed = 0;
    motorL.current_pwm = 0;
    motorL.is_soft_starting = false;
    motor_init(&motorL);

    // === Инициализация правого мотора ===
    motorR.pwm_pin = MOTOR_R_PWM;
    motorR.in1_pin = MOTOR_R_IN1;
    motorR.in2_pin = MOTOR_R_IN2;
    motorR.ledc_channel = LEDC_CH_R;
    motorR.target_speed = 0;
    motorR.current_pwm = 0;
    motorR.is_soft_starting = false;
    motor_init(&motorR);

    // === Инициализация сервопривода ===
    servoWeapon.pin = SERVO_SIG;
    servoWeapon.ledc_channel = LEDC_CH_SERVO;
    servo_init(&servoWeapon);
    servo_set_angle(&servoWeapon, 90);  // Нейтральное положение

    // === Инициализация катапульты ===
    weapon_motor.en_pin = WEAPON_EN;
    weapon_motor.in1_pin = WEAPON_IN1;
    weapon_motor.in2_pin = WEAPON_IN2;
    weapon_motor.ledc_channel = LEDC_CH_WEAPON;
    weapon_motor.motor_rpm = WEAPON_MOTOR_RPM;
    weapon_motor.gear_ratio = WEAPON_GEAR_RATIO;
    weapon_init(&weapon_motor);

    // === Инициализация датчика расстояния ===
    distanceSensor.trig_pin = ULTRA_TRIG;
    distanceSensor.echo_pin = ULTRA_ECHO;
    ultrasonic_init(&distanceSensor);

    // === Инициализация WiFi ===
    wifi_init();

    // === Инициализация глобального массива моторов для FreeRTOS задачи ===
    motors[0] = &motorL;
    motors[1] = &motorR;

    // === Запуск FreeRTOS task для управления моторами ===
    xTaskCreatePinnedToCore(
        motor_control_task,
        "MotorTask",
        4096,
        (void*)motors,  // Теперь это глобальный массив, существует всегда!
        10,
        NULL,
        1  // Core 1
    );

    // === Инициализация таймеров failsafe ===
    lastUpdateTime = millis();
    lastUltrasonicTime = millis();
    setupCompleteTime = millis();

    Serial.println("\n✓ Robot Initialization Complete!\n");
    Serial.println("Waiting for WiFi commands...\n");
}

void checkFailsafe() {
    uint32_t currentTime = millis();
    
    // Не активируем failsafe во время grace period
    int32_t timeSinceSetup = (int32_t)(currentTime - setupCompleteTime);
    if (timeSinceSetup < FAILSAFE_GRACE_PERIOD_MS) {
        return;
    }
    
    int32_t timeSinceLastCommand = (int32_t)(currentTime - lastUpdateTime);
    
    // Если нет команд дольше FAILSAFE_TIMEOUT_MS -> аварийная остановка
    if (timeSinceLastCommand > FAILSAFE_TIMEOUT_MS && !isFailsafeActive) {
        isFailsafeActive = true;
        currentState = STATE_EMERGENCY;
        
        // Немедленно останавливаем моторы
        motor_set_speed(&motorL, 0);
        motor_set_speed(&motorR, 0);
        weapon_stop(&weapon_motor);
        
        Serial.println("\n" + String(50, '='));
        Serial.println("⚠️  FAILSAFE ACTIVATED - No WiFi command!");
        Serial.println(String(50, '=') + "\n");
    } else if (isFailsafeActive && timeSinceLastCommand <= FAILSAFE_TIMEOUT_MS) {
        isFailsafeActive = false;
        currentState = STATE_IDLE;
        Serial.println("[FAILSAFE] ✓ Connection restored!\n");
    }
}

/**
 * Main loop - обработка всех команд через флаги
 * 
 * Архитектура:
 * 1. Проверяем флаги, которые установила WiFi функция
 * 2. Применяем соответствующие команды к моторам/сервоприводу
 * 3. Обновляем состояние устройств
 * 4. WiFi функции НЕ вызывают motor_set_speed() напрямую!
 */
void loop() {
    // === FAILSAFE ПРОВЕРКА ===
    checkFailsafe();

    // === ОБРАБОТКА КОМАНД ХОДОВЫХ МОТОРОВ ===
    if (cmdChanged) {
        motor_set_speed(&motorL, cmdSpeedL);
        motor_set_speed(&motorR, cmdSpeedR);
        cmdChanged = false;
        currentState = (cmdSpeedL == 0 && cmdSpeedR == 0) ? STATE_IDLE : STATE_DRIVING;
    }

    // === ОБРАБОТКА КОМАНД СЕРВОПРИВОДА ===
    if (cmdServoChanged) {
        servo_set_angle(&servoWeapon, cmdServoAngle);
        cmdServoChanged = false;
    }

    // === ОБРАБОТКА КОМАНД КАТАПУЛЬТЫ ===
    if (cmdWeaponChanged) {
        if (cmdWeaponFire) {
            // Проверяем защиту от перегрузки
            uint8_t loadL = motor_get_load_percent(&motorL);
            uint8_t loadR = motor_get_load_percent(&motorR);
            
            bool fireOk = weapon_rotate_to_angle(&weapon_motor, (float)cmdWeaponAngle, 
                                                 cmdWeaponSpeed, loadL, loadR);
            if (!fireOk) {
                Serial.println("⚠️  Fire blocked: Motor load too high!");
            }
            cmdWeaponFire = false;
        }
        cmdWeaponChanged = false;
    }

    // === ОБНОВЛЕНИЕ СОСТОЯНИЯ КАТАПУЛЬТЫ ===
    weapon_update_rotation(&weapon_motor);

    // === ПЕРИОДИЧЕСКИЙ ОПРОС ДАТЧИКА РАССТОЯНИЯ (каждые 100 мс) ===
    uint32_t currentTime = millis();
    if (currentTime - lastUltrasonicTime >= 100) {
        ultrasonic_start_measurement(&distanceSensor);
        lastUltrasonicTime = currentTime;
    }

    // Очень небольшая задержка для предотвращения перегрузки CPU
    delay(5);
}