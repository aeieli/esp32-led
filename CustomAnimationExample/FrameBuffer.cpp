#include "FrameBuffer.h"

FrameBuffer::FrameBuffer(uint16_t w, uint16_t h)
  : width(w), height(h), mode(BUFFER_MODE_DIRECT),
    frontBuffer(nullptr), backBuffer(nullptr),
    dirtyCount(0), fullScreenDirty(false),
    lastFlushTime(0), flushCount(0) {
}

FrameBuffer::~FrameBuffer() {
  freeBuffers();
}

bool FrameBuffer::begin(BufferMode bufferMode) {
  mode = bufferMode;

  if (mode == BUFFER_MODE_DIRECT) {
    return true;  // 直接模式不需要缓冲
  }

  return allocateBuffers();
}

void FrameBuffer::end() {
  freeBuffers();
}

bool FrameBuffer::allocateBuffers() {
  freeBuffers();

  size_t bufferSize = width * height * sizeof(uint16_t);

  // 分配后台缓冲（必需）
  backBuffer = (uint16_t*)malloc(bufferSize);
  if (backBuffer == nullptr) {
    Serial.println("Failed to allocate back buffer");
    return false;
  }

  // 双缓冲模式需要前台缓冲
  if (mode == BUFFER_MODE_DOUBLE) {
    frontBuffer = (uint16_t*)malloc(bufferSize);
    if (frontBuffer == nullptr) {
      Serial.println("Failed to allocate front buffer, fallback to single");
      mode = BUFFER_MODE_SINGLE;
    }
  }

  clear(0x0000);
  Serial.printf("FrameBuffer allocated: %d KB\n", getMemoryUsage() / 1024);
  return true;
}

void FrameBuffer::freeBuffers() {
  if (backBuffer) {
    free(backBuffer);
    backBuffer = nullptr;
  }
  if (frontBuffer) {
    free(frontBuffer);
    frontBuffer = nullptr;
  }
}

bool FrameBuffer::setMode(BufferMode newMode) {
  if (newMode == mode) return true;

  mode = newMode;
  if (mode == BUFFER_MODE_DIRECT) {
    freeBuffers();
    return true;
  }

  return allocateBuffers();
}

// ========== 像素操作 ==========

void FrameBuffer::setPixel(int16_t x, int16_t y, uint16_t color) {
  if (!isValidCoord(x, y)) return;

  if (mode == BUFFER_MODE_DIRECT || backBuffer == nullptr) {
    return;  // 直接模式在flush时由调用者处理
  }

  backBuffer[y * width + x] = color;
  markDirty(x, y, 1, 1);
}

uint16_t FrameBuffer::getPixel(int16_t x, int16_t y) const {
  if (!isValidCoord(x, y) || backBuffer == nullptr) {
    return 0x0000;
  }

  return backBuffer[y * width + x];
}

void FrameBuffer::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                            uint16_t color) {
  if (backBuffer == nullptr) return;

  // 边界裁剪
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (x + w > width) w = width - x;
  if (y + h > height) h = height - y;
  if (w <= 0 || h <= 0) return;

  // 批量填充
  for (int16_t j = 0; j < h; j++) {
    uint16_t* row = &backBuffer[(y + j) * width + x];
    for (int16_t i = 0; i < w; i++) {
      row[i] = color;
    }
  }

  markDirty(x, y, w, h);
}

void FrameBuffer::drawRect(int16_t x, int16_t y, int16_t w, int16_t h,
                            const uint16_t* data) {
  if (backBuffer == nullptr || data == nullptr) return;

  // 边界裁剪
  if (x < 0) { data += -x; w += x; x = 0; }
  if (y < 0) { data += (-y * w); h += y; y = 0; }
  if (x + w > width) w = width - x;
  if (y + h > height) h = height - y;
  if (w <= 0 || h <= 0) return;

  // 批量复制（优化版）
  for (int16_t j = 0; j < h; j++) {
    memcpy(&backBuffer[(y + j) * width + x], &data[j * w], w * sizeof(uint16_t));
  }

  markDirty(x, y, w, h);
}

void FrameBuffer::drawRectScaled(int16_t x, int16_t y, int16_t w, int16_t h,
                                  const uint16_t* srcData, uint16_t srcW,
                                  uint16_t srcH) {
  if (backBuffer == nullptr || srcData == nullptr) return;

  // 最近邻插值缩放（批量版本）
  float xRatio = (float)srcW / w;
  float yRatio = (float)srcH / h;

  for (int16_t j = 0; j < h; j++) {
    if (y + j < 0 || y + j >= height) continue;

    uint16_t srcY = (uint16_t)(j * yRatio);
    uint16_t* destRow = &backBuffer[(y + j) * width + x];
    const uint16_t* srcRow = &srcData[srcY * srcW];

    for (int16_t i = 0; i < w; i++) {
      if (x + i < 0 || x + i >= width) continue;
      uint16_t srcX = (uint16_t)(i * xRatio);
      destRow[i] = srcRow[srcX];
    }
  }

  markDirty(x, y, w, h);
}

