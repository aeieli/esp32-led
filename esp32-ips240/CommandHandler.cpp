#include "CommandHandler.h"
#include "ClockDisplay.h"

CommandHandler::CommandHandler(DisplayManager* display, BLEManager* ble) {
  pDisplay = display;
  pBLE = ble;
  pClock = nullptr;
  currentMode = MODE_DEMO;
}

void CommandHandler::begin() {
  Serial.println("指令处理器已初始化");
}

void CommandHandler::setClockDisplay(ClockDisplay* clock) {
  pClock = clock;
}

void CommandHandler::handleCommand(String command) {
  command.trim();

  if (command.length() == 0) {
    return;
  }

  Serial.println("处理指令: " + command);

  CommandType cmdType = parseCommandType(command);

  switch (cmdType) {
    case CMD_SET_TEXT: {
      String param = extractParameter(command, "TEXT:");
      if (param.length() == 0) {
        param = extractParameter(command, "T:");
      }
      if (param.length() == 0) {
        String cmdUpper = command;
        cmdUpper.toUpperCase();
        if (cmdUpper.startsWith("T ")) {
          param = command.substring(2);  // "T " 后面的所有内容
        }
      }
      executeSetText(param);
      break;
    }

    case CMD_SET_BRIGHTNESS: {
      String param = extractParameter(command, "BRIGHTNESS:");
      if (param.length() == 0) {
        param = extractParameter(command, "B:");
      }
      if (param.length() == 0) {
        String cmdUpper = command;
        cmdUpper.toUpperCase();
        if (cmdUpper.startsWith("B ")) {
          param = command.substring(2);  // "B " 后面的所有内容
        }
      }
      executeSetBrightness(param);
      break;
    }

    case CMD_CLEAR_SCREEN:
      executeClearScreen();
      break;

    case CMD_SET_MODE: {
      String param = extractParameter(command, "MODE:");
      if (param.length() == 0) {
        param = extractParameter(command, "M:");
      }
      if (param.length() == 0) {
        String cmdUpper = command;
        cmdUpper.toUpperCase();
        if (cmdUpper.startsWith("M ")) {
          param = command.substring(2);  // "M " 后面的所有内容
        }
      }
      executeSetMode(param);
      break;
    }

    case CMD_GET_STATUS:
      executeGetStatus();
      break;

    case CMD_SLEEP:
      executeSleep();
      break;

    case CMD_WAKEUP:
      executeWakeup();
      break;

    case CMD_RESTART:
      executeRestart();
      break;

    case CMD_SET_TIME: {
      String param = extractParameter(command, "SETTIME:");
      if (param.length() == 0) {
        param = extractParameter(command, "ST:");
      }
      if (param.length() == 0) {
        String cmdUpper = command;
        cmdUpper.toUpperCase();
        if (cmdUpper.startsWith("ST ")) {
          param = command.substring(3);  // "ST " 后面的所有内容
        }
      }
      executeSetTime(param);
      break;
    }

    case CMD_SET_DATE: {
      String param = extractParameter(command, "SETDATE:");
      if (param.length() == 0) {
        param = extractParameter(command, "SD:");
      }
      if (param.length() == 0) {
        String cmdUpper = command;
        cmdUpper.toUpperCase();
        if (cmdUpper.startsWith("SD ")) {
          param = command.substring(3);  // "SD " 后面的所有内容
        }
      }
      executeSetDate(param);
      break;
    }

    default:
      Serial.println("未知指令: " + command);
      pBLE->sendData("ERROR:Unknown command");
      break;
  }
}

CommandType CommandHandler::parseCommandType(const String& command) {
  String cmd = command;
  cmd.toUpperCase();

  // 完整格式和简化格式指令
  if (cmd.startsWith("TEXT:") || cmd.startsWith("T ") || cmd.startsWith("T:")) {
    return CMD_SET_TEXT;
  } else if (cmd.startsWith("BRIGHTNESS:") || cmd.startsWith("B ") || cmd.startsWith("B:")) {
    return CMD_SET_BRIGHTNESS;
  } else if (cmd == "CLEAR" || cmd == "C") {
    return CMD_CLEAR_SCREEN;
  } else if (cmd.startsWith("MODE:") || cmd.startsWith("M ") || cmd.startsWith("M:")) {
    return CMD_SET_MODE;
  } else if (cmd == "STATUS" || cmd == "GET_STATUS" || cmd == "S") {
    return CMD_GET_STATUS;
  } else if (cmd == "SLEEP") {
    return CMD_SLEEP;
  } else if (cmd == "WAKEUP" || cmd == "WAKE" || cmd == "W") {
    return CMD_WAKEUP;
  } else if (cmd == "RESTART" || cmd == "REBOOT" || cmd == "R") {
    return CMD_RESTART;
  } else if (cmd.startsWith("SETTIME:") || cmd.startsWith("ST ") || cmd.startsWith("ST:")) {
    return CMD_SET_TIME;
  } else if (cmd.startsWith("SETDATE:") || cmd.startsWith("SD ") || cmd.startsWith("SD:")) {
    return CMD_SET_DATE;
  }

  return CMD_UNKNOWN;
}

