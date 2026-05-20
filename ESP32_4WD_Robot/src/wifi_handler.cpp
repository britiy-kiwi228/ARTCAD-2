#include "wifi_handler.h"
#include "secrets.h"
#include "weapon_system.h"
#include <Arduino.h>
#include <esp_wifi.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Ссылки на внешние переменные
extern volatile uint32_t lastUpdateTime;
extern Ultrasonic_t distanceSensor;
extern volatile int cmdSpeedL, cmdSpeedR;
extern volatile bool cmdChanged;
extern volatile int cmdServoAngle;
extern volatile bool cmdServoChanged;
extern volatile int cmdWeaponSpeed, cmdWeaponAngle;
extern volatile bool cmdWeaponFire, cmdWeaponChanged;

// Функция быстрой отправки телеметрии расстояния всем активным клиентам по WebSocket
void wifi_broadcast_distance(float distance) {
    if (ws.count() > 0) { // Только если есть подключенные клиенты
        char buf[16];
        snprintf(buf, sizeof(buf), "D:%.1f", distance);
        ws.textAll(buf);
    }
}

// Эффективный парсинг входящих сообщений
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0; // Гарантируем закрытие строки null-терминатором
        char* msg = (char*)data;
        lastUpdateTime = millis(); // Любое входящее сообщение является хартбитом

        // Делим строку по разделителю ":"
        char* type = strtok(msg, ":");
        if (type == NULL) return;

        if (strcmp(type, "M") == 0) { 
            // Команда движения: "M:направление:скорость"
            char* dir = strtok(NULL, ":");
            char* speed_str = strtok(NULL, ":");
            if (dir != NULL && speed_str != NULL) {
                int sliderVal = atoi(speed_str);
                int spd = (sliderVal < 50) ? 50 : sliderVal;
                
                if (strcmp(dir, "forward") == 0) { 
                    cmdSpeedL = spd; cmdSpeedR = spd; 
                } else if (strcmp(dir, "backward") == 0) { 
                    cmdSpeedL = -spd; cmdSpeedR = -spd; 
                } else if (strcmp(dir, "left") == 0) { 
                    cmdSpeedL = -spd; cmdSpeedR = spd; 
                } else if (strcmp(dir, "right") == 0) { 
                    cmdSpeedL = spd; cmdSpeedR = -spd; 
                } else { 
                    cmdSpeedL = 0; cmdSpeedR = 0; 
                }
                cmdChanged = true;
            }
        } 
        else if (strcmp(type, "S") == 0) { 
            // Команда сервопривода: "S:угол"
            char* angle_str = strtok(NULL, ":");
            if (angle_str != NULL) {
                cmdServoAngle = atoi(angle_str);
                cmdServoChanged = true;
            }
        } 
        else if (strcmp(type, "W") == 0) { 
            // Команда оружия: "W:скорость:угол"
            char* speed_str = strtok(NULL, ":");
            char* angle_str = strtok(NULL, ":");
            if (speed_str != NULL && angle_str != NULL) {
                cmdWeaponSpeed = atoi(speed_str);
                cmdWeaponAngle = atoi(angle_str);
                cmdWeaponFire = true;
                cmdWeaponChanged = true;
            }
        }
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("[WS] Клиент #%u подключен с IP: %s\n", client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("[WS] Клиент #%u отключен\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void wifi_init() {
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false); // Отключаем энергосбережение Wi-Fi для снижения сетевых задержек
    WiFi.begin(AP_SSID, AP_PASS);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 15) {
        delay(500);
        attempts++;
    }
    Serial.println(WiFi.localIP());
    esp_wifi_set_ps(WIFI_PS_NONE); 

    // Регистрация WebSocket в сервере
    ws.onEvent(onEvent);
    server.addHandler(&ws);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html);
    });

    server.begin();
}