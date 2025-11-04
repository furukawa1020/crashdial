#include <M5Dial.h>

void setup() {
  Serial.begin(115200);
  delay(500);
  
  auto cfg = M5.config();
  M5Dial.begin(cfg, true, false);
  
  M5Dial.Display.fillScreen(TFT_RED);
  delay(200);
  M5Dial.Display.fillScreen(TFT_BLACK);
  
  M5Dial.Display.setTextColor(TFT_WHITE);
  M5Dial.Display.setTextSize(2);
  M5Dial.Display.setCursor(20, 80);
  M5Dial.Display.println("M5DIAL");
  M5Dial.Display.println("ENCODER TEST");
  M5Dial.Display.println("Rotate dial!");
}

void loop() {
  M5Dial.update();
  
  static int c = 0;
  c++;
  
  static long last_encoder = -999;
  long encoder_value = M5Dial.Encoder.read();
  long delta = encoder_value - last_encoder;
  
  if (c % 50 == 0 || delta != 0) {
    M5Dial.Display.fillRect(0, 140, 240, 100, TFT_BLACK);
    M5Dial.Display.setCursor(10, 140);
    M5Dial.Display.setTextSize(2);
    M5Dial.Display.setTextColor(TFT_WHITE);
    
    M5Dial.Display.printf("Loop: %d\n", c);
    M5Dial.Display.printf("Encoder: %ld\n", encoder_value);
    
    if (delta != 0) {
      M5Dial.Display.setTextColor(TFT_YELLOW);
      M5Dial.Display.printf("Delta: %ld\n", delta);
      M5Dial.Display.setTextColor(TFT_GREEN);
      M5Dial.Display.printf(">>> MOVING!");
      last_encoder = encoder_value;
    }
  }
  
  delay(20);
}
