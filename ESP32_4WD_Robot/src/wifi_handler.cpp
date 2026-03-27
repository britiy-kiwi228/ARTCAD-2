#include "wifi_handler.h"
#include "secrets.h" // Файл с AP_SSID и AP_PASS
#include <Arduino.h>

// Создаем объект сервера на порту 80 (стандарт для HTTP)
AsyncWebServer server(80);
extern bool isFailsafeActive; // Флаг для отслеживания состояния Failsafe
extern volatile uint32_t lastUpdateTime; // Время последней полученной команды

void wifi_init() {
    // 1. Запуск точки доступа (Access Point)
    WiFi.softAP(AP_SSID, AP_PASS);

    Serial.println("Wi-Fi AP Started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP()); // Обычно это 192.168.4.1

    // 2. Описываем логику обработки запроса "/move"
    // Это называется "Handler" или "Route"
    server.on("/move", HTTP_GET, [](AsyncWebServerRequest *request) {
        isFailsafeActive = false; // Получили команду - отключаем Failsafe
        lastUpdateTime = millis(); // Обновляем время последней команды
        // Проверяем, прислал ли телефон параметры 'l' и 'r'
        if (request->hasParam("l") && request->hasParam("r")) {
            
            // Извлекаем значения из запроса и переводим из текста (String) в целое число (int)
            int valL = request->getParam("l")->value().toInt();
            int valR = request->getParam("r")->value().toInt();
            
            // Валидация параметров: скорость должна быть в диапазоне -255 до 255
            if (valL >= -255 && valL <= 255 && valR >= -255 && valR <= 255) {
                // Управляем моторами
                motor_set_speed(&motorL, valL);
                motor_set_speed(&motorR, valR);
            }
        }

        // Если прислали параметр 's' (servo)
        if (request->hasParam("s")) {
            int valS = request->getParam("s")->value().toInt();
            // Валидация параметра: угол должен быть в диапазоне 0-180 градусов
            if (valS >= 0 && valS <= 180) {
                servo_set_angle(&servoWeapon, valS);
            }
        }

        // Отправляем ответ телефону, что всё ок (код 200)
        request->send(200, "text/plain", "OK");
    });

    // 3. Запуск сервера
    server.begin();
}