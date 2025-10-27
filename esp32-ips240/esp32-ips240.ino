/*
 * ESP32-S3 IPS240 Display Demo
 *
 * 使用模块化的 DisplayManager 类来管理显示屏
 * 支持文字、图片、动画等多种显示方式
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

// 创建显示管理器实例
DisplayManager display;

// 演示模式
enum DemoMode {
  MODE_TEXT,
  MODE_IMAGES,
  MODE_ANIMATION,
  MODE_GRAPHICS
};

DemoMode currentMode = MODE_ANIMATION;
unsigned long lastModeChange = 0;
const unsigned long MODE_DURATION = 5000;  // 每个模式持续5秒

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32-S3 Display Demo Starting...");

  // 初始化显示屏（使用单缓冲 + 高速SPI，解决动画花屏问题）
  display.begin(BUFFER_MODE_SINGLE, SPI_FREQUENCY_FAST);

  // 显示性能信息
  display.printPerformanceInfo();

  // 显示启动画面
  showStartupScreen();
  delay(2000);
}

void loop() {
  unsigned long currentTime = millis();

  // 每5秒切换一次演示模式
  if (currentTime - lastModeChange >= MODE_DURATION) {
    lastModeChange = currentTime;
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
    }
  }

  // 更新动画（如果正在播放）
  display.updateAnimation();

  delay(10);
}

// ========== 演示函数 ==========

void showStartupScreen() {
  display.clear(ST77XX_BLACK);
  display.drawCenteredText("ESP32-S3", 100, ST77XX_CYAN, 3);
  display.drawCenteredText("Display System", 130, ST77XX_WHITE, 2);
  display.drawCenteredText("Modular Design", 160, ST77XX_GREEN, 1);
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
}

void showAnimationDemo() {
  display.clear(ST77XX_BLACK);

  // 标题
  display.drawCenteredText("Animation Demo", 20, ST77XX_YELLOW, 2);

  // 显示说明
  display.drawCenteredText("Beating Heart", 100, ST77XX_WHITE, 1);

  // 播放心跳动画
  display.playAnimation(&heartBeatAnimation);

  // 底部信息
  display.drawCenteredText("Mode: ANIMATION", 220, ST77XX_MAGENTA, 1);
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
}