/**
 * M5Dial Minimum Test - 最小限の動作確認
 */

#include <M5Unified.h>

void setup() {
  // M5Dial初期化
  M5.begin();
  
  // ディスプレイ設定
  M5.Display.setRotation(0);
  M5.Display.setBrightness(128);
  M5.Display.fillScreen(TFT_RED);  // 赤で塗りつぶし
  
  delay(2000);
  
  // 黒背景に白い円
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.fillCircle(120, 120, 80, TFT_WHITE);
  
  // テキスト表示
  M5.Display.setTextColor(TFT_GREEN);
  M5.Display.setTextSize(3);
  M5.Display.setCursor(70, 110);
  M5.Display.println("M5Dial");
}

void loop() {
  M5.update();
  delay(10);
}
