#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include "Display.h"
#include "BLEManager.h"

// 前向声明
class ClockDisplay;
class OTAManager;

// 支持的指令枚举
enum CommandType {
  CMD_UNKNOWN,
  CMD_SET_TEXT,         // 设置文本显示
  CMD_SET_BRIGHTNESS,   // 设置亮度
  CMD_CLEAR_SCREEN,     // 清屏
  CMD_SET_MODE,         // 设置显示模式
  CMD_GET_STATUS,       // 获取状态
  CMD_SLEEP,            // 进入睡眠
  CMD_WAKEUP,           // 唤醒
  CMD_RESTART,          // 重启
  CMD_SET_TIME,         // 设置时间
  CMD_SET_DATE,         // 设置日期
  CMD_OTA_UPDATE        // OTA更新
};

// 显示模式枚举
enum DisplayMode {
  MODE_MANUAL,      // 手动模式（接收指令控制）
  MODE_DEMO,        // 演示模式（自动循环）
  MODE_CLOCK,       // 时钟模式
  MODE_CUSTOM       // 自定义模式
};

class CommandHandler {
public:
  CommandHandler(DisplayManager* display, BLEManager* ble);

  // 初始化
  void begin();

  // 指令处理
  void handleCommand(String command);

  // 模式管理
  void setMode(DisplayMode mode);
  DisplayMode getMode();

  // 时钟管理
  void setClockDisplay(ClockDisplay* clock);

  // OTA管理
  void setOTAManager(OTAManager* ota);

  // 发送状态到手机
  void sendStatus();

private:
  DisplayManager* pDisplay;
  BLEManager* pBLE;
  ClockDisplay* pClock;
  OTAManager* pOTA;
  DisplayMode currentMode;

  // 指令解析
  CommandType parseCommandType(const String& command);
  String extractParameter(const String& command, const String& prefix);

  // 指令执行
  void executeSetText(const String& text);
  void executeSetBrightness(const String& value);
  void executeClearScreen();
  void executeSetMode(const String& mode);
  void executeGetStatus();
  void executeSleep();
  void executeWakeup();
  void executeRestart();
  void executeSetTime(const String& time);
  void executeSetDate(const String& date);
  void executeOTAUpdate(const String& url);

  // 辅助方法
  String buildStatusJson();
};

#endif // COMMAND_HANDLER_H
