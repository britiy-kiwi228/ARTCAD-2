#ifndef WEAPON_SYSTEM_H // Защита от повторного включения
#define WEAPON_SYSTEM_H

#include "config.h"    // Включаем конфигурационный файл для доступа к пинам и параметрам
#include <stdint.h>    // Типы данных (uint8_t, uint32_t)
#include <stdbool.h>   // Булевы типы (true/false)

/**
 * Структура для управления двигателем системы вооружения (катапульта).
 * Инкапсулирует все параметры и состояние двигателя TC1508.
 */
typedef struct {
    uint8_t ina_pin;           // Пин управления направлением A (GPIO 21)
    uint8_t inb_pin;           // Пин управления направлением B (GPIO 19)
    uint8_t pwm_pin;           // Пин ШИМ (GPIO 4)
    uint8_t ledc_channel;      // Канал ШИМ (LEDC_CH_WEAPON = 5)
    
    // Параметры двигателя и его состояния
    float motor_rpm;           // Обороты в минуту (399 RPM)
    float gear_ratio;          // Передаточное число
    
    // Переменные состояния для неблокирующего управления
    int current_speed;         // Текущая установленная скорость (-255 до 255)
    uint32_t rotation_start_ms; // Время начала вращения (milli-секунды)
    float target_time_ms;      // Целевое время вращения для достижения угла (мс)
    bool is_rotating;          // Флаг: выполняется ли вращение на угол
    
} WeaponMotor_t;

#ifndef WEAPON_SYSTEM_H // Защита от повторного включения
#define WEAPON_SYSTEM_H

#include "config.h"    // Включаем конфигурационный файл для доступа к пинам и параметрам
#include <stdint.h>    // Типы данных (uint8_t, uint32_t)
#include <stdbool.h>   // Булевы типы (true/false)

// === ПАРАМЕТРЫ ЗАЩИТЫ TC1508 ===
#define WEAPON_MAX_PWM 200          // Максимальное ШИМ значение (вместо 255) - защита от перегрева
                                     // ~78% = 12.6В * 0.78 = 9.8В (ниже порога перегрева TC1508)
#define WEAPON_MOTOR_LOAD_THRESHOLD 50  // % загрузки ходовых моторов, выше которого блокируем катапульту
#define WEAPON_SOFT_START_TIME_MS 100   // Время плавного разгона катапульты

/**
 * Структура для управления двигателем системы вооружения (катапульта).
 * Инкапсулирует все параметры и состояние двигателя TC1508.
 */
typedef struct {
    uint8_t ina_pin;           // Пин управления направлением A (GPIO 21)
    uint8_t inb_pin;           // Пин управления направлением B (GPIO 19)
    uint8_t pwm_pin;           // Пин ШИМ (GPIO 4)
    uint8_t ledc_channel;      // Канал ШИМ (LEDC_CH_WEAPON = 5)
    
    // Параметры двигателя и его состояния
    float motor_rpm;           // Обороты в минуту (399 RPM)
    float gear_ratio;          // Передаточное число
    
    // Переменные состояния для неблокирующего управления
    int current_speed;         // Текущая установленная скорость (-255 до 255)
    int current_pwm;           // Текущий ШИМ (может быть ограничен WEAPON_MAX_PWM)
    uint32_t rotation_start_ms; // Время начала вращения (milli-секунды)
    float target_time_ms;      // Целевое время вращения для достижения угла (мс)
    bool is_rotating;          // Флаг: выполняется ли вращение на угол
    
} WeaponMotor_t;

/**
 * Инициализация системы вооружения.
 * Настраивает пины GPIO, каналы ШИМ (LEDC) и приводит двигатель в состояние покоя.
 * 
 * @param weapon Указатель на структуру WeaponMotor_t
 */
void weapon_init(WeaponMotor_t* weapon);

/**
 * Установка скорости двигателя системы вооружения С ЗАЩИТОЙ от перегрева.
 * 
 * @param weapon Указатель на структуру WeaponMotor_t
 * @param speed Скорость от -255 (максимум в обратном направлении) 
 *              до 255 (максимум в прямом направлении)
 *              0 - полная остановка
 * 
 * ЗАЩИТА: ШИМ-значение ограничено WEAPON_MAX_PWM (~200), чтобы не перегреть TC1508
 */
void weapon_set_speed(WeaponMotor_t* weapon, int speed);

/**
 * Остановка двигателя (скорость = 0).
 * 
 * @param weapon Указатель на структуру WeaponMotor_t
 */
void weapon_stop(WeaponMotor_t* weapon);

/**
 * Инициирование вращения двигателя на заранее рассчитанный угол.
 * С ЗАЩИТОЙ: проверяет нагрузку на ходовые моторы.
 * Если нагрузка ходовых моторов > WEAPON_MOTOR_LOAD_THRESHOLD → возвращает false
 * 
 * @param weapon Указатель на структуру WeaponMotor_t
 * @param target_angle Целевой угол поворота в градусах (0-360)
 * @param speed Скорость вращения (-255 до 255)
 * @param motor_left_load Текущая нагрузка левого мотора (0-100%)
 * @param motor_right_load Текущая нагрузка правого мотора (0-100%)
 * @return true если вращение успешно инициировано, false если заблокировано протекцией
 */
bool weapon_rotate_to_angle(WeaponMotor_t* weapon, float target_angle, int speed, 
                            uint8_t motor_left_load, uint8_t motor_right_load);

/**
 * Обновление состояния вращения на угол.
 * Должна вызываться в основном цикле для отслеживания завершения вращения.
 * 
 * ВАЖНО: Если функция вернёт true, двигатель будет остановлен автоматически.
 * 
 * @param weapon Указатель на структуру WeaponMotor_t
 * @return true если вращение завершено, false если ещё выполняется
 */
bool weapon_update_rotation(WeaponMotor_t* weapon);

/**
 * Получить текущее состояние вращения.
 * 
 * @param weapon Указатель на структуру WeaponMotor_t
 * @return true если двигатель в процессе вращения на угол, false иначе
 */
bool weapon_is_rotating(WeaponMotor_t* weapon);

/**
 * Получить текущий ШИМ (после ограничений защиты).
 * 
 * @param weapon Указатель на структуру WeaponMotor_t
 * @return Текущое ШИМ значение (0-WEAPON_MAX_PWM)
 */
int weapon_get_current_pwm(WeaponMotor_t* weapon);

#endif // конец WEAPON_SYSTEM_H

#endif // конец WEAPON_SYSTEM_H
