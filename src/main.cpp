/**
 * GlassDial - 触れる破壊、手の中の再生
 * 完全実装版
 */
#include <M5Dial.h>
#include <vector>

// ========== 状態定義 ==========
enum GlassState {
  NORMAL,    // 通常状態 (0.0~0.15)
  CRACK,     // ひび割れ (0.15~0.65)
  SHATTER,   // 粉砕 (0.65~0.85)
  SILENCE,   // 沈黙 (0.85~1.0)
  REBUILD,   // 再構築 (1.00.5)
  RECOVERY   // 回復 (0.50.0)
};

// ========== パーティクル構造体 ==========
struct Particle {
  float x, y;
  float vx, vy;
  float alpha;
  uint16_t color;
  
  Particle(float px, float py, float angle, float speed) {
    x = px;
    y = py;
    vx = cos(angle) * speed;
    vy = sin(angle) * speed;
    alpha = 255.0f;
    color = TFT_WHITE;
  }
  
  void update() {
    x += vx;
    y += vy;
    vy += 0.3f; // 重力
    vx *= 0.98f; // 減衰
    vy *= 0.98f;
    alpha *= 0.95f;
  }
  
  bool isAlive() { return alpha > 10.0f; }
};

// ========== ひび割れ構造体 ==========
struct Crack {
  float x1, y1, x2, y2;
  int generation;
  
  Crack(float px1, float py1, float px2, float py2, int gen) {
    x1 = px1; y1 = py1;
    x2 = px2; y2 = py2;
    generation = gen;
  }
};

// ========== グローバル変数 ==========
GlassState currentState = NORMAL;
float destructionLevel = 0.0f;
std::vector<Crack> cracks;
std::vector<Particle> particles;
unsigned long lastActivityTime = 0;
unsigned long lastStateChangeTime = 0;
bool wasInDestructiveState = false;

// ========== 定数定義 ==========
const float DESTRUCTION_INCREMENT = 0.003f;
const float RECOVERY_SPEED = 0.002f;
const unsigned long IDLE_TIMEOUT = 10000; // 10秒
const int MAX_CRACKS = 80;
const int MAX_PARTICLES = 150;
const int SCREEN_CENTER_X = 120;
const int SCREEN_CENTER_Y = 120;

// ========== 音響定義 ==========
const int SOUND_CRACK = 1500;
const int SOUND_SHATTER = 2000;
const int SOUND_SILENCE = 500;
const int SOUND_REBUILD = 800;
const int SOUND_RECOVERY = 1200;

// ========== 関数プロトタイプ ==========
void updateState();
void renderGlass();
void addCrack(float x, float y, float angle, int generation);
void createShatterParticles(int count);
void playStateSound(GlassState state);
uint16_t getStateColor();

// ========== セットアップ ==========
void setup() {
  auto cfg = M5.config();
  M5Dial.begin(cfg, true, false);
  
  M5Dial.Display.fillScreen(TFT_BLACK);
  M5Dial.Display.setTextDatum(middle_center);
  M5Dial.Display.setTextSize(1);
  M5Dial.Display.setTextColor(TFT_WHITE);
  
  const char* title = "GlassDial";
  const char* subtitle = "Rotate to break";
  M5Dial.Display.drawString(title, SCREEN_CENTER_X, SCREEN_CENTER_Y - 20);
  M5Dial.Display.drawString(subtitle, SCREEN_CENTER_X, SCREEN_CENTER_Y + 20);
  
  delay(2000);
  lastActivityTime = millis();
  lastStateChangeTime = millis();
}

