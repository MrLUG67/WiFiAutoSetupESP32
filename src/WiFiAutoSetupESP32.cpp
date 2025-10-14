#include "WiFiAutoSetupESP32.h"

void WiFiAutoSetup::saveWiFiConfig(const String& s, const String& p) {
  prefs.begin("wifi", false);
  prefs.putString("ssid", s);
  prefs.putString("pass", p);
  prefs.end();
  Serial.println("[NVS] Сохранены параметры сети");
}

void WiFiAutoSetup::loadWiFiConfig() {
  prefs.begin("wifi", true);
  ssid = prefs.getString("ssid", "XXXX");
  password = prefs.getString("pass", "YYYYYYY");
  prefs.end();
  Serial.println("[NVS] Загружены параметры сети");
  Serial.print("SSID: "); Serial.println(ssid);
}

void WiFiAutoSetup::handleRoot() {
  String html = R"rawliteral(
    <html>
      <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>Настройка Wi‑Fi</title>
        <style>
          :root { --bg:#0f172a; --card:#111827; --ink:#e5e7eb; --muted:#9ca3af; --accent:#22c55e; --accent2:#3b82f6; --danger:#ef4444; }
          * { box-sizing: border-box; }
          body { margin:0; font-family: system-ui, -apple-system, Segoe UI, Roboto, Ubuntu, Cantarell, Noto Sans, sans-serif; background: linear-gradient(135deg,#0f172a,#111827); color: var(--ink); }
          .wrap { min-height: 100vh; display:flex; align-items:center; justify-content:center; padding: 24px; }
          .card { width: 100%; max-width: 720px; background: rgba(17,24,39,0.9); border: 1px solid rgba(255,255,255,0.06); border-radius: 16px; padding: 24px; box-shadow: 0 10px 30px rgba(0,0,0,0.35); }
          .head { display:flex; align-items:center; justify-content:space-between; gap:12px; margin-bottom: 16px; }
          h1 { margin:0; font-size: 20px; font-weight: 700; letter-spacing: .3px; }
          .muted { color: var(--muted); font-size: 13px; }
          .grid { display:grid; grid-template-columns: 1fr; gap:12px; }
          @media (min-width:740px) { .grid { grid-template-columns: 1fr 1fr; } }
          label { font-size: 13px; color: var(--muted); display:block; margin-bottom:6px; }
          input[type=text], select { width: 100%; padding: 10px 12px; border-radius: 10px; border: 1px solid rgba(255,255,255,0.12); background: rgba(255,255,255,0.04); color: var(--ink); outline: none; }
          .row { display:flex; gap:12px; align-items:center; }
          .btn { appearance:none; border:0; border-radius: 10px; padding: 10px 14px; font-weight:600; color:#0b1220; cursor:pointer; transition: transform .04s ease; }
          .btn:active { transform: translateY(1px); }
          .btn-primary { background: linear-gradient(135deg, var(--accent), #16a34a); color:#071410; }
          .btn-blue { background: linear-gradient(135deg, var(--accent2), #2563eb); color:#061021; }
          .btn-danger { background: linear-gradient(135deg, var(--danger), #dc2626); color:#1a0707; }
          .footer { display:flex; justify-content:space-between; align-items:center; gap:12px; margin-top: 8px; flex-wrap: wrap; }
          .select-wrap { display:flex; gap: 8px; align-items: end; }
          .hint { font-size: 12px; color: var(--muted); }
          .hr { height:1px; background: rgba(255,255,255,0.08); margin: 12px 0; border-radius:1px; }
          .tag { display:inline-flex; align-items:center; gap:6px; font-size:12px; padding:6px 10px; border-radius:999px; background: rgba(255,255,255,0.06); color: var(--ink); }
        </style>
      </head>
      <body>
        <div class="wrap">
          <div class="card">
            <div class="head">
              <h1>Настройка Wi‑Fi</h1>
              <span class="tag"><svg width="14" height="14" viewBox="0 0 24 24" fill="currentColor" xmlns="http://www.w3.org/2000/svg"><path d="M12 20C13.1 20 14 19.1 14 18H10C10 19.1 10.9 20 12 20ZM18 14V10C18 6.93 16.36 4.36 13.5 3.68V3C13.5 2.17 12.83 1.5 12 1.5C11.17 1.5 10.5 2.17 10.5 3V3.68C7.63 4.36 6 6.92 6 10V14L4 16V17H20V16L18 14Z"/></svg> AP: 192.168.10.1 • WEB: 80/8080</span>
            </div>

            <form action="/save" class="grid">
              <div>
                <label for="ssid">Сеть (SSID)</label>
                <input name="s" id="ssid" type="text" placeholder="Выберите из списка или введите вручную">
              </div>
              <div>
                <label for="pass">Пароль</label>
                <input name="p" id="pass" type="text" placeholder="Пароль сети">
              </div>

              <div class="select-wrap">
                <div style="flex:1">
                  <label for="ssidList">Доступные сети</label>
                  <select id="ssidList"></select>
                </div>
                <button type="button" id="refresh" class="btn btn-blue">Обновить список</button>
              </div>

              <div class="row">
                <button type="submit" class="btn btn-primary">Сохранить и подключиться</button>
                <a class="hint">Сохранение произойдёт только при успешном подключении</a>
              </div>
            </form>

            <div class="hr"></div>

            <div class="footer">
              <form action="/scan" method="get"><button class="btn btn-blue" type="submit">Сканировать (перезагрузить список)</button></form>
              <form action="/reboot" method="get"><button class="btn btn-danger" type="submit">Рестарт контроллера</button></form>
            </div>
            <div class="muted" id="status" style="margin-top:8px"></div>
          </div>
        </div>

        <script>
          const ssidInput = document.getElementById('ssid');
          const list = document.getElementById('ssidList');
          const statusEl = document.getElementById('status');
          const refreshBtn = document.getElementById('refresh');

          function setStatus(msg) {
            statusEl.textContent = msg || '';
          }

          function fillList(items) {
            list.innerHTML = '';
            const opt0 = document.createElement('option');
            opt0.value = '';
            opt0.textContent = items && items.length ? '— выбрать сеть —' : 'сетей не найдено';
            list.appendChild(opt0);
            (items || []).forEach(e => {
              const o = document.createElement('option');
              o.value = e; o.textContent = e; list.appendChild(o);
            });
          }

          list.addEventListener('change', () => {
            if (list.value) ssidInput.value = list.value;
          });

          async function loadList() {
            setStatus('Поиск сетей...');
            try {
              const r = await fetch('/list', {cache:'no-store'});
              const j = await r.json();
              fillList(j);
              setStatus('Готово');
            } catch (e) {
              setStatus('Ошибка получения списка сетей');
            }
          }

          refreshBtn.addEventListener('click', loadList);
          loadList();
        </script>
      </body>
    </html>
  )rawliteral";
  activeServer->send(200, "text/html; charset=utf-8", html);
}

void WiFiAutoSetup::handleSave() {
  if (activeServer->hasArg("s") && activeServer->hasArg("p")) {
    String newSsid = activeServer->arg("s");
    String newPass = activeServer->arg("p");

    // Try to connect before saving
    Serial.println("[WiFi] Проверка новых параметров подключения...");
    wifi_mode_t prevMode = WiFi.getMode();
    if (!(prevMode & WIFI_STA)) {
      WiFi.mode((wifi_mode_t)(prevMode | WIFI_STA));
    }
    WiFi.begin(newSsid.c_str(), newPass.c_str());
    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 10000) {
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n[WiFi] Успешно подключено. Сохраняю параметры...");
      saveWiFiConfig(newSsid, newPass);
      activeServer->send(200, "text/plain; charset=utf-8", "Подключено. Параметры сохранены. Перезагрузка...");
      delay(1000);
      ESP.restart();
    } else {
      Serial.println("\n[WiFi] Ошибка подключения. Параметры не сохранены.");
      activeServer->send(400, "text/plain; charset=utf-8", "Не удалось подключиться с указанными параметрами. Проверьте SSID/пароль.");
      // Restore previous mode if we changed it
      if (prevMode != WiFi.getMode()) {
        WiFi.mode(prevMode);
        if (prevMode & WIFI_AP) {
          // Ensure AP config remains intact
          WiFi.softAPConfig(apIP, apGW, apMSK);
          WiFi.softAP(apSSID, apPASS);
        }
      }
    }
  } else {
    activeServer->send(400, "text/plain; charset=utf-8", "Ошибка: отсутствует имя сети или пароль");
  }
}

void WiFiAutoSetup::handleScan() {
  activeServer->sendHeader("Location", "/");
  activeServer->send(303);
}

void WiFiAutoSetup::handleList() {
  Serial.println("[WiFi] Безопасное сканирование сетей...");
  wifi_mode_t previousMode = WiFi.getMode();
  bool addedSta = false;
  if (!(previousMode & WIFI_STA)) {
    WiFi.mode((wifi_mode_t)(previousMode | WIFI_STA));
    addedSta = true;
    delay(150);
  }
  int n = WiFi.scanNetworks();
  String json = "[";
  for (int i = 0; i < n; i++) {
    if (i) json += ",";
    json += '"' + WiFi.SSID(i) + '"';
  }
  json += "]";
  if (addedSta) {
    WiFi.mode(previousMode);
  }
  activeServer->send(200, "application/json", json);
}

void WiFiAutoSetup::handleReboot() {
  activeServer->send(200, "text/plain; charset=utf-8", "Перезагрузка...");
  delay(1000);
  ESP.restart();
}

void WiFiAutoSetup::startWebServer(WebServer& serverRef) {
  activeServer = &serverRef;
  activeServer->on("/", [&]() { handleRoot(); });
  activeServer->on("/save", [&]() { handleSave(); });
  activeServer->on("/scan", [&]() { handleScan(); });
  activeServer->on("/list", [&]() { handleList(); });
  activeServer->on("/reboot", [&]() { handleReboot(); });
  activeServer->begin();
  Serial.println("[Web] Веб-сервер запущен (порт зависит от режима)");
}

void WiFiAutoSetup::setupAPMode() {
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apGW, apMSK);
  WiFi.softAP(apSSID, apPASS);
  delay(100);
  Serial.println("[WiFi] Точка доступа активна (порт 80)");
  if (MDNS.begin(mdnsHostname)) {
    MDNS.addService("http", "tcp", 80);
    Serial.print("[mDNS] Хостнейм (AP): "); Serial.print(mdnsHostname); Serial.println(".local");
  } else {
    Serial.println("[mDNS] Ошибка старта mDNS в AP");
  }
  startWebServer(configServerAP);
}

void WiFiAutoSetup::begin() {
  Serial.begin(115200);
  delay(500);
  Serial.println("[System] Запуск контроллера...");
  loadWiFiConfig();
  bool hasValidCreds = (ssid.length() > 0 && password.length() > 0 && ssid != "XXXX");
  if (hasValidCreds) {
    Serial.print("[WiFi] Попытка подключения к сети: "); Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      delay(500); Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n[WiFi] Подключено!");
      Serial.print("[WiFi] IP адрес: "); Serial.println(WiFi.localIP());
      Serial.println("[Web] Интерфейс настройки доступен на порту 8080");
      if (MDNS.begin(mdnsHostname)) {
        MDNS.addService("http", "tcp", 8080);
        Serial.print("[mDNS] Хостнейм: "); Serial.print(mdnsHostname); Serial.println(".local");
      } else {
        Serial.println("[mDNS] Ошибка старта mDNS");
      }
      startWebServer(configServerSTA);
      return;
    }
    Serial.println("\n[WiFi] Не удалось подключиться по сохранённым параметрам.");
  } else {
    Serial.println("[WiFi] Параметры сети отсутствуют. Запускаю AP для настройки...");
  }
  setupAPMode();
}

void WiFiAutoSetup::handleClient() {
  if (activeServer) activeServer->handleClient();
}
