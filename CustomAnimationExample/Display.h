#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include "FrameBuffer.h"

// 显示屏配置
#define TFT_CS    5     // 片选
#define TFT_DC    15    // 数据/命令选择
#define TFT_RST   17    // 复位
#define TFT_MOSI  10    // SDA (数据)
#define TFT_SCLK  11    // SCK (时钟)
#define TFT_BL    16    // 背光
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240

// SPI 速度配置 (Hz)
#define SPI_FREQUENCY_DEFAULT  40000000  // 40 MHz (默认)
#define SPI_FREQUENCY_FAST     80000000  // 80 MHz (高速)

// 图片数据结构
struct ImageData {
  const uint16_t* data;  // RGB565 格式的图片数据
  uint16_t width;
  uint16_t height;
};

// 动画帧结构
struct AnimationFrame {
  const uint16_t* data;
  uint16_t width;      // 帧宽度（新增）
  uint16_t height;     // 帧高度（新增）
  uint16_t duration;   // 帧持续时间(ms)
};

// 动画数据结构
struct Animation {
  AnimationFrame* frames;
  uint8_t frameCount;
  int16_t x;           // 动画显示位置X（新增，-1表示居中）
  int16_t y;           // 动画显示位置Y（新增，-1表示居中）
  bool loop;           // 是否循环播放
  bool clearBackground; // 每帧是否清除背景（新增）
};

// 显示管理类
class DisplayManager {
private:
  SPIClass* spi;
  Adafruit_ST7789* tft;
  FrameBuffer* frameBuffer;

  // 动画状态
  Animation* currentAnimation;
  uint8_t currentFrame;
  unsigned long lastFrameTime;
  bool animationPlaying;

  void renderAnimationFrame(bool forceClear, unsigned long timestamp);

  // 配置
  uint32_t spiFrequency;
  bool autoFlush;  // 自动刷新模式

public:
  DisplayManager();
  ~DisplayManager();

  // 初始化
  void begin(BufferMode bufferMode = BUFFER_MODE_SINGLE,
             uint32_t spiFreq = SPI_FREQUENCY_DEFAULT);

  // 基础功能
  void clear(uint16_t color = ST77XX_BLACK);
  void setBrightness(uint8_t level);  // 0-255
  void sleep();
  void wakeup();

  // 文字显示
  void drawText(const char* text, int16_t x, int16_t y,
                uint16_t color = ST77XX_WHITE, uint8_t size = 1);
  void drawCenteredText(const char* text, int16_t y,
                        uint16_t color = ST77XX_WHITE, uint8_t size = 2);
  void drawTextBox(int16_t x, int16_t y, int16_t w, int16_t h,
                   const char* text, uint16_t textColor, uint16_t boxColor);

  // 图形绘制
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
  void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

  // 图片显示
  void drawImage(const ImageData& img, int16_t x, int16_t y);
  void drawImageScaled(const ImageData& img, int16_t x, int16_t y,
                       uint16_t newWidth, uint16_t newHeight);

  // 动画控制
  void playAnimation(Animation* anim);
  void stopAnimation();
  void updateAnimation();  // 在 loop 中调用
  bool isAnimationPlaying();

  // 特效
  void fadeTransition(void (*drawFunc)(), uint16_t duration = 500);
  void scrollText(const char* text, int16_t y, int16_t speed = 2,
                  uint16_t color = ST77XX_WHITE, uint8_t size = 2);

  // 缓冲控制（新增）
  void flush();                    // 刷新所有脏区域到屏幕
  void flushImmediate();           // 立即刷新全屏
  void setAutoFlush(bool enabled); // 设置自动刷新模式
  bool getAutoFlush() const { return autoFlush; }

  // 性能信息
  void printPerformanceInfo();
  size_t getBufferMemoryUsage();

  // 直接访问底层对象（高级功能）
  Adafruit_ST7789* getTFT() { return tft; }
  FrameBuffer* getFrameBuffer() { return frameBuffer; }
};

#endif
