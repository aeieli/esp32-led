#include "SnakeGame.h"

SnakeGame::SnakeGame(DisplayManager* display) {
  pDisplay = display;
  gridWidth = SCREEN_WIDTH / cellSize;
  gridHeight = SCREEN_HEIGHT / cellSize;
  snake = new GridPoint[kMaxSnakeLength];
  snakeLength = kInitialSnakeLength;
  score = 0;
  gameOver = false;
  lastStepTime = 0;
}

void SnakeGame::begin() {
  reset();
}

void SnakeGame::reset() {
  snakeLength = kInitialSnakeLength;
  score = 0;
  gameOver = false;

  pDisplay->setAutoFlush(false);
  pDisplay->clear(ST77XX_BLACK);

  // 蛇从中间开始，水平排列
  int16_t startX = gridWidth / 2 + snakeLength / 2;
  int16_t startY = gridHeight / 2;

  for (uint16_t i = 0; i < snakeLength; ++i) {
    snake[i].x = startX - i;
    snake[i].y = startY;
    drawCell(snake[i], ST77XX_RED);
  }

  direction.x = 1;
  direction.y = 0;

  spawnFood();
  displayStats();

  pDisplay->flush();
  pDisplay->setAutoFlush(true);

  lastStepTime = millis();

  Serial.printf("Snake game reset. Length: %u, Score: %lu\n",
                snakeLength, (unsigned long)score);
}

void SnakeGame::update() {
  if (gameOver) {
    return;
  }

  advanceSnake();
}

bool SnakeGame::isGameOver() {
  return gameOver;
}

uint16_t SnakeGame::getLength() {
  return snakeLength;
}

uint32_t SnakeGame::getScore() {
  return score;
}

void SnakeGame::drawCell(const GridPoint& p, uint16_t color) {
  int16_t px = p.x * cellSize;
  int16_t py = p.y * cellSize;
  pDisplay->fillRect(px, py, cellSize, cellSize, color);
}

bool SnakeGame::pointsEqual(const GridPoint& a, const GridPoint& b) {
  return a.x == b.x && a.y == b.y;
}

bool SnakeGame::snakeContains(uint16_t count, int16_t x, int16_t y) {
  for (uint16_t i = 0; i < count; ++i) {
    if (snake[i].x == x && snake[i].y == y) {
      return true;
    }
  }
  return false;
}

bool SnakeGame::headHitsBody() {
  if (snakeLength < 2) return false;
  for (uint16_t i = 1; i < snakeLength; ++i) {
    if (pointsEqual(snake[0], snake[i])) {
      return true;
    }
  }
  return false;
}

GridPoint SnakeGame::wrapPoint(int16_t x, int16_t y) {
  if (x < 0) x += gridWidth;
  if (x >= gridWidth) x -= gridWidth;
  if (y < 0) y += gridHeight;
  if (y >= gridHeight) y -= gridHeight;
  return {x, y};
}

void SnakeGame::spawnFood() {
  GridPoint candidate;
  uint16_t attempts = 0;

  do {
    candidate.x = random(gridWidth);
    candidate.y = random(gridHeight);
    attempts++;
    if (attempts > kMaxSnakeLength * 2) {
      return;
    }
  } while (snakeContains(snakeLength, candidate.x, candidate.y));

  food = candidate;
  drawCell(food, ST77XX_BLUE);
}

void SnakeGame::displayStats() {
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "Len:%u Score:%lu",
           snakeLength, (unsigned long)score);

  bool wasAutoFlush = pDisplay->getAutoFlush();
  pDisplay->setAutoFlush(false);

  pDisplay->fillRect(0, 0, SCREEN_WIDTH, cellSize, ST77XX_BLACK);
  pDisplay->drawText(buffer, 2, 1, ST77XX_YELLOW, 1);

  pDisplay->setAutoFlush(wasAutoFlush);
}

GridPoint SnakeGame::computeNextHead(const GridPoint& dir) {
  return wrapPoint(snake[0].x + dir.x, snake[0].y + dir.y);
}

