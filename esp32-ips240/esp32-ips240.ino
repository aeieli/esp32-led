/*
 * ESP32-S3 IPS240 Display Demo with BLE & WiFi
 *
 * 功能：
 * - BLE蓝牙配对和通信
 * - 接收手机指令控制显示
 * - WiFi配网和连接
 * - 自动演示模式
 *
 * 接线：
 * TFT_CS   -> GPIO 5
 * TFT_DC   -> GPIO 15
 * TFT_RST  -> GPIO 17
 * TFT_MOSI -> GPIO 10
 * TFT_SCLK -> GPIO 11
 * TFT_BL   -> GPIO 16
 */

#include "Display.h"
#include "ExampleImages.h"
#include "BLEManager.h"
#include "WiFiManager.h"
#include "ConfigStorage.h"
#include "CommandHandler.h"
#include "SnakeGame.h"
#include "ClockDisplay.h"

// 创建模块实例
DisplayManager display;
BLEManager bleManager;
WiFiManager wifiManager;
ConfigStorage config;
CommandHandler* commandHandler;  // 使用指针，在setup中初始化
SnakeGame* snakeGame;             // 贪吃蛇游戏实例
ClockDisplay* clockDisplay;       // 时钟显示实例

// 演示模式
enum DemoMode {
  MODE_TEXT,
  MODE_IMAGES,
  MODE_ANIMATION,
  MODE_GRAPHICS,
  MODE_SNAKE        // 新增：贪吃蛇模式
};

DemoMode currentMode = MODE_TEXT;  // 初始模式（会在setup中设置）
unsigned long lastModeChange = 0;
const unsigned long MODE_DURATION = 5000;  // 每个模式持续5秒

// 状态变量
bool isManualMode = false;  // 手动控制模式
bool isClockMode = false;   // 时钟模式标志
bool wifiConnected = false;

// 前向声明回调函数
void onBLECommandReceived(String command);
void onWiFiCredentialsReceived(String ssid, String password);

void setup() {
  Serial.begin(115200);
  Serial.println("\n========================================");
  Serial.println("ESP32-S3 Display with BLE & WiFi");
  Serial.println("========================================");

  // 1. 初始化显示屏
  display.begin(BUFFER_MODE_DIRECT, SPI_FREQUENCY_FAST);  // 使用直接模式
  display.printPerformanceInfo();
  Serial.println("开始显示启动界面...");
  showStartupScreen();
  Serial.println("启动界面显示完成");
  delay(1500);

  // 2. 初始化配置存储
  config.begin();

  // 3. 初始化BLE
  bleManager.begin("ESP32-LED");
  bleManager.setCommandCallback(onBLECommandReceived);
  bleManager.setWiFiCredentialsCallback(onWiFiCredentialsReceived);
  showBLEStatus();
  delay(1500);

  // 4. 初始化WiFi
  wifiManager.begin();

  // 检查是否有保存的WiFi配置
  if (config.hasWiFiCredentials()) {
    String ssid, password;
    if (config.loadWiFiCredentials(ssid, password)) {
      showWiFiConnecting(ssid);
      if (wifiManager.connect(ssid, password)) {
        wifiConnected = true;
        showWiFiConnected();
        delay(2000);
      } else {
        showWiFiFailed();
        delay(2000);
      }
    }
  } else {
    showWiFiNotConfigured();
    delay(1500);
  }

  // 5. 初始化时钟显示
  clockDisplay = new ClockDisplay(&display);
  clockDisplay->begin();

  // 6. 初始化指令处理器
  commandHandler = new CommandHandler(&display, &bleManager);
  commandHandler->setClockDisplay(clockDisplay);  // 设置时钟
  commandHandler->begin();

  // 7. 初始化贪吃蛇游戏
  snakeGame = new SnakeGame(&display);
  randomSeed(micros());  // 随机数种子

  // 8. 显示就绪界面
  showReadyScreen();
  delay(1500);

  // 9. 启动第一个演示模式
  currentMode = MODE_TEXT;  // 从文本模式开始
  display.clear();
  showTextDemo();
  lastModeChange = millis();  // 记录启动时间

  Serial.println("系统初始化完成!");
  Serial.println("========================================\n");
}

