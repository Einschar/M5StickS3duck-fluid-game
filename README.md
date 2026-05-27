# M5StickS3 海洋球小黄鸭漂浮游戏

利用 M5StickS3 的 6 轴陀螺仪，模拟流体效果，让小黄鸭在海洋球中保持漂浮。

## 功能特性

- **陀螺仪控制**：通过设备摇摆控制小黄鸭移动
- **360个海洋球**：彩色海洋球随重力感应动态移动
- **流体模拟**：带阻尼效果的重力跟随物理引擎
- **碰撞系统**：海洋球之间和边界碰撞处理
- **持续运行**：无游戏结束条件，无限循环

## 硬件要求

- M5StickS3 开发板
- USB 数据线

## 开发环境

- Arduino IDE 2.0+
- M5Unified 库

## 安装步骤

1. 安装 Arduino IDE 
2. 下载此项目到本地
3. 用 Arduino IDE 打开 `main.cpp` 文件

## 使用方法

1. 长按 M5StickS3 的复位键进入烧录状态，连接 M5StickS3 到电脑
2. 点击 Arduino IDE 中的 "Upload" 按钮编译上传
3. 按下M5StickS3 的复位键，摇摆设备控制小黄鸭在海洋球中漂浮
4. 按钮A或B可重置小黄鸭位置

## 参数配置

在 `main.cpp` 中可调整以下参数：

```cpp
#define BALL_NUM  360        // 粒子（海洋球）总数量
#define SUB_STEPS 2          // 物理子迭代步数
#define DTIME    0.36f       // 单步物理时间系数

#define DUCK_SIZE    11     // 小黄鸭碰撞体半径
#define BALL_SIZE    2      // 海洋球半径
#define DAMP         0.97f  // 速度阻尼系数
#define REPEL        2.8f   // 粒子短程排斥力（流体核心）
#define BOUNCE       0.6f   // 边界弹性系数
#define GRAV_FACTOR  1.5f   // 重力场强度（灵敏度）

BALL_NUM（粒子总数）
控制屏幕中海洋球的数量
数值越大，流体密度越高、画面越饱满
数值过大会导致设备性能下降、卡顿
推荐范围：100 ~ 400（M5StickS3 最优值 300~360）
SUB_STEPS（物理子步数）
控制物理计算精度，每帧执行多次微小物理更新
数值越大：物理更稳定、流体效果更真实，但更消耗性能
数值越小：运行更快，但可能出现粒子穿透、抖动
推荐范围：1 ~ 3
DTIME（时间步长）
控制整体流速 / 运动速度
数值越大 → 所有物体移动越快，反应更灵敏
数值越小 → 移动越慢、越轻柔
当前 0.36 = 标准速度的 2 倍速


DUCK_SIZE（鸭子大小）
小黄鸭的物理碰撞半径
决定鸭子占多大空间、能推开多少粒子
仅影响物理，不直接控制绘制大小
BALL_SIZE（小球半径）
单个海洋球的物理 + 显示大小
越小，粒子越密集，流体效果越细腻
DAMP（阻尼）
控制速度衰减，模拟空气阻力 / 液体粘性
越接近 1：阻力越小，滑动越远、越丝滑
越小：阻力越大，物体移动越慢、容易停下
流体推荐值：0.95 ~ 0.99
REPEL（排斥强度）
流体效果核心参数
控制粒子之间、粒子与鸭子的推开力度
数值越大：粒子越 “挤”，越像高密度液体
数值越小：粒子越松散，接近气体
BOUNCE（弹性）
物体撞到屏幕边界时的反弹强度
1 = 完全弹性反弹
0 = 不反弹，直接贴墙
推荐范围：0.4 ~ 0.8
GRAV_FACTOR（重力 / 灵敏度）
控制体感倾斜灵敏度
数值越大，倾斜设备时物体移动越快
数值越小，反应越平缓


#define BACKGROUND   0x080C  // 背景色
#define COLOR_DUCK   0xFFE0  // 小黄鸭主体颜色
#define COLOR_CHEEK  0xF86B  // 鸭子腮红颜色
#define COLOR_BEAK   0xFE60  // 鸭子嘴巴颜色

const uint16_t COLOR_BALL[] = {
  0xF86B, 0x4EF7, 0xFFE6, 0x1A53,
  0xFF9F, 0x55EF, 0x06D6, 0x48C9
};

参数含义
BACKGROUND：屏幕背景底色
COLOR_DUCK：小黄鸭身体主色调
COLOR_CHEEK：鸭子腮红颜色
COLOR_BEAK：鸭子嘴巴颜色
COLOR_BALL[8]：8 种彩色海洋球的配色数组
格式：RGB565（M5Stick 屏幕专用 16 位色值

## 库依赖

- M5Unified: https://github.com/m5stack/M5Unified

## 许可证

MIT License
