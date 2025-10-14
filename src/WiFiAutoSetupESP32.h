/* WiFiAutoSetupESP32.h
 * #wifi_2025-05-21_007
 * Автоподключение к Wi-Fi с Preferences: порт 80 в AP, порт 8080 в STA
 */
#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ESP32SSDP.h>

class WiFiAutoSetup {
  const char* apSSID = "ESP32";
  const char* apPASS = "12345678";
  const IPAddress apIP = IPAddress(192,168,10,1);
  const IPAddress apGW = IPAddress(192,168,10,1);
  const IPAddress apMSK = IPAddress(255,255,255,0);

  const char* ssdpName = "ESP32 WiFi Config"; // Имя устройства для SSDP

  WebServer configServerAP = WebServer(80);
  WebServer configServerSTA = WebServer(8080);
  WebServer* activeServer = nullptr;
  Preferences prefs;

  String ssid = "XXXX";
  String password = "YYYYYYY";

public:
  void begin();
  void handleClient();

private:
  void loadWiFiConfig();
  void saveWiFiConfig(const String& ssid, const String& password);
  void setupAPMode();
  void setupWiFi();
  void startWebServer(WebServer& server);

  void handleRoot();
  void handleSave();
  void handleScan();
  void handleList();
  void handleReboot();
  void handleDescription();
};