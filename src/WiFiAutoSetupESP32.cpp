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
    <html><head><meta charset="UTF-8"></head><body>
    <h2>Настройка Wi-Fi</h2>
    <form action="/save">Сеть: <input name="s" id="ssid"><br>
    Пароль: <input name="p" type="text"><br>
    <input type="submit" value="Запомнить сеть"></form>
    <form action="/scan"><input type="submit" value="Сканировать сети"></form>
    <form action="/reboot"><input type="submit" value="Рестарт контроллера"></form>
    <div id="networks"></div>
    <script>
    fetch('/list').then(r => r.json()).then(j => {
      let s = '<select onchange=\"document.getElementById(\\'ssid\\').value=this.value\">';
      j.forEach(e => s += `<option>${e}</option>`);
      s += '</select>'; document.getElementById('networks').innerHTML = s;
    });
    </script></body></html>
  )rawliteral";
  activeServer->send(200, "text/html; charset=utf-8", html);
}

void WiFiAutoSetup::handleSave() {
  if (activeServer->hasArg("s") && activeServer->hasArg("p")) {
    saveWiFiConfig(activeServer->arg("s"), activeServer->arg("p"));
    activeServer->send(200, "text/plain; charset=utf-8", "Сеть сохранена. Перезагрузите контроллер.");
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
  WiFi.softAPdisconnect(true);
  delay(150);
  WiFi.mode(WIFI_STA);
  delay(200);

  int n = WiFi.scanNetworks();
  String json = "[";
  for (int i = 0; i < n; i++) {
    if (i) json += ",";
    json += '"' + WiFi.SSID(i) + '"';
  }
  json += "]";

  WiFi.mode(previousMode);
  if (previousMode & WIFI_AP) {
    WiFi.softAP(apSSID, apPASS);
    WiFi.softAPConfig(apIP, apGW, apMSK);
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
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apGW, apMSK);
  WiFi.softAP(apSSID, apPASS);
  delay(100);
  Serial.println("[WiFi] Точка доступа активна (порт 80)");
  startWebServer(configServerAP);
}

void WiFiAutoSetup::begin() {
  Serial.begin(115200);
  delay(500);
  Serial.println("[System] Запуск контроллера...");
  loadWiFiConfig();
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
    startWebServer(configServerSTA);
  } else {
    Serial.println("\n[WiFi] Не удалось подключиться. Включаем AP...");
    setupAPMode();
  }
}

void WiFiAutoSetup::handleClient() {
  if (activeServer) activeServer->handleClient();
}