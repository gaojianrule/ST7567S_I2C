#include <ST7567S_I2C.h>

ST7567S_I2C lcd;
unsigned page = 0;
uint32_t lastChange = 0;

// Origin (0,0) is top left. x increases right; y increases down.
// Valid pixels: x=0..127, y=0..63. Out-of-screen pixels are clipped.
void showPage() {
  lcd.clear();
  lcd.setTextSize(1); lcd.setTextColor(true);
  lcd.setCursor(0, 0);
  switch (page % 6) {
    case 0: // Points and lines: endpoints are included
      lcd.print("POINTS AND LINES");
      for (int y = 16; y < 64; y += 8)
        for (int x = 4; x < 56; x += 8) lcd.drawPixel(x, y);
      lcd.drawLine(64, 16, 127, 63);
      lcd.drawLine(64, 63, 127, 16);
      lcd.drawLine(64, 40, 127, 40);
      break;
    case 1: // Rectangles: x,y,width,height (not opposite corner)
      lcd.print("RECT AND FILL");
      lcd.drawRect(2, 16, 54, 44);
      lcd.fillRect(66, 16, 58, 44);
      lcd.fillRect(74, 24, 42, 28, false); // false erases pixels
      break;
    case 2: // Circles: center x,y,radius
      lcd.print("CIRCLES");
      lcd.drawCircle(30, 38, 22);
      lcd.drawCircle(30, 38, 12);
      lcd.fillCircle(94, 38, 22);
      lcd.fillCircle(94, 38, 10, false);
      break;
    case 3: // Text: each glyph occupies 6*size by 8*size pixels
      lcd.print("TEXT SIZE 1");
      lcd.setTextSize(2); lcd.setCursor(0, 14); lcd.print("ESP32 C3");
      lcd.setTextSize(1); lcd.setCursor(0, 34);
      lcd.print("VALUE: "); lcd.print(123);
      lcd.fillRect(0, 48, 128, 16);
      lcd.setTextColor(false); lcd.setCursor(6, 52); lcd.print("REVERSED TEXT");
      break;
    case 4: { // 16x16 bitmap, two bytes per row; 1=draw, 0=transparent
      lcd.print("BITMAP AND BARS");
      const uint8_t icon[] = {
        0x03,0xC0, 0x0F,0xF0, 0x1C,0x38, 0x30,0x0C,
        0x60,0x06, 0x66,0x66, 0x66,0x66, 0x60,0x06,
        0x60,0x06, 0x64,0x26, 0x32,0x4C, 0x19,0x98,
        0x0F,0xF0, 0x03,0xC0, 0x00,0x00, 0x00,0x00
      };
      lcd.drawBitmap(4, 24, icon, 16, 16);
      for (int i = 0; i < 5; ++i) lcd.fillRect(35+i*18, 60-(i+1)*8, 12, (i+1)*8);
      break;
    }
    case 5: // A filled area built from individual pixels
      lcd.print("CHECKERBOARD");
      for (int y = 12; y < 64; ++y)
        for (int x = 0; x < 128; ++x)
          if (((x / 8) + (y / 8)) & 1) lcd.drawPixel(x,y);
      break;
  }
  if (!lcd.display()) Serial.println("I2C write failed");
}

void setup() {
  Serial.begin(115200);
  if (!lcd.begin(4, 5, 6)) {
    Serial.println("Check LCD wiring");
    while (true) delay(1000);
  }
  lcd.setContrast(38);
  showPage(); lastChange = millis();
}

void loop() {
  if (millis() - lastChange >= 3000) {
    lastChange = millis(); ++page; showPage();
  }
}
