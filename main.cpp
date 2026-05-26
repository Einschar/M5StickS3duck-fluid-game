#include <M5Unified.h>
#include <vector>
#include <cmath>

// 配置参数
const int BALL_COUNT = 100;
const float DAMPING = 0.95f;          // 阻尼系数
const float GRAVITY_SENSITIVITY = 0.3f; // 重力感应灵敏度
const float BOUNCE_DAMPING = 0.7f;    // 边界反弹阻尼
const int BALL_RADIUS = 4;            // 海洋球半径

// 颜色定义
const m5::display::palette_t COLOR_BG = m5::display::color565(10, 20, 40);
const m5::display::palette_t COLOR_DUCK = m5::display::color565(255, 215, 0);
const m5::display::palette_t COLOR_BALL[] = {
    m5::display::color565(255, 107, 107),
    m5::display::color565(78, 205, 196),
    m5::display::color565(255, 230, 109),
    m5::display::color565(26, 83, 92),
    m5::display::color565(255, 159, 67),
    m5::display::color565(85, 239, 196),
    m5::display::color565(6, 214, 160),
    m5::display::color565(72, 201, 176),
};

// 小黄鸭结构体
struct Duck {
    float x, y;           // 位置
    float vx, vy;         // 速度
    float width, height;  // 尺寸

    Duck(float w, float h) : x(w / 2), y(h / 2), vx(0), vy(0), width(w * 0.15), height(w * 0.12) {}

    void update(float tiltX, float tiltY, float screenW, float screenH) {
        // 根据倾斜角度施加加速度
        vx += tiltX * GRAVITY_SENSITIVITY;
        vy += tiltY * GRAVITY_SENSITIVITY;

        // 应用阻尼
        vx *= DAMPING;
        vy *= DAMPING;

        // 更新位置
        x += vx;
        y += vy;

        // 边界检测
        float halfW = width / 2;
        float halfH = height / 2;

        if (x - halfW < 0) {
            x = halfW;
            vx *= -BOUNCE_DAMPING;
        }
        if (x + halfW > screenW) {
            x = screenW - halfW;
            vx *= -BOUNCE_DAMPING;
        }
        if (y - halfH < 0) {
            y = halfH;
            vy *= -BOUNCE_DAMPING;
        }
        if (y + halfH > screenH) {
            y = screenH - halfH;
            vy *= -BOUNCE_DAMPING;
        }
    }

    void draw(M5.Display& display) {
        float halfW = width / 2;
        float halfH = height / 2;

        // 画鸭身
        display.fillCircle(x, y, halfW, COLOR_DUCK);

        // 画鸭头
        display.fillCircle(x, y - halfH * 0.6, halfW * 0.7, COLOR_DUCK);

        // 画眼睛
        display.fillCircle(x - halfW * 0.25, y - halfH * 0.7, halfW * 0.15, m5::display::black);
        display.fillCircle(x + halfW * 0.25, y - halfH * 0.7, halfW * 0.15, m5::display::black);

        // 画嘴巴
        display.fillCircle(x + halfW * 0.6, y - halfH * 0.6, halfW * 0.3, m5::display::color565(255, 165, 0));
    }
};

// 海洋球结构体
struct Ball {
    float x, y;           // 位置
    float vx, vy;         // 速度
    float radius;
    int colorIndex;

    Ball(float startX, float startY, int r, int c) : x(startX), y(startY), radius(r), colorIndex(c), vx(0), vy(0) {}

    void update(float tiltX, float tiltY, float screenW, float screenH) {
        // 海洋球跟随整体倾斜，但幅度更大
        vx += tiltX * GRAVITY_SENSITIVITY * 1.5f;
        vy += tiltY * GRAVITY_SENSITIVITY * 1.5f;

        // 应用阻尼
        vx *= DAMPING;
        vy *= DAMPING;

        // 更新位置
        x += vx;
        y += vy;

        // 边界碰撞
        if (x - radius < 0) {
            x = radius;
            vx *= -BOUNCE_DAMPING;
        }
        if (x + radius > screenW) {
            x = screenW - radius;
            vx *= -BOUNCE_DAMPING;
        }
        if (y - radius < 0) {
            y = radius;
            vy *= -BOUNCE_DAMPING;
        }
        if (y + radius > screenH) {
            y = screenH - radius;
            vy *= -BOUNCE_DAMPING;
        }
    }

    void draw(M5.Display& display) {
        display.fillCircle(x, y, radius, COLOR_BALL[colorIndex % 8]);
    }
};

