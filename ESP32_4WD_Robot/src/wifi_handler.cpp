#include "wifi_handler.h"
#include "secrets.h" // Файл с AP_SSID и AP_PASS
#include <Arduino.h>

// Создаем объект сервера на порту 80 (стандарт для HTTP)
AsyncWebServer server(80);

void wifi_init() {
    // 1. Запуск точки доступа (Access Point)
    WiFi.softAP(AP_SSID, AP_PASS);

    Serial.println("Wi-Fi AP Started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP()); // Обычно это 192.168.4.1

    // 2. Описываем логику обработки запроса "/move"
    // Это называется "Handler" или "Route"
    server.on("/move", HTTP_GET, [](AsyncWebServerRequest *request) {
        
        // Проверяем, прислал ли телефон параметры 'l' и 'r'
        if (request->hasParam("l") && request->hasParam("r")) {
            
            // Извлекаем значения из запроса и переводим из текста (String) в целое число (int)
            int valL = request->getParam("l")->value().toInt();
            int valR = request->getParam("r")->value().toInt();
            
            // Управляем моторами
            motor_set_speed(&motorL, valL);
            motor_set_speed(&motorR, valR);
            
            // ОБЯЗАТЕЛЬНО обновляем время последней команды для Failsafe
            lastUpdateTime = millis();
        }

        // Если прислали параметр 's' (servo)
        if (request->hasParam("s")) {
            int valS = request->getParam("s")->value().toInt();
            servo_set_angle(&servoWeapon, valS);
            lastUpdateTime = millis();
        }

        // Отправляем ответ телефону, что всё ок (код 200)
        request->send(200, "text/plain", "OK");
    });

    // 3. Запуск сервера
    server.begin();
}