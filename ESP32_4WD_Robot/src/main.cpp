#include <Arduino.h>
#include "config.h"
#include "motor_control.h"
#include "WiFi.h"
#include "wifi_handler.h"
// --- 1. Создание объектов моторов ---
// Мы выделяем память под две структуры типа Motor_t.
// Эти переменные глобальные, чтобы они были доступны во всём файле.

Motor_t motorL; // Левый борт
Motor_t motorR; // Правый борт
Servo_t servoWeapon; 
volatile uint32_t lastUpdateTime = 0; // Время последней полученной команды (для Failsafe)
void setup() {
    Serial.begin(115200);
    
    // --- 2. Заполнение данных левого мотора ---
    motorL.pwm_pin = MOTOR_L_PWM;
    motorL.in1_pin = MOTOR_L_IN1;
    motorL.in2_pin = MOTOR_L_IN2;
    motorL.ledc_channel = LEDC_CH_L;

    // --- 3. Заполнение данных правого мотора ---
    motorR.pwm_pin = MOTOR_R_PWM;
    motorR.in1_pin = MOTOR_R_IN1;
    motorR.in2_pin = MOTOR_R_IN2;
    motorR.ledc_channel = LEDC_CH_R;
    // --- 4. Заполнение данных сервопривода ---
    servoWeapon.sig_pin = SERVO_SIG;
    servoWeapon.ledc_channel = LEDC_CH_SERVO;



    // --- 5. Физическая инициализация ---
    // Вызываем нашу функцию, которая настроит пины и ШИМ внутри ESP32
    motor_init(&motorL);
    motor_init(&motorR);
    servo_init(&servoWeapon);
    wifi_init();

    // Для отладки откроем последовательный порт (монитор порта)
    Serial.println("Robot 4WD Initialized!");
    
    lastUpdateTime = millis(); // Инициализируем время последней команды
}


/**
 * Функция проверки безопасности.
 * Если связь потеряна (команд нет > 500 мс), останавливаем моторы.
 */
void checkFailsafe() {
    uint32_t currentTime = millis(); // Берем текущее время
    
    // Проверяем разницу между "сейчас" и "последней командой"
    if (currentTime - lastUpdateTime > 500) {
        // Вызываем нашу функцию со скоростью 0
        motor_set_speed(&motorL, 0);
        motor_set_speed(&motorR, 0);
        Serial.println("Failsafe activated: Motors stopped due to lost connection."); //вывод в консоль 

    }
}

void loop() {
    // Пока оставим пустым, здесь будет Wi-Fi и FSM
}