void FrameBuffer::clear(uint16_t color) {
  if (backBuffer) {
    uint32_t pixelCount = width * height;
    for (uint32_t i = 0; i < pixelCount; i++) {
      backBuffer[i] = color;
    }
  }

  if (frontBuffer) {
    uint32_t pixelCount = width * height;
    for (uint32_t i = 0; i < pixelCount; i++) {
      frontBuffer[i] = color;
    }
  }

  fullScreenDirty = true;
  dirtyCount = 0;
}

void FrameBuffer::drawFullScreen(const uint16_t* data) {
  if (backBuffer == nullptr || data == nullptr) return;

  memcpy(backBuffer, data, width * height * sizeof(uint16_t));
  fullScreenDirty = true;
  dirtyCount = 0;
}

// ========== 刷新控制 ==========

void FrameBuffer::flush(Adafruit_ST7789* tft) {
  if (tft == nullptr) return;

  unsigned long startTime = millis();

  if (mode == BUFFER_MODE_DIRECT) {
    // 直接模式不处理，由调用者直接操作tft
    return;
  }

  if (backBuffer == nullptr) return;

  tft->startWrite();

  if (fullScreenDirty) {
    // 全屏刷新
    tft->setAddrWindow(0, 0, width, height);
    tft->writePixels(backBuffer, width * height);
    fullScreenDirty = false;
  } else {
    // 仅刷新脏区域
    for (uint8_t i = 0; i < dirtyCount; i++) {
      DirtyRegion& region = dirtyRegions[i];
      if (!region.isDirty) continue;

      tft->setAddrWindow(region.x, region.y, region.width, region.height);

      // 逐行传输脏区域
      for (int16_t y = 0; y < region.height; y++) {
        uint16_t* row = &backBuffer[(region.y + y) * width + region.x];
        tft->writePixels(row, region.width);
      }
    }
  }

  tft->endWrite();

  markClean();

  lastFlushTime = millis() - startTime;
  flushCount++;
}

void FrameBuffer::flushRegion(Adafruit_ST7789* tft, int16_t x, int16_t y,
                               int16_t w, int16_t h) {
  if (tft == nullptr || backBuffer == nullptr) return;

  tft->startWrite();
  tft->setAddrWindow(x, y, w, h);

  for (int16_t j = 0; j < h; j++) {
    uint16_t* row = &backBuffer[(y + j) * width + x];
    tft->writePixels(row, w);
  }

  tft->endWrite();
}

void FrameBuffer::flushImmediate(Adafruit_ST7789* tft) {
  fullScreenDirty = true;
  dirtyCount = 0;
  flush(tft);
}

void FrameBuffer::swapBuffers() {
  if (mode != BUFFER_MODE_DOUBLE || frontBuffer == nullptr) {
    return;
  }

  uint16_t* temp = frontBuffer;
  frontBuffer = backBuffer;
  backBuffer = temp;
}

// ========== 脏区域管理 ==========

void FrameBuffer::markDirty(int16_t x, int16_t y, int16_t w, int16_t h) {
  if (fullScreenDirty) return;

  // 如果区域过大，标记全屏脏
  if (w * h > (width * height) / 4) {
    fullScreenDirty = true;
    dirtyCount = 0;
    return;
  }

  expandDirtyRegion(x, y, w, h);

  // 如果脏区域过多，合并或标记全屏
  if (dirtyCount >= MAX_DIRTY_REGIONS) {
    mergeDirtyRegions();
  }
}

void FrameBuffer::expandDirtyRegion(int16_t x, int16_t y, int16_t w, int16_t h) {
  // 尝试扩展现有脏区域
  for (uint8_t i = 0; i < dirtyCount; i++) {
    DirtyRegion& region = dirtyRegions[i];
    if (!region.isDirty) continue;

    // 检查是否相邻或重叠
    int16_t x1 = min(region.x, x);
    int16_t y1 = min(region.y, y);
    int16_t x2 = max(region.x + region.width, x + w);
    int16_t y2 = max(region.y + region.height, y + h);

    if (x2 - x1 <= region.width + w && y2 - y1 <= region.height + h) {
      // 可以合并
      region.x = x1;
      region.y = y1;
      region.width = x2 - x1;
      region.height = y2 - y1;
      return;
    }
  }

  // 添加新脏区域
  if (dirtyCount < MAX_DIRTY_REGIONS) {
    dirtyRegions[dirtyCount].x = x;
    dirtyRegions[dirtyCount].y = y;
    dirtyRegions[dirtyCount].width = w;
    dirtyRegions[dirtyCount].height = h;
    dirtyRegions[dirtyCount].isDirty = true;
    dirtyCount++;
  }
}

void FrameBuffer::mergeDirtyRegions() {
  // 简化合并：标记全屏脏
  fullScreenDirty = true;
  dirtyCount = 0;
}

void FrameBuffer::markClean() {
  fullScreenDirty = false;
  dirtyCount = 0;

  for (uint8_t i = 0; i < MAX_DIRTY_REGIONS; i++) {
    dirtyRegions[i].isDirty = false;
  }
}

size_t FrameBuffer::getMemoryUsage() const {
  size_t usage = 0;
  if (backBuffer) usage += width * height * sizeof(uint16_t);
  if (frontBuffer) usage += width * height * sizeof(uint16_t);
  return usage;
}
