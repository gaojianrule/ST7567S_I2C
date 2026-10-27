#include <ST7567S_I2C.h>

ST7567S_I2C lcd;
uint8_t contrast = 38, offset = 0;
bool inverted = false, flipped = false, enabled = true;

void redraw() {
  lcd.clear(); lcd.drawRect(0, 0, 128, 64);
  lcd.setCursor(6, 6); lcd.print("CONTRAST: "); lcd.print(contrast);
  lcd.setCursor(6, 18); lcd.print("OFFSET: "); lcd.print(offset);
  lcd.setCursor(6, 30); lcd.print("ABCDEF 012345");
  lcd.setCursor(6, 44); lcd.print("ST7567S TEST");
  lcd.display();
  Serial.printf("Contrast=%u Offset=%u Errors=%lu\n", contrast, offset, (unsigned long)lcd.errors());
}

void setup() {
  Serial.begin(115200);
  if (!lcd.begin(4, 5, 6)) {
    Serial.println("LCD init failed");
    while (true) delay(1000);
  }
  Serial.println("+/- contrast; o offset; i invert; f rotate; s display on/off");
  redraw();
}

void loop() {
  if (!Serial.available()) return;
  switch (Serial.read()) {
    case '+': if (contrast < 63) ++contrast; lcd.setContrast(contrast); break;
    case '-': if (contrast > 0) --contrast; lcd.setContrast(contrast); break;
    case 'o': offset = (offset + 1) % 5; lcd.setColumnOffset(offset); break;
    case 'i': inverted = !inverted; lcd.setInverted(inverted); break;
    case 'f': flipped = !flipped; lcd.setFlipped(flipped); break;
    case 's': enabled = !enabled; lcd.setDisplayEnabled(enabled); break;
    default: return;
  }
  redraw();
}
