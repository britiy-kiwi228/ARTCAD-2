#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

// HTML/CSS/JS код интерфейса управления роботом с ВИРТУАЛЬНЫМ ДЖОЙСТИКОМ
// Хранится в PROGMEM для экономии оперативной памяти ESP32
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>4WD Robot Control</title>
    <style>
        /* === БАЗОВЫЕ СТИЛИ === */
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #1e1e1e 0%, #0a0a0a 100%);
            color: #e0e0e0;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
            padding: 20px;
        }

        /* === КОНТЕЙНЕР === */
        .container {
            width: 100%;
            max-width: 500px;
            background: rgba(30, 30, 30, 0.95);
            border-radius: 20px;
            padding: 30px 20px;
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.5);
            border: 1px solid rgba(255, 255, 255, 0.1);
        }

        h1 {
            text-align: center;
            font-size: 24px;
            margin-bottom: 30px;
            color: #4daeff;
            text-shadow: 0 0 10px rgba(77, 174, 255, 0.3);
        }

        /* === ДИСПЛЕЙ РАССТОЯНИЯ (НОВОЕ) === */
        .distance-display {
            background: rgba(77, 174, 255, 0.1);
            border: 2px solid rgba(77, 174, 255, 0.5);
            border-radius: 15px;
            padding: 20px;
            margin-bottom: 25px;
            text-align: center;
        }

        .distance-label {
            font-size: 14px;
            color: #a0a0a0;
            margin-bottom: 8px;
            text-transform: uppercase;
            letter-spacing: 1px;
        }

        .distance-value {
            font-size: 48px;
            font-weight: bold;
            color: #00ff00;
            text-shadow: 0 0 15px rgba(0, 255, 0, 0.6);
            line-height: 1;
        }

        .distance-unit {
            font-size: 16px;
            color: #a0a0a0;
            margin-left: 8px;
        }

        .distance-status {
            font-size: 12px;
            color: #666;
            margin-top: 8px;
        }

        /* === КОНТРОЛЬ СКОРОСТИ === */
        .speed-control {
            margin-bottom: 25px;
        }

        .slider-label {
            display: flex;
            justify-content: space-between;
            margin-bottom: 10px;
            font-size: 14px;
            color: #a0a0a0;
        }

        .slider-label .value {
            color: #4daeff;
            font-weight: bold;
        }

        input[type="range"] {
            width: 100%;
            height: 6px;
            border-radius: 3px;
            background: linear-gradient(to right, #444, #666);
            outline: none;
            -webkit-appearance: none;
            appearance: none;
        }

        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 24px;
            height: 24px;
            border-radius: 50%;
            background: linear-gradient(135deg, #4daeff, #00d4ff);
            cursor: pointer;
            box-shadow: 0 0 10px rgba(77, 174, 255, 0.5);
        }

        input[type="range"]::-moz-range-thumb {
            width: 24px;
            height: 24px;
            border-radius: 50%;
            background: linear-gradient(135deg, #4daeff, #00d4ff);
            cursor: pointer;
            border: none;
            box-shadow: 0 0 10px rgba(77, 174, 255, 0.5);
        }

        /* === ДЖОЙСТИК КОНТЕЙНЕР === */
        .joystick-container {
            display: flex;
            justify-content: center;
            align-items: center;
            margin: 30px 0;
            flex-direction: column;
        }

        /* Canvas для виртуального джойстика */
        #joystickCanvas {
            border: 2px solid rgba(77, 174, 255, 0.3);
            border-radius: 50%;
            background: radial-gradient(circle at 30% 30%, rgba(77, 174, 255, 0.1), rgba(0, 0, 0, 0.3));
            cursor: grab;
            max-width: 100%;
            display: block;
            touch-action: none;
            -webkit-user-select: none;
            user-select: none;
        }

        #joystickCanvas:active {
            cursor: grabbing;
        }

        /* Текст подсказка */
        .joystick-hint {
            margin-top: 15px;
            font-size: 12px;
            color: #666;
            text-align: center;
        }

        /* === КНОПКА СТОП === */
        .stop-btn {
            width: 100%;
            padding: 15px;
            margin-top: 30px;
            border: none;
            border-radius: 10px;
            background: linear-gradient(135deg, #ff4444, #ff0000);
            color: white;
            font-size: 18px;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.2s ease;
            box-shadow: 0 4px 15px rgba(255, 0, 0, 0.3);
        }

        .stop-btn:hover {
            background: linear-gradient(135deg, #ff6666, #ff2222);
            box-shadow: 0 6px 20px rgba(255, 0, 0, 0.5);
        }

        .stop-btn:active {
            transform: scale(0.98);
        }

        /* === ПАНЕЛЬ УПРАВЛЕНИЯ КАТАПУЛЬТОЙ === */
        .weapon-panel {
            background: rgba(255, 100, 0, 0.08);
            border: 2px solid rgba(255, 100, 0, 0.3);
            border-radius: 15px;
            padding: 20px;
            margin-top: 25px;
            margin-bottom: 25px;
        }

        .weapon-panel h2 {
            font-size: 16px;
            color: #ff6400;
            margin-bottom: 15px;
            text-shadow: 0 0 8px rgba(255, 100, 0, 0.4);
            text-align: center;
        }

        .weapon-control {
            margin-bottom: 15px;
        }

        /* === КНОПКА ВЫСТРЕЛА === */
        .fire-btn {
            width: 100%;
            padding: 12px;
            margin-bottom: 15px;
            border: none;
            border-radius: 8px;
            background: linear-gradient(135deg, #ff6400, #ff8c00);
            color: white;
            font-size: 16px;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.2s ease;
            box-shadow: 0 4px 12px rgba(255, 100, 0, 0.4);
        }

        .fire-btn:hover {
            background: linear-gradient(135deg, #ff7722, #ff9922);
            box-shadow: 0 6px 16px rgba(255, 100, 0, 0.6);
        }

        .fire-btn:active {
            transform: scale(0.95);
        }

        /* === СТАТУС === */
        .status {
            text-align: center;
            margin-top: 20px;
            font-size: 12px;
            color: #666;
        }

        .status-indicator {
            display: inline-block;
            width: 10px;
            height: 10px;
            border-radius: 50%;
            margin-right: 8px;
            background-color: #ff4444;
            transition: background-color 0.3s ease;
        }

        .status-indicator.online {
            background-color: #44ff44;
            box-shadow: 0 0 5px rgba(68, 255, 68, 0.6);
        }

        /* === АДАПТИВНАЯ ВЕРСТКА === */
        @media (max-width: 480px) {
            .container {
                padding: 20px 10px;
            }

            h1 {
                font-size: 20px;
                margin-bottom: 20px;
            }

            #joystickCanvas {
                max-width: 200px;
                height: auto;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🤖 4WD Robot Control</h1>

        <!-- ДИСПЛЕЙ РАССТОЯНИЯ -->
        <div class="distance-display">
            <div class="distance-label">📏 Расстояние до стены</div>
            <div>
                <span class="distance-value" id="distanceValue">---</span>
                <span class="distance-unit" id="distanceUnit">см</span>
            </div>
            <div class="distance-status" id="distanceStatus">загрузка...</div>
        </div>

        <!-- Контроль скорости -->
        <div class="speed-control">
            <div class="slider-label">
                <span>Скорость (0-255)</span>
                <span class="value" id="speedValue">128</span>
            </div>
            <input type="range" id="speedSlider" min="0" max="255" value="128">
        </div>

        <!-- Контроль серво -->
        <div class="speed-control">
            <div class="slider-label">
                <span>Позиция серво (0-180°)</span>
                <span class="value" id="servoValue">90</span>
            </div>
            <input type="range" id="servoSlider" min="0" max="180" value="90">
        </div>

        <!-- === ПАНЕЛЬ УПРАВЛЕНИЯ КАТАПУЛЬТОЙ === -->
        <div class="weapon-panel">
            <h2>⚔️ Система вооружения</h2>
            
            <!-- Контроль скорости катапульты -->
            <div class="weapon-control">
                <div class="slider-label">
                    <span>Скорость мотора (-255...255)</span>
                    <span class="value" id="weaponSpeedValue">0</span>
                </div>
                <input type="range" id="weaponSpeedSlider" min="-255" max="255" value="0">
            </div>

            <!-- Контроль угла поворота -->
            <div class="weapon-control">
                <div class="slider-label">
                    <span>Угол поворота (0-360°)</span>
                    <span class="value" id="weaponAngleValue">0</span>
                </div>
                <input type="range" id="weaponAngleSlider" min="0" max="360" value="0">
            </div>

            <!-- Кнопка выстрела -->
            <button class="fire-btn" id="fireBtn">🔫 ВЫСТРЕЛ</button>
        </div>

        <!-- ВИРТУАЛЬНЫЙ ДЖОЙСТИК -->
        <div class="joystick-container">
            <canvas 
                id="joystickCanvas" 
                width="250" 
                height="250"
            ></canvas>
            <div class="joystick-hint">
                Потяни джойстик для управления движением
            </div>
        </div>

        <!-- Кнопка СТОП -->
        <button class="stop-btn" id="emergencyStop">🛑 СТОП</button>

        <div class="status">
            <span class="status-indicator" id="statusIndicator"></span>
            <span id="statusText">Готов к управлению</span>
        </div>
    </div>

    <script>
        // === ФУНКЦИЯ: Heartbeat - регулярно обновляет таймер failsafe на сервере ===
        // Отправляется НЕЗАВИСИМО от команд управления
        // Это гарантирует, что failsafe не сработает при throttle (задержке команд)
        async function sendHeartbeat() {
            try {
                await fetch('/heartbeat');
                // Успешно отправили heartbeat - failsafe получил сигнал жизни
            } catch (error) {
                // Ошибка соединения - это нормально, failsafe справится с таймаутом
            }
        }

        // === ФУНКЦИЯ: Обновление расстояния от датчика США (НОВОЕ) ===
        async function updateDistance() {
            try {
                const response = await fetch('/distance');
                if (response.ok) {
                    const data = await response.json();
                    const distanceElement = document.getElementById('distanceValue');
                    const statusElement = document.getElementById('distanceStatus');
                    
                    if (data.distance < 0) {
                        // Таймаут или ошибка датчика
                        distanceElement.textContent = '---';
                        distanceElement.style.color = '#ff6666';
                        statusElement.textContent = '❌ Датчик не отвечает';
                        statusElement.style.color = '#ff6666';
                    } else {
                        // Нормальное значение
                        distanceElement.textContent = data.distance.toFixed(1);
                        distanceElement.style.color = '#00ff00';
                        statusElement.textContent = '✓ Обновлено сейчас';
                        statusElement.style.color = '#66ff66';
                    }
                }
            } catch (error) {
                document.getElementById('distanceValue').textContent = '---';
                document.getElementById('distanceStatus').textContent = '❌ Нет соединения';
                document.getElementById('distanceStatus').style.color = '#ff6666';
            }
        }

        // === КОНФИГУРАЦИЯ ===
        const CONFIG = {
            throttleMs: 50,      // Минимальный интервал между командами (50мс = 20 команд/сек)
            joystickRadius: 80,  // Радиус движения джойстика в пиксели
            canvasSize: 250      // Размер canvas элемента (квадратный)
        };

        // === ПЕРЕМЕННЫЕ СОСТОЯНИЯ ===
        let lastCommandTime = 0;
        let currentSpeed = 128;      // Значение скорости из слайдера (0-255)
        let currentServo = 90;       // Значение угла серво из слайдера (0-180)
        let joystickActive = false;  // Флаг: пользователь держит джойстик
        let queuedCommand = null;    // Очередная команда, если throttle активен
        let lastSentCommand = { l: 0, r: 0, s: 90 }; // Последняя отправленная команда
        
        // === ПЕРЕМЕННЫЕ СОСТОЯНИЯ КАТАПУЛЬТЫ ===
        let currentWeaponSpeed = 0;  // Скорость мотора катапульты (-255...255)
        let currentWeaponAngle = 0;  // Угол поворота (0-360°)
        let weaponRotating = false;  // Флаг: сейчас вращаемся?

        // === ЭЛЕМЕНТЫ DOM ===
        const canvas = document.getElementById('joystickCanvas');
        const ctx = canvas.getContext('2d');
        const speedSlider = document.getElementById('speedSlider');
        const servoSlider = document.getElementById('servoSlider');
        const statusText = document.getElementById('statusText');
        const statusIndicator = document.getElementById('statusIndicator');
        
        // === ЭЛЕМЕНТЫ DOM КАТАПУЛЬТЫ ===
        const weaponSpeedSlider = document.getElementById('weaponSpeedSlider');
        const weaponAngleSlider = document.getElementById('weaponAngleSlider');
        const fireBtn = document.getElementById('fireBtn');
        const weaponSpeedValue = document.getElementById('weaponSpeedValue');
        const weaponAngleValue = document.getElementById('weaponAngleValue');

        // === ОБНОВЛЕНИЕ СЛАЙДЕРА СКОРОСТИ ===
        speedSlider.addEventListener('input', (e) => {
            currentSpeed = parseInt(e.target.value);
            document.getElementById('speedValue').textContent = currentSpeed;
        });

        // === ОБНОВЛЕНИЕ СЛАЙДЕРА СЕРВО ===
        servoSlider.addEventListener('input', (e) => {
            currentServo = parseInt(e.target.value);
            document.getElementById('servoValue').textContent = currentServo;
            // Отправляем команду для обновления угла серво
            sendCommand(lastSentCommand.l, lastSentCommand.r, currentServo);
        });

        // === ОБНОВЛЕНИЕ СЛАЙДЕРА СКОРОСТИ КАТАПУЛЬТЫ ===
        weaponSpeedSlider.addEventListener('input', (e) => {
            currentWeaponSpeed = parseInt(e.target.value);
            weaponSpeedValue.textContent = currentWeaponSpeed;
        });

        // === ОБНОВЛЕНИЕ СЛАЙДЕРА УГЛА КАТАПУЛЬТЫ ===
        weaponAngleSlider.addEventListener('input', (e) => {
            currentWeaponAngle = parseInt(e.target.value);
            weaponAngleValue.textContent = currentWeaponAngle;
        });

        // === ОБРАБОТЧИК КНОПКИ ВЫСТРЕЛА ===
        fireBtn.addEventListener('click', () => {
            sendFire(currentWeaponSpeed, currentWeaponAngle);
        });

        // === ФУНКЦИЯ: Отправка команды на ESP32 ===
        async function sendCommand(left, right, servo) {
            const now = Date.now();
            
            // Сохраняем последнюю отправленную команду
            lastSentCommand = { l: left, r: right, s: servo };

            // THROTTLE: если прошло меньше чем throttleMs, ставим в очередь
            if (now - lastCommandTime < CONFIG.throttleMs) {
                queuedCommand = { left, right, servo };
                return;
            }

            // Отправляем команду
            lastCommandTime = now;
            queuedCommand = null;

            try {
                // Формируем URL: /move?l=[значение]&r=[значение]&s=[угол]
                const response = await fetch(`/move?l=${Math.round(left)}&r=${Math.round(right)}&s=${Math.round(servo)}`);
                
                if (response.ok) {
                    statusText.textContent = `✓ L:${Math.round(left)} R:${Math.round(right)} S:${Math.round(servo)}`;
                    statusIndicator.classList.add('online');
                    
                    // Если была очередная команда, отправляем её через промежуток времени
                    if (queuedCommand) {
                        setTimeout(() => {
                            sendCommand(queuedCommand.left, queuedCommand.right, queuedCommand.servo);
                        }, CONFIG.throttleMs);
                    }
                } else {
                    statusText.textContent = `✗ Ошибка: ${response.status}`;
                    statusIndicator.classList.remove('online');
                }
            } catch (error) {
                statusText.textContent = `✗ Нет соединения`;
                statusIndicator.classList.remove('online');
            }
        }

        // === ФУНКЦИЯ: Отправка команды ВЫСТРЕЛА на ESP32 ===
        async function sendFire(weaponSpeed, weaponAngle) {
            // Блокируем повторный выстрел, если уже вращаемся
            if (weaponRotating) {
                statusText.textContent = `⚠ Катапульта уже работает!`;
                return;
            }

            weaponRotating = true;
            fireBtn.disabled = true;
            fireBtn.textContent = '⏳ Выстрел...';

            try {
                // Отправляем команду выстрела
                // Параметры: w_speed (скорость), w_angle (угол)
                const response = await fetch(`/fire?w_speed=${Math.round(weaponSpeed)}&w_angle=${Math.round(weaponAngle)}`);
                
                if (response.ok) {
                    // Выстрел успешно инициирован
                    statusText.textContent = `💥 ВЫСТРЕЛ! Скорость: ${Math.round(weaponSpeed)}, Угол: ${Math.round(weaponAngle)}°`;
                    
                    // Имитируем время вращения: примерно 150мс за полный оборот
                    const rotationTime = (weaponAngle / 360) * 150;  // мс
                    
                    // Ждём завершения вращения
                    setTimeout(() => {
                        fireBtn.disabled = false;
                        fireBtn.textContent = '🔫 ВЫСТРЕЛ';
                        weaponRotating = false;
                        statusText.textContent = `✓ Выстрел завершён`;
                    }, rotationTime + 100);  // +100мс буфер
                } else if (response.status === 409) {
                    // === ЗАЩИТА: Выстрел заблокирован из-за высокой нагрузки ===
                    statusText.textContent = `⚠️ ВЫСТРЕЛ ЗАБЛОКИРОВАН! Снизьте скорость моторов (ходовые моторы перегружены)`;
                    fireBtn.disabled = false;
                    fireBtn.textContent = '🔫 ВЫСТРЕЛ';
                    weaponRotating = false;
                } else {
                    // Другая ошибка
                    statusText.textContent = `✗ Ошибка выстрела: ${response.status}`;
                    fireBtn.disabled = false;
                    fireBtn.textContent = '🔫 ВЫСТРЕЛ';
                    weaponRotating = false;
                }
            } catch (error) {
                statusText.textContent = `✗ Нет соединения с катапультой`;
                fireBtn.disabled = false;
                fireBtn.textContent = '🔫 ВЫСТРЕЛ';
                weaponRotating = false;
            }
        }

        // === ФУНКЦИЯ: Рисование джойстика на Canvas ===
        function drawJoystick(thumbX = 0, thumbY = 0) {
            const centerX = CONFIG.canvasSize / 2;
            const centerY = CONFIG.canvasSize / 2;
            const radius = CONFIG.joystickRadius;

            // Очищаем canvas от предыдущего рисунка
            ctx.clearRect(0, 0, CONFIG.canvasSize, CONFIG.canvasSize);

            // === РИСУЕМ ВНЕШНИЙ КРУГ (BOUNDARY) ===
            ctx.strokeStyle = 'rgba(77, 174, 255, 0.4)';
            ctx.lineWidth = 2;
            ctx.beginPath();
            ctx.arc(centerX, centerY, radius, 0, Math.PI * 2);
            ctx.stroke();

            // === РИСУЕМ СЕТКУ (КРЕСТОВИНА) ===
            ctx.strokeStyle = 'rgba(77, 174, 255, 0.15)';
            ctx.lineWidth = 1;
            
            // Горизонтальная линия
            ctx.beginPath();
            ctx.moveTo(centerX - radius, centerY);
            ctx.lineTo(centerX + radius, centerY);
            ctx.stroke();
            
            // Вертикальная линия
            ctx.beginPath();
            ctx.moveTo(centerX, centerY - radius);
            ctx.lineTo(centerX, centerY + radius);
            ctx.stroke();

            // === РИСУЕМ ЦЕНТРАЛЬНУЮ ТОЧКУ ===
            ctx.fillStyle = 'rgba(77, 174, 255, 0.6)';
            ctx.beginPath();
            ctx.arc(centerX, centerY, 5, 0, Math.PI * 2);
            ctx.fill();

            // === ОГРАНИЧИВАЕМ ДВИЖЕНИЕ THUMB ВНУТРИ КРУГА ===
            const distance = Math.sqrt(thumbX * thumbX + thumbY * thumbY);
            if (distance > radius) {
                const angle = Math.atan2(thumbY, thumbX);
                thumbX = Math.cos(angle) * radius;
                thumbY = Math.sin(angle) * radius;
            }

            // === РИСУЕМ THUMB (УКАЗАТЕЛЬ ДЖОЙСТИКА) ===
            const thumbRadius = 20;
            ctx.fillStyle = joystickActive 
                ? 'rgba(77, 174, 255, 0.9)' 
                : 'rgba(77, 174, 255, 0.6)';
            ctx.shadowColor = 'rgba(77, 174, 255, 0.8)';
            ctx.shadowBlur = 15;
            ctx.beginPath();
            ctx.arc(centerX + thumbX, centerY + thumbY, thumbRadius, 0, Math.PI * 2);
            ctx.fill();
            ctx.shadowBlur = 0;

            return { thumbX, thumbY, distance: distance > radius ? radius : distance };
        }

        // === ФУНКЦИЯ: Преобразование координат джойстика в команды движения (TANK DRIVE) ===
        function joystickToMotorCommand(thumbX, thumbY, maxSpeed = currentSpeed) {
            const radius = CONFIG.joystickRadius;
            
            // Нормализуем расстояние от центра (0..1)
            const normalizedDistance = Math.min(1, Math.sqrt(thumbX * thumbX + thumbY * thumbY) / radius);
            
            // Вычисляем угол от центра джойстика (в радианах)
            // 0 радиан = вправо, PI/2 = вниз, PI/-PI = влево, -PI/2 = вверх
            const angle = Math.atan2(thumbY, thumbX);
            
            // === ТАНК-КОНТРОЛЬ ФОРМУЛА ===
            // forward = движение вперёд (зависит от угла и расстояния)
            // turn = поворот (положительный = вправо, отрицательный = влево)
            // left_speed = forward - turn
            // right_speed = forward + turn
            
            // Используем atan2 с корректировкой угла, чтобы "вверх" был вперёд (0°)
            const correctAngle = angle + Math.PI / 2; // Смещаем так, чтобы 0 был вверху
            
            // Вычисляем forward и turn
            const forward = normalizedDistance * Math.cos(correctAngle) * maxSpeed;
            const turn = normalizedDistance * Math.sin(correctAngle) * maxSpeed;
            
            let leftSpeed = forward - turn;
            let rightSpeed = forward + turn;
            
            // Ограничиваем диапазон [-255, 255]
            leftSpeed = Math.max(-255, Math.min(255, leftSpeed));
            rightSpeed = Math.max(-255, Math.min(255, rightSpeed));
            
            return { left: leftSpeed, right: rightSpeed };
        }

        // === ФУНКЦИЯ: Получение координат взаимодействия (touch/mouse) ===
        function getJoystickCoordinates(e) {
            const rect = canvas.getBoundingClientRect();
            const centerX = CONFIG.canvasSize / 2;
            const centerY = CONFIG.canvasSize / 2;
            
            let clientX, clientY;
            
            // Проверяем тип события (touch или mouse)
            if (e.touches && e.touches.length > 0) {
                // Touch event - берём первый палец
                clientX = e.touches[0].clientX;
                clientY = e.touches[0].clientY;
            } else {
                // Mouse event
                clientX = e.clientX;
                clientY = e.clientY;
            }
            
            // Нормализуем координаты относительно canvas с учётом DPI
            const scaleX = canvas.width / rect.width;
            const scaleY = canvas.height / rect.height;
            
            const x = (clientX - rect.left) * scaleX - centerX;
            const y = (clientY - rect.top) * scaleY - centerY;
            
            return { x, y };
        }

        // === ОБРАБОТЧИК: MOUSEDOWN / TOUCHSTART (начало взаимодействия) ===
        canvas.addEventListener('mousedown', handleJoystickStart);
        canvas.addEventListener('touchstart', handleJoystickStart, { passive: false });

        function handleJoystickStart(e) {
            e.preventDefault();
            joystickActive = true;
            handleJoystickMove(e); // Сразу обновляем положение
        }

        // === ОБРАБОТЧИК: MOUSEMOVE / TOUCHMOVE (движение джойстика) ===
        document.addEventListener('mousemove', handleJoystickMove);
        document.addEventListener('touchmove', handleJoystickMove, { passive: false });

        function handleJoystickMove(e) {
            if (!joystickActive) return;
            
            e.preventDefault();
            const { x, y } = getJoystickCoordinates(e);
            
            // Рисуем обновленное положение джойстика
            const { thumbX, thumbY } = drawJoystick(x, y);
            
            // Преобразуем координаты в команды моторов
            const motorCommand = joystickToMotorCommand(thumbX, thumbY);
            
            // Отправляем команду с throttle
            sendCommand(motorCommand.left, motorCommand.right, currentServo);
        }

        // === ОБРАБОТЧИК: MOUSEUP / TOUCHEND (отпускание джойстика) ===
        document.addEventListener('mouseup', handleJoystickEnd);
        document.addEventListener('touchend', handleJoystickEnd, { passive: false });

        function handleJoystickEnd(e) {
            if (!joystickActive) return;
            
            e.preventDefault();
            joystickActive = false;
            
            // Возвращаем джойстик в центр
            drawJoystick(0, 0);
            
            // Отправляем команду стопа (l=0, r=0, сохраняем текущий угол серво)
            sendCommand(0, 0, currentServo);
        }

        // === ОБРАБОТЧИК: КНОПКА АВАРИЙНОГО СТОПА ===
        document.getElementById('emergencyStop').addEventListener('click', () => {
            joystickActive = false;
            drawJoystick(0, 0);
            sendCommand(0, 0, currentServo);
            statusText.textContent = '🛑 АВАРИЙНАЯ ОСТАНОВКА!';
        });

        // === ИНИЦИАЛИЗАЦИЯ ===
        // При загрузке страницы рисуем джойстик в нейтральном положении (центр)
        window.addEventListener('load', () => {
            drawJoystick(0, 0);
            statusIndicator.classList.add('online');
            
            // === АВТООБНОВЛЕНИЕ РАССТОЯНИЯ КАЖДЫЕ 100 МС ===
            // Запускаем первое обновление сразу
            updateDistance();
            
            // Затем обновляем каждые 100 миллисекунд
            setInterval(updateDistance, 100);
            
            // === HEARTBEAT ДЛЯ FAILSAFE ===
            // Отправляем пустой heartbeat запрос каждые 100мс
            // Это гарантирует, что failsafe на сервере не сработает
            // даже если команды ставятся в очередь (throttle)
            sendHeartbeat();  // Первый heartbeat сразу
            setInterval(sendHeartbeat, 100);  // Затем каждые 100мс
        });
    </script>
</body>
</html>
)rawliteral";

#endif // WEB_INTERFACE_H
