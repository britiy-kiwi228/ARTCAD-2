#include "ultrasonic.h"
#include "config.h"
#include <Arduino.h>

// ===== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ДЛЯ ISR =====
// Эти переменные ДОЛЖНЫ быть глобальными, чтобы ISR (прерывание) могло их изменять
// volatile гарантирует, что компилятор не будет оптимизировать доступ к ним

volatile uint32_t echo_start_time_us = 0;     // Время (в микросекундах) когда пришел rising edge на ECHO
volatile uint32_t last_pulse_duration_us = 0; // Длительность импульса (разница между falling и rising)
volatile bool measurement_complete = false;   // Флаг: измерение завершено, можно читать результат

/**
 * INTERRUPT SERVICE ROUTINE (ISR) - самая критичная часть
 * 
 * Эта функция вызывается ОЧЕНЬ быстро каждый раз когда меняется уровень на пине ECHO.
 * ВАЖНО:
 *   - ISR должна быть максимально короткой (не вызывать Serial.print, delay и т.д.)
 *   - ISR работает с ОЧЕНЬ высокой точностью (микросекунды!)
 *   - Используем IRAM_ATTR чтобы функция была в быстрой оперативной памяти
 * 
 * Логика:
 *   1. Если это rising edge (уровень поднялся): запомнить текущее время
 *   2. Если это falling edge (уровень упал): вычислить разницу времени - это длительность импульса
 */
void IRAM_ATTR ultrasonic_isr() {
    static uint32_t start_time = 0;
    
    // Читаем текущее время в микросекундах
    uint32_t current_time = micros();
    
    // Проверяем уровень на пине ECHO
    if (digitalRead(ULTRA_ECHO) == HIGH) {
        // Это RISING EDGE - звук начал возвращаться
        start_time = current_time;
    } else {
        // Это FALLING EDGE - звук закончил возвращаться
        // Вычисляем длительность импульса (как долго звук шел туда и обратно)
        last_pulse_duration_us = current_time - start_time;
        measurement_complete = true;
    }
}

/**
 * Инициализация датчика:
 *   1. Устанавливаем режимы GPIO пинов
 *   2. Подключаем функцию ISR к пину ECHO
 *   3. ISR будет срабатывать на любое изменение уровня (CHANGE)
 */
void ultrasonic_init(Ultrasonic_t *sensor) {
    // Проверка валидации
    if (sensor == NULL) {
        return;
    }
    
    // TRIGGER должен быть ВЫХОДОМ (мы отправляем импульс)
    pinMode(sensor->trig_pin, OUTPUT);
    digitalWrite(sensor->trig_pin, LOW);  // Начинаем с низкого уровня
    
    // ECHO должен быть ВХОДОМ (мы принимаем сигнал)
    pinMode(sensor->echo_pin, INPUT);
    
    // Подключаем прерывание на пин ECHO
    // При ЛЮБОМ изменении уровня (LOW→HIGH или HIGH→LOW) будет вызвана функция ultrasonic_isr()
    attachInterrupt(digitalPinToInterrupt(sensor->echo_pin), ultrasonic_isr, CHANGE);
    
    // Инициализируем начальные значения
    sensor->last_distance_cm = -1.0f;
    sensor->pulse_start_us = 0;
}

/**
 * Запуск одного измерения:
 *   1. Отправляем импульс 10 микросекунд на пин TRIGGER
 *   2. Датчик сам "поймет" что пора мерить и выдаст сигнал на ECHO
 *   3. ISR автоматически обработает ECHO без блокировки основного loop'а
 */
void ultrasonic_start_measurement(Ultrasonic_t *sensor) {
    if (sensor == NULL) {
        return;
    }
    
    // Генерируем импульс запуска:
    // Поднимаем сигнал на TRIGGER
    digitalWrite(sensor->trig_pin, HIGH);
    
    // Удерживаем высокий уровень ровно 10 микросекунд
    // delayMicroseconds - это неблокирующая функция, работает в микросекундах
    delayMicroseconds(10);
    
    // Опускаем сигнал обратно
    digitalWrite(sensor->trig_pin, LOW);
    
    // Датчик теперь знает что нужно отправить ультразвуковой импульс
    // ECHO пин начнет изменяться в течение микросекунд (не миллисекунд!)
    // Наша ISR скоро поймет это и рассчитает расстояние
}

/**
 * Получить текущее расстояние:
 *   1. Проверяем был ли статус "измерение завершено"
 *   2. Если да - конвертируем время импульса в расстояние
 *   3. Если данные невалидны (таймаут) - возвращаем -1.0 (ошибка)
 */
float ultrasonic_get_distance_cm(Ultrasonic_t *sensor) {
    if (sensor == NULL) {
        return -1.0f;
    }
    
    // Проверяем есть ли свежие данные от ISR
    if (measurement_complete) {
        // Отключаем прерывания рко читаем volatile переменные
        // (чтобы ISR не изменил их в середине нашего чтения)
        noInterrupts();
        
        uint32_t pulse_duration = last_pulse_duration_us;
        measurement_complete = false;  // Сбрасываем флаг
        
        // Включаем прерывания обратно
        interrupts();
        
        // ===== ВАЖНАЯ ФОРМУЛА =====
        // Скорость звука: 343 м/с = 0.0343 см/мкс
        // Импульс прошел туда И обратно (2 раза), поэтому:
        // расстояние = (время_микросекунд * 0.0343) / 2
        //            = время_микросекунд / 58.05 (упрощённая формула)
        // 
        // Диапазон датчика: примерно 2 см - 400 см
        // Таймаут: если время > 30000 мкс (~5 метров), считаем ошибкой
        
        // Проверка таймаута (никакого сигнала или очень дальний объект)
        if (pulse_duration == 0 || pulse_duration > 30000) {
            sensor->last_distance_cm = -1.0f;
            return -1.0f;
        }
        
        // Конвертируем время в расстояние
        float distance = pulse_duration / 58.0f;
        
        // Еще одна проверка: если расстояние слишком маленькое
        if (distance < 2.0f || distance > 400.0f) {
            sensor->last_distance_cm = -1.0f;
            return -1.0f;
        }
        
        // Сохраняем результат и возвращаем
        sensor->last_distance_cm = distance;
        return distance;
    }
    
    // Если нет свежих данных - возвращаем последнее известное значение
    return sensor->last_distance_cm;
}
