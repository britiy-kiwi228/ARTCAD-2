#include "wifi_handler.h"
#include "secrets.h"
#include "weapon_system.h"
#include <Arduino.h>
#include <esp_wifi.h>

AsyncWebServer server(80);

// Ссылки на внешние переменные
extern volatile uint32_t lastUpdateTime;
extern Ultrasonic_t distanceSensor;
extern volatile int cmdSpeedL, cmdSpeedR;
extern volatile bool cmdChanged;
extern volatile int cmdServoAngle;
extern volatile bool cmdServoChanged;
extern volatile int cmdWeaponSpeed, cmdWeaponAngle;
extern volatile bool cmdWeaponFire, cmdWeaponChanged;

void wifi_init() {
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false); // Отключаем энергосбережение Wi-Fi
    WiFi.begin(AP_SSID, AP_PASS);
    
    // Ждем подключения без блокировки надолго
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 15) {
        delay(500);
        attempts++;
    }
    Serial.println(WiFi.localIP());
    esp_wifi_set_ps(WIFI_PS_NONE); 

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html);
    });

    server.on("/move", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Используем временные переменные, чтобы парсинг не блокировал волатильные переменные
        int tempSpd = 0;
        String tempBtn = "";

        if (request->hasParam("btn")) tempBtn = request->getParam("btn")->value();
        if (request->hasParam("speed")) tempSpd = request->getParam("speed")->value().toInt();

        // Применяем логику мгновенно
        if (tempBtn == "forward")  { cmdSpeedL = tempSpd; cmdSpeedR = tempSpd; }
        else if (tempBtn == "backward") { cmdSpeedL = -tempSpd; cmdSpeedR = -tempSpd; }
        else if (tempBtn == "left")     { cmdSpeedL = -160; cmdSpeedR = 160; }
        else if (tempBtn == "right")    { cmdSpeedL = 160; cmdSpeedR = -160; }
        else { cmdSpeedL = 0; cmdSpeedR = 0; }
    
        cmdChanged = true;
        lastUpdateTime = millis();
        request->send(204); 
    });

    server.on("/heartbeat", HTTP_GET, [](AsyncWebServerRequest *request) {
        lastUpdateTime = millis();
        request->send(200, "text/plain", "OK");
    });

    server.on("/distance", HTTP_GET, [](AsyncWebServerRequest *request) {
        // ПРОСТО отдаем последнее число, ничего не запуская!
        float d = distanceSensor.last_distance_cm;
        request->send(200, "application/json", "{\"distance\": " + String(d, 1) + "}");
    });

    server.on("/servo", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("angle")) {
            cmdServoAngle = request->getParam("angle")->value().toInt();
            cmdServoChanged = true;
        }
        request->send(204);
    });

    server.on("/fire", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("speed") && request->hasParam("angle")) {
            cmdWeaponSpeed = request->getParam("speed")->value().toInt();
            cmdWeaponAngle = request->getParam("angle")->value().toInt(); // ИСПРАВЛЕНО
            cmdWeaponFire = true;
            cmdWeaponChanged = true;
            lastUpdateTime = millis();
            request->send(200, "text/plain", "OK");
        } else {
            request->send(400, "text/plain", "Bad Request");
        }
    });

    server.begin();
}