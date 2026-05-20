#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <WiFi.h>
#include <ESPAsyncWebServer.h> // библиотека для асинхронного сервера
#include "motor_control.h" // для управления моторами
#include "servo_control.h" // для управления сервоприводом
#include "ultrasonic.h" // для датчика расстояния
#include "web_interface.h" // web-интерфейс управления роботом
#include "secrets.h"

// объявления внешних переменных 
extern Motor_t motorL;
extern Motor_t motorR;
extern Servo_t servoWeapon;
extern Ultrasonic_t distanceSensor;
extern volatile uint32_t lastUpdateTime;

// Прототипы функций
void wifi_init();
void wifi_broadcast_distance(float distance); // Отправка телеметрии

#endif // WIFI_HANDLER_H