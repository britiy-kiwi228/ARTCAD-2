#include "wifi_handler.h"
#include "secrets.h" // Файл с AP_SSID и AP_PASS
#include "weapon_system.h" // Управление катапультой
#include <Arduino.h>

// Создаем объект сервера на порту 80 (стандарт для HTTP)
AsyncWebServer server(80);
extern bool isFailsafeActive; // Флаг для отслеживания состояния Failsafe
extern volatile uint32_t lastUpdateTime; // Время последней полученной команды
extern WeaponMotor_t weapon_motor; // Доступ к структуре двигателя катапульты
extern Motor_t motorL; // Левый мотор (для проверки нагрузки)
extern Motor_t motorR; // Правый мотор (для проверки нагрузки)

void wifi_init() {
    // ===== ИСПРАВЛЕНИЕ: Переключение с Access Point на режим клиента (STA) =====
    // Вместо того чтобы создавать собственную WiFi сеть, ESP32 теперь подключается
    // к домашней сети (hollow512) как обычный клиент.
    // Преимущество: смартфон и робот находятся в одной сети, можно контролировать со смартфона
    
    Serial.println("\n===== WiFi Initialization =====");
    Serial.println("Connecting to WiFi network: " + String(AP_SSID));
    
    // Устанавливаем режим STA (Station/Client) вместо AP (Access Point)
    WiFi.mode(WIFI_STA);
    
    // Подключаемся к домашней WiFi сети с учеными данными из secrets.h
    WiFi.begin(AP_SSID, AP_PASS);
    
    // Ждем подключения с таймаутом 20 секунд (40 попыток по 500 мс)
    int attempts = 0;
    constexpr int MAX_ATTEMPTS = 40;
    
    while (WiFi.status() != WL_CONNECTED && attempts < MAX_ATTEMPTS) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    Serial.println(); // Новая строка после точек прогресса
    
    // Проверяем результат подключения
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n█████████████████████████████████████████");
        Serial.println("✓ WiFi Connected Successfully!");
        Serial.println("█████████████████████████████████████████");
        
        IPAddress localIP = WiFi.localIP();
        Serial.println("\n>>> ROBOT IP ADDRESS <<<");
        Serial.print("    http://");
        Serial.print(localIP);
        Serial.println("/");
        Serial.println("\nOpen this in your browser on smartphone:");
        Serial.print("    http://");
        Serial.print(localIP);
        Serial.println("/distance");
        
        Serial.println("\nNetwork Details:");
        Serial.print("  Local IP:  ");
        Serial.println(localIP);
        Serial.print("  Gateway:   ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("  MAC:       ");
        Serial.println(WiFi.macAddress());
        Serial.println("█████████████████████████████████████████\n");
    } else {
        Serial.println("\n✗ WiFi Connection Failed!");
        Serial.println("Please check SSID and password in secrets.h");
        Serial.println("Continuing without WiFi...\n");
    }

    // ===== ВЕХОВОЕ МЕНЮ ВЕБА: Endpoints (маршруты) =====
    
    // 1. Обработчик корневого пути "/" - возвращает HTML интерфейс
    // Когда ты заходишь на http://[IP_РОБОТА]/ в браузер смартфона
    // Клиент получит красивую HTML страницу управления роботом
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Отправляем HTML страницу из PROGMEM с типом Content-Type: text/html
        request->send_P(200, "text/html", index_html);
    });

    // 2. Endpoint "/move" - команда управления моторами и сервом через кнопки
    // Пример запроса: http://192.168.X.X/move?btn=forward&speed=128
    // btn - направление: forward, backward, left, right, stop
    // speed - скорость моторов (0-255)
    // s - угол сервопривода оружия (0...180) - опционально
    server.on("/move", HTTP_GET, [](AsyncWebServerRequest *request) {
        // ===== КРИТИЧНО: Обновляем lastUpdateTime В ПЕРВУЮ ОЧЕРЕДЬ =====
        // Это должно быть ПЕРВОЕ действие при получении команды!
        // Защищаем от race condition отключением прерываний
        portDISABLE_INTERRUPTS();
        lastUpdateTime = millis();
        isFailsafeActive = false;  // Сбрасываем флаг Failsafe (робот на связи!)
        portENABLE_INTERRUPTS();
        
        // Проверяем, прислал ли смартфон параметр 'btn' (направление движения)
        if (request->hasParam("btn")) {
            String btn = request->getParam("btn")->value();
            
            // Получаем скорость из параметра (по умолчанию 128 если не указана)
            int speed = 128;
            if (request->hasParam("speed")) {
                speed = request->getParam("speed")->value().toInt();
                // Ограничиваем диапазон
                if (speed < 0) speed = 0;
                if (speed > 255) speed = 255;
            }
            
            // Логика управления движением робота со скоростью из ползунка
            if (btn == "forward") {
                // Вперед: оба колеса крутятся вперед с заданной скоростью
                motor_set_speed(&motorL, speed);
                motor_set_speed(&motorR, speed);
            }
            else if (btn == "backward") {
                // Назад: оба колеса крутятся назад с заданной скоростью
                motor_set_speed(&motorL, -speed);
                motor_set_speed(&motorR, -speed);
            }
            else if (btn == "left") {
                // Влево: правый вперед на полной скорости, левый помогает подворачиванию (30%)
                int turnSpeed = (speed * 30) / 100;  // 30% от заданной скорости
                motor_set_speed(&motorL, turnSpeed);     
                motor_set_speed(&motorR, speed);       
            }
            else if (btn == "right") {
                // Вправо: левый вперед на полной скорости, правый помогает подворачиванию (30%)
                int turnSpeed = (speed * 30) / 100;  // 30% от заданной скорости
                motor_set_speed(&motorL, speed);       
                motor_set_speed(&motorR, turnSpeed);     
            }
            else if (btn == "stop") {
                // Стоп: оба колеса останавливаются
                motor_set_speed(&motorL, 0);
                motor_set_speed(&motorR, 0);
            }
        }

        // Если прислали параметр 's' (servo angle - угол сервопривода)
        if (request->hasParam("s")) {
            int valS = request->getParam("s")->value().toInt();
            // Валидация параметра: угол должен быть в диапазоне 0-180 градусов
            if (valS >= 0 && valS <= 180) {
                servo_set_angle(&servoWeapon, valS);
            }
        }

        // Отправляем ответ смартфону, что всё ОК (код HTTP 200)
        request->send(200, "text/plain", "OK");
    });

    // 3. Endpoint "/heartbeat" - пустой запрос для обновления таймера failsafe
    // Отправляется каждые 100мс из JavaScript, чтобы failsafe не срабатывал
    // Это критично когда команды ставятся в очередь (throttle) и не отправляются
    server.on("/heartbeat", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Просто обновляем время последней команды
        portDISABLE_INTERRUPTS();
        lastUpdateTime = millis();
        isFailsafeActive = false;
        portENABLE_INTERRUPTS();
        
        // Отправляем простой ответ ОК
        request->send(200, "text/plain", "OK");
    });

    // 4. Endpoint "/distance" - получить текущее расстояние от датчика
    // Пример: GET http://192.168.X.X/distance
    // Ответ: {"distance": 45.3, "status": "ok"} или {"distance": -1.0, "status": "timeout"}
    server.on("/distance", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Получаем текущее расстояние из ультразвукового датчика
        float distance = ultrasonic_get_distance_cm(&distanceSensor);
        
        // Формируем JSON ответ вручную (без внешних библиотек)
        String response = "{\"distance\": ";
        response += distance;
        
        // Добавляем статус: OK или TIMEOUT
        if (distance < 0) {
            response += ", \"status\": \"timeout\"}";
        } else {
            response += ", \"status\": \"ok\"}";
        }
        
        // Отправляем JSON ответ с корректным типом Content-Type
        request->send(200, "application/json", response);
    });

    // 5. Endpoint "/fire" - команда для системы вооружения (выстрел катапульты)
    // Пример запроса: http://192.168.X.X/fire?w_speed=200&w_angle=45
    // w_speed - скорость мотора (1-255)
    // w_angle - угол поворота (0-360 градусов, обычно 45)
    server.on("/fire", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Обновляем таймер failsafe (робот получил команду)
        portDISABLE_INTERRUPTS();
        lastUpdateTime = millis();
        isFailsafeActive = false;
        portENABLE_INTERRUPTS();
        
        // Проверяем, прислали ли параметры скорости и угла
        if (request->hasParam("w_speed") && request->hasParam("w_angle")) {
            int weaponSpeed = request->getParam("w_speed")->value().toInt();
            int weaponAngle = request->getParam("w_angle")->value().toInt();
            
            // Валидация скорости (1...255)
            if (weaponSpeed > 0 && weaponSpeed <= 255) {
                // Валидация угла (0...360)
                if (weaponAngle >= 0 && weaponAngle <= 360) {
                    // ===== ЛОГИКА ВЫСТРЕЛА С ЗАЩИТОЙ =====
                    // Получаем текущую нагрузку ходовых моторов (%)
                    uint8_t loadLeft = motor_get_load_percent(&motorL);
                    uint8_t loadRight = motor_get_load_percent(&motorR);
                    
                    // Пытаемся выстрелить (функция проверит защиту)
                    bool fireSuccess = weapon_rotate_to_angle(&weapon_motor, (float)weaponAngle, weaponSpeed,
                                                              loadLeft, loadRight);
                    
                    if (fireSuccess) {
                        // Выстрел успешно инициирован
                        request->send(200, "text/plain", "FIRE");
                    } else {
                        // Выстрел заблокирован защитой (высокая нагрузка на моторы)
                        request->send(409, "text/plain", "BLOCKED_HIGH_LOAD");
                    }
                } else {
                    request->send(400, "text/plain", "Invalid angle (0-360)");
                }
            } else {
                request->send(400, "text/plain", "Invalid speed (1-255)");
            }
        } else {
            request->send(400, "text/plain", "Missing w_speed or w_angle parameter");
        }
    });

    // 6. Запуск асинхронного веб-сервера на порту 80
    // Сервер будет слушать входящие HTTP запросы
    // Это не блокирует выполнение loop() - работает асинхронно!
    server.begin();
}