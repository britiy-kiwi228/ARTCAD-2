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
// Драйвер L298N
#define WEAPON_EN  21  // EN1 - пин Enable для управления скоростью (ШИМ)
#define WEAPON_IN1 19  // IN1 - пин направления 1
#define WEAPON_IN2 17  // IN2 - пин направления 2

// --- Настройки для Сервопривода ---
// Пин для подключения сервопривода
#define SERVO_SIG 15
#define SERVO_FREQ 50          // 50 Гц (стандарт для аналоговых серво)
#define SERVO_RES  10          // 10 бит разрешение (0...1023)
// Рабочие параметры MG995 (в долях от разрешения 10-бит)
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
#define WEAPON_MOTOR_RPM 205           // Обороты в минуту (JGA25-370B)
#define WEAPON_GEAR_RATIO 1.0f         // Передаточное число (при необходимости отрегулировать)
#define WEAPON_MAX_PWM 200             // Максимум ШИМ для защиты L298N от перегрева
#define WEAPON_MOTOR_LOAD_THRESHOLD 50 // Порог нагрузки (%) для блокировки выстрела
#define WEAPON_ROTATION_ANGLE 45.0f    // Угол поворота для выстрела (градусы)
#define WEAPON_ROTATION_SPEED 200      // Скорость мотора для поворота

// --- Параметры движения робота ---
#define MOTOR_MAX_SPEED 255              // Максимальная скорость вперед/назад
#define MOTOR_TURN_SPEED 150             // Скорость медленного вращения при повороте (ниже максимальной)
#define MOTOR_TURN_ASSIST 76             // 30% от MAX_SPEED для плавного поворота (255*0.3=76.5)

// --- Пины для ультразвукового датчика HC-SR04 ---
#define ULTRA_TRIG 5   // Пин TRIGGER (запуск импульса)
#define ULTRA_ECHO 18  // Пин ECHO (прием сигнала, через делитель напряжения 5V → 3.3V)

// --- Пины для кнопок управления ---
#define BTN_FORWARD  12  // Кнопка вперед
#define BTN_BACKWARD 13  // Кнопка назад
#define BTN_LEFT     4   // Кнопка влево
#define BTN_RIGHT    2   // Кнопка вправо

#endif