#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <WiFi.h>
#include <ESPAsyncWebServer.h> // библиотека для асинхронного сервера

// Параметры сети
#include "secrets.h"

// Прототипы функций
void wifi_init();

#endif