void loop() {
  unsigned long currentTime = millis();

  // 更新WiFi状态
  wifiManager.update();

  // 只在非手动模式下运行演示
  if (!isManualMode) {
    // 贪吃蛇模式特殊处理：持续运行，不自动切换（DEMO2专属）
    if (currentMode == MODE_SNAKE) {
      snakeGame->update();
    } else {
      // 其他模式每5秒切换一次（DEMO循环：文本→图片→动画→图形）
      if (currentTime - lastModeChange >= MODE_DURATION) {
        lastModeChange = currentTime;

        // 停止当前模式的后台活动
        stopCurrentMode();

        // 切换到下一个模式（循环前4个模式，跳过贪吃蛇）
        currentMode = (DemoMode)((currentMode + 1) % 4);
        display.clear();

        switch (currentMode) {
          case MODE_TEXT:
            showTextDemo();
            break;

          case MODE_IMAGES:
            showImageDemo();
            break;

          case MODE_ANIMATION:
            showAnimationDemo();
            break;

          case MODE_GRAPHICS:
            showGraphicsDemo();
            break;

          default:
            break;
        }
      }

      // 在动画模式下更新动画
      if (currentMode == MODE_ANIMATION) {
        display.updateAnimation();
      }
    }
  }

  // 在时钟模式下更新时钟显示
  if (isClockMode) {
    clockDisplay->update();
  }

  delay(10);
}

// 停止当前模式的后台活动
void stopCurrentMode() {
  switch (currentMode) {
    case MODE_ANIMATION:
      display.stopAnimation();
      break;

    case MODE_SNAKE:
      // 贪吃蛇会在下次 begin() 时自动重置
      break;

    default:
      break;
  }
}

// ========== BLE回调函数 ==========

void onBLECommandReceived(String command) {
  Serial.println("BLE指令: " + command);

  // 检查是否是切换模式指令
  String cmd = command;
  cmd.toUpperCase();

  if (cmd == "MODE:DEMO" || cmd == "DEMO" || cmd == "M:DEMO" || cmd == "D") {
    // 切换回自动演示模式（循环演示）
    isManualMode = false;
    isClockMode = false;  // 退出时钟模式
    lastModeChange = millis();  // 重置计时器
    currentMode = MODE_TEXT;     // 从文本模式开始
    display.stopAnimation();
    display.clear();
    showTextDemo();
    bleManager.sendData("OK:Auto demo mode");
    Serial.println("切换到自动演示模式");
  } else if (cmd == "MODE:DEMO2" || cmd == "DEMO2" || cmd == "M:DEMO2" || cmd == "D2") {
    // 切换到贪吃蛇演示模式
    isManualMode = false;
    isClockMode = false;  // 退出时钟模式
    currentMode = MODE_SNAKE;
    display.stopAnimation();
    display.clear();
    showSnakeDemo();
    bleManager.sendData("OK:Snake game mode");
    Serial.println("切换到贪吃蛇演示模式");
  } else if (cmd == "MODE:CLOCK" || cmd == "CLOCK" || cmd == "M:CLOCK" || cmd == "CL") {
    // 切换到时钟模式
    isManualMode = true;  // 时钟模式不自动切换
    isClockMode = true;   // 进入时钟模式
    display.stopAnimation();
    display.clear();
    clockDisplay->show();
    bleManager.sendData("OK:Clock mode");
    Serial.println("切换到时钟模式");
  } else if (cmd == "MODE:MANUAL" || cmd == "M:MANUAL") {
    // 显式切换到手动模式
    isManualMode = true;
    isClockMode = false;  // 退出时钟模式
    display.stopAnimation();  // 停止可能正在播放的动画
    bleManager.sendData("OK:Manual mode");
    Serial.println("切换到手动模式");
  } else if (!isManualMode) {
    // 收到控制指令，自动切换到手动模式
    isManualMode = true;
    isClockMode = false;  // 退出时钟模式
    display.stopAnimation();  // 停止可能正在播放的动画
    Serial.println("收到控制指令，自动切换到手动模式");
  }

  // 处理指令
  commandHandler->handleCommand(command);
}

