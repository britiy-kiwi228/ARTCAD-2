#include "weapon_system.h"
#include <Arduino.h>           // Базовые функции Arduino (pinMode, digitalWrite, millis)
#include <driver/ledc.h>       // Драйвер LEDC для ШИМ
#include <stdlib.h>            // Функция abs() для работы с модулем число

/**
 * Инициализация системы вооружения (драйвер TC1508 + двигатель).
 * 
 * Этапы:
 * 1. Сохранение параметров двигателя в структуру
 * 2. Настройка пинов направления как выходы (INA, INB)
 * 3. Настройка канала ШИМ (LEDC) для PWM пина
 * 4. Приведение двигателя в состояние покоя
 */
void weapon_init(WeaponMotor_t* weapon) {
    // Шаг 1: Убедимся, что структура инициализирована корректно
    if (!weapon) {
        return;
    }

    // Сохраняем параметры из конфига (если они не установлены извне)
    if (weapon->motor_rpm == 0) {
        weapon->motor_rpm = WEAPON_MOTOR_RPM;
    }
    if (weapon->gear_ratio == 0) {
        weapon->gear_ratio = WEAPON_GEAR_RATIO;
    }
    
    // Инициализируем переменные состояния
    weapon->current_speed = 0;
    weapon->current_pwm = 0;  // ШИМ начинается с 0
    weapon->is_rotating = false;
    weapon->rotation_start_ms = 0;
    weapon->target_time_ms = 0.0f;

    // Шаг 2: Настройка пинов управления направлением как выходы
    pinMode(weapon->ina_pin, OUTPUT);
    pinMode(weapon->inb_pin, OUTPUT);
    
    // Приводим их в состояние LOW (двигатель не включен)
    digitalWrite(weapon->ina_pin, LOW);
    digitalWrite(weapon->inb_pin, LOW);

    // Шаг 3: Настройка ШИМ через LEDC
    // ledcSetup(канал, частота_в_Гц, разрешение_в_битах)
    // PWM_FREQ = 5000 Гц, PWM_RES = 8 бит (значения 0-255)
    ledcSetup(weapon->ledc_channel, PWM_FREQ, PWM_RES);
    
    // Привязываем физический пин PWM к настроенному каналу LEDC
    ledcAttachPin(weapon->pwm_pin, weapon->ledc_channel);
    
    // Устанавливаем начальное значение ШИМ на 0 (двигатель не вращается)
    ledcWrite(weapon->ledc_channel, 0);
}

/**
 * Установка скорости вращения двигателя.
 * 
 * Управление двигателем через драйвер TC1508:
 * - INA и INB являются логическими входами управления направлением
 * - PWM контролирует скорость (мощность)
 * 
 * Логика:
 *   speed > 0:  INA=HIGH, INB=LOW  → вращение в прямом направлении
 *   speed < 0:  INA=LOW,  INB=HIGH → вращение в обратном направлении
 *   speed = 0:  INA=LOW,  INB=LOW  → полная остановка / торможение
 */
void weapon_set_speed(WeaponMotor_t* weapon, int speed) {
    if (!weapon) {
        return;
    }

    // Ограничиваем скорость в диапазон [-255, 255]
    if (speed > 255)  speed = 255;
    if (speed < -255) speed = -255;

    // Сохраняем текущую скорость
    weapon->current_speed = speed;

    // Управление направлением через INA/INB логику
    if (speed > 0) {
        // Вращение вперед
        digitalWrite(weapon->ina_pin, HIGH);
        digitalWrite(weapon->inb_pin, LOW);
    } 
    else if (speed < 0) {
        // Вращение назад
        digitalWrite(weapon->ina_pin, LOW);
        digitalWrite(weapon->inb_pin, HIGH);
    } 
    else {
        // Полная остановка
        digitalWrite(weapon->ina_pin, LOW);
        digitalWrite(weapon->inb_pin, LOW);
    }

    // === ЗАЩИТА: ШИМ-ЛИМИТИРОВАНИЕ ===
    // Максимум WEAPON_MAX_PWM (200 вместо 255) для защиты TC1508 от перегрева
    // 200/255 = ~78% = 12.6В * 0.78 = 9.8В (ниже порога перегрева TC1508)
    int pwm_value = abs(speed);
    if (pwm_value > WEAPON_MAX_PWM) {
        pwm_value = WEAPON_MAX_PWM;
    }
    
    weapon->current_pwm = pwm_value;
    ledcWrite(weapon->ledc_channel, pwm_value);
}

/**
 * Остановка двигателя (безопасная остановка).
 */
