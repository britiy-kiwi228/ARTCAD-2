#ifndef CONFIG_H // Защита от повторного включения 
#define CONFIG_H

#include "driver/ledc.h"

// --- Пины для подключения ходовой части (L298N) ---
// Мотор 1 (Левый): ENA (PWM) = 32, INA = 25, INB = 26
#define MOTOR_L_PWM 32
#define MOTOR_L_IN1 25
#define MOTOR_L_IN2 26
// Мотор 2 (Правый): ENB (PWM) = 33, INC = 27, IND = 14
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
#define SERVO_RES  16          // 16 бит разрешение (0...65535)
// Рабочие параметры MG995 (в долях от разрешения 16-бит)
#define SERVO_MIN_DUTY 819     // ~0.5 мс (0 градусов)
#define SERVO_MAX_DUTY 3932    // ~2.4 мс (180 градусов)

// === ПРАВИЛЬНОЕ РАСПРЕДЕЛЕНИЕ ТАЙМЕРОВ ===
// Таймер 0 (HIGH SPEED @ 1000 Hz): Ходовые моторы (левый + правый) - LEDC_CH0, LEDC_CH1
// Таймер 1 (HIGH SPEED @ 1000 Hz): Катапульта - LEDC_CH2
// Таймер 2 (HIGH SPEED @ 50 Hz): Сервопривод - LEDC_CH3

// Параметры PWM для моторов
#define MOTOR_PWM_FREQ 1000    // 1000 Hz (Оптимально для L298N)
#define MOTOR_PWM_RES 8        // 8 бит разрешение (0-255)
#define MOTOR_TIMER LEDC_TIMER_0
#define MOTOR_SPEED_MODE LEDC_HIGH_SPEED_MODE

// LEDC каналы для ходовых моторов
#define LEDC_CH_L LEDC_CHANNEL_0  // Левый мотор
#define LEDC_CH_R LEDC_CHANNEL_1  // Правый мотор

// Параметры PWM для катапульты
#define WEAPON_PWM_FREQ 1000   // 1000 Hz
#define WEAPON_PWM_RES 8       // 8 бит
#define WEAPON_TIMER LEDC_TIMER_1
#define WEAPON_SPEED_MODE LEDC_HIGH_SPEED_MODE

// LEDC канал для катапульты
#define LEDC_CH_WEAPON LEDC_CHANNEL_2

// Параметры PWM для сервопривода
#define SERVO_PWM_FREQ 50      // 50 Hz (стандарт)
#define SERVO_PWM_RES 16       // 16 бит разрешение
#define SERVO_TIMER LEDC_TIMER_2
#define SERVO_SPEED_MODE LEDC_HIGH_SPEED_MODE

// LEDC канал для сервопривода
#define LEDC_CH_SERVO LEDC_CHANNEL_3

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