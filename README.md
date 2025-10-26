# ESP32-S3 IPS240 显示屏模块化驱动

这是一个为 ESP32-S3 设计的模块化显示屏驱动库，支持 ST7789 驱动的 240x240 IPS 显示屏。该库提供了简洁易用的 API，支持文字、图片、动画等多种显示方式，适合与其他功能模块一起使用。

## 特性

- ✅ **模块化设计**: 封装良好的 DisplayManager 类，易于集成到任何项目
- ✅ **丰富的功能**:
  - 文字显示（支持多种大小、颜色、居中对齐）
  - 图形绘制（矩形、圆形、线条等）
  - 图片显示（支持缩放）
  - 动画播放（支持多帧动画和循环播放）
  - 特效（淡入淡出、滚动文字等）
- ✅ **易于使用**: 简单直观的 API 设计
- ✅ **高性能**: 优化的绘图算法
- ✅ **完整文档**: 详细的使用示例和说明

## 硬件接线

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

## 项目结构

```
esp32-ips240/
├── esp32-ips240.ino      # 主程序演示文件
├── Display.h             # 显示管理类头文件
├── Display.cpp           # 显示管理类实现
├── ExampleImages.h       # 示例图片数据
├── USAGE_EXAMPLES.md     # 详细使用示例
└── README.md             # 本文件
```

## 快速开始

### 1. 安装依赖库

在 Arduino IDE 中安装以下库：
- Adafruit GFX Library
- Adafruit ST7789 Library

### 2. 基本使用

```cpp
#include "Display.h"

DisplayManager display;

void setup() {
  display.begin();
  display.clear(ST77XX_BLACK);
  display.drawCenteredText("Hello ESP32!", 120, ST77XX_CYAN, 2);
}

void loop() {
  // 你的代码
}
```

### 3. 上传并运行

1. 选择开发板: ESP32S3 Dev Module
2. 配置正确的端口
3. 上传代码

## 功能演示

主程序 (`esp32-ips240.ino`) 包含 4 个演示模式，每 5 秒自动切换：

1. **文字演示**: 展示不同大小的文字和文字框
2. **图片演示**: 展示图标显示和缩放功能
3. **动画演示**: 展示简单的心跳动画
4. **图形演示**: 展示各种图形绘制功能

## API 参考

### 初始化

```cpp
DisplayManager display;
display.begin();
```

### 基础功能

```cpp
display.clear(color);              // 清屏
display.setBrightness(level);      // 设置亮度 (0-255)
display.sleep();                   // 睡眠
display.wakeup();                  // 唤醒
```

### 文字显示

```cpp
display.drawText(text, x, y, color, size);
display.drawCenteredText(text, y, color, size);
display.drawTextBox(x, y, w, h, text, textColor, boxColor);
```

### 图形绘制

```cpp
display.drawRect(x, y, w, h, color);
display.fillRect(x, y, w, h, color);
display.drawCircle(x, y, r, color);
display.fillCircle(x, y, r, color);
display.drawLine(x0, y0, x1, y1, color);
```

### 图片显示

```cpp
display.drawImage(imageData, x, y);
display.drawImageScaled(imageData, x, y, newWidth, newHeight);
```

### 动画控制

```cpp
display.playAnimation(animation);
display.stopAnimation();
display.updateAnimation();  // 在 loop() 中调用
```

## 实际应用示例

查看 `USAGE_EXAMPLES.md` 获取详细的使用示例，包括：

- 温度显示器
- 状态指示器
- 菜单系统
- WiFi 连接显示
- 自定义图片和动画

## 自定义图片

### 使用在线工具转换

1. 访问 http://www.rinkydinkelectronics.com/t_imageconverter565.php
2. 上传你的图片
3. 选择 RGB565 格式
4. 下载生成的代码

### 在代码中使用

```cpp
const uint16_t myImage[] PROGMEM = {
  // 粘贴转换后的数据
};

ImageData myImageData = {
  myImage,
  32,  // 宽度
  32   // 高度
};

display.drawImage(myImageData, 100, 100);
```

## 性能优化

1. 使用 `PROGMEM` 存储大型图片数据
2. 避免频繁全屏刷新，只更新需要改变的区域
3. 对于大图片，考虑压缩或分块加载

## 与其他功能集成

DisplayManager 设计为非阻塞式，可以轻松与其他功能一起使用：

```cpp
#include "Display.h"
#include <WiFi.h>

DisplayManager display;

void setup() {
  display.begin();

  // WiFi 初始化
  WiFi.begin("SSID", "PASSWORD");

  // 传感器初始化
  initSensors();

  // 其他功能...
}

void loop() {
  // 读取传感器
  float temp = readTemperature();

  // 更新显示
  char buf[20];
  sprintf(buf, "Temp: %.1fC", temp);
  display.fillRect(0, 100, 240, 40, ST77XX_BLACK);
  display.drawCenteredText(buf, 120, ST77XX_YELLOW, 2);

  // 更新动画（如果在播放）
  display.updateAnimation();

  // 其他任务...
  delay(100);
}
```

## 故障排除

### 屏幕不亮
- 检查接线是否正确
- 确认背光引脚已连接到 GPIO 16
- 尝试调用 `display.setBrightness(255)`

### 显示混乱
- 确认 SPI 引脚配置正确
- 检查电源是否稳定（需要足够的电流）
- 尝试降低 SPI 时钟速度

### 编译错误
- 确保已安装 Adafruit GFX 和 ST7789 库
- 检查 Arduino IDE 是否选择了正确的开发板

## 许可证

本项目采用 MIT 许可证。

## 贡献

欢迎提交问题和改进建议！

## 更新日志

### v1.0.0 (2025-01-XX)
- 初始版本
- 支持文字、图片、动画显示
- 模块化设计
- 完整文档

## 相关资源

- [Adafruit GFX Graphics Library](https://github.com/adafruit/Adafruit-GFX-Library)
- [Adafruit ST7789 Library](https://github.com/adafruit/Adafruit-ST7735-Library)
- [ESP32-S3 文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)

## 联系方式

如有问题或建议，请提交 Issue。
