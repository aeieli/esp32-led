# Display Manager 使用示例

本文档展示如何在你的项目中使用模块化的 DisplayManager 类。

## 基础用法

### 1. 初始化显示屏

```cpp
#include "Display.h"

DisplayManager display;

void setup() {
  Serial.begin(115200);
  display.begin();
  display.clear(ST77XX_BLACK);
}
```

### 2. 显示文字

```cpp
// 简单文字
display.drawText("Hello", 10, 20, ST77XX_WHITE, 2);

// 居中文字
display.drawCenteredText("Centered Text", 100, ST77XX_CYAN, 2);

// 文字框
display.drawTextBox(10, 50, 220, 60,
                    "This is a message box",
                    ST77XX_WHITE, ST77XX_BLUE);
```

### 3. 绘制图形

```cpp
// 矩形
display.drawRect(10, 10, 100, 50, ST77XX_RED);
display.fillRect(20, 20, 80, 30, ST77XX_GREEN);

// 圆形
display.drawCircle(120, 120, 30, ST77XX_BLUE);
display.fillCircle(120, 120, 20, ST77XX_YELLOW);

// 线条
display.drawLine(0, 0, 240, 240, ST77XX_WHITE);
```

### 4. 显示图片

```cpp
#include "ExampleImages.h"

// 显示原始大小的图片
display.drawImage(heartImage, 100, 100);

// 缩放显示图片
display.drawImageScaled(heartImage, 50, 50, 64, 64);
```

### 5. 播放动画

```cpp
#include "ExampleImages.h"

void setup() {
  display.begin();
  display.playAnimation(&heartBeatAnimation);
}

void loop() {
  display.updateAnimation();  // 必须在 loop 中调用
  delay(10);
}
```

## 高级功能

### 自定义图片数据

创建你自己的图片数据（RGB565 格式）：

```cpp
// 使用在线工具将图片转换为 RGB565 数组
// 推荐工具：http://www.rinkydinkelectronics.com/t_imageconverter565.php

const uint16_t myIcon[] PROGMEM = {
  // 你的图片数据...
};

ImageData myImage = {
  myIcon,
  32,  // 宽度
  32   // 高度
};

// 使用
display.drawImage(myImage, 50, 50);
```

### 创建自定义动画

```cpp
// 定义动画帧
AnimationFrame myFrames[] = {
  {frame1Data, 100},  // 第1帧，持续100ms
  {frame2Data, 150},  // 第2帧，持续150ms
  {frame3Data, 100},  // 第3帧，持续100ms
};

// 创建动画
Animation myAnimation = {
  myFrames,
  3,      // 帧数
  true    // 是否循环
};

// 播放动画
display.playAnimation(&myAnimation);
```

### 亮度控制和电源管理

```cpp
// 调节亮度 (0-255)
display.setBrightness(128);

// 睡眠模式
display.sleep();

// 唤醒
display.wakeup();
```

### 淡入淡出效果

```cpp
void updateDisplay() {
  display.clear();
  display.drawCenteredText("New Screen", 120, ST77XX_WHITE, 2);
}

// 带淡入淡出效果的屏幕切换
display.fadeTransition(updateDisplay, 500);
```

## 实际应用示例

### 示例 1: 温度显示器

```cpp
#include "Display.h"

DisplayManager display;
float temperature = 25.5;

void setup() {
  display.begin();
  display.clear(ST77XX_BLACK);
}

void loop() {
  // 读取温度传感器
  temperature = readTemperature();

  // 显示温度
  display.fillRect(0, 80, 240, 80, ST77XX_BLACK);  // 清除旧数据
  display.drawCenteredText("Temperature", 60, ST77XX_CYAN, 2);

  char tempStr[10];
  sprintf(tempStr, "%.1f C", temperature);
  display.drawCenteredText(tempStr, 120, ST77XX_YELLOW, 3);

  delay(1000);
}
```

### 示例 2: 状态指示器

```cpp
#include "Display.h"
#include "ExampleImages.h"

DisplayManager display;

enum Status { OK, WARNING, ERROR };
Status currentStatus = OK;

void updateStatusDisplay() {
  display.clear(ST77XX_BLACK);
  display.drawCenteredText("System Status", 30, ST77XX_WHITE, 2);

  switch(currentStatus) {
    case OK:
      display.drawImage(smileImage, 104, 80);  // 居中显示
      display.drawCenteredText("All Good!", 140, ST77XX_GREEN, 2);
      break;

    case WARNING:
      display.fillCircle(120, 96, 20, ST77XX_YELLOW);
      display.drawCenteredText("Warning", 140, ST77XX_YELLOW, 2);
      break;

    case ERROR:
      display.drawImage(heartImage, 112, 88);
      display.drawCenteredText("Error!", 140, ST77XX_RED, 2);
      break;
  }
}

void setup() {
  display.begin();
  updateStatusDisplay();
}

void loop() {
  // 检查系统状态并更新
  Status newStatus = checkSystemStatus();
  if (newStatus != currentStatus) {
    currentStatus = newStatus;
    display.fadeTransition(updateStatusDisplay, 300);
  }

  delay(100);
}
```

