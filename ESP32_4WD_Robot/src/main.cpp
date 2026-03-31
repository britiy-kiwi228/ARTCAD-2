#include <Arduino.h>
#include "config.h"
#include "motor_control.h"
#include "WiFi.h"
#include "wifi_handler.h"
#include "ultrasonic.h"
// --- 1. Создание объектов моторов ---
// Мы выделяем память под две структуры типа Motor_t.
// Эти переменные глобальные, чтобы они были доступны во всём файле.

Motor_t motorL; // Левый борт
Motor_t motorR; // Правый борт
Servo_t servoWeapon; 
Ultrasonic_t distanceSensor; // Датчик ультразвукового расстояния HC-SR04
volatile uint32_t lastUpdateTime = 0; // Время последней полученной команды (для Failsafe)
bool isFailsafeActive = false; // Флаг для отслеживания состояния Failsafe
volatile uint32_t lastUltrasonicTime = 0; // Время последнего измерения расстояния (для таймера 100 мс)
volatile uint32_t setupCompleteTime = 0; // Время завершения инициализации (для grace period failsafe)
typedef enum{
    STATE_IDLE, // Робот стоит на месте
    STATE_DRIVING, // Робот движется
    STATE_EMERGENCY // Аварийное состояние 
} RobotState_t;
RobotState_t currentState = STATE_IDLE; // Начальное состояние робота
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
    servoWeapon.pin = SERVO_SIG;  // Исправлено: использовать 'pin' вместо 'sig_pin'
    servoWeapon.ledc_channel = LEDC_CH_SERVO;



    // --- 5. Физическая инициализация ---
    // Вызываем нашу функцию, которая настроит пины и ШИМ внутри ESP32
    motor_init(&motorL);
    motor_init(&motorR);
    servo_init(&servoWeapon);
    ultrasonic_init(&distanceSensor);
    wifi_init();

    // Для отладки откроем последовательный порт (монитор порта)
    Serial.println("Robot 4WD Initialized!");
    
    lastUpdateTime = millis(); // Инициализируем время последней команды
    lastUltrasonicTime = millis(); // Инициализируем таймер для ультразвукового датчика
    
    // ===== ИСПРАВЛЕНИЕ: Grace period для Failsafe =====
    // Даем 3 секунды на полную инициализацию WiFi и веб-сервера
    // Без этого failsafe может срабатуть до того, как первая команда придет
    setupCompleteTime = millis();
    Serial.println("\n[FAILSAFE] Grace period: 3 seconds for WiFi initialization...\n");
}


/**
 * Функция проверки безопасности (FAILSAFE).
 * Если связь потеряна (команд нет > 500 мс), останавливаем моторы.
 * 
 * ИСПРАВЛЕНИЯ:
 *   - Grace period (3 секунды после старта): failsafe не проверяется, пока WiFi инициализируется
 *   - Увеличен таймаут до 750 мс (вместо 500 мс) чтобы избежать ложных срабатываний
 *   - Добавлена отладка для диагностики проблем
 */
void checkFailsafe() {
    uint32_t currentTime = millis();
    
    // ===== GRACE PERIOD: Первые 3 секунды после инициализации =====
    // Во время инициализации WiFi и веб-сервера могут быть задержки
    // Не проверяем failsafe, пока это не завершится
    uint32_t timeSinceSetup = currentTime - setupCompleteTime;
    if (timeSinceSetup < 3000) {
        // Еще в grace period - не проверяем failsafe
        return;
    }
    
    // === ОСНОВНАЯ ПРОВЕРКА FAILSAFE ===
    // Проверяем разницу между "сейчас" и "последней командой"
    // Используем безопасное сравнение для защиты от переполнения millis()
    uint32_t timeSinceLastCommand = (uint32_t)(currentTime - lastUpdateTime);
    
    // УВЕЛИЧИЛИ до 1000 мс (вместо 750 мс) для дополнительной подстраховки
    // Heartbeat отправляется каждые 100мс из браузера, поэтому даже при throttle
    // таймер будет обновляться минимум каждые 100мс
    const uint32_t FAILSAFE_TIMEOUT = 1000;  // миллисекунды
    
    if (timeSinceLastCommand > FAILSAFE_TIMEOUT && !isFailsafeActive) {
        // ===== FAILSAFE АКТИВИРОВАЛСЯ =====
        isFailsafeActive = true;
        currentState = STATE_EMERGENCY;
        
        // Останавливаем моторы немедленно
        motor_set_speed(&motorL, 0);
        motor_set_speed(&motorR, 0);
        
        // Отладочный вывод
        Serial.println("\n" + String(50, '='));
        Serial.println("⚠️  FAILSAFE ACTIVATED!");
        Serial.print("Current time: ");
        Serial.println(currentTime);
        Serial.print("Last command time: ");
        Serial.println(lastUpdateTime);
        Serial.print("Time since last command: ");
        Serial.print(timeSinceLastCommand);
        Serial.println(" ms");
        Serial.println("Motors stopped due to lost connection.");
        Serial.println(String(50, '=') + "\n");
    } else if (isFailsafeActive && timeSinceLastCommand <= FAILSAFE_TIMEOUT) {
        // ===== FAILSAFE ДЕАКТИВИРОВАЛСЯ (восстановлена связь) =====
        isFailsafeActive = false;
        currentState = STATE_IDLE;
        Serial.println("[FAILSAFE] ✓ Connection restored! Failsafe deactivated.\n");
    }
}

void loop() {
    checkFailsafe(); // Проверяем безопасность в каждом цикле
    
    // ===== НЕБЛОКИРУЮЩИЙ ТАЙМЕР ДЛЯ УЛЬТРАЗВУКОВОГО ДАТЧИКА =====
    // Запускаем новое измерение расстояния каждые 100 миллисекунд
    // Это дает нам частоту измерения ~10 раз в секунду
    // Результаты отправляются на веб-страницу через API /distance
    uint32_t currentTime = millis();
    if ((uint32_t)(currentTime - lastUltrasonicTime) >= 100) {
        // Прошло 100 мс или больше - пора запустить новое измерение
        ultrasonic_start_measurement(&distanceSensor);
        lastUltrasonicTime = currentTime;
        
        // Получить результат последнего завершенного измерения
        // НЕ выводим в Serial Monitor - только отправляем в браузер
        float distance = ultrasonic_get_distance_cm(&distanceSensor);
        // Данные автоматически доступны через API /distance для веб-интерфейса
        (void)distance; // Убираем warning о неиспользованной переменной
    }
    
    switch (currentState) {
        case STATE_IDLE:
            // В режиме ожидания можно выполнять какие-то фоновые задачи
            break;
        case STATE_DRIVING:
            // Здесь будет чето будет 
            break;
        case STATE_EMERGENCY:
            // Здесь будет логика для аварийного состояния
            break;
    }
    
}