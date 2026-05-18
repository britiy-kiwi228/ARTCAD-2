#include "wifi_handler.h"
#include "secrets.h"
#include "weapon_system.h"
#include <Arduino.h>
#include <esp_wifi.h>

AsyncWebServer server(80);

extern bool isFailsafeActive;
extern volatile uint32_t lastUpdateTime;
extern WeaponMotor_t weapon_motor;
extern Motor_t motorL;
extern Motor_t motorR;

extern volatile int cmdSpeedL;
extern volatile int cmdSpeedR;
extern volatile bool cmdChanged;
extern volatile int cmdServoAngle;
extern volatile bool cmdServoChanged;
extern volatile int cmdWeaponSpeed;
extern volatile int cmdWeaponAngle; // Добавлено
extern volatile bool cmdWeaponFire;
extern volatile bool cmdWeaponChanged;

void wifi_init() {
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false); // КРИТИЧНО для мгновенного отклика
    WiFi.begin(AP_SSID, AP_PASS);
    
    Serial.print("Connecting to WiFi");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✓ Connected! IP: " + WiFi.localIP().toString());
    }

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html);
    });

    server.on("/move", HTTP_GET, [](AsyncWebServerRequest *request) {
        lastUpdateTime = millis();
        if (request->hasParam("btn")) {
            String btn = request->getParam("btn")->value();
            int speed = request->hasParam("speed") ? request->getParam("speed")->value().toInt() : 150;
            
            if (btn == "forward") { cmdSpeedL = speed; cmdSpeedR = speed; }
            else if (btn == "backward") { cmdSpeedL = -speed; cmdSpeedR = -speed; }
            else if (btn == "left") { cmdSpeedL = -MOTOR_TURN_SPEED; cmdSpeedR = MOTOR_TURN_SPEED; }
            else if (btn == "right") { cmdSpeedL = MOTOR_TURN_SPEED; cmdSpeedR = -MOTOR_TURN_SPEED; }
            else { cmdSpeedL = 0; cmdSpeedR = 0; }
            cmdChanged = true;
        }
        request->send(200, "text/plain", "OK");
    });

    server.on("/heartbeat", HTTP_GET, [](AsyncWebServerRequest *request) {
        lastUpdateTime = millis();
        request->send(200, "text/plain", "OK");
    });

    server.on("/distance", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Мы не запускаем измерение здесь, оно идет в loop()
        // Здесь только отдаем ПОСЛЕДНЕЕ готовое значение
        float dist = distanceSensor.last_distance_cm;
        String response = "{\"distance\": " + String(dist, 1) + ", \"status\": \"ok\"}";
        request->send(200, "application/json", response);
    });

    server.on("/servo", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("angle")) {
            cmdServoAngle = request->getParam("angle")->value().toInt();
            cmdServoChanged = true;
        }
        request->send(200, "text/plain", "OK");
    });

    server.on("/fire", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("speed") && request->hasParam("angle")) {
            cmdWeaponSpeed = request->getParam("speed")->value().toInt();
            cmdWeaponAngle = request->getParam("angle")->value().toInt(); // Исправлено!
            cmdWeaponFire = true;
            cmdWeaponChanged = true;
            lastUpdateTime = millis();
        }
        request->send(200, "text/plain", "OK");
    });

    server.begin();
}