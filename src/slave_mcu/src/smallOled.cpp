#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#include <TFLI2C.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
static const uint8_t OLED_ADDR = 0x3C;   // 如果I2C扫描是0x3D就改

// ===== TF-Luna =====
TFLI2C tfl;
static const uint8_t TF_ADDR = 0x10;     // 你扫描确认过

// ===== 90° Radar params (适配128x64 扇形) =====
// 扇形雷达建议中心放在屏幕底部中间
static const int16_t cx = 64;
static const int16_t cy = 63;
static const int16_t R  = 60;

static const int16_t Dmax_cm = 200;      // 最大显示距离（可改）

// 0..90° 扫描：每个角度存一个距离（cm）
static int16_t distAtDeg[91];            // 0..90 都能存（更安全）

// 距离(cm) -> 像素半径
static int16_t distToRadius(int16_t d_cm) {
  if (d_cm <= 0 || d_cm > Dmax_cm) return -1;
  long r = (long)d_cm * R / Dmax_cm;
  if (r < 1) r = 1;
  if (r > R) r = R;
  return (int16_t)r;
}

// 0..90° -> 屏幕坐标（让 45° 指向正前方）
// deg=0  -> 左前(-45°)
// deg=45 -> 正前(0°)
// deg=90 -> 右前(+45°)
static void polarToXY90(int16_t deg, int16_t r, int16_t &x, int16_t &y) {
  if (deg < 0) deg = 0;
  if (deg > 90) deg = 90;

  // 数学角度：让正前方向朝上
  // th = 90° - (deg-45°) = 135° - deg
  float th = (135.0f - deg) * (float)M_PI / 180.0f;

  x = (int16_t)lroundf(cx + r * cosf(th));
  y = (int16_t)lroundf(cy - r * sinf(th));
}

// 读 TF-Luna 距离（cm）
static int16_t readLidarCm() {
  int16_t dist = 0;
  if (tfl.getData(dist, TF_ADDR)) return dist;
  return -1;
}

// 存点：更新当前角度的距离
static void pushPolarPoint90(int16_t deg, int16_t dist_cm) {
  if (deg < 0) deg = 0;
  if (deg > 90) deg = 90;
  distAtDeg[deg] = dist_cm;
}

// 画雷达背景（90°扇形 + 同心弧线 + 边界线）
static void drawRadarBackground90() {
  display.clearDisplay();

  // 同心弧线：用圆代替（下半部分也画出来没关系，视觉上OK）
  display.drawCircle(cx, cy, 15, SSD1306_WHITE);
  display.drawCircle(cx, cy, 30, SSD1306_WHITE);
  display.drawCircle(cx, cy, 45, SSD1306_WHITE);
  display.drawCircle(cx, cy, 60, SSD1306_WHITE);

  // FOV边界线（0°和90°）
  int16_t x1, y1, x2, y2, xm, ym;
  polarToXY90(0,  R, x1, y1);
  polarToXY90(90, R, x2, y2);
  polarToXY90(45, R, xm, ym);   // 中线（正前）

  display.drawLine(cx, cy, x1, y1, SSD1306_WHITE);
  display.drawLine(cx, cy, x2, y2, SSD1306_WHITE);
  display.drawLine(cx, cy, xm, ym, SSD1306_WHITE);
}

// 画扫描线（用当前角度）
static void drawSweepLine90(int16_t deg) {
  int16_t x, y;
  polarToXY90(deg, R, x, y);
  display.drawLine(cx, cy, x, y, SSD1306_WHITE);
}

// 画所有已记录点
static void drawAllPoints90() {
  for (int deg = 0; deg <= 90; deg += 8) {     // 每2度画一次
    int16_t d = distAtDeg[deg];
    int16_t r = distToRadius(d);
    if (r < 0) continue;

    int16_t x, y;
    polarToXY90(deg, r, x, y);
    display.fillCircle(x, y, 1, SSD1306_WHITE);
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin();
  Wire.setClock(400000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found (0x3C/0x3D?)");
    while (1) {}
  }

  // 初始化点缓存
  for (int i = 0; i <= 90; i++) distAtDeg[i] = -1;

  display.clearDisplay();
  display.display();

  Serial.println("90deg Radar UI start");
}

void loop() {
  // 1) 读真实距离
  int16_t distCm = readLidarCm();

  // 2) 角度（现在先用“模拟舵机扫角度” 0..90 来回）
  static int16_t angle = 0;
  static int8_t dir = 1;

  angle += dir * 2;
  if (angle >= 90) dir = -1;
  if (angle <= 0)  dir = 1;

  // 3) 存点
  if (distCm > 0) pushPolarPoint90(angle, distCm);

  // 4) 渲染
  drawRadarBackground90();
  drawSweepLine90(angle);
  drawAllPoints90();

  // 文本
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("d=");
  if (distCm > 0) display.print(distCm);
  else display.print("ERR");
  display.print("cm  a=");
  display.print(angle);

  display.display();
  delay(40);
}