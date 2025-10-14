// Тестовый скетч для демонстрации работы библиотеки WiFiAutoSetupESP32
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiAutoSetupESP32.h>

WiFiAutoSetup wifiAuto;
WebServer appServer80(80);

bool appServerStarted = false;

void handleRoot80() {
  IPAddress ip = WiFi.localIP();
  String url8080 = String("http://") + ip.toString() + ":8080/";
  String html = F(
    "<!doctype html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><title>ESP32</title>"
    "<style>body{font-family:system-ui,Segoe UI,Roboto,Ubuntu,sans-serif;background:#0f172a;color:#e5e7eb;margin:0;display:flex;min-height:100vh;align-items:center;justify-content:center}"
    ".card{background:#111827;border:1px solid rgba(255,255,255,.08);padding:24px 28px;border-radius:14px;box-shadow:0 10px 30px rgba(0,0,0,.35);max-width:520px}"
    "h2{margin:0 0 8px;font-size:20px}p{margin:0 0 14px;color:#9ca3af}a.btn{display:inline-block;padding:10px 14px;border-radius:10px;background:linear-gradient(135deg,#3b82f6,#2563eb);color:#061021;text-decoration:none;font-weight:600}"
    ".muted{color:#9ca3af;font-size:13px;margin-top:10px}</style></head><body><div class=card>"
  );
  html += F("<h2>Сеть Wi‑Fi подключена</h2><p>Для настроек нажмите на ссылку:</p>");
  html += String("<p><a class=\"btn\" href=\"") + url8080 + String("\">Открыть настройки (8080)</a></p>");
  // mDNS не используется; SSDP/UPnP обеспечит обнаружение в сети
  html += String("<div class=\"muted\">IP: ") + ip.toString() + String("</div></div></body></html>");
  appServer80.send(200, "text/html; charset=utf-8", html);
}

void maybeStartAppServer80() {
  if (!appServerStarted && WiFi.status() == WL_CONNECTED) {
    // В STA режиме порт 80 свободен для прикладной задачи
    appServer80.on("/", handleRoot80);
    appServer80.begin();
    appServerStarted = true;
  }
}

void setup() {
  wifiAuto.begin();
}

void loop() {
  wifiAuto.handleClient();
  maybeStartAppServer80();
  if (appServerStarted) {
    appServer80.handleClient();
  }
}
