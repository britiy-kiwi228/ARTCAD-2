#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

// HTML/CSS/JS код интерфейса управления роботом
// Хранится в PROGMEM для экономии оперативной памяти ESP32
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>4WD Robot Control</title>
    <style>
        /* === ГЛОБАЛЬНЫЕ СТИЛИ === */
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        /* Темная тема: dark colorable для промышленного стиля */
        body {
            background: linear-gradient(135deg, #1a1a1a 0%, #2d2d2d 100%);
            color: #e0e0e0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 10px;
        }

        /* Основной контейнер */
        .container {
            width: 100%;
            max-width: 500px;
            background: #1a1a1a;
            border-radius: 15px;
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.5), 0 0 20px rgba(0, 255, 100, 0.2);
            border: 1px solid #00ff64;
            padding: 20px;
        }

        /* Заголовок */
        h1 {
            text-align: center;
            font-size: 28px;
            margin-bottom: 20px;
            color: #00ff64;
            text-shadow: 0 0 10px rgba(0, 255, 100, 0.3);
        }

        /* === D-PAD (КРЕСТОВИНА УПРАВЛЕНИЯ) === */
        .dpad-section {
            display: flex;
            justify-content: center;
            margin-bottom: 30px;
        }

        .dpad {
            position: relative;
            width: 200px;
            height: 200px;
        }

        /* Кнопки D-pad */
        .dpad-btn {
            position: absolute;
            width: 60px;
            height: 60px;
            border: 2px solid #00ff64;
            background: #0f3d1f;
            color: #00ff64;
            font-size: 24px;
            font-weight: bold;
            border-radius: 8px;
            cursor: pointer;
            transition: all 0.1s ease;
            user-select: none;
            display: flex;
            align-items: center;
            justify-content: center;
        }

        /* Вперед */
        .dpad-btn.up {
            top: 0;
            left: 50%;
            transform: translateX(-50%);
        }

        /* Назад */
        .dpad-btn.down {
            bottom: 0;
            left: 50%;
            transform: translateX(-50%);
        }

        /* Влево */
        .dpad-btn.left {
            left: 0;
            top: 50%;
            transform: translateY(-50%);
        }

        /* Вправо */
        .dpad-btn.right {
            right: 0;
            top: 50%;
            transform: translateY(-50%);
        }

        /* Эффект нажатия кнопки */
        .dpad-btn:active,
        .dpad-btn.pressed {
            background: #00ff64;
            color: #000;
            box-shadow: 0 0 15px rgba(0, 255, 100, 0.8);
            transform: translate(-50%, -50%) scale(0.95);
        }

        .dpad-btn.left:active,
        .dpad-btn.left.pressed {
            transform: translateY(-50%) scale(0.95);
        }

        .dpad-btn.right:active,
        .dpad-btn.right.pressed {
            transform: translateY(-50%) scale(0.95);
        }

        .dpad-btn.down:active,
        .dpad-btn.down.pressed {
            transform: translateX(-50%) scale(0.95);
        }

        /* === СЛАЙДЕРЫ === */
        .slider-section {
            margin-bottom: 25px;
        }

        .slider-group {
            margin-bottom: 20px;
        }

        .slider-label {
            display: flex;
            justify-content: space-between;
            margin-bottom: 8px;
            font-size: 14px;
            color: #b0b0b0;
        }

        .slider-label span {
            font-weight: 600;
            color: #00ff64;
        }

        /* Кастомизация input[type="range"] */
        input[type="range"] {
            width: 100%;
            height: 6px;
            -webkit-appearance: none;
            appearance: none;
            background: #333;
            border-radius: 3px;
            outline: none;
            cursor: pointer;
        }

        /* Ползунок (thumb) для Webkit браузеров (Chrome, Safari) */
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 18px;
            height: 18px;
            border-radius: 50%;
            background: #00ff64;
            cursor: pointer;
            box-shadow: 0 0 10px rgba(0, 255, 100, 0.6);
        }

        /* Ползунок для Firefox */
        input[type="range"]::-moz-range-thumb {
            width: 18px;
            height: 18px;
            border-radius: 50%;
            background: #00ff64;
            cursor: pointer;
            border: none;
            box-shadow: 0 0 10px rgba(0, 255, 100, 0.6);
        }

        /* === КНОПКИ === */
        .button-section {
            display: flex;
            gap: 10px;
            margin-bottom: 20px;
        }

        button {
            flex: 1;
            padding: 12px 20px;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s ease;
        }

        /* Кнопка СТОП (ярко-красная) */
        .btn-stop {
            background: linear-gradient(135deg, #ff2e2e 0%, #cc0000 100%);
            color: white;
            grid-column: 1 / -1;
        }

        .btn-stop:active {
            background: linear-gradient(135deg, #ff1a1a 0%, #990000 100%);
            box-shadow: 0 0 15px rgba(255, 46, 46, 0.8);
        }

        /* Статус индикатор */
        .status-indicator {
            width: 12px;
            height: 12px;
            border-radius: 50%;
            background: #888;
            display: inline-block;
            margin-right: 8px;
            transition: background 0.3s ease;
        }

        .status-indicator.connected {
            background: #00ff64;
            box-shadow: 0 0 8px rgba(0, 255, 100, 0.6);
        }

        .status-text {
            font-size: 12px;
            color: #888;
            text-align: center;
            margin-top: 10px;
        }

        /* === АДАПТИВНОСТЬ (MOBILE-FIRST) === */
        @media (max-width: 480px) {
            .container {
                padding: 15px;
                max-width: 100%;
            }

            h1 {
                font-size: 24px;
                margin-bottom: 15px;
            }

            .dpad {
                width: 180px;
                height: 180px;
            }

            .dpad-btn {
                width: 50px;
                height: 50px;
                font-size: 20px;
            }

            input[type="range"] {
                height: 5px;
            }

            .slider-label {
                font-size: 12px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <!-- Заголовок -->
        <h1>🤖 Robot Control</h1>

        <!-- D-PAD управление движением -->
        <div class="dpad-section">
            <div class="dpad">
                <!-- Вперед -->
                <button class="dpad-btn up" id="btnUp" data-direction="forward">▲</button>
                <!-- Назад -->
                <button class="dpad-btn down" id="btnDown" data-direction="backward">▼</button>
                <!-- Влево -->
                <button class="dpad-btn left" id="btnLeft" data-direction="left">◄</button>
                <!-- Вправо -->
                <button class="dpad-btn right" id="btnRight" data-direction="right">►</button>
            </div>
        </div>

        <!-- Слайдеры управления -->
        <div class="slider-section">
            <!-- Слайдер скорости (0-255) -->
            <div class="slider-group">
                <div class="slider-label">
                    <span>Скорость (PWM)</span>
                    <span id="speedValue">128</span>
                </div>
                <input type="range" id="speedSlider" min="0" max="255" value="128">
            </div>

            <!-- Слайдер позиции серво (0-180 градусов) -->
            <div class="slider-group">
                <div class="slider-label">
                    <span>Сервопривод (°)</span>
                    <span id="servoValue">90</span>
                </div>
                <input type="range" id="servoSlider" min="0" max="180" value="90">
            </div>
        </div>

        <!-- Кнопка СТОП -->
        <div class="button-section">
            <button class="btn-stop" id="btnStop">⏹ СТОП</button>
        </div>

        <!-- Статус соединения -->
        <div class="status-text">
            <span class="status-indicator" id="statusIndicator"></span>
            <span id="statusText">Подключение...</span>
        </div>
    </div>

    <script>
        // === КОНФИГУРАЦИЯ И ПЕРЕМЕННЫЕ ===
        // Объект состояния робота
        const robotState = {
            speedLeft: 0,      // Скорость левого мотора (-255 до 255)
            speedRight: 0,     // Скорость правого мотора (-255 до 255)
            servoAngle: 90,    // Угол серво (0-180°)
            speed: 128,        // Текущая скорость из слайдера
            isMoving: false,   // Флаг: робот движется или стоит
            lastCommandTime: 0, // Время последней отправленной команды
        };

        // Переменные для throttle (защита от перегрузки ESP32)
        const THROTTLE_DELAY = 75; // мс (отправлять команду не чаще, чем раз в 75 мс)
        let lastThrottleTime = 0;

        // DOM элементы
        const btnUp = document.getElementById('btnUp');
        const btnDown = document.getElementById('btnDown');
        const btnLeft = document.getElementById('btnLeft');
        const btnRight = document.getElementById('btnRight');
        const btnStop = document.getElementById('btnStop');
        const speedSlider = document.getElementById('speedSlider');
        const servoSlider = document.getElementById('servoSlider');
        const speedValue = document.getElementById('speedValue');
        const servoValue = document.getElementById('servoValue');
        const statusIndicator = document.getElementById('statusIndicator');
        const statusText = document.getElementById('statusText');

        // === ФУНКЦИЯ: Отправка команды на ESP32 ===
        async function sendCommand(l, r, s = null) {
            // Используем сервопривод из слайдера, если не передан явно
            const servoAngle = s !== null ? s : robotState.servoAngle;

            // Формируем URL запроса: /move?l=[левый_мотор]&r=[правый_мотор]&s=[серво]
            const url = `/move?l=${l}&r=${r}&s=${servoAngle}`;

            try {
                // Отправляем асинхронный GET-запрос
                const response = await fetch(url);

                if (response.ok) {
                    // Успешный ответ - обновляем статус
                    statusIndicator.classList.add('connected');
                    statusText.textContent = 'Подключено ✓';
                } else {
                    statusIndicator.classList.remove('connected');
                    statusText.textContent = 'Ошибка: ' + response.status;
                }
            } catch (error) {
                // Ошибка соединения
                statusIndicator.classList.remove('connected');
                statusText.textContent = 'Нет соединения';
                console.error('Ошибка отправки команды:', error);
            }
        }

        // === ФУНКЦИЯ: Throttle (защита от перегрузки) ===
        // Ограничивает частоту отправки команд
        function throttleCommand(callback) {
            const now = Date.now();

            // Проверяем, прошло ли достаточно времени с последней команды
            if (now - lastThrottleTime >= THROTTLE_DELAY) {
                lastThrottleTime = now;
                callback();
            }
        }

        // === ФУНКЦИЯ: Обновление слайдера скорости ===
        speedSlider.addEventListener('input', (e) => {
            robotState.speed = parseInt(e.target.value);
            speedValue.textContent = robotState.speed;

            // Если робот движется, отправляем обновленную скорость
            if (robotState.isMoving) {
                throttleCommand(() => {
                    sendCommand(robotState.speedLeft, robotState.speedRight);
                });
            }
        });

        // === ФУНКЦИЯ: Обновление слайдера серво ===
        servoSlider.addEventListener('input', (e) => {
            robotState.servoAngle = parseInt(e.target.value);
            servoValue.textContent = robotState.servoAngle;

            // Отправляем команду с новым углом серво (throttle)
            throttleCommand(() => {
                sendCommand(robotState.speedLeft, robotState.speedRight, robotState.servoAngle);
            });
        });

        // === ВСПОМОГАТЕЛЬНАЯ ФУНКЦИЯ: Установка скорости боков ===
        function setMotorSpeeds(left, right) {
            robotState.speedLeft = left;
            robotState.speedRight = right;
            robotState.isMoving = !(left === 0 && right === 0);
        }

        // === ОБРАБОТЧИК D-PAD КНОПОК ===
        // Логика: "Ехать, пока держишь" (touchstart -> отправить команду, touchend -> стоп)

        function setupDpadButton(button, direction) {
            // Обработчик нажатия кнопки (touchstart / mousedown)
            function handleStart(e) {
                e.preventDefault();
                button.classList.add('pressed');

                // Определяем скорость на основе направления
                const speed = robotState.speed;

                if (direction === 'forward') {
                    // Вперед: оба мотора вперед
                    setMotorSpeeds(speed, speed);
                } else if (direction === 'backward') {
                    // Назад: оба мотора назад
                    setMotorSpeeds(-speed, -speed);
                } else if (direction === 'left') {
                    // Влево (разворот на месте): левый мотор назад, правый вперед
                    setMotorSpeeds(-speed, speed);
                } else if (direction === 'right') {
                    // Вправо (разворот на месте): левый мотор вперед, правый назад
                    setMotorSpeeds(speed, -speed);
                }

                // Отправляем команду движения
                throttleCommand(() => {
                    sendCommand(robotState.speedLeft, robotState.speedRight);
                });
            }

            // Обработчик отпускания кнопки (touchend / mouseup)
            function handleEnd(e) {
                e.preventDefault();
                button.classList.remove('pressed');

                // Отправляем команду стопа (l=0, r=0)
                setMotorSpeeds(0, 0);
                throttleCommand(() => {
                    sendCommand(0, 0);
                });
            }

            // Регистрируем обработчики для мыши и сенсорных экранов
            button.addEventListener('mousedown', handleStart);
            button.addEventListener('mouseup', handleEnd);
            button.addEventListener('mouseleave', handleEnd);
            button.addEventListener('touchstart', handleStart);
            button.addEventListener('touchend', handleEnd);
        }

        // Инициализируем все D-pad кнопки
        setupDpadButton(btnUp, 'forward');
        setupDpadButton(btnDown, 'backward');
        setupDpadButton(btnLeft, 'left');
        setupDpadButton(btnRight, 'right');

        // === КНОПКА СТОП ===
        btnStop.addEventListener('click', () => {
            // Отправляем команду полной остановки
            setMotorSpeeds(0, 0);
            sendCommand(0, 0);
            console.log('Робот остановлен');
        });

        // === ИНИЦИАЛИЗАЦИЯ ===
        // При загрузке страницы устанавливаем начальное значение серво (90°)
        window.addEventListener('load', () => {
            statusText.textContent = 'Готово к управлению';
            // Отправляем начальную команду для инициализации
            setTimeout(() => {
                sendCommand(0, 0, robotState.servoAngle);
            }, 500);
        });
    </script>
</body>
</html>
)rawliteral";

#endif // WEB_INTERFACE_H
