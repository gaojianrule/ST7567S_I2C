#include <ST7567S_I2C.h>

ST7567S_I2C lcd;

void setup() {
  Serial.begin(115200);
  // ESP32-C3 Mini: SDA=4, SCL=5, optional LCD RESET=6.
  // Four-pin screen: use -1 instead of 6. Address 0 = auto scan.
  if (!lcd.begin(4, 5, 6, 0)) {
    Serial.println("LCD not found or I2C error");
    while (true) delay(1000);
  }
  lcd.setContrast(38); // 0..63; does not change backlight brightness
  lcd.clear();
  lcd.drawRect(0, 0, 128, 64); // Border should reach all four edges
  lcd.setCursor(7, 8);
  lcd.println("HELLO WORLD");
  lcd.setCursor(7, 24);
  lcd.println("ST7567S 128X64");
  lcd.setCursor(7, 40);
  lcd.print("I2C: 0X"); lcd.print(lcd.address(), HEX);
  lcd.display(); // Drawing alone does not update the screen
}

void loop() {}
