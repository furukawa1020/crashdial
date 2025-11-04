/**
 * M5Dial Encoder Test
 */
#include <M5Unified.h>

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n=== M5DIAL ENCODER TEST ===");
  
  M5.begin();
  Serial.println("M5.begin() OK");
  
  M5.Display.fillScreen(TFT_RED);
  delay(500);
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(10, 60);
  M5.Display.println("ENCODER TEST");
  M5.Display.setCursor(10, 90);
  M5.Display.println("Rotate dial!");
  
  Serial.println("Display OK");
  Serial.println("Rotate the dial now!\n");
}

void loop() {
  M5.update();
  
  static int c = 0;
  c++;
  
  // I2Cから4バイト読み取り（エンコーダ）
  uint8_t data[4] = {0};
  bool success = M5.In_I2C.readRegister(0x40, 0x10, data, 4, 400000);
  
  // 全てのバイト組み合わせを計算
  static int16_t last_val = 0;
  int16_t val_bytes23 = (int16_t)((data[2] << 8) | data[3]);
  int16_t delta = val_bytes23 - last_val;
  
  // 30回ごと、または変化があったときに出力
  if (c % 30 == 0 || delta != 0) {
    Serial.printf("[%d] I2C:%s Raw:%02X %02X %02X %02X | Val=%d", 
                  c, success?"OK":"FAIL", data[0], data[1], data[2], data[3], val_bytes23);
    
    if (delta != 0) {
      Serial.printf(" <<< DELTA=%d", delta);
      
      // 画面更新
      M5.Display.fillRect(10, 120, 220, 80, TFT_BLACK);
      M5.Display.setCursor(10, 120);
      M5.Display.setTextSize(2);
      M5.Display.printf("Enc: %d\n", val_bytes23);
      M5.Display.printf("Delta: %d\n", delta);
      M5.Display.printf("Loop: %d", c);
    }
    Serial.println();
    
    last_val = val_bytes23;
  }
  
  delay(20);
}