void onWiFiCredentialsReceived(String ssid, String password) {
  Serial.println("收到WiFi配网请求");

  // 显示连接界面
  showWiFiConnecting(ssid);

  // 尝试连接
  if (wifiManager.connect(ssid, password)) {
    wifiConnected = true;

    // 保存配置
    config.saveWiFiCredentials(ssid, password);

    // 显示成功界面
    showWiFiConnected();
    bleManager.sendData("WiFi connected: " + wifiManager.getLocalIP());

    delay(3000);
  } else {
    wifiConnected = false;
    showWiFiFailed();
    bleManager.sendData("WiFi connection failed");

    delay(3000);
  }
}

// ========== 演示函数 ==========

void showStartupScreen() {
  display.clear(ST77XX_BLACK);
  display.drawCenteredText("ESP32-S3", 90, ST77XX_CYAN, 3);
  display.drawCenteredText("BLE & WiFi", 120, ST77XX_WHITE, 2);
  display.drawCenteredText("Display System", 145, ST77XX_GREEN, 1);
  display.flush();  // 强制刷新到屏幕
}

void showBLEStatus() {
  display.clear(ST77XX_BLACK);
  display.drawCenteredText("BLE Status", 30, ST77XX_YELLOW, 2);
  display.drawCenteredText("Device: ESP32-LED", 80, ST77XX_WHITE, 1);
  display.drawCenteredText("Status: Advertising", 110, ST77XX_GREEN, 1);
  display.drawCenteredText("Ready to pair", 140, ST77XX_CYAN, 1);
  display.flush();
}

void showWiFiNotConfigured() {
  display.clear(ST77XX_BLACK);
  display.drawCenteredText("WiFi Status", 30, ST77XX_YELLOW, 2);
  display.drawCenteredText("Not Configured", 100, ST77XX_ORANGE, 2);
  display.drawCenteredText("Use BLE to setup", 130, ST77XX_WHITE, 1);
  display.flush();
}

void showWiFiConnecting(String ssid) {
  display.clear(ST77XX_BLACK);
  display.drawCenteredText("Connecting WiFi", 60, ST77XX_YELLOW, 2);
  display.drawCenteredText("SSID:", 100, ST77XX_WHITE, 1);
  display.drawCenteredText(ssid.c_str(), 120, ST77XX_CYAN, 1);
  display.drawCenteredText("Please wait...", 160, ST77XX_WHITE, 1);
  display.flush();
}

void showWiFiConnected() {
  display.clear(ST77XX_BLACK);
  display.drawCenteredText("WiFi Connected!", 50, ST77XX_GREEN, 2);
  display.drawCenteredText("SSID:", 90, ST77XX_WHITE, 1);
  display.drawCenteredText(wifiManager.getSSID().c_str(), 110, ST77XX_CYAN, 1);
  display.drawCenteredText("IP:", 140, ST77XX_WHITE, 1);
  display.drawCenteredText(wifiManager.getLocalIP().c_str(), 160, ST77XX_YELLOW, 1);

  String rssiText = String(wifiManager.getRSSI()) + " dBm";
  display.drawCenteredText(rssiText.c_str(), 190, ST77XX_WHITE, 1);
  display.flush();
}

void showWiFiFailed() {
  display.clear(ST77XX_BLACK);
  display.drawCenteredText("WiFi Failed", 80, ST77XX_RED, 2);
  display.drawCenteredText("Connection timeout", 120, ST77XX_WHITE, 1);
  display.drawCenteredText("Check credentials", 150, ST77XX_ORANGE, 1);
  display.flush();
}

