#ifndef CONFIG_STORAGE_H
#define CONFIG_STORAGE_H

#include <Arduino.h>
#include <Preferences.h>

class ConfigStorage {
public:
  ConfigStorage();

  // 初始化
  bool begin();

  // WiFi配置管理
  bool saveWiFiCredentials(const String& ssid, const String& password);
  bool loadWiFiCredentials(String& ssid, String& password);
  bool hasWiFiCredentials();
  void clearWiFiCredentials();

  // 通用设置管理
  bool saveString(const char* key, const String& value);
  String loadString(const char* key, const String& defaultValue = "");
  bool saveInt(const char* key, int value);
  int loadInt(const char* key, int defaultValue = 0);
  bool saveBool(const char* key, bool value);
  bool loadBool(const char* key, bool defaultValue = false);

  // 清除所有配置
  void clearAll();

private:
  Preferences prefs;
  bool initialized;

  // 命名空间和键名
  static const char* NAMESPACE;
  static const char* KEY_WIFI_SSID;
  static const char* KEY_WIFI_PASSWORD;
  static const char* KEY_WIFI_CONFIGURED;
};

#endif // CONFIG_STORAGE_H