void weapon_stop(WeaponMotor_t* weapon) {
    if (!weapon) {
        return;
    }
    weapon_set_speed(weapon, 0);
    weapon->is_rotating = false;
}

/**
 * Инициирование вращения на заданный угол С ЗАЩИТОЙ от перегрузки.
 * 
 * ЗАЩИТА: Проверяет нагрузку ходовых моторов.
 * Если нагрузка > WEAPON_MOTOR_LOAD_THRESHOLD (50%) → блокирует выстрел
 */
bool weapon_rotate_to_angle(WeaponMotor_t* weapon, float target_angle, int speed,
                            uint8_t motor_left_load, uint8_t motor_right_load) {
    if (!weapon) {
        return false;
    }

    // === ЗАЩИТА: ПРОВЕРКА НАГРУЗКИ ХОДОВЫХ МОТОРОВ ===
    // Если оба мотора работают на мощности > 50% → блокируем катапульту
    // Это защищает предохранитель (макс 7.5А) от перегрузки
    uint8_t max_motor_load = (motor_left_load > motor_right_load) ? motor_left_load : motor_right_load;
    
    if (max_motor_load > WEAPON_MOTOR_LOAD_THRESHOLD) {
        // Нагрузка слишком высока - заблокирован для безопасности
        Serial.printf("[WEAPON BLOCKED] Motor load too high: L=%d%%, R=%d%% (max: %d%%)\n", 
                      motor_left_load, motor_right_load, WEAPON_MOTOR_LOAD_THRESHOLD);
        return false;  // Выстрел заблокирован
    }

    // Нормализуем угол к диапазону [0, 360)
    while (target_angle >= 360.0f) {
        target_angle -= 360.0f;
    }
    while (target_angle < 0.0f) {
        target_angle += 360.0f;
    }

    // Рассчитываем время для полного оборота (360°) в миллисекундах
    // T_360_ms = 60000 / RPM
    float time_for_360_ms = 60000.0f / weapon->motor_rpm;

    // Рассчитываем время для требуемого угла
    // T_angle_ms = (angle / 360) * T_360
    float required_time_ms = (target_angle / 360.0f) * time_for_360_ms;

    // Если есть передаточное число (например, > 1.0), оно замедляет вращение
    // Увеличиваем требуемое время
    required_time_ms = required_time_ms / weapon->gear_ratio;

    // Сохраняем параметры вращения
    weapon->target_time_ms = required_time_ms;
    weapon->rotation_start_ms = millis();  // Запоминаем текущее время в мс
    weaЛогируем успешное начало выстрела
    Serial.printf("[WEAPON FIRE] Angle: %.1f°, Speed: %d, Load: L=%d%% R=%d%%\n",
                  target_angle, speed, motor_left_load, motor_right_load);

    // Запускаем двигатель с указанной скоростью
    weapon_set_speed(weapon, speed);
    return true;  // Выстрел успешно инициирован
    // Запускаем двигатель с указанной скоростью
    weapon_set_speed(weapon, speed);
}

/**
 * Обновление состояния вращения.
 * Должна вызываться регулярно (примерно каждые 10-50 мс) в основном loop().
 * 
 * Логика:
 * 1. Проверяем, активно ли вращение на угол
 * 2. Если прошло достаточно времени — останавливаем двигатель и возвращаем true
 * 3. Иначе возвращаем false
 * 
 * @return true если вращение завершено (двигатель остановлен)
 */
bool weapon_update_rotation(WeaponMotor_t* weapon) {
    if (!weapon || !weapon->is_rotating) {
        return false;
    }

    // Получаем прошедшее время с момента начала вращения
    uint32_t elapsed_ms = millis() - weapon->rotation_start_ms;

    // Проверяем, прошло ли требуемое время
    if (elapsed_ms >= weapon->target_time_ms) {
        // Вращение завершено
        weapon_stop(weapon);
        weapon->is_rotating = false;
        return true;
    }

    // Вращение еще продолжается
    return false;
}

/**
 * Получить текущее состояние вращения.
 * 
 * @return true если двигатель в процессе вращения на угол, false иначе
 */
b

/**
 * Получить текущий ШИМ (после ограничений защиты).
 * 
 * @return Текущое ШИМ значение (0-WEAPON_MAX_PWM)
 */
int weapon_get_current_pwm(WeaponMotor_t* weapon) {
    if (!weapon) {
        return 0;
    }
    return weapon->current_pwm;
}ool weapon_is_rotating(WeaponMotor_t* weapon) {
    if (!weapon) {
        return false;
    }
    return weapon->is_rotating;
}
