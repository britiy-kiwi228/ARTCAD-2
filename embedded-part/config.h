#ifndef CONFIG_H // Защита от повторного включения 
#define CONFIG_H
// Пины для подключения левого мотора
#define MOTOR_L_PWM 32
#define MOTOR_L_IN1 25
#define MOTOR_L_IN2 26
// Пины для подключения правого мотора 
#define MOTOR_R_PWM 33
#define MOTOR_R_IN1 27
#define MOTOR_R_IN2 14

// Пин для подключения сервопривода
#define SERVO_SIG 15
// Параметры для настройки ШИМ
#define PWM_FREQ 5000
#define PWM_RES 8 
// Каналы для ШИМ
#define LEDC_CH_L 0
#define LEDC_CH_R 1
#define LEDC_CH_SERVO 2

#endif