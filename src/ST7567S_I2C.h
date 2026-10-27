#pragma once
#include <Arduino.h>
#include <Wire.h>

// Sorted Unicode codepoints and fixed-size, row-major monochrome glyphs in Flash.
struct ST7567S_ChineseFont {
  uint8_t size;
  uint16_t count;
  const uint32_t *codepoints;
  const uint8_t *bitmaps;
};

// Coordinates: top left (0,0), bottom right (127,63).
// Drawing changes RAM only; call display() to update the LCD.
class ST7567S_I2C : public Print {
public:
  static constexpr int16_t WIDTH = 128, HEIGHT = 64;
  explicit ST7567S_I2C(TwoWire &wire = Wire) : wire_(wire) {}
  // address=0 auto-detects 0x3C..0x3F; reset=-1 for four-pin modules.
  bool begin(int sda = 4, int scl = 5, int reset = -1,
             uint8_t address = 0, uint32_t frequency = 100000);
  bool display();
  void clear(bool color = false);
  void drawPixel(int16_t x, int16_t y, bool color = true);
  bool getPixel(int16_t x, int16_t y) const;
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool color = true);
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, bool color = true);
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, bool color = true);
  void drawCircle(int16_t x, int16_t y, int16_t r, bool color = true);
  void fillCircle(int16_t x, int16_t y, int16_t r, bool color = true);
  // Row-major bitmap: each row is ceil(w/8) bytes, bit 7 is leftmost.
  void drawBitmap(int16_t x, int16_t y, const uint8_t *bits, int16_t w,
                  int16_t h, bool color = true);
  void setCursor(int16_t x, int16_t y) { cursorX_ = x; cursorY_ = y; }
  void setTextSize(uint8_t size) { textSize_ = constrain(size, 1, 8); }
  void setTextColor(bool color) { textColor_ = color; }
  void setTextWrap(bool wrap) { wrap_ = wrap; }
  void drawChar(int16_t x, int16_t y, char c, uint8_t size = 1, bool color = true);
  // UTF-8 text; ASCII uses the built-in font, Chinese uses the selected subset.
  // Returns false on malformed UTF-8 or a missing glyph (drawn as a box).
  bool drawUTF8(int16_t x, int16_t y, const char *text,
                const ST7567S_ChineseFont &font, bool color = true);
  // Width of the widest line without wrapping; measured in pixels.
  uint32_t textWidthUTF8(const char *text, const ST7567S_ChineseFont &font) const;
  using Print::write;
  size_t write(uint8_t c) override;
  bool setContrast(uint8_t value);
  bool setInverted(bool inverted);
  bool setDisplayEnabled(bool enabled);
  bool setFlipped(bool flipped);
  void setColumnOffset(uint8_t offset) { offset_ = min(offset, uint8_t(4)); }
  uint8_t address() const { return address_; }
  uint8_t contrast() const { return contrast_; }
  uint32_t errors() const { return errors_; }
private:
  bool command(uint8_t value);
  TwoWire &wire_;
  uint8_t buffer_[1024] = {};
  uint8_t address_ = 0, contrast_ = 38, offset_ = 0, textSize_ = 1;
  int16_t cursorX_ = 0, cursorY_ = 0;
  bool ready_ = false, textColor_ = true, wrap_ = true;
  uint32_t errors_ = 0;
};
