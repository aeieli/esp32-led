#include "BLEManager.h"

BLEManager::BLEManager() {
  pServer = nullptr;
  pService = nullptr;
  pCharCommand = nullptr;
  pCharData = nullptr;
  pCharWiFiSSID = nullptr;
  pCharWiFiPassword = nullptr;
  pCharStatus = nullptr;

  deviceConnected = false;
  oldDeviceConnected = false;
  connectedCount = 0;

  commandCallback = nullptr;
  wifiCallback = nullptr;

  ssidReceived = false;
  passwordReceived = false;
}

void BLEManager::begin(const char* deviceName) {
  this->deviceName = String(deviceName);

  Serial.println("初始化BLE设备...");

  // 创建BLE设备
  BLEDevice::init(deviceName);

  // 创建BLE服务器
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks(this));

  // 设置服务
  setupService();

  // 开始广播
  startAdvertising();

  Serial.println("BLE设备已启动，设备名称: " + this->deviceName);
  Serial.println("等待手机连接...");
}

void BLEManager::setupService() {
  // 创建BLE服务
  pService = pServer->createService(SERVICE_UUID);

  // 创建指令特征值 (Write)
  pCharCommand = pService->createCharacteristic(
    CHAR_COMMAND_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pCharCommand->setCallbacks(new MyCommandCallbacks(this));

  // 创建数据特征值 (Notify)
  pCharData = pService->createCharacteristic(
    CHAR_DATA_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharData->addDescriptor(new BLE2902());

  // 创建WiFi SSID特征值 (Write)
  pCharWiFiSSID = pService->createCharacteristic(
    CHAR_WIFI_SSID_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pCharWiFiSSID->setCallbacks(new MyWiFiSSIDCallbacks(this));

  // 创建WiFi密码特征值 (Write)
  pCharWiFiPassword = pService->createCharacteristic(
    CHAR_WIFI_PWD_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pCharWiFiPassword->setCallbacks(new MyWiFiPasswordCallbacks(this));

  // 创建状态特征值 (Read + Notify)
  pCharStatus = pService->createCharacteristic(
    CHAR_STATUS_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharStatus->addDescriptor(new BLE2902());
  pCharStatus->setValue("ready");

  // 启动服务
  pService->start();
}

void BLEManager::startAdvertising() {
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("开始BLE广播");
}

void BLEManager::stopAdvertising() {
  BLEDevice::stopAdvertising();
  Serial.println("停止BLE广播");
}

void BLEManager::stop() {
  if (pServer) {
    stopAdvertising();
    pServer->disconnect(pServer->getConnId());
  }
  BLEDevice::deinit(true);
  Serial.println("BLE设备已关闭");
}

bool BLEManager::isConnected() {
  return deviceConnected;
}

bool BLEManager::isAdvertising() {
  return BLEDevice::getAdvertising()->isAdvertising();
}

uint32_t BLEManager::getConnectedDeviceCount() {
  return connectedCount;
}

String BLEManager::getDeviceName() {
  return deviceName;
}

bool BLEManager::sendData(const String& data) {
  if (!deviceConnected || !pCharData) {
    return false;
  }

  pCharData->setValue(data.c_str());
  pCharData->notify();
  return true;
}

bool BLEManager::sendData(const uint8_t* data, size_t length) {
  if (!deviceConnected || !pCharData) {
    return false;
  }

  pCharData->setValue((uint8_t*)data, length);
  pCharData->notify();
  return true;
}

bool BLEManager::updateStatus(const String& status) {
  if (!pCharStatus) {
    return false;
  }

  pCharStatus->setValue(status.c_str());
  if (deviceConnected) {
    pCharStatus->notify();
  }
  return true;
}

void BLEManager::setCommandCallback(CommandCallback callback) {
  commandCallback = callback;
}

void BLEManager::setWiFiCredentialsCallback(WiFiCredentialsCallback callback) {
  wifiCallback = callback;
}

void BLEManager::handleConnection() {
  deviceConnected = true;
  connectedCount++;
  Serial.println("设备已连接");
  updateStatus("connected");
}

void BLEManager::handleDisconnection() {
  deviceConnected = false;
  Serial.println("设备已断开连接");

  // 断开后重新开始广播
  delay(500);
  startAdvertising();
  updateStatus("ready");
}

void BLEManager::handleCommandReceived(String command) {
  Serial.println("收到指令: " + command);

  // 发送确认消息
  sendData("ACK:" + command);

  // 调用用户设置的回调函数
  if (commandCallback) {
    commandCallback(command);
  }
}

void BLEManager::handleWiFiSSIDReceived(String ssid) {
  receivedSSID = ssid;
  ssidReceived = true;
  Serial.println("收到WiFi SSID: " + ssid);

  // 检查是否同时收到SSID和密码
  checkWiFiCredentials();
}

void BLEManager::handleWiFiPasswordReceived(String password) {
  receivedPassword = password;
  passwordReceived = true;
  Serial.println("收到WiFi密码: " + String(password.length()) + "字符");

  // 检查是否同时收到SSID和密码
  checkWiFiCredentials();
}

void BLEManager::checkWiFiCredentials() {
  // 当SSID和密码都收到时，触发WiFi配网回调
  if (ssidReceived && passwordReceived && wifiCallback) {
    Serial.println("WiFi凭证接收完成，触发配网流程");
    wifiCallback(receivedSSID, receivedPassword);

    // 重置标志
    ssidReceived = false;
    passwordReceived = false;
  }
}
