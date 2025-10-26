#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// 显示屏配置
#define TFT_CS    5     // 片选
#define TFT_DC    15    // 数据/命令选择
#define TFT_RST   17    // 复位
#define TFT_MOSI  10    // SDA (数据)
#define TFT_SCLK  11    // SCK (时钟)
#define TFT_BL    16    // 背光
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240

// 图片数据结构
struct ImageData {
  const uint16_t* data;  // RGB565 格式的图片数据
  uint16_t width;
  uint16_t height;
};

// 动画帧结构
struct AnimationFrame {
  const uint16_t* data;
  uint16_t duration;  // 帧持续时间(ms)
};

// 动画数据结构
struct Animation {
  AnimationFrame* frames;
  uint8_t frameCount;
  bool loop;  // 是否循环播放
};

// 显示管理类
class DisplayManager {
private:
  SPIClass* spi;
  Adafruit_ST7789* tft;

  // 动画状态
  Animation* currentAnimation;
  uint8_t currentFrame;
  unsigned long lastFrameTime;
  bool animationPlaying;

public:
  DisplayManager();
  ~DisplayManager();

  // 初始化
  void begin();

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

  // 直接访问 TFT 对象（用于高级功能）
  Adafruit_ST7789* getTFT() { return tft; }
};

#endif
