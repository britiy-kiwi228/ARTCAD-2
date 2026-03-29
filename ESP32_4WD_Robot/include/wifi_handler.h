#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <WiFi.h>
#include <ESPAsyncWebServer.h> // библиотека для асинхронного сервера
#include "motor_control.h" // для управления моторами
#include "servo_control.h" // для управления сервоприводом
#include "web_interface.h" // web-интерфейс управления роботом
// Параметры сети
#include "secrets.h"
// объявления экстерных переменных 
extern Motor_t motorL;
extern Motor_t motorR;
extern Servo_t servoWeapon;
extern volatile uint32_t lastUpdateTime;
extern bool isFailsafeActive;

// Прототипы функций
void wifi_init();

#endif