String CommandHandler::extractParameter(const String& command, const String& prefix) {
  int startIndex = command.indexOf(prefix);
  if (startIndex == -1) {
    return "";
  }

  String param = command.substring(startIndex + prefix.length());
  param.trim();
  return param;
}

void CommandHandler::executeSetText(const String& text) {
  if (text.length() == 0) {
    pBLE->sendData("ERROR:Empty text");
    return;
  }

  pDisplay->clear();
  pDisplay->drawCenteredText(text.c_str(), 120, ST77XX_WHITE, 2);

  pBLE->sendData("OK:Text displayed");
  Serial.println("显示文本: " + text);
}

void CommandHandler::executeSetBrightness(const String& value) {
  int brightness = value.toInt();

  if (brightness < 0 || brightness > 255) {
    pBLE->sendData("ERROR:Brightness must be 0-255");
    return;
  }

  pDisplay->setBrightness(brightness);
  pBLE->sendData("OK:Brightness set to " + String(brightness));
  Serial.println("亮度设置为: " + String(brightness));
}

void CommandHandler::executeClearScreen() {
  pDisplay->clear();
  pBLE->sendData("OK:Screen cleared");
  Serial.println("清屏");
}

void CommandHandler::executeSetMode(const String& mode) {
  String modeStr = mode;
  modeStr.toUpperCase();

  if (modeStr == "MANUAL") {
    setMode(MODE_MANUAL);
  } else if (modeStr == "DEMO") {
    setMode(MODE_DEMO);
  } else if (modeStr == "CLOCK") {
    setMode(MODE_CLOCK);
  } else if (modeStr == "CUSTOM") {
    setMode(MODE_CUSTOM);
  } else {
    pBLE->sendData("ERROR:Unknown mode");
    return;
  }

  pBLE->sendData("OK:Mode set to " + mode);
}

void CommandHandler::executeGetStatus() {
  String status = buildStatusJson();
  pBLE->sendData(status);
  Serial.println("状态已发送: " + status);
}

void CommandHandler::executeSleep() {
  pDisplay->sleep();
  pBLE->sendData("OK:Display sleeping");
  Serial.println("显示屏进入睡眠");
}

void CommandHandler::executeWakeup() {
  pDisplay->wakeup();
  pBLE->sendData("OK:Display awake");
  Serial.println("显示屏唤醒");
}

void CommandHandler::executeRestart() {
  pBLE->sendData("OK:Restarting...");
  delay(1000);
  Serial.println("重启中...");
  ESP.restart();
}

void CommandHandler::setMode(DisplayMode mode) {
  currentMode = mode;
  Serial.println("模式切换到: " + String(mode));
}

DisplayMode CommandHandler::getMode() {
  return currentMode;
}

void CommandHandler::sendStatus() {
  executeGetStatus();
}

void CommandHandler::executeSetTime(const String& time) {
  if (!pClock) {
    pBLE->sendData("ERROR:Clock not initialized");
    Serial.println("错误: 时钟未初始化");
    return;
  }

  if (time.length() == 0) {
    pBLE->sendData("ERROR:Empty time");
    return;
  }

  uint8_t hour, minute, second;

  // 检查是否是6位数字格式 (hhmmss)
  if (time.length() == 6 && time.indexOf(':') == -1) {
    // 6位数字格式：hhmmss
    String hourStr = time.substring(0, 2);
    String minuteStr = time.substring(2, 4);
    String secondStr = time.substring(4, 6);

    hour = hourStr.toInt();
    minute = minuteStr.toInt();
    second = secondStr.toInt();
  }
  // 传统格式 HH:MM:SS
  else {
    int firstColon = time.indexOf(':');
    int secondColon = time.indexOf(':', firstColon + 1);

    if (firstColon == -1 || secondColon == -1) {
      pBLE->sendData("ERROR:Invalid time format. Use HHMMSS or HH:MM:SS");
      Serial.println("错误: 时间格式错误，应使用 HHMMSS 或 HH:MM:SS");
      return;
    }

    String hourStr = time.substring(0, firstColon);
    String minuteStr = time.substring(firstColon + 1, secondColon);
    String secondStr = time.substring(secondColon + 1);

    hour = hourStr.toInt();
    minute = minuteStr.toInt();
    second = secondStr.toInt();
  }

  // 验证时间有效性
  if (hour >= 24 || minute >= 60 || second >= 60) {
    pBLE->sendData("ERROR:Invalid time values");
    Serial.println("错误: 时间值无效");
    return;
  }

  // 设置时间
  pClock->setTime(hour, minute, second);

  // 格式化显示时间（统一为 HH:MM:SS 格式）
  char timeStr[9];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hour, minute, second);

  pBLE->sendData("OK:Time set to " + String(timeStr));
  Serial.println("时间已设置为: " + String(timeStr));
}

