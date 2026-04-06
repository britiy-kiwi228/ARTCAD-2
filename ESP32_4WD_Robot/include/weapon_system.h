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

/**
 * Инициализация системы вооружения.
 * Настраивает пины GPIO, каналы ШИМ (LEDC) и приводит двигатель в состояние покоя.
 * 
 * @param weapon Указатель на структуру WeaponMotor_t
 */
void weapon_init(WeaponMotor_t* weapon);

/**
 * Установка скорости двигателя системы вооружения.
 * 
 * @param weapon Указатель на структуру WeaponMotor_t
 * @param speed Скорость от -255 (максимум в обратном направлении) 
 *              до 255 (максимум в прямом направлении)
 *              0 - полная остановка
 * 
 * Логика управления TC1508:
 * - speed > 0:  INA=1, INB=0 (вращение в прямом направлении)
 * - speed < 0:  INA=0, INB=1 (вращение в обратном направлении)
 * - speed = 0:  INA=0, INB=0 (полная остановка)
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
 * 
 * Функция рассчитывает необходимое время вращения на основе:
 * - Скорости двигателя (399 RPM)
 * - Передаточного числа
 * - Требуемого угла поворота
 * 
 * Управление неблокирующее: функция только запускает таймер и не дожидается завершения.
 * 
 * @param weapon Указатель на структуру WeaponMotor_t
 * @param target_angle Целевой угол поворота в градусах (0-360)
 * @param speed Скорость вращения (-255 до 255)
 */
void weapon_rotate_to_angle(WeaponMotor_t* weapon, float target_angle, int speed);

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

#endif // конец WEAPON_SYSTEM_H