bool SnakeGame::isReverseDirection(const GridPoint& dir) {
  if (snakeLength < 2) return false;
  GridPoint nextHead = computeNextHead(dir);
  return pointsEqual(nextHead, snake[1]);
}

bool SnakeGame::wouldCollide(const GridPoint& dir) {
  GridPoint nextHead = computeNextHead(dir);
  bool grows = pointsEqual(nextHead, food);

  uint16_t limit = snakeLength;
  if (!grows && snakeLength > 0) {
    limit -= 1;
  }

  return snakeContains(limit, nextHead.x, nextHead.y);
}

bool SnakeGame::chooseDirection() {
  GridPoint head = snake[0];
  GridPoint candidates[6];
  uint8_t count = 0;

  // 优先朝向食物的方向移动
  if (head.x != food.x) {
    candidates[count++] = {(int16_t)((food.x > head.x) ? 1 : -1), 0};
  }
  if (head.y != food.y) {
    candidates[count++] = {0, (int16_t)((food.y > head.y) ? 1 : -1)};
  }

  // 当前方向和左右转向作为备选
  candidates[count++] = direction;
  candidates[count++] = {direction.y, (int16_t)(-direction.x)};
  candidates[count++] = {(int16_t)(-direction.y), direction.x};

  // 最后备选：反方向
  candidates[count++] = {(int16_t)(-direction.x), (int16_t)(-direction.y)};

  for (uint8_t i = 0; i < count; ++i) {
    GridPoint candidate = candidates[i];
    if (candidate.x == 0 && candidate.y == 0) continue;
    if (isReverseDirection(candidate)) continue;
    if (wouldCollide(candidate)) continue;
    direction = candidate;
    return true;
  }

  // 如果所有方向都会碰撞，尝试至少避免掉头
  for (uint8_t i = 0; i < count; ++i) {
    GridPoint candidate = candidates[i];
    if (candidate.x == 0 && candidate.y == 0) continue;
    if (isReverseDirection(candidate)) continue;
    direction = candidate;
    return true;
  }

  return false;
}

void SnakeGame::advanceSnake() {
  if (gameOver) return;

  unsigned long now = millis();
  if (now - lastStepTime < stepIntervalMs) {
    return;
  }

  if (!chooseDirection()) {
    gameOver = true;
    pDisplay->drawCenteredText("GAME OVER!", 100, ST77XX_RED, 2);
    pDisplay->flush();
    Serial.println("No safe direction. Game over.");
    return;
  }

  pDisplay->setAutoFlush(false);

  GridPoint nextHead = computeNextHead(direction);
  bool ateFood = pointsEqual(nextHead, food);
  GridPoint tail = snake[snakeLength - 1];

  // 蛇身向前移动
  for (int i = snakeLength - 1; i > 0; --i) {
    snake[i] = snake[i - 1];
  }

  snake[0] = nextHead;
  drawCell(snake[0], ST77XX_RED);

  bool selfHit = headHitsBody();

  if (selfHit) {
    gameOver = true;
    pDisplay->drawCenteredText("SELF HIT!", SCREEN_HEIGHT / 2, ST77XX_RED, 2);
    pDisplay->flush();
    pDisplay->setAutoFlush(true);
    Serial.println("Snake collided with itself. Restarting in 1.5s...");
    delay(1500);
    reset();
    return;
  }

  if (ateFood) {
    // 增长
    if (snakeLength < kMaxSnakeLength) {
      snake[snakeLength] = tail;
      snakeLength++;
    }
    score += 10;
    displayStats();
    spawnFood();
    Serial.printf("Ate food! Length: %u, Score: %lu\n", snakeLength, (unsigned long)score);
  } else {
    // 正常移动
    drawCell(tail, ST77XX_BLACK);
    drawCell(food, ST77XX_BLUE);
  }

  pDisplay->flush();
  pDisplay->setAutoFlush(true);

  lastStepTime = now;
}
