#include "ST7567S_I2C.h"

namespace {
// Consume one UTF-8 codepoint; reject overlong sequences and surrogate values.
uint32_t nextCodepoint(const char *&text) {
  uint8_t a = static_cast<uint8_t>(*text++);
  if (a < 0x80) return a;
  uint8_t count; uint32_t value, minimum;
  if (a >= 0xC2 && a <= 0xDF) { count = 1; value = a & 0x1F; minimum = 0x80; }
  else if (a >= 0xE0 && a <= 0xEF) { count = 2; value = a & 0x0F; minimum = 0x800; }
  else if (a >= 0xF0 && a <= 0xF4) { count = 3; value = a & 7; minimum = 0x10000; }
  else return 0xFFFD;
  for (uint8_t i = 0; i < count; ++i) {
    uint8_t b = static_cast<uint8_t>(*text);
    if ((b & 0xC0) != 0x80) return 0xFFFD;
    ++text; value = (value << 6) | (b & 0x3F);
  }
  if (value < minimum || value > 0x10FFFF || (value >= 0xD800 && value <= 0xDFFF)) return 0xFFFD;
  return value;
}
uint8_t asciiScale(const ST7567S_ChineseFont &font) { return font.size >= 24 ? 2 : 1; }
int findGlyph(const ST7567S_ChineseFont &font, uint32_t codepoint) {
  int low = 0, high = font.count - 1;
  while (low <= high) {
    int mid = (low + high) / 2;
    uint32_t candidate = font.codepoints[mid];
    if (candidate == codepoint) return mid;
    if (candidate < codepoint) low = mid + 1; else high = mid - 1;
  }
  return -1;
}
}

uint32_t ST7567S_I2C::textWidthUTF8(const char *text, const ST7567S_ChineseFont &font) const {
  if (!text || !font.size) return 0;
  uint32_t width = 0, widest = 0;
  while (*text) {
    uint32_t cp = nextCodepoint(text);
    if (cp == '\r') continue;
    if (cp == '\n') { widest = max(widest, width); width = 0; continue; }
    width += cp < 128 ? 6 * asciiScale(font) : font.size;
  }
  return max(widest, width);
}

bool ST7567S_I2C::drawUTF8(int16_t x, int16_t y, const char *text,
                          const ST7567S_ChineseFont &font, bool color) {
  if (!text || !font.size || !font.codepoints || !font.bitmaps) return false;
  int32_t px = x, py = y;
  bool complete = true;
  const uint16_t stride = (font.size + 7) / 8;
  while (*text) {
    uint32_t cp = nextCodepoint(text);
    if (cp == '\r') continue;
    if (cp == '\n') { px = x; py += font.size; continue; }
    int width = cp < 128 ? 6 * asciiScale(font) : font.size;
    if (wrap_ && px != x && px + width > WIDTH) { px = x; py += font.size; }
    int index = cp < 128 ? -1 : findGlyph(font, cp);
    if (cp >= 128 && index < 0) complete = false;
    if (py < HEIGHT && py + font.size > 0 && px < WIDTH && px + width > 0) {
      if (cp < 128) {
        drawChar(px, py + (font.size - 8 * asciiScale(font)) / 2, cp, asciiScale(font), color);
      } else if (index >= 0) {
        drawBitmap(px, py, font.bitmaps + uint32_t(index) * stride * font.size,
                   font.size, font.size, color);
      } else {
        drawRect(px + 1, py + 1, font.size - 2, font.size - 2, color);
      }
    }
    px += width;
  }
  return complete;
}
