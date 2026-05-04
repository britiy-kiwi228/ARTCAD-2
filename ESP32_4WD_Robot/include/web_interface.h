#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

// HTML/CSS/JS код интерфейса управления роботом с КНОПКАМИ ДВИЖЕНИЯ
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

        /* === КНОПКИ УПРАВЛЕНИЯ ДВИЖЕНИЕМ === */
        .movement-buttons {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 10px;
            margin-bottom: 25px;
            padding: 20px;
            background: rgba(77, 174, 255, 0.08);
            border: 2px solid rgba(77, 174, 255, 0.3);
            border-radius: 15px;
        }

        .movement-btn {
            padding: 15px 10px;
            border: none;
            border-radius: 8px;
            background: linear-gradient(135deg, #4daeff, #0088ff);
            color: white;
            font-size: 14px;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.2s ease;
            box-shadow: 0 4px 12px rgba(77, 174, 255, 0.3);
        }

        .movement-btn:hover {
            background: linear-gradient(135deg, #5dbfff, #0099ff);
            box-shadow: 0 6px 16px rgba(77, 174, 255, 0.5);
        }

        .movement-btn:active {
            transform: scale(0.95);
            box-shadow: 0 2px 8px rgba(77, 174, 255, 0.3);
        }

        .movement-btn-forward {
            grid-column: 2;
            grid-row: 1;
        }

        .movement-btn-left {
            grid-column: 1;
            grid-row: 2;
        }

        .movement-btn-stop {
            grid-column: 2;
            grid-row: 2;
            background: linear-gradient(135deg, #ff4444, #ff0000);
            box-shadow: 0 4px 12px rgba(255, 68, 68, 0.3);
        }

        .movement-btn-stop:hover {
            background: linear-gradient(135deg, #ff6666, #ff2222);
            box-shadow: 0 6px 16px rgba(255, 68, 68, 0.5);
        }

        .movement-btn-right {
            grid-column: 3;
            grid-row: 2;
        }

        .movement-btn-backward {
            grid-column: 2;
            grid-row: 3;
        }
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

        <!-- === КНОПКИ УПРАВЛЕНИЯ ДВИЖЕНИЕМ === -->
        <div class="movement-buttons">
            <button class="movement-btn movement-btn-forward" id="btnForward">▲ Вперед</button>
            <button class="movement-btn movement-btn-left" id="btnLeft">◀ Влево</button>
            <button class="movement-btn movement-btn-stop" id="btnStop">🛑 СТОП</button>
            <button class="movement-btn movement-btn-right" id="btnRight">▶ Вправо</button>
            <button class="movement-btn movement-btn-backward" id="btnBackward">▼ Назад</button>
        </div>

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

        // === ПЕРЕМЕННЫЕ СОСТОЯНИЯ ===
        let currentServo = 90;       // Значение угла серво из слайдера (0-180)
        
        // === ПЕРЕМЕННЫЕ СОСТОЯНИЯ КАТАПУЛЬТЫ ===
        let currentWeaponSpeed = 0;  // Скорость мотора катапульты (-255...255)
        let currentWeaponAngle = 0;  // Угол поворота (0-360°)
        let weaponRotating = false;  // Флаг: сейчас вращаемся?

        // === ЭЛЕМЕНТЫ DOM ===
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

        // === ОБНОВЛЕНИЕ СЛАЙДЕРА СЕРВО ===
        servoSlider.addEventListener('input', (e) => {
            currentServo = parseInt(e.target.value);
            document.getElementById('servoValue').textContent = currentServo;
            // Отправляем команду для обновления угла серво (без движения, используем последнюю команду)
            sendCommand('stop');
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

        // === ОБРАБОТЧИКИ КНОПОК УПРАВЛЕНИЯ ДВИЖЕНИЕМ ===
        document.getElementById('btnForward').addEventListener('click', () => {
            sendCommand('forward');
        });

        document.getElementById('btnBackward').addEventListener('click', () => {
            sendCommand('backward');
        });

        document.getElementById('btnLeft').addEventListener('click', () => {
            sendCommand('left');
        });

        document.getElementById('btnRight').addEventListener('click', () => {
            sendCommand('right');
        });

        document.getElementById('btnStop').addEventListener('click', () => {
            sendCommand('stop');
        });

        // === ФУНКЦИЯ: Отправка команды на ESP32 (с кнопками управления) ===
        async function sendCommand(button) {
            const now = Date.now();
            
            // Формируем URL с параметром btn и углом серво
            const url = `/move?btn=${button}&s=${Math.round(currentServo)}`;
            
            try {
                const response = await fetch(url);
                
                if (response.ok) {
                    statusText.textContent = `✓ Команда: ${button} | Серво: ${Math.round(currentServo)}°`;
                    statusIndicator.classList.add('online');
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

        // === ИНИЦИАЛИЗАЦИЯ ===
        // При загрузке страницы инициализируем интерфейс
        window.addEventListener('load', () => {
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
