#include "ClockDisplay.h"

ClockDisplay::ClockDisplay(DisplayManager* display) {
  pDisplay = display;
  hour = 0;
  minute = 0;
  second = 0;
  year = 2025;
  month = 1;
  day = 1;
  timeSet = false;
  lastUpdateTime = 0;
  lastDisplayTime = 0;
}

void ClockDisplay::begin() {
  Serial.println("时钟显示模块已初始化");
}

void ClockDisplay::setTime(uint8_t h, uint8_t m, uint8_t s) {
  hour = h % 24;
  minute = m % 60;
  second = s % 60;
  timeSet = true;
  lastUpdateTime = millis();

  Serial.printf("时间已设置: %02d:%02d:%02d\n", hour, minute, second);
}

void ClockDisplay::setTime(uint32_t timestamp) {
  // 简化的Unix时间戳转换（UTC+0）
  // 注意：这是简化版本，实际使用建议用完整的时间库
  uint32_t seconds = timestamp % 86400;
  hour = (seconds / 3600) % 24;
  minute = (seconds % 3600) / 60;
  second = seconds % 60;

  // 计算日期（简化版，从2000年开始）
  uint32_t days = timestamp / 86400;
  year = 2000 + (days / 365);  // 简化计算，忽略闰年
  uint16_t dayOfYear = days % 365;
  month = (dayOfYear / 30) + 1;  // 简化为每月30天
  day = (dayOfYear % 30) + 1;

  timeSet = true;
  lastUpdateTime = millis();

  Serial.printf("时间已设置: %04d-%02d-%02d %02d:%02d:%02d\n",
                year, month, day, hour, minute, second);
}

void ClockDisplay::setDate(uint16_t y, uint8_t m, uint8_t d) {
  year = y;
  month = m;
  day = d;

  Serial.printf("日期已设置: %04d-%02d-%02d\n", year, month, day);
}

void ClockDisplay::update() {
  if (!timeSet) {
    return;
  }

  // 每秒更新一次时间
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime >= 1000) {
    updateTime();
    lastUpdateTime = currentTime;
  }

  // 每秒刷新显示（如果在时钟模式）
  if (currentTime - lastDisplayTime >= 1000) {
    displayClock();
    lastDisplayTime = currentTime;
  }
}

void ClockDisplay::show() {
  if (!timeSet) {
    // 显示未设置时间的提示
    pDisplay->clear(ST77XX_BLACK);
    pDisplay->drawCenteredText("Clock Mode", 60, ST77XX_YELLOW, 2);
    pDisplay->drawCenteredText("Time not set", 100, ST77XX_RED, 1);
    pDisplay->drawCenteredText("Use SETTIME command", 130, ST77XX_WHITE, 1);
    pDisplay->drawCenteredText("Format: SETTIME:HH:MM:SS", 150, ST77XX_CYAN, 1);
    return;
  }

  // 显示时钟
  displayClock();
  lastDisplayTime = millis();
}

bool ClockDisplay::isTimeSet() {
  return timeSet;
}

String ClockDisplay::getTimeString() {
  return formatTwoDigits(hour) + ":" +
         formatTwoDigits(minute) + ":" +
         formatTwoDigits(second);
}

String ClockDisplay::getDateString() {
  return String(year) + "-" +
         formatTwoDigits(month) + "-" +
         formatTwoDigits(day);
}

void ClockDisplay::updateTime() {
  second++;
  if (second >= 60) {
    second = 0;
    minute++;
    if (minute >= 60) {
      minute = 0;
      hour++;
      if (hour >= 24) {
        hour = 0;
        // 简化：不处理日期翻转
      }
    }
  }
}

void ClockDisplay::displayClock() {
  pDisplay->clear(ST77XX_BLACK);

  // 显示日期（顶部）
  String dateStr = getDateString();
  pDisplay->drawCenteredText(dateStr.c_str(), 30, ST77XX_CYAN, 1);

  // 显示时间（大号字体，居中）
  String timeStr = getTimeString();
  pDisplay->drawCenteredText(timeStr.c_str(), 100, ST77XX_WHITE, 3);

  // 显示星期几的位置（可选，目前显示"Clock Mode"）
  pDisplay->drawCenteredText("Clock Mode", 160, ST77XX_GREEN, 1);

  // 显示分隔线装饰
  pDisplay->drawLine(40, 80, 200, 80, ST77XX_BLUE);
  pDisplay->drawLine(40, 145, 200, 145, ST77XX_BLUE);

  // 底部信息
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "Uptime: %lus", millis() / 1000);
  pDisplay->drawCenteredText(buffer, 200, ST77XX_MAGENTA, 1);
}

String ClockDisplay::formatTwoDigits(uint8_t value) {
  if (value < 10) {
    return "0" + String(value);
  }
  return String(value);
}
