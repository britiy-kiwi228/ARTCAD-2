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

#endif