void CommandHandler::executeSetDate(const String& date) {
  if (!pClock) {
    pBLE->sendData("ERROR:Clock not initialized");
    Serial.println("错误: 时钟未初始化");
    return;
  }

  if (date.length() == 0) {
    pBLE->sendData("ERROR:Empty date");
    return;
  }

  uint16_t year;
  uint8_t month, day;

  // 检查是否是8位数字格式 (YYYYMMDD)
  if (date.length() == 8 && date.indexOf('-') == -1 && date.indexOf('/') == -1) {
    // 8位数字格式：YYYYMMDD
    String yearStr = date.substring(0, 4);
    String monthStr = date.substring(4, 6);
    String dayStr = date.substring(6, 8);

    year = yearStr.toInt();
    month = monthStr.toInt();
    day = dayStr.toInt();
  }
  // 传统格式 YYYY-MM-DD 或 YYYY/MM/DD
  else {
    int firstSep = date.indexOf('-');
    if (firstSep == -1) {
      firstSep = date.indexOf('/');
    }
    int secondSep = date.indexOf('-', firstSep + 1);
    if (secondSep == -1) {
      secondSep = date.indexOf('/', firstSep + 1);
    }

    if (firstSep == -1 || secondSep == -1) {
      pBLE->sendData("ERROR:Invalid date format. Use YYYYMMDD or YYYY-MM-DD");
      Serial.println("错误: 日期格式错误，应使用 YYYYMMDD 或 YYYY-MM-DD");
      return;
    }

    String yearStr = date.substring(0, firstSep);
    String monthStr = date.substring(firstSep + 1, secondSep);
    String dayStr = date.substring(secondSep + 1);

    year = yearStr.toInt();
    month = monthStr.toInt();
    day = dayStr.toInt();
  }

  // 验证日期有效性
  if (year < 2000 || year > 2099) {
    pBLE->sendData("ERROR:Invalid year (2000-2099)");
    Serial.println("错误: 年份无效");
    return;
  }
  if (month < 1 || month > 12) {
    pBLE->sendData("ERROR:Invalid month (1-12)");
    Serial.println("错误: 月份无效");
    return;
  }
  if (day < 1 || day > 31) {
    pBLE->sendData("ERROR:Invalid day (1-31)");
    Serial.println("错误: 日期无效");
    return;
  }

  // 设置日期
  pClock->setDate(year, month, day);

  // 格式化显示日期（统一为 YYYY-MM-DD 格式）
  char dateStr[11];
  snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d", year, month, day);

  pBLE->sendData("OK:Date set to " + String(dateStr));
  Serial.println("日期已设置为: " + String(dateStr));
}

String CommandHandler::buildStatusJson() {
  // 构建简单的状态字符串（JSON格式）
  String json = "{";
  json += "\"mode\":\"";

  switch (currentMode) {
    case MODE_MANUAL: json += "MANUAL"; break;
    case MODE_DEMO:   json += "DEMO"; break;
    case MODE_CLOCK:  json += "CLOCK"; break;
    case MODE_CUSTOM: json += "CUSTOM"; break;
  }

  json += "\",";
  json += "\"uptime\":" + String(millis() / 1000);
  json += ",\"heap\":" + String(ESP.getFreeHeap());

  // 添加时钟时间（如果有）
  if (pClock && pClock->isTimeSet()) {
    json += ",\"time\":\"" + pClock->getTimeString() + "\"";
    json += ",\"date\":\"" + pClock->getDateString() + "\"";
  }

  json += "}";

  return json;
}
