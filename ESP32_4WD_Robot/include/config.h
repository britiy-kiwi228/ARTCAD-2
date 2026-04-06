#ifndef CONFIG_H // Защита от повторного включения 
#define CONFIG_H
// Пины для подключения левого мотора
#define MOTOR_L_PWM 32
#define MOTOR_L_IN1 25
#define MOTOR_L_IN2 26
// Пины для подключения правого мотора 
#define MOTOR_R_PWM 33
#define MOTOR_R_IN1 27
#define MOTOR_R_IN2 14

// --- Пины для двигателя системы вооружения (катапульта) ---
// Драйвер TC1508
#define WEAPON_INA 21  // Пин управления направлением A
#define WEAPON_INB 19  // Пин управления направлением B
#define WEAPON_PWM 4   // ШИМ пин для управления скоростью

// --- Настройки для Сервопривода ---
// Пин для подключения сервопривода
#define SERVO_SIG 15
#define SERVO_FREQ 50          // 50 Гц (стандарт для аналоговых серво)
#define SERVO_RES  10          // 10 бит разрешение (0...1023)
// Рабочие параметры MG995 (в долях от разрешения)
#define SERVO_MIN_DUTY 26      // ~0.5 мс (0 градусов)
#define SERVO_MAX_DUTY 123     // ~2.4 мс (180 градусов)

// Параметры для настройки ШИМ
#define PWM_FREQ 5000
#define PWM_RES 8 
// Каналы для ШИМ
#define LEDC_CH_L 0
#define LEDC_CH_R 1
#define LEDC_CH_SERVO 4  // Используем Timer 1 (избегаем конфликта с моторами на Timer 0)
#define LEDC_CH_WEAPON 5 // Канал для двигателя системы вооружения

// --- Параметры двигателя системы вооружения ---
#define WEAPON_MOTOR_RPM 399      // Обороты в минуту
#define WEAPON_GEAR_RATIO 1.0f    // Передаточное число (при необходимости отрегулировать)

// --- Пины для ультразвукового датчика HC-SR04 ---
#define ULTRA_TRIG 5   // Пин TRIGGER (запуск импульса)
#define ULTRA_ECHO 18  // Пин ECHO (прием сигнала, через делитель напряжения 5V → 3.3V)

#endif