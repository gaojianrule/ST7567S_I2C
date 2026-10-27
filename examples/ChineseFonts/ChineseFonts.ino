#include <ST7567S_I2C.h>
#include <ST7567S_ChineseFonts.h>

ST7567S_I2C lcd;
unsigned page = 0;
uint32_t lastChange = 0;

void drawPage() {
  lcd.clear();
  // Save this sketch as UTF-8. Chinese uses drawUTF8; print remains ASCII.
  // Font size is independent of setTextSize(). Coordinates are in pixels.
  // 12px: 10 Chinese characters per line, up to 5 lines.
  // 16px: 8 Chinese characters per line, up to 4 lines.
  // 24px: 5 Chinese characters per line, up to 2 lines.
  switch (page % 4) {
    case 0:
      lcd.drawUTF8(0, 0, "中文显示测试", ST7567S_CN12);
      lcd.drawUTF8(0, 13, "小字体适合状态菜单", ST7567S_CN12);
      lcd.drawUTF8(0, 26, "温度: 25.6℃", ST7567S_CN12);
      lcd.drawUTF8(0, 39, "湿度: 60%", ST7567S_CN12);
      lcd.drawUTF8(0, 52, "设备运行正常", ST7567S_CN12);
      break;
    case 1:
      lcd.drawUTF8(0, 0, "中文显示测试", ST7567S_CN16);
      lcd.drawUTF8(0, 16, "中字体菜单", ST7567S_CN16);
      lcd.drawUTF8(0, 32, "温度: 25℃", ST7567S_CN16);
      lcd.drawUTF8(0, 48, "湿度: 60%", ST7567S_CN16);
      break;
    case 2:
      lcd.drawUTF8(4, 4, "大字体测试", ST7567S_CN24);
      lcd.drawUTF8(4, 36, "运行正常", ST7567S_CN24);
      break;
    case 3: {
      // Center using measured width. Disable wrapping for centered text.
      lcd.setTextWrap(false);
      const char *title = "你好世界";
      int x = (128 - int(lcd.textWidthUTF8(title, ST7567S_CN16))) / 2;
      lcd.drawUTF8(x, 2, title, ST7567S_CN16);
      lcd.drawLine(0, 21, 127, 21);
      lcd.fillRect(0, 28, 128, 20);
      lcd.drawUTF8(16, 30, "设备已连接", ST7567S_CN16, false);
      lcd.drawUTF8(4, 51, "中文混合 ESP32", ST7567S_CN12);
      lcd.setTextWrap(true);
      break;
    }
  }
  if (!lcd.display()) Serial.println("I2C error");
}

void setup() {
  Serial.begin(115200);
  // Four-pin screen: replace 6 with -1. Other ESP32 boards: change pins.
  if (!lcd.begin(4, 5, 6)) {
    Serial.println("LCD init failed");
    while (true) delay(1000);
  }
  lcd.setContrast(38);
  drawPage(); lastChange = millis();
}

void loop() {
  if (millis() - lastChange >= 3500) {
    lastChange = millis(); ++page; drawPage();
  }
}
