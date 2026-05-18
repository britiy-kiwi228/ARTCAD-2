#include "wifi_handler.h"
#include "secrets.h"
#include "weapon_system.h"
#include <Arduino.h>
#include <esp_wifi.h>

// Создаем объект сервера на порту 80
AsyncWebServer server(80);

// ===== ВНЕШНИЕ ПЕРЕМЕННЫЕ (определены в main.cpp) =====
extern bool isFailsafeActive;
extern volatile uint32_t lastUpdateTime;
extern WeaponMotor_t weapon_motor;
extern Motor_t motorL;
extern Motor_t motorR;

// ===== АРХИТЕКТУРА "ФЛАГИ И ПЕРЕМЕННЫЕ" =====
// Эти переменные используются для передачи команд из WiFi обработчиков в main loop.
// Обработчики ТОЛЬКО меняют эти флаги, НЕ вызывают функции напрямую!
extern volatile int cmdSpeedL;    // Целевая скорость левого мотора
extern volatile int cmdSpeedR;    // Целевая скорость правого мотора
extern volatile bool cmdChanged;  // Флаг: пришла новая команда

// Флаги для сервопривода и катапульты
extern volatile int cmdServoAngle;    // Целевой угол сервопривода (0-180)
extern volatile bool cmdServoChanged; // Флаг: пришлась новая команда для серво

extern volatile int cmdWeaponSpeed;   // Целевая скорость катапульты
extern volatile bool cmdWeaponFire;   // Флаг: выстрелить
extern volatile bool cmdWeaponChanged;// Флаг: пришлась новая команда для катапульты

void wifi_init() {
    Serial.println("\n===== WiFi Initialization (STA Mode) =====");
    Serial.println("SSID: " + String(AP_SSID));
    
    // Устанавливаем режим STA (Station/Client)
    WiFi.mode(WIFI_STA);
    
    // КРИТИЧНО: Отключаем sleep режим WiFi для стабильности
    WiFi.setSleep(false);
    
    // Снижаем мощность передатчика для снижения помех на GPIO 32/33
    WiFi.setTxPower(WIFI_POWER_11dBm);
    
    // Подключаемся к домашней сети
    WiFi.begin(AP_SSID, AP_PASS);
    
    // Ждем подключения (максимум 20 секунд)
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✓ WiFi Connected!");
        IPAddress ip = WiFi.localIP();
        Serial.print("IP: http://");
        Serial.println(ip);
    } else {
        Serial.println("\n✗ WiFi Connection Failed");
    }

    // ===== ЭНДПОИНТЫ =====
    
    // GET / - HTML интерфейс
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html);
    });

    // GET /move - команды движения (флаги вместо прямых вызовов!)
    // Параметры: btn=forward|backward|left|right|stop&speed=0-255
    server.on("/move", HTTP_GET, [](AsyncWebServerRequest *request) {
        lastUpdateTime = millis();
        isFailsafeActive = false;

        if (request->hasParam("btn")) {
            String btn = request->getParam("btn")->value();
            int speed = request->hasParam("speed") ? request->getParam("speed")->value().toInt() : 128;
            speed = constrain(speed, 0, 255);

            if (btn == "forward") {
                cmdSpeedL = speed;
                cmdSpeedR = speed;
            } else if (btn == "backward") {
                cmdSpeedL = -speed;
                cmdSpeedR = -speed;
            } else if (btn == "left") {
                cmdSpeedL = speed / 3;
                cmdSpeedR = speed;
            } else if (btn == "right") {
                cmdSpeedL = speed;
                cmdSpeedR = speed / 3;
            } else if (btn == "stop") {
                cmdSpeedL = 0;
                cmdSpeedR = 0;
            }
            
            cmdChanged = true;  // Флаг для main loop
        }
        request->send(200, "text/plain", "OK");
    });

    // GET /heartbeat - сигнал жизни (предотвращает failsafe)
    server.on("/heartbeat", HTTP_GET, [](AsyncWebServerRequest *request) {
        lastUpdateTime = millis();
        isFailsafeActive = false;
        request->send(200, "text/plain", "OK");
    });

    // GET /distance - текущее расстояние от датчика
    server.on("/distance", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Запускаем измерение расстояния
        ultrasonic_start_measurement(&distanceSensor);
        float distance = ultrasonic_get_distance_cm(&distanceSensor);
        
        String status = (distance > 0) ? "ok" : "timeout";
        String response = "{\"distance\": " + String(distance, 1) + ", \"status\": \"" + status + "\"}";
        
        request->send(200, "application/json", response);
    });

    // GET /servo - команда для сервопривода
    // Параметры: angle=0-180
    server.on("/servo", HTTP_GET, [](AsyncWebServerRequest *request) {
        lastUpdateTime = millis();
        isFailsafeActive = false;

        if (request->hasParam("angle")) {
            int angle = request->getParam("angle")->value().toInt();
            cmdServoAngle = constrain(angle, 0, 180);
            cmdServoChanged = true;
        }
        request->send(200, "text/plain", "OK");
    });

    // GET /fire - команда выстрела катапультой (ставит флаг, не вызывает функции!)
    // Параметры: speed=0-255&angle=0-360
    server.on("/fire", HTTP_GET, [](AsyncWebServerRequest *request) {
        lastUpdateTime = millis();
        isFailsafeActive = false;

        if (request->hasParam("speed") && request->hasParam("angle")) {
            int speed = request->getParam("speed")->value().toInt();
            int angle = request->getParam("angle")->value().toInt();
            
            cmdWeaponSpeed = constrain(speed, 0, 255);
            cmdWeaponSpeed = constrain(angle, 0, 360);
            cmdWeaponFire = true;
            cmdWeaponChanged = true;
        }
        request->send(200, "text/plain", "OK");
    });

    // GET /status - статус робота (JSON)
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"motorL_load\": " + String(motor_get_load_percent(&motorL));
        json += ", \"motorR_load\": " + String(motor_get_load_percent(&motorR));
        json += ", \"weapon_speed\": " + String(weapon_motor.current_speed);
        json += ", \"wifi_signal\": " + String(WiFi.RSSI());
        json += "}";
        request->send(200, "application/json", json);
    });

    // Запуск сервера
    server.begin();
    Serial.println("Web server started on port 80");
}