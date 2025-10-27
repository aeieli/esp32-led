# 贪吃蛇游戏功能说明

## 🎮 游戏特性

### ✅ 已实现功能

#### 1. **真正的增长机制**
- ✅ 初始长度：3 个方块
- ✅ 最大长度：100 个方块
- ✅ 每吃到一个食物：长度 +1
- ✅ 吃到食物时**不清除尾巴**（实现增长）
- ✅ 没吃到食物时**清除尾巴**（正常移动）

#### 2. **得分系统**
- ✅ 每吃到一个食物：+10 分
- ✅ 实时显示在屏幕顶部：`Len:X Score:Y`
- ✅ 串口输出详细信息

#### 3. **自动 AI 控制**
- ✅ 蛇会自动追踪蓝色食物
- ✅ 水平优先移动策略
- ✅ 无需手动控制

#### 4. **碰撞检测**（可选）
- ✅ 已实现自己碰撞检测函数 `checkSelfCollision()`
- ⚠️ 当前**已注释禁用**（第137-143行）
- 💡 取消注释即可启用 Game Over 功能

#### 5. **视觉效果**
- 🔴 红色方块：蛇身
- 🔵 蓝色方块：食物
- 🟡 黄色文字：统计信息

#### 6. **边界处理**
- ✅ 穿墙循环（蛇从右边出去，从左边进来）
- ✅ 上下左右都可以循环

---

## 🐛 Bug 修复历史

### Bug #1：红色方块残留（已修复）
**原因：** 吃到食物时没有清除旧尾巴
**修复：** 在增长逻辑中正确处理尾巴清除

### Bug #2：碰撞检测错误（已修复）
**原因：** 在移动后才检测食物碰撞
**修复：** 在移动前使用 `nextX/nextY` 检测碰撞

---

## 📊 核心逻辑

### 增长机制 (CustomAnimationExample.ino:160-176)

```cpp
if (ateFood) {
  // ✅ 吃到食物：增长
  if (snakeLength < kMaxSnakeLength) {
    snakeLength++;      // 长度增加
    score += 10;        // 得分增加
    displayStats();     // 更新显示
  }
  spawnFood();          // 生成新食物
  // ⚠️ 注意：这里不清除 tail！
} else {
  // ✅ 没吃到：正常移动
  drawCell(tail, ST77XX_BLACK);  // 清除旧尾巴
  drawCell(food, ST77XX_BLUE);   // 重绘食物
}
```

### 移动流程

```
1. 计算下一个头部位置 (nextX, nextY)
   ↓
2. 边界检测 + 循环处理
   ↓
3. 检测是否吃到食物 (在移动前)
   ↓
4. 蛇身向前移动（数组操作）
   ↓
5. 更新头部位置
   ↓
6. 绘制新头部
   ↓
7. 根据是否吃到食物：
   - 吃到 → 增长 + 生成新食物
   - 没吃到 → 清除尾巴 + 重绘食物
```

---

## 🎯 配置参数

### 调整游戏难度

```cpp
// 移动速度（越小越快）
const unsigned long stepIntervalMs = 150;  // 150ms 每步

// 初始长度
const uint8_t kInitialSnakeLength = 3;

// 最大长度
const uint8_t kMaxSnakeLength = 100;

// 单元格大小（越小越密集）
const uint8_t cellSize = 8;  // 8×8 像素
```

---

## 🔧 可选功能

### 启用碰撞自己检测

取消注释 `advanceSnake()` 中的代码（第137-143行）：

```cpp
// 检测碰撞自己
if (checkSelfCollision(nextX, nextY)) {
  gameOver = true;
  display.drawCenteredText("GAME OVER!", 100, ST77XX_RED, 2);
  Serial.printf("Game Over! Final score: %d, Length: %d\n", score, snakeLength);
  return;
}
```

### 添加重启功能

在 `loop()` 中添加：

```cpp
void loop() {
  advanceSnake();

  // 游戏结束后，5秒后自动重启
  if (gameOver) {
    static unsigned long gameOverTime = 0;
    if (gameOverTime == 0) {
      gameOverTime = millis();
    }

    if (millis() - gameOverTime > 5000) {
      display.clear(ST77XX_BLACK);
      resetSnake();
      gameOverTime = 0;
    }
  }

  delay(10);
}
```

---

## 📈 性能优化

### 当前性能

| 指标 | 值 |
|------|-----|
| 网格大小 | 30×30 (240÷8) |
| 最大蛇长 | 100 个方块 |
| 移动间隔 | 150ms (约6.7步/秒) |
| 主循环 | 10ms delay |
| 内存占用 | ~1.2KB (蛇数组) |

### 显示优化

- ✅ 使用 `fillRect()` 而非 `drawPixel()` - 快50倍
- ✅ 顶部统计信息仅在变化时更新
- ✅ 食物仅在必要时重绘

---

## 🎮 串口输出示例

```
=== Growing Snake Demo ===
Snake will grow when eating food!
Display initialized: 80 MHz, Buffer mode: 1
Snake reset. Length: 3, Score: 0
Game started!
Watch the snake grow as it eats food

Ate food! Length: 4, Score: 10
Ate food! Length: 5, Score: 20
Ate food! Length: 6, Score: 30
Ate food! Length: 7, Score: 40
...
```

---

## 📸 屏幕显示

```
┌────────────────────────────┐
│ Len:7 Score:40             │ ← 黄色统计信息
├────────────────────────────┤
│                            │
│      █                     │ ← 蓝色食物
│                            │
│   ████████                 │ ← 红色蛇身
│                            │
│                            │
└────────────────────────────┘
```

---

## 🚀 使用方法

### 编译上传

```bash
# Arduino IDE
# 打开：CustomAnimationExample/CustomAnimationExample.ino
# 编译上传到 ESP32-S3
```

### 观察效果

1. ✅ 启动画面 2.5 秒
2. ✅ 蛇从中间开始（长度3）
3. ✅ 自动追逐蓝色食物
4. ✅ 每吃一个食物增长1格
5. ✅ 顶部显示实时长度和得分
6. ✅ 蛇会越来越长！

---

## 🎓 学习要点

### 关键技术

1. **固定长度 vs 动态增长**
   - 固定长度：总是清除尾巴
   - 动态增长：吃到食物时不清除尾巴

2. **数组操作**
   ```cpp
   // 蛇身向前移动
   for (int i = snakeLength - 1; i > 0; --i) {
     snake[i] = snake[i - 1];
   }
   snake[0] = newHead;  // 新头部
   ```

3. **碰撞检测时机**
   - ✅ 正确：移动前使用 `nextX/nextY` 检测
   - ❌ 错误：移动后使用 `snake[0]` 检测

4. **食物生成**
   ```cpp
   do {
     candidate = random position
   } while (snakeContains(candidate));
   ```
   确保食物不在蛇身上

---

## 📝 总结

**完整功能的贪吃蛇游戏：**
- ✅ 真正增长
- ✅ 得分系统
- ✅ 统计显示
- ✅ AI 自动玩
- ✅ 无残留 bug
- ✅ 可选碰撞检测
- ✅ 流畅渲染

**现在可以愉快地观看蛇越长越长了！** 🐍🎉
