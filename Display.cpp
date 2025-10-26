#include "Display.h"

DisplayManager::DisplayManager() {
  spi = new SPIClass(FSPI);
  tft = new Adafruit_ST7789(spi, TFT_CS, TFT_DC, TFT_RST);
  currentAnimation = nullptr;
  currentFrame = 0;
  lastFrameTime = 0;
  animationPlaying = false;
}

DisplayManager::~DisplayManager() {
  delete tft;
  delete spi;
}

void DisplayManager::begin() {
  // 初始化引脚
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // 初始化 SPI 和显示屏
  spi->begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft->init(SCREEN_WIDTH, SCREEN_HEIGHT, SPI_MODE3);
  tft->fillScreen(ST77XX_BLACK);

  Serial.println("Display initialized");
}

void DisplayManager::clear(uint16_t color) {
  tft->fillScreen(color);
}

void DisplayManager::setBrightness(uint8_t level) {
  analogWrite(TFT_BL, level);
}

void DisplayManager::sleep() {
  digitalWrite(TFT_BL, LOW);
}

void DisplayManager::wakeup() {
  digitalWrite(TFT_BL, HIGH);
}

// ========== 文字显示 ==========

void DisplayManager::drawText(const char* text, int16_t x, int16_t y,
                               uint16_t color, uint8_t size) {
  tft->setCursor(x, y);
  tft->setTextColor(color);
  tft->setTextSize(size);
  tft->setTextWrap(true);
  tft->print(text);
}

void DisplayManager::drawCenteredText(const char* text, int16_t y,
                                       uint16_t color, uint8_t size) {
  tft->setTextSize(size);
  tft->setTextColor(color);

  int16_t x1, y1;
  uint16_t w, h;
  tft->getTextBounds(text, 0, y, &x1, &y1, &w, &h);

  int16_t x = (SCREEN_WIDTH - w) / 2;
  tft->setCursor(x, y);
  tft->print(text);
}

void DisplayManager::drawTextBox(int16_t x, int16_t y, int16_t w, int16_t h,
                                  const char* text, uint16_t textColor,
                                  uint16_t boxColor) {
  // 绘制边框
  tft->drawRect(x, y, w, h, boxColor);
  tft->drawRect(x + 1, y + 1, w - 2, h - 2, boxColor);

  // 显示文字
  tft->setCursor(x + 5, y + 8);
  tft->setTextColor(textColor);
  tft->setTextSize(1);

  // 简单的文字换行
  int16_t cursorX = x + 5;
  int16_t cursorY = y + 8;

  for (int i = 0; text[i] != '\0'; i++) {
    char c = text[i];

    if (cursorX > x + w - 10) {
      cursorX = x + 5;
      cursorY += 10;
    }

    if (cursorY > y + h - 10) break;

    tft->setCursor(cursorX, cursorY);
    tft->print(c);
    cursorX += 6;
  }
}

// ========== 图形绘制 ==========

void DisplayManager::drawRect(int16_t x, int16_t y, int16_t w, int16_t h,
                               uint16_t color) {
  tft->drawRect(x, y, w, h, color);
}

void DisplayManager::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                               uint16_t color) {
  tft->fillRect(x, y, w, h, color);
}

void DisplayManager::drawCircle(int16_t x, int16_t y, int16_t r,
                                 uint16_t color) {
  tft->drawCircle(x, y, r, color);
}

void DisplayManager::fillCircle(int16_t x, int16_t y, int16_t r,
                                 uint16_t color) {
  tft->fillCircle(x, y, r, color);
}

void DisplayManager::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                               uint16_t color) {
  tft->drawLine(x0, y0, x1, y1, color);
}

// ========== 图片显示 ==========

void DisplayManager::drawImage(const ImageData& img, int16_t x, int16_t y) {
  if (img.data == nullptr) return;

  tft->startWrite();
  tft->setAddrWindow(x, y, img.width, img.height);
  tft->writePixels((uint16_t*)img.data, img.width * img.height);
  tft->endWrite();
}

void DisplayManager::drawImageScaled(const ImageData& img, int16_t x, int16_t y,
                                      uint16_t newWidth, uint16_t newHeight) {
  if (img.data == nullptr) return;

  // 简单的最近邻缩放算法
  float xRatio = (float)img.width / newWidth;
  float yRatio = (float)img.height / newHeight;

  for (uint16_t j = 0; j < newHeight; j++) {
    for (uint16_t i = 0; i < newWidth; i++) {
      uint16_t srcX = i * xRatio;
      uint16_t srcY = j * yRatio;
      uint16_t pixelIndex = srcY * img.width + srcX;
      uint16_t color = img.data[pixelIndex];
      tft->drawPixel(x + i, y + j, color);
    }
  }
}

// ========== 动画控制 ==========

void DisplayManager::playAnimation(Animation* anim) {
  if (anim == nullptr || anim->frameCount == 0) return;

  currentAnimation = anim;
  currentFrame = 0;
  lastFrameTime = millis();
  animationPlaying = true;
}

void DisplayManager::stopAnimation() {
  animationPlaying = false;
  currentAnimation = nullptr;
}

void DisplayManager::updateAnimation() {
  if (!animationPlaying || currentAnimation == nullptr) return;

  unsigned long currentTime = millis();
  AnimationFrame& frame = currentAnimation->frames[currentFrame];

  if (currentTime - lastFrameTime >= frame.duration) {
    currentFrame++;

    if (currentFrame >= currentAnimation->frameCount) {
      if (currentAnimation->loop) {
        currentFrame = 0;  // 循环播放
      } else {
        stopAnimation();  // 停止播放
        return;
      }
    }

    // 显示当前帧
    frame = currentAnimation->frames[currentFrame];
    if (frame.data != nullptr) {
      tft->startWrite();
      tft->setAddrWindow(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
      tft->writePixels((uint16_t*)frame.data, SCREEN_WIDTH * SCREEN_HEIGHT);
      tft->endWrite();
    }

    lastFrameTime = currentTime;
  }
}

bool DisplayManager::isAnimationPlaying() {
  return animationPlaying;
}

// ========== 特效 ==========

void DisplayManager::fadeTransition(void (*drawFunc)(), uint16_t duration) {
  // 淡出
  for (int i = 255; i >= 0; i -= 15) {
    setBrightness(i);
    delay(duration / 34);  // 17 steps * 2
  }

  // 执行绘制函数
  if (drawFunc != nullptr) {
    drawFunc();
  }

  // 淡入
  for (int i = 0; i <= 255; i += 15) {
    setBrightness(i);
    delay(duration / 34);
  }
  setBrightness(255);
}

void DisplayManager::scrollText(const char* text, int16_t y, int16_t speed,
                                 uint16_t color, uint8_t size) {
  static int16_t scrollX = SCREEN_WIDTH;
  static unsigned long lastUpdate = 0;

  unsigned long currentTime = millis();
  if (currentTime - lastUpdate >= 20) {
    // 清除之前的文字
    tft->fillRect(0, y - size * 8, SCREEN_WIDTH, size * 8 + 8, ST77XX_BLACK);

    // 绘制滚动文字
    tft->setCursor(scrollX, y);
    tft->setTextColor(color);
    tft->setTextSize(size);
    tft->print(text);

    scrollX -= speed;

    // 计算文字宽度
    int16_t x1, y1;
    uint16_t w, h;
    tft->getTextBounds(text, 0, y, &x1, &y1, &w, &h);

    if (scrollX < -w) {
      scrollX = SCREEN_WIDTH;
    }

    lastUpdate = currentTime;
  }
}