// ========== メインループ ==========
void loop() {
  M5Dial.update();
  
  // エンコーダ読み取り
  static long lastEncoder = 0;
  long encoderValue = M5Dial.Encoder.read();
  long delta = abs(encoderValue - lastEncoder);
  
  if (delta > 0) {
    lastActivityTime = millis();
    
    // 破壊レベル増加
    if (currentState != REBUILD && currentState != RECOVERY) {
      destructionLevel += delta * DESTRUCTION_INCREMENT;
      if (destructionLevel > 1.0f) destructionLevel = 1.0f;
    }
    
    lastEncoder = encoderValue;
  }
  
  // アイドルタイムアウト処理
  unsigned long idleTime = millis() - lastActivityTime;
  if (idleTime > IDLE_TIMEOUT && destructionLevel > 0.0f) {
    if (currentState != REBUILD && currentState != RECOVERY) {
      wasInDestructiveState = true;
    }
  }
  
  // 自動回復
  if (wasInDestructiveState && idleTime > IDLE_TIMEOUT) {
    if (currentState == SILENCE || currentState == SHATTER || currentState == CRACK) {
      currentState = REBUILD;
      playStateSound(REBUILD);
      lastStateChangeTime = millis();
      wasInDestructiveState = false;
    }
  }
  
  if (currentState == REBUILD) {
    destructionLevel -= RECOVERY_SPEED;
    if (destructionLevel <= 0.5f) {
      currentState = RECOVERY;
      playStateSound(RECOVERY);
      lastStateChangeTime = millis();
    }
  }
  
  if (currentState == RECOVERY) {
    destructionLevel -= RECOVERY_SPEED;
    if (destructionLevel <= 0.0f) {
      destructionLevel = 0.0f;
      currentState = NORMAL;
      cracks.clear();
      particles.clear();
    }
  }
  
  // 状態更新
  updateState();
  
  // 描画
  renderGlass();
  
  delay(16); // 約60FPS
}

// ========== 状態更新 ==========
void updateState() {
  GlassState newState = currentState;
  
  if (destructionLevel < 0.15f) {
    newState = NORMAL;
  } else if (destructionLevel < 0.65f) {
    newState = CRACK;
  } else if (destructionLevel < 0.85f) {
    newState = SHATTER;
  } else if (destructionLevel >= 0.85f) {
    newState = SILENCE;
  }
  
  // 状態遷移処理
  if (newState != currentState && currentState != REBUILD && currentState != RECOVERY) {
    GlassState oldState = currentState;
    currentState = newState;
    
    // 状態遷移時の処理
    if (currentState == CRACK && oldState == NORMAL) {
      float angle = random(0, 628) / 100.0f;
      addCrack(SCREEN_CENTER_X, SCREEN_CENTER_Y, angle, 0);
      playStateSound(CRACK);
    }
    else if (currentState == SHATTER && oldState == CRACK) {
      createShatterParticles(50);
      playStateSound(SHATTER);
    }
    else if (currentState == SILENCE && oldState == SHATTER) {
      playStateSound(SILENCE);
    }
    
    lastStateChangeTime = millis();
  }
  
  // CRACK状態では定期的にひび追加
  if (currentState == CRACK && cracks.size() < MAX_CRACKS) {
    if ((millis() - lastStateChangeTime) % 500 < 16) {
      if (cracks.size() > 0) {
        int idx = random(0, cracks.size());
        Crack& c = cracks[idx];
        if (c.generation < 3) {
          float midX = (c.x1 + c.x2) / 2.0f;
          float midY = (c.y1 + c.y2) / 2.0f;
          float angle = atan2(c.y2 - c.y1, c.x2 - c.x1) + random(-157, 157) / 100.0f;
          addCrack(midX, midY, angle, c.generation + 1);
        }
      }
    }
  }
}

// ========== ひび割れ追加 ==========
void addCrack(float x, float y, float angle, int generation) {
  if (cracks.size() >= MAX_CRACKS) return;
  
  float length = 40.0f / (generation + 1);
  float x2 = x + cos(angle) * length;
  float y2 = y + sin(angle) * length;
  
  cracks.push_back(Crack(x, y, x2, y2, generation));
  
  // 分岐
  if (generation < 3 && random(0, 100) > 50) {
    float branchAngle1 = angle + random(30, 90) / 100.0f;
    float branchAngle2 = angle - random(30, 90) / 100.0f;
    addCrack(x2, y2, branchAngle1, generation + 1);
    addCrack(x2, y2, branchAngle2, generation + 1);
  }
}

