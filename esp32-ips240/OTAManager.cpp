#include "OTAManager.h"

OTAManager::OTAManager() {
  status = OTA_IDLE;
  progress = 0;
  progressCallback = nullptr;
  errorCallback = nullptr;
}

void OTAManager::begin(const char* hostname, const char* password) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("警告: WiFi未连接，OTA功能将不可用");
    status = OTA_NO_WIFI;
    return;
  }

  setupArduinoOTA(hostname, password);
  Serial.println("OTA已启动");
  Serial.printf("设备名称: %s\n", hostname);
  Serial.printf("IP地址: %s\n", WiFi.localIP().toString().c_str());
}

void OTAManager::setupArduinoOTA(const char* hostname, const char* password) {
  // 设置主机名
  ArduinoOTA.setHostname(hostname);

  // 设置密码（可选，为空则不需要密码）
  if (strlen(password) > 0) {
    ArduinoOTA.setPassword(password);
  }

  // 设置端口（默认3232）
  ArduinoOTA.setPort(3232);

  // OTA开始回调
  ArduinoOTA.onStart([this]() {
    onOTAStart();
  });

  // OTA结束回调
  ArduinoOTA.onEnd([this]() {
    onOTAEnd();
  });

  // OTA进度回调
  ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
    onOTAProgress(progress, total);
  });

  // OTA错误回调
  ArduinoOTA.onError([this](ota_error_t error) {
    onOTAError(error);
  });

  ArduinoOTA.begin();
}

void OTAManager::handle() {
  if (WiFi.status() != WL_CONNECTED) {
    if (status != OTA_NO_WIFI) {
      status = OTA_NO_WIFI;
    }
    return;
  }

  // 处理Arduino OTA请求
  ArduinoOTA.handle();
}

bool OTAManager::updateFromURL(const char* url) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("错误: WiFi未连接");
    status = OTA_NO_WIFI;
    return false;
  }

  Serial.println("开始HTTP OTA更新...");
  Serial.printf("URL: %s\n", url);

  status = OTA_UPDATING;
  progress = 0;

  // 配置HTTP更新
  httpUpdate.setLedPin(LED_BUILTIN, LOW);

  // 设置进度回调
  httpUpdate.onStart([this]() {
    Serial.println("[HTTP] OTA开始");
    if (progressCallback) {
      progressCallback(0, 100);
    }
  });

  httpUpdate.onEnd([this]() {
    Serial.println("\n[HTTP] OTA完成");
    if (progressCallback) {
      progressCallback(100, 100);
    }
  });

  httpUpdate.onProgress([this](int cur, int total) {
    int percent = (cur * 100) / total;
    progress = percent;
    Serial.printf("[HTTP] 进度: %d%%\n", percent);
    if (progressCallback) {
      progressCallback(cur, total);
    }
  });

  httpUpdate.onError([this](int err) {
    Serial.printf("[HTTP] 错误: %d\n", err);
  });

  // 执行更新
  WiFiClient client;
  t_httpUpdate_return ret = httpUpdate.update(client, url);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP更新失败 错误 (%d): %s\n",
                    httpUpdate.getLastError(),
                    httpUpdate.getLastErrorString().c_str());
      status = OTA_FAILED;
      return false;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP更新: 无可用更新");
      status = OTA_IDLE;
      return false;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP更新成功，重启中...");
      status = OTA_SUCCESS;
      return true;
  }

  return false;
}

void OTAManager::setProgressCallback(OTAProgressCallback callback) {
  progressCallback = callback;
}

void OTAManager::setErrorCallback(OTAErrorCallback callback) {
  errorCallback = callback;
}

OTAStatus OTAManager::getStatus() {
  return status;
}

String OTAManager::getStatusString() {
  switch (status) {
    case OTA_IDLE:      return "IDLE";
    case OTA_UPDATING:  return "UPDATING";
    case OTA_SUCCESS:   return "SUCCESS";
    case OTA_FAILED:    return "FAILED";
    case OTA_NO_WIFI:   return "NO_WIFI";
    default:            return "UNKNOWN";
  }
}

int OTAManager::getProgress() {
  return progress;
}

void OTAManager::onOTAStart() {
  String type;
  if (ArduinoOTA.getCommand() == U_FLASH) {
    type = "sketch";
  } else {  // U_SPIFFS
    type = "filesystem";
  }

  Serial.println("[Arduino OTA] 开始更新 " + type);
  status = OTA_UPDATING;
  progress = 0;

  if (progressCallback) {
    progressCallback(0, 100);
  }
}

void OTAManager::onOTAEnd() {
  Serial.println("\n[Arduino OTA] 更新完成");
  status = OTA_SUCCESS;
  progress = 100;

  if (progressCallback) {
    progressCallback(100, 100);
  }
}

void OTAManager::onOTAProgress(unsigned int progress, unsigned int total) {
  int percent = (progress * 100) / total;
  this->progress = percent;

  Serial.printf("[Arduino OTA] 进度: %u%%\r", percent);

  if (progressCallback) {
    progressCallback(progress, total);
  }
}

void OTAManager::onOTAError(ota_error_t error) {
  Serial.printf("[Arduino OTA] 错误[%u]: ", error);

  switch (error) {
    case OTA_AUTH_ERROR:
      Serial.println("认证失败");
      break;
    case OTA_BEGIN_ERROR:
      Serial.println("开始失败");
      break;
    case OTA_CONNECT_ERROR:
      Serial.println("连接失败");
      break;
    case OTA_RECEIVE_ERROR:
      Serial.println("接收失败");
      break;
    case OTA_END_ERROR:
      Serial.println("结束失败");
      break;
  }

  status = OTA_FAILED;

  if (errorCallback) {
    errorCallback(error);
  }
}
