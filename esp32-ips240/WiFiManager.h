#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

// WiFi连接状态枚举
enum WiFiConnectionStatus {
  WIFI_STATUS_IDLE,          // 空闲状态
  WIFI_STATUS_CONNECTING,    // 连接中
  WIFI_STATUS_CONNECTED,     // 已连接
  WIFI_STATUS_FAILED,        // 连接失败
  WIFI_STATUS_DISCONNECTED   // 已断开
};

// 回调函数类型
typedef void (*WiFiConnectedCallback)();
typedef void (*WiFiDisconnectedCallback)();

class WiFiManager {
public:
  WiFiManager();

  // 初始化
  void begin();

  // WiFi连接控制
  bool connect(const String& ssid, const String& password, uint32_t timeoutMs = 15000);
  void disconnect();
  void reconnect();

  // 状态查询
  WiFiConnectionStatus getStatus();
  bool isConnected();
  String getSSID();
  String getLocalIP();
  int32_t getRSSI();
  String getMacAddress();

  // 回调设置
  void setConnectedCallback(WiFiConnectedCallback callback);
  void setDisconnectedCallback(WiFiDisconnectedCallback callback);

  // 更新方法（在loop中调用）
  void update();

private:
  WiFiConnectionStatus status;
  String currentSSID;
  String currentPassword;
  uint32_t connectStartTime;
  uint32_t connectTimeout;
  bool autoReconnect;

  WiFiConnectedCallback connectedCallback;
  WiFiDisconnectedCallback disconnectedCallback;

  void handleWiFiEvent();
  void updateStatus();
};

#endif // WIFI_MANAGER_H
