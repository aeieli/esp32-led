#include "WiFiManager.h"

WiFiManager::WiFiManager() {
  status = WIFI_STATUS_IDLE;
  connectStartTime = 0;
  connectTimeout = 15000;
  autoReconnect = true;
  connectedCallback = nullptr;
  disconnectedCallback = nullptr;
}

void WiFiManager::begin() {
  WiFi.mode(WIFI_STA);
  Serial.println("WiFi管理器已初始化");
}

bool WiFiManager::connect(const String& ssid, const String& password, uint32_t timeoutMs) {
  if (ssid.length() == 0) {
    Serial.println("错误: SSID为空");
    return false;
  }

  currentSSID = ssid;
  currentPassword = password;
  connectTimeout = timeoutMs;

  Serial.println("开始连接WiFi...");
  Serial.println("SSID: " + ssid);

  // 断开现有连接
  WiFi.disconnect();
  delay(100);

  // 开始连接
  WiFi.begin(ssid.c_str(), password.c_str());
  status = WIFI_STATUS_CONNECTING;
  connectStartTime = millis();

  // 等待连接或超时
  while (status == WIFI_STATUS_CONNECTING) {
    updateStatus();

    if (WiFi.status() == WL_CONNECTED) {
      status = WIFI_STATUS_CONNECTED;
      Serial.println("WiFi连接成功!");
      Serial.println("IP地址: " + WiFi.localIP().toString());
      Serial.println("信号强度: " + String(WiFi.RSSI()) + " dBm");

      if (connectedCallback) {
        connectedCallback();
      }
      return true;
    }

    if (millis() - connectStartTime > connectTimeout) {
      status = WIFI_STATUS_FAILED;
      Serial.println("WiFi连接超时");
      return false;
    }

    delay(100);
  }

  return false;
}

void WiFiManager::disconnect() {
  WiFi.disconnect();
  status = WIFI_STATUS_DISCONNECTED;
  Serial.println("WiFi已断开");

  if (disconnectedCallback) {
    disconnectedCallback();
  }
}

void WiFiManager::reconnect() {
  if (currentSSID.length() > 0) {
    Serial.println("尝试重新连接WiFi...");
    connect(currentSSID, currentPassword, connectTimeout);
  } else {
    Serial.println("错误: 没有保存的WiFi凭证");
  }
}

WiFiConnectionStatus WiFiManager::getStatus() {
  return status;
}

bool WiFiManager::isConnected() {
  return (status == WIFI_STATUS_CONNECTED && WiFi.status() == WL_CONNECTED);
}

String WiFiManager::getSSID() {
  if (isConnected()) {
    return WiFi.SSID();
  }
  return currentSSID;
}

String WiFiManager::getLocalIP() {
  if (isConnected()) {
    return WiFi.localIP().toString();
  }
  return "0.0.0.0";
}

int32_t WiFiManager::getRSSI() {
  if (isConnected()) {
    return WiFi.RSSI();
  }
  return 0;
}

String WiFiManager::getMacAddress() {
  return WiFi.macAddress();
}

void WiFiManager::setConnectedCallback(WiFiConnectedCallback callback) {
  connectedCallback = callback;
}

void WiFiManager::setDisconnectedCallback(WiFiDisconnectedCallback callback) {
  disconnectedCallback = callback;
}

void WiFiManager::update() {
  updateStatus();
}

void WiFiManager::updateStatus() {
  wl_status_t wifiStatus = WiFi.status();

  // 检测连接状态变化
  if (status == WIFI_STATUS_CONNECTED && wifiStatus != WL_CONNECTED) {
    // 连接丢失
    status = WIFI_STATUS_DISCONNECTED;
    Serial.println("WiFi连接丢失");

    if (disconnectedCallback) {
      disconnectedCallback();
    }

    // 自动重连
    if (autoReconnect && currentSSID.length() > 0) {
      Serial.println("5秒后尝试重新连接...");
      delay(5000);
      reconnect();
    }
  }
}
