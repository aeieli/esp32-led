#include "Display.h"

DisplayManager display;

struct GridPoint {
  int16_t x;
  int16_t y;
};

// 简单的贪吃蛇演示参数
const uint8_t cellSize = 8;
const uint8_t gridWidth = SCREEN_WIDTH / cellSize;
const uint8_t gridHeight = SCREEN_HEIGHT / cellSize;

const uint16_t kMaxSnakeLength = gridWidth * gridHeight;
const uint8_t kInitialSnakeLength = 6;
GridPoint snake[kMaxSnakeLength];
uint16_t snakeLength = kInitialSnakeLength;
GridPoint direction = {1, 0};
GridPoint food;

unsigned long lastStepTime = 0;
const unsigned long stepIntervalMs = 150;

uint32_t score = 0;
bool gameOver = false;

void drawCell(const GridPoint& p, uint16_t color) {
  int16_t px = p.x * cellSize;
  int16_t py = p.y * cellSize;
  display.fillRect(px, py, cellSize, cellSize, color);
}

bool pointsEqual(const GridPoint& a, const GridPoint& b) {
  return a.x == b.x && a.y == b.y;
}

bool snakeContains(uint16_t count, int16_t x, int16_t y) {
  for (uint16_t i = 0; i < count; ++i) {
    if (snake[i].x == x && snake[i].y == y) {
      return true;
    }
  }
  return false;
}

bool headHitsBody() {
  if (snakeLength < 2) return false;
  for (uint16_t i = 1; i < snakeLength; ++i) {
    if (pointsEqual(snake[0], snake[i])) {
      return true;
    }
  }
  return false;
}

GridPoint wrapPoint(int16_t x, int16_t y) {
  if (x < 0) x += gridWidth;
  if (x >= gridWidth) x -= gridWidth;
  if (y < 0) y += gridHeight;
  if (y >= gridHeight) y -= gridHeight;
  return {x, y};
}

void spawnFood() {
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

void displayStats() {
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "Len:%u Score:%lu",
           snakeLength, (unsigned long)score);

  bool wasAutoFlush = display.getAutoFlush();
  display.setAutoFlush(false);

  display.fillRect(0, 0, SCREEN_WIDTH, cellSize, ST77XX_BLACK);
  display.drawText(buffer, 2, 1, ST77XX_YELLOW, 1);

  display.setAutoFlush(wasAutoFlush);
}

void resetSnake() {
  snakeLength = kInitialSnakeLength;
  score = 0;
  gameOver = false;

  display.setAutoFlush(false);
  display.clear(ST77XX_BLACK);

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

  display.flush();
  display.setAutoFlush(true);

  lastStepTime = millis();

  Serial.printf("Snake reset. Length: %u, Score: %lu\n",
                snakeLength, (unsigned long)score);
}

GridPoint computeNextHead(const GridPoint& dir) {
  return wrapPoint(snake[0].x + dir.x, snake[0].y + dir.y);
}

bool isReverseDirection(const GridPoint& dir) {
  if (snakeLength < 2) return false;
  GridPoint nextHead = computeNextHead(dir);
  return pointsEqual(nextHead, snake[1]);
}

bool wouldCollide(const GridPoint& dir) {
  GridPoint nextHead = computeNextHead(dir);
  bool grows = pointsEqual(nextHead, food);

  uint16_t limit = snakeLength;
  if (!grows && snakeLength > 0) {
    limit -= 1;  // 尾巴会离开原位置，可以忽略最后一个节点
  }

  return snakeContains(limit, nextHead.x, nextHead.y);
}

bool chooseDirection() {
  GridPoint head = snake[0];
  GridPoint candidates[6];
  uint8_t count = 0;

  // 优先朝向食物的方向移动
  if (head.x != food.x) {
    candidates[count++] = {(food.x > head.x) ? 1 : -1, 0};
  }
  if (head.y != food.y) {
    candidates[count++] = {0, (food.y > head.y) ? 1 : -1};
  }

  // 当前方向和左右转向作为备选
  candidates[count++] = direction;
  candidates[count++] = {direction.y, -direction.x};
  candidates[count++] = {-direction.y, direction.x};

  // 最后备选：反方向（仅在万不得已时使用）
  candidates[count++] = {-direction.x, -direction.y};

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

void advanceSnake() {
  if (gameOver) return;

  unsigned long now = millis();
  if (now - lastStepTime < stepIntervalMs) {
    return;
  }

  if (!chooseDirection()) {
    gameOver = true;
    display.drawCenteredText("GAME OVER!", 100, ST77XX_RED, 2);
    display.flush();
    Serial.println("No safe direction. Game over.");
    return;
  }

  display.setAutoFlush(false);

  GridPoint nextHead = computeNextHead(direction);
  bool ateFood = pointsEqual(nextHead, food);
  GridPoint tail = snake[snakeLength - 1];

  for (int i = snakeLength - 1; i > 0; --i) {
    snake[i] = snake[i - 1];
  }

  snake[0] = nextHead;
  drawCell(snake[0], ST77XX_RED);

  bool selfHit = headHitsBody();

  if (selfHit) {
    gameOver = true;
    display.drawCenteredText("SELF HIT!", SCREEN_HEIGHT / 2, ST77XX_RED, 2);
    display.flush();
    display.setAutoFlush(true);
    Serial.println("Snake collided with itself. Restarting...");
    delay(1200);
    resetSnake();
    return;
  }

  if (ateFood) {
    if (snakeLength < kMaxSnakeLength) {
      snake[snakeLength] = tail;
      snakeLength++;
    }
    score += 10;
    displayStats();
    spawnFood();
  } else {
    drawCell(tail, ST77XX_BLACK);
    drawCell(food, ST77XX_BLUE);
  }

  display.flush();
  display.setAutoFlush(true);

  lastStepTime = now;
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Growing Snake Demo ===");
  Serial.println("Snake will grow when eating food!");

  randomSeed(micros());

  display.begin(BUFFER_MODE_SINGLE, SPI_FREQUENCY_FAST);
  display.clear(ST77XX_BLACK);

  display.drawCenteredText("Growing Snake", 60, ST77XX_CYAN, 2);
  display.drawCenteredText("Auto-play demo", 90, ST77XX_WHITE, 1);
  display.drawCenteredText("Snake grows!", 110, ST77XX_GREEN, 1);
  display.flush();

  delay(2500);
  resetSnake();

  Serial.println("Game started!");
  Serial.println("Watch the snake grow as it eats food");
}

void loop() {
  advanceSnake();
  delay(10);
}
