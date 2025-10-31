#include "CommandHandler.h"

CommandHandler::CommandHandler(DisplayManager* display, BLEManager* ble) {
  pDisplay = display;
  pBLE = ble;
  currentMode = MODE_DEMO;
}

void CommandHandler::begin() {
  Serial.println("指令处理器已初始化");
}

void CommandHandler::handleCommand(String command) {
  command.trim();

  if (command.length() == 0) {
    return;
  }

  Serial.println("处理指令: " + command);

  CommandType cmdType = parseCommandType(command);

  switch (cmdType) {
    case CMD_SET_TEXT:
      executeSetText(extractParameter(command, "TEXT:"));
      break;

    case CMD_SET_BRIGHTNESS:
      executeSetBrightness(extractParameter(command, "BRIGHTNESS:"));
      break;

    case CMD_CLEAR_SCREEN:
      executeClearScreen();
      break;

    case CMD_SET_MODE:
      executeSetMode(extractParameter(command, "MODE:"));
      break;

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

    default:
      Serial.println("未知指令: " + command);
      pBLE->sendData("ERROR:Unknown command");
      break;
  }
}

CommandType CommandHandler::parseCommandType(const String& command) {
  String cmd = command;
  cmd.toUpperCase();

  if (cmd.startsWith("TEXT:")) {
    return CMD_SET_TEXT;
  } else if (cmd.startsWith("BRIGHTNESS:")) {
    return CMD_SET_BRIGHTNESS;
  } else if (cmd == "CLEAR") {
    return CMD_CLEAR_SCREEN;
  } else if (cmd.startsWith("MODE:")) {
    return CMD_SET_MODE;
  } else if (cmd == "STATUS" || cmd == "GET_STATUS") {
    return CMD_GET_STATUS;
  } else if (cmd == "SLEEP") {
    return CMD_SLEEP;
  } else if (cmd == "WAKEUP" || cmd == "WAKE") {
    return CMD_WAKEUP;
  } else if (cmd == "RESTART" || cmd == "REBOOT") {
    return CMD_RESTART;
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
  json += "}";

  return json;
}
