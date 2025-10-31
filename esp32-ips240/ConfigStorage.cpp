#include "ConfigStorage.h"

// 静态常量定义
const char* ConfigStorage::NAMESPACE = "esp32_config";
const char* ConfigStorage::KEY_WIFI_SSID = "wifi_ssid";
const char* ConfigStorage::KEY_WIFI_PASSWORD = "wifi_pwd";
const char* ConfigStorage::KEY_WIFI_CONFIGURED = "wifi_cfg";

ConfigStorage::ConfigStorage() {
  initialized = false;
}

bool ConfigStorage::begin() {
  if (initialized) {
    return true;
  }

  initialized = true;
  Serial.println("配置存储已初始化");
  return true;
}

bool ConfigStorage::saveWiFiCredentials(const String& ssid, const String& password) {
  if (!initialized) {
    Serial.println("错误: ConfigStorage未初始化");
    return false;
  }

  bool success = true;

  // 打开Preferences进行写入
  if (!prefs.begin(NAMESPACE, false)) {
    Serial.println("错误: 无法打开Preferences");
    return false;
  }

  // 保存SSID
  if (prefs.putString(KEY_WIFI_SSID, ssid) == 0) {
    Serial.println("错误: 保存SSID失败");
    success = false;
  }

  // 保存密码
  if (prefs.putString(KEY_WIFI_PASSWORD, password) == 0) {
    Serial.println("错误: 保存密码失败");
    success = false;
  }

  // 标记已配置
  if (success) {
    prefs.putBool(KEY_WIFI_CONFIGURED, true);
    Serial.println("WiFi配置已保存");
  }

  prefs.end();
  return success;
}

bool ConfigStorage::loadWiFiCredentials(String& ssid, String& password) {
  if (!initialized) {
    Serial.println("错误: ConfigStorage未初始化");
    return false;
  }

  // 打开Preferences进行读取
  if (!prefs.begin(NAMESPACE, true)) {  // true = 只读模式
    Serial.println("错误: 无法打开Preferences");
    return false;
  }

  // 检查是否已配置
  if (!prefs.getBool(KEY_WIFI_CONFIGURED, false)) {
    Serial.println("WiFi尚未配置");
    prefs.end();
    return false;
  }

  // 读取SSID和密码
  ssid = prefs.getString(KEY_WIFI_SSID, "");
  password = prefs.getString(KEY_WIFI_PASSWORD, "");

  prefs.end();

  if (ssid.length() == 0) {
    Serial.println("错误: SSID为空");
    return false;
  }

  Serial.println("WiFi配置已加载");
  return true;
}

bool ConfigStorage::hasWiFiCredentials() {
  if (!initialized) {
    return false;
  }

  if (!prefs.begin(NAMESPACE, true)) {
    return false;
  }

  bool configured = prefs.getBool(KEY_WIFI_CONFIGURED, false);
  prefs.end();

  return configured;
}

void ConfigStorage::clearWiFiCredentials() {
  if (!initialized) {
    Serial.println("错误: ConfigStorage未初始化");
    return;
  }

  if (!prefs.begin(NAMESPACE, false)) {
    Serial.println("错误: 无法打开Preferences");
    return;
  }

  prefs.remove(KEY_WIFI_SSID);
  prefs.remove(KEY_WIFI_PASSWORD);
  prefs.remove(KEY_WIFI_CONFIGURED);
  prefs.end();

  Serial.println("WiFi配置已清除");
}

bool ConfigStorage::saveString(const char* key, const String& value) {
  if (!initialized || !prefs.begin(NAMESPACE, false)) {
    return false;
  }

  bool success = prefs.putString(key, value) > 0;
  prefs.end();
  return success;
}

String ConfigStorage::loadString(const char* key, const String& defaultValue) {
  if (!initialized || !prefs.begin(NAMESPACE, true)) {
    return defaultValue;
  }

  String value = prefs.getString(key, defaultValue);
  prefs.end();
  return value;
}

bool ConfigStorage::saveInt(const char* key, int value) {
  if (!initialized || !prefs.begin(NAMESPACE, false)) {
    return false;
  }

  bool success = prefs.putInt(key, value) > 0;
  prefs.end();
  return success;
}

int ConfigStorage::loadInt(const char* key, int defaultValue) {
  if (!initialized || !prefs.begin(NAMESPACE, true)) {
    return defaultValue;
  }

  int value = prefs.getInt(key, defaultValue);
  prefs.end();
  return value;
}

bool ConfigStorage::saveBool(const char* key, bool value) {
  if (!initialized || !prefs.begin(NAMESPACE, false)) {
    return false;
  }

  bool success = prefs.putBool(key, value) > 0;
  prefs.end();
  return success;
}

bool ConfigStorage::loadBool(const char* key, bool defaultValue) {
  if (!initialized || !prefs.begin(NAMESPACE, true)) {
    return defaultValue;
  }

  bool value = prefs.getBool(key, defaultValue);
  prefs.end();
  return value;
}

void ConfigStorage::clearAll() {
  if (!initialized) {
    Serial.println("错误: ConfigStorage未初始化");
    return;
  }

  if (!prefs.begin(NAMESPACE, false)) {
    Serial.println("错误: 无法打开Preferences");
    return;
  }

  prefs.clear();
  prefs.end();

  Serial.println("所有配置已清除");
}
