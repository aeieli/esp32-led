#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <Arduino.h>
#include <Adafruit_ST7789.h>

// 脏区域结构
struct DirtyRegion {
  int16_t x;
  int16_t y;
  int16_t width;
  int16_t height;
  bool isDirty;
};

// 缓冲模式
enum BufferMode {
  BUFFER_MODE_DIRECT,      // 直接写入（无缓冲，低内存占用）
  BUFFER_MODE_SINGLE,      // 单缓冲+脏区域（中等内存占用）
  BUFFER_MODE_DOUBLE       // 双缓冲（高内存占用，最佳性能）
};

/**
 * 帧缓冲管理类
 * 提供双缓冲、脏区域跟踪、批量刷新功能
 * 解决动画花屏问题
 */
class FrameBuffer {
private:
  uint16_t width;
  uint16_t height;
  BufferMode mode;

  // 缓冲区指针
  uint16_t* frontBuffer;   // 前台缓冲（显示中）
  uint16_t* backBuffer;    // 后台缓冲（绘制中）

  // 脏区域管理
  static const uint8_t MAX_DIRTY_REGIONS = 8;
  DirtyRegion dirtyRegions[MAX_DIRTY_REGIONS];
  uint8_t dirtyCount;

  // 全屏脏标记
  bool fullScreenDirty;

  // 性能统计
  unsigned long lastFlushTime;
  uint32_t flushCount;

  // 私有方法
  void mergeDirtyRegions();
  void expandDirtyRegion(int16_t x, int16_t y, int16_t w, int16_t h);
  bool allocateBuffers();
  void freeBuffers();

public:
  FrameBuffer(uint16_t w, uint16_t h);
  ~FrameBuffer();

  // 初始化
  bool begin(BufferMode bufferMode = BUFFER_MODE_SINGLE);
  void end();

  // 缓冲模式控制
  BufferMode getMode() const { return mode; }
  bool setMode(BufferMode newMode);

  // 像素操作
  void setPixel(int16_t x, int16_t y, uint16_t color);
  uint16_t getPixel(int16_t x, int16_t y) const;

  // 区域操作（批量写入）
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h,
                const uint16_t* data);
  void drawRectScaled(int16_t x, int16_t y, int16_t w, int16_t h,
                      const uint16_t* srcData, uint16_t srcW, uint16_t srcH);

  // 全屏操作
  void clear(uint16_t color = 0x0000);
  void drawFullScreen(const uint16_t* data);

  // 刷新控制
  void flush(Adafruit_ST7789* tft);           // 刷新所有脏区域
  void flushRegion(Adafruit_ST7789* tft,      // 刷新指定区域
                   int16_t x, int16_t y, int16_t w, int16_t h);
  void flushImmediate(Adafruit_ST7789* tft);  // 强制立即刷新全屏

  // 双缓冲交换
  void swapBuffers();

  // 脏区域管理
  void markDirty(int16_t x, int16_t y, int16_t w, int16_t h);
  void markClean();
  bool isDirty() const { return fullScreenDirty || dirtyCount > 0; }
  uint8_t getDirtyRegionCount() const { return dirtyCount; }

  // 性能信息
  uint32_t getFlushCount() const { return flushCount; }
  unsigned long getLastFlushTime() const { return lastFlushTime; }
  size_t getMemoryUsage() const;

  // 辅助方法
  bool isValidCoord(int16_t x, int16_t y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
  }
};

#endif
