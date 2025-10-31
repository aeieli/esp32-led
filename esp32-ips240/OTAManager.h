#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <HTTPUpdate.h>
#include <WiFi.h>

// OTA状态枚举
enum OTAStatus {
  OTA_IDLE,           // 空闲
  OTA_UPDATING,       // 更新中
  OTA_SUCCESS,        // 更新成功
  OTA_FAILED,         // 更新失败
  OTA_NO_WIFI         // WiFi未连接
};

// 回调函数类型
typedef void (*OTAProgressCallback)(unsigned int progress, unsigned int total);
typedef void (*OTAErrorCallback)(ota_error_t error);

class OTAManager {
public:
  OTAManager();

  // 初始化（Arduino OTA）
  void begin(const char* hostname = "ESP32-LED", const char* password = "");

  // 更新处理（在loop中调用）
  void handle();

  // HTTP OTA更新
  bool updateFromURL(const char* url);

  // 设置回调
  void setProgressCallback(OTAProgressCallback callback);
  void setErrorCallback(OTAErrorCallback callback);

  // 状态查询
  OTAStatus getStatus();
  String getStatusString();
  int getProgress();

private:
  OTAStatus status;
  int progress;
  OTAProgressCallback progressCallback;
  OTAErrorCallback errorCallback;

  void setupArduinoOTA(const char* hostname, const char* password);
  void onOTAStart();
  void onOTAEnd();
  void onOTAProgress(unsigned int progress, unsigned int total);
  void onOTAError(ota_error_t error);
};

#endif // OTA_MANAGER_H