// 检测两球碰撞
bool checkBallCollision(Ball& a, Ball& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dist = sqrt(dx * dx + dy * dy);
    return dist < (a.radius + b.radius);
}

// 处理球体碰撞
void handleBallCollisions(std::vector<Ball>& balls) {
    for (int i = 0; i < balls.size(); i++) {
        for (int j = i + 1; j < balls.size(); j++) {
            if (checkBallCollision(balls[i], balls[j])) {
                float dx = balls[j].x - balls[i].x;
                float dy = balls[j].y - balls[i].y;
                float dist = sqrt(dx * dx + dy * dy);

                if (dist == 0) continue;

                // 归一化碰撞法向量
                float nx = dx / dist;
                float ny = dy / dist;

                // 分离重叠的球
                float overlap = (balls[i].radius + balls[j].radius) - dist;
                balls[i].x -= nx * overlap * 0.5f;
                balls[i].y -= ny * overlap * 0.5f;
                balls[j].x += nx * overlap * 0.5f;
                balls[j].y += ny * overlap * 0.5f;

                // 简单的弹性碰撞响应
                float dvx = balls[i].vx - balls[j].vx;
                float dvy = balls[i].vy - balls[j].vy;
                float dvn = dvx * nx + dvy * ny;

                if (dvn > 0) continue;

                balls[i].vx -= dvn * nx * 0.8f;
                balls[i].vy -= dvn * ny * 0.8f;
                balls[j].vx += dvn * nx * 0.8f;
                balls[j].vy += dvn * ny * 0.8f;
            }
        }
    }
}

// 生成随机球体分布
std::vector<Ball> createBalls(int count, float screenW, float screenH) {
    std::vector<Ball> balls;
    for (int i = 0; i < count; i++) {
        float x = random(10, screenW - 10);
        float y = random(10, screenH - 10);
        balls.push_back(Ball(x, y, BALL_RADIUS, random(0, 8)));
    }
    return balls;
}

// 低通滤波器
class LowPassFilter {
private:
    float alpha;
    float output;

public:
    LowPassFilter(float a = 0.1f) : alpha(a), output(0) {}

    float filter(float input) {
        output = alpha * input + (1 - alpha) * output;
        return output;
    }
};

LowPassFilter filterX(0.1f);
LowPassFilter filterY(0.1f);

Duck* duck;
std::vector<Ball>* balls;

void setup() {
    M5.begin();
    M5.Lcd.fillScreen(COLOR_BG);
    M5.Lcd.setRotation(3);

    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(m5::display::white);
    M5.Lcd.println("海洋球小黄鸭漂浮游戏");
    M5.Lcd.println("请摇摆设备控制小黄鸭");
    M5.Lcd.println("按按钮重置位置");
    delay(2000);
    M5.Lcd.clear();

    int width = M5.Lcd.width();
    int height = M5.Lcd.height();

    duck = new Duck(width, height);
    balls = new std::vector<Ball>(createBalls(BALL_COUNT, width, height));
}

void loop() {
    M5.update();

    if (duck && balls) {
        // 获取陀螺仪数据（加速度计 + 陀螺仪融合）
        auto accel = M5.Imu.getAccelData();
        auto gyro = M5.Imu.getGyroData();

        // 计算倾斜角度（基于加速度计）
        float tiltX = filterX.filter(accel.x);
        float tiltY = filterY.filter(accel.y);

        // 获取屏幕尺寸
        int width = M5.Lcd.width();
        int height = M5.Lcd.height();

        // 更新鸭子位置
        duck->update(tiltX, tiltY, width, height);

        // 更新所有海洋球位置
        for (auto& ball : *balls) {
            ball.update(tiltX, tiltY, width, height);
        }

        // 处理球体碰撞
        handleBallCollisions(*balls);

        // 清屏并绘制
        M5.Lcd.fillScreen(COLOR_BG);

        // 绘制海洋球
        for (const auto& ball : *balls) {
            ball.draw(M5.Lcd);
        }

        // 绘制小黄鸭
        duck->draw(M5.Lcd);

        // 显示状态信息
        M5.Lcd.setTextSize(0);
        M5.Lcd.setCursor(5, 5);
        M5.Lcd.printf("X: %.2f Y: %.2f", accel.x, accel.y);
    }

    // 按钮重置功能
    if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed()) {
        if (duck) {
            duck->x = M5.Lcd.width() / 2;
            duck->y = M5.Lcd.height() / 2;
            duck->vx = 0;
            duck->vy = 0;
        }
    }

    delay(16); // 约 60 FPS
}
