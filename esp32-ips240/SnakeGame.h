#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <Arduino.h>
#include "Display.h"

// 网格点结构
struct GridPoint {
  int16_t x;
  int16_t y;
};

class SnakeGame {
public:
  SnakeGame(DisplayManager* display);

  // 游戏控制
  void begin();
  void reset();
  void update();  // 在loop中调用

  // 状态查询
  bool isGameOver();
  uint16_t getLength();
  uint32_t getScore();

private:
  DisplayManager* pDisplay;

  // 游戏配置
  static const uint8_t cellSize = 8;
  uint8_t gridWidth;
  uint8_t gridHeight;

  static const uint16_t kMaxSnakeLength = 100;
  static const uint8_t kInitialSnakeLength = 6;
  static const unsigned long stepIntervalMs = 150;

  // 游戏状态
  GridPoint* snake;
  uint16_t snakeLength;
  GridPoint direction;
  GridPoint food;
  uint32_t score;
  bool gameOver;
  unsigned long lastStepTime;

  // 内部方法
  void drawCell(const GridPoint& p, uint16_t color);
  bool pointsEqual(const GridPoint& a, const GridPoint& b);
  bool snakeContains(uint16_t count, int16_t x, int16_t y);
  bool headHitsBody();
  GridPoint wrapPoint(int16_t x, int16_t y);
  void spawnFood();
  void displayStats();
  GridPoint computeNextHead(const GridPoint& dir);
  bool isReverseDirection(const GridPoint& dir);
  bool wouldCollide(const GridPoint& dir);
  bool chooseDirection();
  void advanceSnake();
};

#endif // SNAKE_GAME_H
