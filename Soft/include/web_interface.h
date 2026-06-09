#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>4WD Robot Control</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', sans-serif;
            background: linear-gradient(135deg, #1e1e1e 0%, #0a0a0a 100%);
            color: #e0e0e0;
            display: flex; flex-direction: column; align-items: center;
            min-height: 100vh; padding: 20px;
        }
        .container { width: 100%; max-width: 500px; background: rgba(30, 30, 30, 0.95); border-radius: 20px; padding: 20px; box-shadow: 0 8px 32px rgba(0, 0, 0, 0.5); }
        h1 { text-align: center; color: #4daeff; margin-bottom: 20px; }
        
        .distance-display { background: rgba(77, 174, 255, 0.1); border: 2px solid rgba(77, 174, 255, 0.3); border-radius: 15px; padding: 15px; margin-bottom: 20px; text-align: center; }
        .distance-value { font-size: 40px; font-weight: bold; color: #00ff00; }

        .control-group { margin-bottom: 20px; background: rgba(255,255,255,0.05); padding: 15px; border-radius: 10px; }
        .slider-label { display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px; font-size: 14px; }
        
        /* Стилизация полей ввода для ручной калибровки */
        .val-input {
            width: 70px;
            background: rgba(255,255,255,0.05);
            border: 1px solid rgba(77,174,255,0.3);
            border-radius: 8px;
            color: #4daeff;
            font-weight: bold;
            text-align: center;
            font-size: 15px;
            padding: 4px;
            outline: none;
            transition: 0.2s;
        }
        .val-input:focus {
            border-color: #4daeff;
            background: rgba(77,174,255,0.1);
            box-shadow: 0 0 8px rgba(77,174,255,0.4);
        }

        /* Прячем стрелочки вверх-вниз в полях типа "number" для чистоты дизайна */
        input[type="number"]::-webkit-outer-spin-button,
        input[type="number"]::-webkit-inner-spin-button {
            -webkit-appearance: none;
            margin: 0;
        }
        input[type="number"] {
            -moz-appearance: textfield;
        }

        input[type="range"] { width: 100%; height: 8px; border-radius: 5px; cursor: pointer; }

        .movement-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; margin: 20px 0; }
        .btn { 
            padding: 20px; border: none; border-radius: 10px; color: white; font-weight: bold; 
            cursor: pointer; transition: 0.2s; user-select: none; -webkit-tap-highlight-color: transparent;
        }
        .btn-blue { background: linear-gradient(135deg, #4daeff, #0088ff); }
        .btn-red { background: linear-gradient(135deg, #ff4444, #ff0000); }
        .btn:active { transform: scale(0.92); opacity: 0.8; }

        .weapon-panel { border: 2px solid #ff6400; background: rgba(255, 100, 0, 0.05); }
        .fire-btn { background: linear-gradient(135deg, #ff6400, #ff8c00); width: 100%; margin-top: 15px; }

        .status { text-align: center; font-size: 12px; color: #666; margin-top: 15px; }
        .status-indicator { display: inline-block; width: 10px; height: 10px; border-radius: 50%; background: #ff4444; margin-right: 5px; }
        .online { background: #44ff44; box-shadow: 0 0 8px #44ff44; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🤖 4WD Robot</h1>

        <div class="distance-display">
            <div style="font-size: 12px; color: #aaa;">РАССТОЯНИЕ</div>
            <span class="distance-value" id="dist">---</span> см
        </div>

        <!-- Ходовая -->
        <div class="control-group">
            <div class="slider-label">
                <span>Скорость езды:</span>
                <input type="number" class="val-input" id="spdInput" min="0" max="255" value="128">
            </div>
            <input type="range" id="spdSlider" min="0" max="255" value="128">
        </div>

        <div class="movement-grid">
            <div></div><button class="btn btn-blue" id="fwd">▲</button><div></div>
            <button class="btn btn-blue" id="lft">◀</button>
            <button class="btn btn-red" id="stp">🛑</button>
            <button class="btn btn-blue" id="rgt">▶</button>
            <div></div><button class="btn btn-blue" id="bak">▼</button><div></div>
        </div>

        <!-- Серво -->
        <div class="control-group">
            <div class="slider-label">
                <span>Угол Серво:</span>
                <span style="display:flex; align-items:center;">
                    <input type="number" class="val-input" id="srvInput" min="0" max="180" value="90">
                    <span style="color:#4daeff; margin-left:4px; font-weight:bold;">°</span>
                </span>
            </div>
            <input type="range" id="srvSlider" min="0" max="180" value="90">
        </div>

        <!-- ОРУЖИЕ -->
        <div class="control-group weapon-panel">
            <h3 style="text-align:center; color:#ff6400; margin-bottom:10px;">⚔️ КАТАПУЛЬТА</h3>
            
            <div class="slider-label">
                <span>Мощность мотора:</span>
                <input type="number" class="val-input" id="wSpdInput" min="50" max="255" value="200" style="color:#ff6400; border-color:rgba(255,100,0,0.3);">
            </div>
            <input type="range" id="wSpdSlider" min="50" max="255" value="200">
            
            <div style="margin-top:10px;" class="slider-label">
                <span>Угол выстрела:</span>
                <span style="display:flex; align-items:center;">
                    <input type="number" class="val-input" id="wAngInput" min="-360" max="360" value="45" style="color:#ff6400; border-color:rgba(255,100,0,0.3);">
                    <span style="color:#ff6400; margin-left:4px; font-weight:bold;">°</span>
                </span>
            </div>
            <input type="range" id="wAngSlider" min="-360" max="360" value="45">
            
            <div style="display: flex; gap: 10px; margin-top: 15px;">
                <button class="btn btn-blue" id="move" style="flex: 1; padding: 15px 5px; font-size: 13px; font-weight: bold;">⚙️ ПЕРЕМЕСТИТЬ</button>
                <button class="btn fire-btn" id="fire" style="flex: 1; padding: 15px 5px; font-size: 13px; font-weight: bold; margin-top: 0;">🔫 ВЫСТРЕЛ</button>
            </div>
        </div>

        <div class="status">
            <span class="status-indicator" id="indicator"></span>
            <span id="statText">Подключение к WebSocket...</span>
        </div>
    </div>

    <script>
        let curDir = 'stop';
        let curSpd = 128;
        let lastMoveTime = 0;
        let weaponRotating = false;

        const distEl = document.getElementById('dist');
        const statEl = document.getElementById('statText');
        const indEl = document.getElementById('indicator');
        
        // Слайдеры и Поля ввода
        const spdSlider = document.getElementById('spdSlider');
        const spdInput = document.getElementById('spdInput');
        const srvSlider = document.getElementById('srvSlider');
        const srvInput = document.getElementById('srvInput');
        const wSpdSlider = document.getElementById('wSpdSlider');
        const wSpdInput = document.getElementById('wSpdInput');
        const wAngSlider = document.getElementById('wAngSlider');
        const wAngInput = document.getElementById('wAngInput');

        // Инициализация WebSocket
        let gateway = `ws://${window.location.host}/ws`;
        let websocket;

        function initWebSocket() {
            console.log('Попытка установить WebSocket соединение...');
            websocket = new WebSocket(gateway);
            websocket.onopen = onOpen;
            websocket.onclose = onClose;
            websocket.onmessage = onMessage;
        }

        function onOpen(event) {
            indEl.classList.add('online');
            statEl.textContent = "Соединение активно";
        }

        function onClose(event) {
            indEl.classList.remove('online');
            statEl.textContent = "Связь потеряна. Повторное подключение...";
            setTimeout(initWebSocket, 2000);
        }

        function onMessage(event) {
            let data = event.data;
            if (data.startsWith("D:")) {
                let dist = parseFloat(data.substring(2));
                distEl.textContent = dist > 0 ? dist.toFixed(1) : "---";
            }
        }

        // Движение
        function sendMove(dir, spd) {
            const now = Date.now();
            if (dir !== 'stop' && dir === curDir && spd === curSpd && (now - lastMoveTime < 100)) return;
            curDir = dir; curSpd = spd; lastMoveTime = now;
            
            if (websocket.readyState === WebSocket.OPEN) {
                websocket.send(`M:${dir}:${spd}`);
            }
        }

        document.getElementById('fwd').onclick = () => sendMove('forward', curSpd);
        document.getElementById('bak').onclick = () => sendMove('backward', curSpd);
        document.getElementById('lft').onclick = () => sendMove('left', curSpd);
        document.getElementById('rgt').onclick = () => sendMove('right', curSpd);
        document.getElementById('stp').onclick = () => sendMove('stop', 0);

        // === СИНХРОНИЗАЦИЯ: Скорость езды ===
        let throttleTimeout = null;
        spdSlider.oninput = (e) => {
            spdInput.value = e.target.value;
            if (!throttleTimeout) {
                throttleTimeout = setTimeout(() => {
                    curSpd = e.target.value;
                    if (curDir !== 'stop') sendMove(curDir, curSpd);
                    throttleTimeout = null;
                }, 50);
            }
        };
        spdInput.oninput = (e) => {
            let val = parseInt(e.target.value);
            if (isNaN(val)) return;
            if (val < 0) val = 0;
            if (val > 255) val = 255;
            spdSlider.value = val;
            curSpd = val;
            if (curDir !== 'stop') sendMove(curDir, curSpd);
        };
        spdInput.onblur = (e) => {
            if (e.target.value === "") spdInput.value = spdSlider.value;
        };

        // === СИНХРОНИЗАЦИЯ: Угол Серво ===
        srvSlider.oninput = (e) => {
            srvInput.value = e.target.value;
        };
        srvSlider.onchange = (e) => {
            if (websocket.readyState === WebSocket.OPEN) {
                websocket.send(`S:${e.target.value}`);
            }
        };
        srvInput.oninput = (e) => {
            let val = parseInt(e.target.value);
            if (isNaN(val)) return;
            if (val < 0) val = 0;
            if (val > 180) val = 180;
            srvSlider.value = val;
            if (websocket.readyState === WebSocket.OPEN) {
                websocket.send(`S:${val}`);
            }
        };
        srvInput.onblur = (e) => {
            if (e.target.value === "") srvInput.value = srvSlider.value;
        };

        // === СИНХРОНИЗАЦИЯ: Мощность оружия ===
        wSpdSlider.oninput = (e) => {
            wSpdInput.value = e.target.value;
        };
        wSpdInput.oninput = (e) => {
            let val = parseInt(e.target.value);
            if (isNaN(val)) return;
            if (val < 50) val = 50;
            if (val > 255) val = 255;
            wSpdSlider.value = val;
        };
        wSpdInput.onblur = (e) => {
            if (e.target.value === "") wSpdInput.value = wSpdSlider.value;
        };

        // === СИНХРОНИЗАЦИЯ: Угол оружия ===
        wAngSlider.oninput = (e) => {
            wAngInput.value = e.target.value;
        };
        wAngInput.oninput = (e) => {
            let val = parseInt(e.target.value);
            if (isNaN(val)) return;
            if (val < -360) val = -360;
            if (val > 360) val = 360;
            wAngSlider.value = val;
        };
        wAngInput.onblur = (e) => {
            if (e.target.value === "") wAngInput.value = wAngSlider.value;
        };


        // Оружие: Переместить
        const moveBtn = document.getElementById('move');
        moveBtn.onclick = () => {
            if (weaponRotating) return;
            const wSpd = wSpdSlider.value;
            const wAng = wAngSlider.value;
            
            statEl.textContent = "⚙️ Перемещение ложки (" + wAng + "°)...";
            weaponRotating = true;

            if (websocket.readyState === WebSocket.OPEN) {
                websocket.send(`W:${wSpd}:${wAng}`);
            }

            setTimeout(() => { 
                weaponRotating = false; 
                statEl.textContent = "Готов к бою"; 
            }, 1500);
        };

        // Оружие: Выстрел с авто-серво
        const fireBtn = document.getElementById('fire');
        fireBtn.onclick = () => {
            if (weaponRotating) return;
            const wSpd = wSpdSlider.value;
            const wAng = wAngSlider.value;
            
            statEl.textContent = "🔥 ВЫСТРЕЛ С СЕРВО...";
            weaponRotating = true;

            if (websocket.readyState === WebSocket.OPEN) {
                websocket.send(`F:${wSpd}:${wAng}`);
            }

            setTimeout(() => { 
                weaponRotating = false; 
                statEl.textContent = "Готов к бою"; 
            }, 1500);
        };

        // WebSocket Heartbeat
        setInterval(() => {
            if (websocket && websocket.readyState === WebSocket.OPEN) {
                websocket.send('H');
            }
        }, 500);
        
        window.onload = () => {
            initWebSocket();
        };
    </script>
</body>
</html>
)rawliteral";

#endif