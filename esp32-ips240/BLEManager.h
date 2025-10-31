#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE服务和特征值UUID定义
#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_COMMAND_UUID      "beb5483e-36e1-4688-b7f5-ea07361b26a8"  // 接收指令
#define CHAR_DATA_UUID         "beb5483e-36e1-4688-b7f5-ea07361b26a9"  // 发送数据
#define CHAR_WIFI_SSID_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26aa"  // WiFi SSID
#define CHAR_WIFI_PWD_UUID     "beb5483e-36e1-4688-b7f5-ea07361b26ab"  // WiFi密码
#define CHAR_STATUS_UUID       "beb5483e-36e1-4688-b7f5-ea07361b26ac"  // 状态

// 回调函数类型定义
typedef void (*CommandCallback)(String command);
typedef void (*WiFiCredentialsCallback)(String ssid, String password);

class BLEManager {
public:
  BLEManager();

  // 初始化和控制
  void begin(const char* deviceName = "ESP32-LED");
  void stop();
  void startAdvertising();
  void stopAdvertising();

  // 状态查询
  bool isConnected();
  bool isAdvertising();
  uint32_t getConnectedDeviceCount();
  String getDeviceName();

  // 数据发送
  bool sendData(const String& data);
  bool sendData(const uint8_t* data, size_t length);
  bool updateStatus(const String& status);

  // 回调函数设置
  void setCommandCallback(CommandCallback callback);
  void setWiFiCredentialsCallback(WiFiCredentialsCallback callback);

  // 内部使用的服务器回调类（需要访问私有成员）
  friend class MyServerCallbacks;
  friend class MyCommandCallbacks;
  friend class MyWiFiSSIDCallbacks;
  friend class MyWiFiPasswordCallbacks;

private:
  BLEServer* pServer;
  BLEService* pService;
  BLECharacteristic* pCharCommand;
  BLECharacteristic* pCharData;
  BLECharacteristic* pCharWiFiSSID;
  BLECharacteristic* pCharWiFiPassword;
  BLECharacteristic* pCharStatus;

  String deviceName;
  bool deviceConnected;
  bool oldDeviceConnected;
  uint32_t connectedCount;

  // 回调函数
  CommandCallback commandCallback;
  WiFiCredentialsCallback wifiCallback;

  // WiFi凭证临时存储
  String receivedSSID;
  String receivedPassword;
  bool ssidReceived;
  bool passwordReceived;

  // 内部方法
  void setupService();
  void handleConnection();
  void handleDisconnection();
  void handleCommandReceived(String command);
  void handleWiFiSSIDReceived(String ssid);
  void handleWiFiPasswordReceived(String password);
  void checkWiFiCredentials();
};

// BLE服务器连接回调
class MyServerCallbacks: public BLEServerCallbacks {
public:
  MyServerCallbacks(BLEManager* manager) : bleManager(manager) {}

  void onConnect(BLEServer* pServer) {
    bleManager->handleConnection();
  }

  void onDisconnect(BLEServer* pServer) {
    bleManager->handleDisconnection();
  }

private:
  BLEManager* bleManager;
};

// 指令接收回调
class MyCommandCallbacks: public BLECharacteristicCallbacks {
public:
  MyCommandCallbacks(BLEManager* manager) : bleManager(manager) {}

  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = String(pCharacteristic->getValue().c_str());
    if (value.length() > 0) {
      bleManager->handleCommandReceived(value);
    }
  }

private:
  BLEManager* bleManager;
};

// WiFi SSID接收回调
class MyWiFiSSIDCallbacks: public BLECharacteristicCallbacks {
public:
  MyWiFiSSIDCallbacks(BLEManager* manager) : bleManager(manager) {}

  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = String(pCharacteristic->getValue().c_str());
    if (value.length() > 0) {
      bleManager->handleWiFiSSIDReceived(value);
    }
  }

private:
  BLEManager* bleManager;
};

// WiFi密码接收回调
class MyWiFiPasswordCallbacks: public BLECharacteristicCallbacks {
public:
  MyWiFiPasswordCallbacks(BLEManager* manager) : bleManager(manager) {}

  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = String(pCharacteristic->getValue().c_str());
    if (value.length() > 0) {
      bleManager->handleWiFiPasswordReceived(value);
    }
  }

private:
  BLEManager* bleManager;
};

#endif // BLE_MANAGER_H