// ========== パーティクル生成 ==========
void createShatterParticles(int count) {
  for (int i = 0; i < count && particles.size() < MAX_PARTICLES; i++) {
    float angle = random(0, 628) / 100.0f;
    float speed = random(10, 40) / 10.0f;
    float x = SCREEN_CENTER_X + random(-30, 30);
    float y = SCREEN_CENTER_Y + random(-30, 30);
    particles.push_back(Particle(x, y, angle, speed));
  }
}

// ========== 音再生 ==========
void playStateSound(GlassState state) {
  int freq = 0;
  int duration = 100;
  
  switch (state) {
    case CRACK: freq = SOUND_CRACK; break;
    case SHATTER: freq = SOUND_SHATTER; duration = 200; break;
    case SILENCE: freq = SOUND_SILENCE; duration = 300; break;
    case REBUILD: freq = SOUND_REBUILD; duration = 150; break;
    case RECOVERY: freq = SOUND_RECOVERY; duration = 150; break;
    default: return;
  }
  
  M5Dial.Speaker.tone(freq, duration);
}

// ========== 描画 ==========
void renderGlass() {
  M5Dial.Display.fillScreen(TFT_BLACK);
  
  // 状態による背景色
  uint16_t bgColor = getStateColor();
  if (bgColor != TFT_BLACK) {
    int alpha = (int)(destructionLevel * 255);
    M5Dial.Display.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, 80, bgColor);
  }
  
  // ひび割れ描画
  for (auto& crack : cracks) {
    uint16_t color = TFT_WHITE;
    if (currentState == SILENCE) color = TFT_DARKGREY;
    M5Dial.Display.drawLine((int)crack.x1, (int)crack.y1, (int)crack.x2, (int)crack.y2, color);
  }
  
  // パーティクル更新と描画
  for (int i = particles.size() - 1; i >= 0; i--) {
    particles[i].update();
    if (!particles[i].isAlive()) {
      particles.erase(particles.begin() + i);
    } else {
      int x = (int)particles[i].x;
      int y = (int)particles[i].y;
      if (x >= 0 && x < 240 && y >= 0 && y < 240) {
        M5Dial.Display.drawPixel(x, y, TFT_WHITE);
      }
    }
  }
  
  // 破壊レベル表示
  M5Dial.Display.setTextDatum(top_center);
  M5Dial.Display.setTextSize(1);
  M5Dial.Display.setTextColor(TFT_WHITE);
  M5Dial.Display.drawString(String(destructionLevel, 2), SCREEN_CENTER_X, 10);
  
  // 状態名表示
  const char* stateName = nullptr;
  const char* s_normal = "NORMAL";
  const char* s_crack = "CRACK";
  const char* s_shatter = "SHATTER";
  const char* s_silence = "SILENCE";
  const char* s_rebuild = "REBUILD";
  const char* s_recovery = "RECOVERY";
  
  switch (currentState) {
    case NORMAL: stateName = s_normal; break;
    case CRACK: stateName = s_crack; break;
    case SHATTER: stateName = s_shatter; break;
    case SILENCE: stateName = s_silence; break;
    case REBUILD: stateName = s_rebuild; break;
    case RECOVERY: stateName = s_recovery; break;
  }
  
  if (stateName) {
    M5Dial.Display.setTextDatum(bottom_center);
    M5Dial.Display.drawString(stateName, SCREEN_CENTER_X, 230);
  }
}

// ========== 状態色取得 ==========
uint16_t getStateColor() {
  switch (currentState) {
    case CRACK: return M5Dial.Display.color565(40, 40, 60);
    case SHATTER: return M5Dial.Display.color565(80, 20, 20);
    case SILENCE: return M5Dial.Display.color565(20, 20, 20);
    case REBUILD: return M5Dial.Display.color565(20, 60, 40);
    case RECOVERY: return M5Dial.Display.color565(40, 80, 60);
    default: return TFT_BLACK;
  }
}