### 示例 3: 菜单系统

```cpp
#include "Display.h"

DisplayManager display;
int selectedItem = 0;
const char* menuItems[] = {"Settings", "WiFi", "Display", "About"};
const int menuCount = 4;

void drawMenu() {
  display.clear(ST77XX_BLACK);
  display.drawCenteredText("Menu", 20, ST77XX_CYAN, 2);

  for (int i = 0; i < menuCount; i++) {
    int y = 60 + i * 35;
    uint16_t color = (i == selectedItem) ? ST77XX_YELLOW : ST77XX_WHITE;
    uint16_t boxColor = (i == selectedItem) ? ST77XX_BLUE : ST77XX_BLACK;

    if (i == selectedItem) {
      display.fillRect(5, y - 5, 230, 30, boxColor);
    }

    display.drawText(menuItems[i], 20, y, color, 2);
  }
}

void setup() {
  display.begin();
  drawMenu();
}

void loop() {
  // 按钮控制 (假设已连接按钮)
  if (buttonUpPressed()) {
    selectedItem = (selectedItem - 1 + menuCount) % menuCount;
    drawMenu();
  }

  if (buttonDownPressed()) {
    selectedItem = (selectedItem + 1) % menuCount;
    drawMenu();
  }

  delay(100);
}
```

### 示例 4: 与其他功能结合使用

```cpp
#include "Display.h"
#include <WiFi.h>

DisplayManager display;
bool wifiConnected = false;

void setup() {
  Serial.begin(115200);

  // 初始化显示屏
  display.begin();
  display.drawCenteredText("Connecting WiFi...", 120, ST77XX_YELLOW, 2);

  // 连接 WiFi
  WiFi.begin("SSID", "PASSWORD");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  wifiConnected = true;

  // 更新显示
  display.clear();
  display.drawCenteredText("WiFi Connected!", 100, ST77XX_GREEN, 2);
  display.drawCenteredText(WiFi.localIP().toString().c_str(), 130, ST77XX_WHITE, 1);

  delay(2000);
}

void loop() {
  // 你的主程序逻辑
  // 显示屏可以根据需要随时更新

  if (WiFi.status() != WL_CONNECTED && wifiConnected) {
    wifiConnected = false;
    display.clear();
    display.drawCenteredText("WiFi Disconnected", 120, ST77XX_RED, 2);
  }

  delay(1000);
}
```

## 图片转换工具

要创建自己的图片，你可以使用以下工具：

1. **在线转换器**:
   - http://www.rinkydinkelectronics.com/t_imageconverter565.php
   - https://javl.github.io/image2cpp/

2. **Python 脚本**:
   ```python
   from PIL import Image
   import sys

   def convert_image_to_rgb565(image_path, output_path):
       img = Image.open(image_path).convert('RGB')
       width, height = img.size

       with open(output_path, 'w') as f:
           f.write(f'const uint16_t image{width}x{height}[] PROGMEM = {{\n')
           for y in range(height):
               for x in range(width):
                   r, g, b = img.getpixel((x, y))
                   rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
                   f.write(f'0x{rgb565:04X}, ')
               f.write('\n')
           f.write('};\n')

   # 使用: python convert.py input.png output.h
   convert_image_to_rgb565(sys.argv[1], sys.argv[2])
   ```

## 性能优化建议

1. **减少重绘**: 只更新需要改变的区域
   ```cpp
   display.fillRect(x, y, w, h, ST77XX_BLACK);  // 清除特定区域
   ```

2. **使用 PROGMEM**: 将大型图片数据存储在 Flash 中
   ```cpp
   const uint16_t image[] PROGMEM = { ... };
   ```

3. **批量操作**: 使用 `startWrite()` 和 `endWrite()`
   ```cpp
   Adafruit_ST7789* tft = display.getTFT();
   tft->startWrite();
   // 多个绘图操作
   tft->endWrite();
   ```

## 接线说明

```
ESP32-S3        ST7789 Display
---------       --------------
GPIO 5     -->  CS (片选)
GPIO 15    -->  DC (数据/命令)
GPIO 17    -->  RST (复位)
GPIO 10    -->  SDA/MOSI (数据)
GPIO 11    -->  SCK/SCLK (时钟)
GPIO 16    -->  BL (背光)
3.3V       -->  VCC
GND        -->  GND
```

## 故障排除

1. **屏幕不亮**:
   - 检查背光引脚连接
   - 使用 `display.setBrightness(255)`

2. **显示混乱**:
   - 确认 SPI 引脚配置正确
   - 检查电源供应是否稳定

3. **图片显示不正确**:
   - 确认图片数据格式为 RGB565
   - 检查图片尺寸是否正确

4. **动画卡顿**:
   - 减小图片尺寸
   - 增加帧间隔时间
   - 确保 `updateAnimation()` 在 loop 中被调用