void showReadyScreen() {
  display.clear(ST77XX_BLACK);
  display.drawCenteredText("System Ready!", 70, ST77XX_GREEN, 2);

  // BLE状态
  if (bleManager.isConnected()) {
    display.drawCenteredText("BLE: Connected", 110, ST77XX_CYAN, 1);
  } else {
    display.drawCenteredText("BLE: Waiting", 110, ST77XX_YELLOW, 1);
  }

  // WiFi状态
  if (wifiConnected) {
    display.drawCenteredText("WiFi: Connected", 130, ST77XX_CYAN, 1);
  } else {
    display.drawCenteredText("WiFi: Not connected", 130, ST77XX_ORANGE, 1);
  }

  display.drawCenteredText("Starting demo...", 170, ST77XX_WHITE, 1);
  display.flush();
}

void showTextDemo() {
  display.clear(ST77XX_BLACK);

  // 标题
  display.drawCenteredText("Text Demo", 20, ST77XX_YELLOW, 2);

  // 不同大小的文字
  display.drawText("Size 1 Text", 10, 50, ST77XX_WHITE, 1);
  display.drawText("Size 2 Text", 10, 70, ST77XX_CYAN, 2);
  display.drawText("Size 3", 10, 95, ST77XX_GREEN, 3);

  // 文字框
  display.drawTextBox(10, 140, 220, 50,
                      "Text box with automatic wrapping!",
                      ST77XX_WHITE, ST77XX_BLUE);

  // 底部信息
  display.drawCenteredText("Mode: TEXT", 220, ST77XX_MAGENTA, 1);
  display.flush();
}

void showImageDemo() {
  display.clear(ST77XX_BLACK);

  // 标题
  display.drawCenteredText("Image Demo", 20, ST77XX_YELLOW, 2);

  // 显示心形图标
  display.drawImage(heartImage, 50, 60);
  display.drawText("Heart", 80, 65, ST77XX_WHITE, 1);

  // 显示笑脸图标
  display.drawImage(smileImage, 50, 100);
  display.drawText("Smile", 90, 115, ST77XX_WHITE, 1);

  // 缩放显示
  display.drawText("Scaled:", 10, 160, ST77XX_CYAN, 1);
  display.drawImageScaled(heartImage, 80, 150, 32, 32);

  // 底部信息
  display.drawCenteredText("Mode: IMAGES", 220, ST77XX_MAGENTA, 1);
  display.flush();
}

void showAnimationDemo() {
  display.clear(ST77XX_BLACK);

  // 标题
  display.drawCenteredText("Animation Demo", 20, ST77XX_YELLOW, 2);

  // 显示说明
  display.drawCenteredText("Beating Heart", 50, ST77XX_WHITE, 1);

  // 播放心跳动画（居中显示在屏幕中央）
  display.playAnimation(&heartBeatAnimation);

  // 底部信息
  display.drawCenteredText("Mode: ANIMATION", 220, ST77XX_MAGENTA, 1);
  display.flush();
}

void showGraphicsDemo() {
  display.clear(ST77XX_BLACK);

  // 标题
  display.drawCenteredText("Graphics Demo", 20, ST77XX_YELLOW, 2);

  // 绘制各种图形
  display.drawRect(10, 50, 60, 40, ST77XX_RED);
  display.drawText("Rect", 15, 100, ST77XX_WHITE, 1);

  display.fillRect(80, 50, 60, 40, ST77XX_GREEN);
  display.drawText("Filled", 85, 100, ST77XX_BLACK, 1);

  display.drawCircle(180, 70, 20, ST77XX_BLUE);
  display.drawText("Circle", 160, 100, ST77XX_WHITE, 1);

  display.fillCircle(35, 140, 15, ST77XX_MAGENTA);
  display.fillCircle(100, 140, 15, ST77XX_CYAN);
  display.fillCircle(165, 140, 15, ST77XX_YELLOW);

  // 绘制线条
  for (int i = 0; i < 5; i++) {
    display.drawLine(10 + i * 45, 180, 40 + i * 45, 200, ST77XX_WHITE);
  }

  // 底部信息
  display.drawCenteredText("Mode: GRAPHICS", 220, ST77XX_MAGENTA, 1);
  display.flush();
}

void showSnakeDemo() {
  // 贪吃蛇游戏演示
  snakeGame->begin();
  Serial.println("贪吃蛇游戏开始!");
}