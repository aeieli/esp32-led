#ifndef CLOCK_DISPLAY_H
#define CLOCK_DISPLAY_H

#include <Arduino.h>
#include "Display.h"

class ClockDisplay {
public:
  ClockDisplay(DisplayManager* display);

  // 时间设置
  void setTime(uint8_t hour, uint8_t minute, uint8_t second);
  void setTime(uint32_t timestamp);  // Unix timestamp
  void setDate(uint16_t year, uint8_t month, uint8_t day);

  // 时钟控制
  void begin();
  void update();  // 在loop中调用，更新时间显示
  void show();    // 显示时钟界面

  // 状态查询
  String getTimeString();
  String getDateString();
  bool isTimeSet();

private:
  DisplayManager* pDisplay;

  // 时间变量
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint16_t year;
  uint8_t month;
  uint8_t day;

  // 状态
  bool timeSet;
  unsigned long lastUpdateTime;
  unsigned long lastDisplayTime;

  // 内部方法
  void updateTime();
  void displayClock();
  String formatTwoDigits(uint8_t value);
};

#endif // CLOCK_DISPLAY_H
