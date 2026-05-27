#include <M5Unified.h>

#define BALL_NUM  360
#define SUB_STEPS 2
#define DTIME    0.36f

// 流体物理参数
#define DUCK_SIZE    11
#define BALL_SIZE    2
#define DAMP         0.97f
#define REPEL        2.8f
#define BOUNCE       0.6f
#define GRAV_FACTOR  1.5f

// 颜色(RGB565 已修复编译错误)
#define BACKGROUND   0x080C
#define COLOR_DUCK   0xFFE0
#define COLOR_CHEEK  0xF86B
#define COLOR_BEAK   0xFE60

const uint16_t COLOR_BALL[] = {
  0xF86B, 0x4EF7, 0xFFE6, 0x1A53,
  0xFF9F, 0x55EF, 0x06D6, 0x48C9
};

// 音效防重复计时
unsigned long ballSoundTimer = 0;
unsigned long duckSoundTimer = 0;
const unsigned long SOUND_INTERVAL = 120; // 最小发声间隔(ms)

struct Particle {
  float x, y;
  float vx, vy;
  uint8_t colorIdx;
};

Particle balls[BALL_NUM];
Particle duck;
float gx, gy;

// 球体撞墙 8bit 碰撞音效
void playBallHitSound()
{
  if (millis() - ballSoundTimer < SOUND_INTERVAL) return;
  ballSoundTimer = millis();
  M5.Speaker.tone(880, 40);
  delay(15);
  M5.Speaker.tone(660, 30);
}

// 鸭子撞墙 嘎嘎音效
void playDuckHitSound()
{
  if (millis() - duckSoundTimer < SOUND_INTERVAL) return;
  duckSoundTimer = millis();
  M5.Speaker.tone(440, 120);
}

void resetAll()
{
  int w = M5.Display.width();
  int h = M5.Display.height();
  
  duck.x = w * 0.5f;
  duck.y = h * 0.5f;
  duck.vx = duck.vy = 0;

  for (int i = 0; i < BALL_NUM; i++)
  {
    balls[i].x = random(w);
    balls[i].y = random(h);
    balls[i].vx = 0;
    balls[i].vy = 0;
    balls[i].colorIdx = random(8);
  }
}

void drawDuck(M5GFX& d)
{
  float x = duck.x;
  float y = duck.y;
  float r = DUCK_SIZE;

  d.fillCircle(x, y, r, COLOR_DUCK);
  d.fillCircle(x, y - 5, r * 0.75f, COLOR_DUCK);
  d.fillCircle(x - 4, y - 7, 2, BLACK);
  d.fillCircle(x + 4, y - 7, 2, BLACK);
  d.fillCircle(x - 7, y - 2, 2, COLOR_CHEEK);
  d.fillCircle(x + 7, y - 2, 2, COLOR_CHEEK);
  d.fillCircle(x + 6, y - 4, 3, COLOR_BEAK);
}

void setup(void)
{
  M5.begin();
  M5.Display.setRotation(3);
  M5.Display.setBrightness(110);
  M5.Speaker.setVolume(128); // 音量 0~255
  resetAll();
}

void loop(void)
{
  M5.update();

  if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed()) resetAll();

  float ax, ay, az;
  M5.Imu.getAccel(&ax, &ay, &az);
  gx = ax * GRAV_FACTOR;
  gy = -ay * GRAV_FACTOR;

  int w = M5.Display.width();
  int h = M5.Display.height();

  for (int s = 0; s < SUB_STEPS; s++)
  {
    // 鸭子物理更新
    duck.vx += gx * DTIME;
    duck.vy += gy * DTIME;
    duck.vx *= DAMP;
    duck.vy *= DAMP;
    duck.x += duck.vx * DTIME;
    duck.y += duck.vy * DTIME;

    // 小球物理更新
    for (int i = 0; i < BALL_NUM; i++)
    {
      balls[i].vx += gx * DTIME;
      balls[i].vy += gy * DTIME;
      balls[i].vx *= DAMP;
      balls[i].vy *= DAMP;
      balls[i].x += balls[i].vx * DTIME;
      balls[i].y += balls[i].vy * DTIME;
    }

    // 鸭子 <-> 小球 短程排斥
    const float minDist = BALL_SIZE + DUCK_SIZE;
    const float minDist2 = minDist * minDist;
    for (int i = 0; i < BALL_NUM; i++)
    {
      float dx = duck.x - balls[i].x;
      float dy = duck.y - balls[i].y;
      float d2 = dx*dx + dy*dy;
      if (d2 < minDist2 && d2 > 0.001f)
      {
        float d = sqrtf(d2);
        float f = (minDist - d) * REPEL;
        float fx = dx / d * f;
        float fy = dy / d * f;
        duck.x += fx * 0.15f;
        duck.y += fy * 0.15f;
        balls[i].x -= fx * 0.85f;
        balls[i].y -= fy * 0.85f;
      }
    }

    // 小球之间排斥
    const float ballMin = BALL_SIZE * 2;
    const float ballMin2 = ballMin * ballMin;
    for (int i = 0; i < BALL_NUM; i++)
    {
      for (int j = i + 1; j < BALL_NUM; j++)
      {
        float dx = balls[i].x - balls[j].x;
        float dy = balls[i].y - balls[j].y;
        float d2 = dx*dx + dy*dy;
        if (d2 < ballMin2 && d2 > 0.001f)
        {
          float d = sqrtf(d2);
          float f = (ballMin - d) * 0.75f;
          float fx = dx / d * f * 0.5f;
          float fy = dy / d * f * 0.5f;
          balls[i].x += fx;
          balls[i].y += fy;
          balls[j].x -= fx;
          balls[j].y -= fy;
        }
      }
    }

    // ========== 边界约束 + 音效触发 ==========
    // 鸭子撞墙检测+音效
    bool duckHitWall = false;
    if (duck.x < DUCK_SIZE)
    {
      duck.x = DUCK_SIZE;
      duck.vx *= -BOUNCE;
      duckHitWall = true;
    }
    if (duck.x > w - DUCK_SIZE)
    {
      duck.x = w - DUCK_SIZE;
      duck.vx *= -BOUNCE;
      duckHitWall = true;
    }
    if (duck.y < DUCK_SIZE)
    {
      duck.y = DUCK_SIZE;
      duck.vy *= -BOUNCE;
      duckHitWall = true;
    }
    if (duck.y > h - DUCK_SIZE)
    {
      duck.y = h - DUCK_SIZE;
      duck.vy *= -BOUNCE;
      duckHitWall = true;
    }
    if (duckHitWall) playDuckHitSound();

    // 小球撞墙检测+音效
    for (int i = 0; i < BALL_NUM; i++)
    {
      bool ballHit = false;
      if (balls[i].x < BALL_SIZE)
      {
        balls[i].x = BALL_SIZE;
        balls[i].vx *= -BOUNCE;
        ballHit = true;
      }
      if (balls[i].x > w - BALL_SIZE)
      {
        balls[i].x = w - BALL_SIZE;
        balls[i].vx *= -BOUNCE;
        ballHit = true;
      }
      if (balls[i].y < BALL_SIZE)
      {
        balls[i].y = BALL_SIZE;
        balls[i].vy *= -BOUNCE;
        ballHit = true;
      }
      if (balls[i].y > h - BALL_SIZE)
      {
        balls[i].y = h - BALL_SIZE;
        balls[i].vy *= -BOUNCE;
        ballHit = true;
      }
      if (ballHit) playBallHitSound();
    }
  }

  // 高速渲染
  M5.Display.startWrite();
  M5.Display.fillScreen(BACKGROUND);

  for (int i = 0; i < BALL_NUM; i++)
  {
    M5.Display.fillCircle(balls[i].x, balls[i].y, BALL_SIZE, COLOR_BALL[balls[i].colorIdx]);
  }

  drawDuck(M5.Display);
  M5.Display.endWrite();

  delay(24);
}