#include "ST7567S_I2C.h"
#include <ctype.h>
#include <string.h>

static const char chars[] = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:-./%!?+=_()><";
static const uint8_t glyphs[][5] = {
 {0,0,0,0,0},{62,81,73,69,62},{0,66,127,64,0},{66,97,81,73,70},
 {33,65,69,75,49},{24,20,18,127,16},{39,69,69,69,57},{60,74,73,73,48},
 {1,113,9,5,3},{54,73,73,73,54},{6,73,73,41,30},
 {126,17,17,17,126},{127,73,73,73,54},{62,65,65,65,34},
 {127,65,65,34,28},{127,73,73,73,65},{127,9,9,9,1},
 {62,65,73,73,122},{127,8,8,8,127},{0,65,127,65,0},
 {32,64,65,63,1},{127,8,20,34,65},{127,64,64,64,64},
 {127,2,12,2,127},{127,4,8,16,127},{62,65,65,65,62},
 {127,9,9,9,6},{62,65,81,33,94},{127,9,25,41,70},
 {70,73,73,73,49},{1,1,127,1,1},{63,64,64,64,63},
 {31,32,64,32,31},{63,64,56,64,63},{99,20,8,20,99},
 {7,8,112,8,7},{97,81,73,69,67},{0,54,54,0,0},{8,8,8,8,8},
 {0,96,96,0,0},{32,16,8,4,2},{99,19,8,100,99},{0,0,95,0,0},
 {2,1,81,9,6},{8,8,62,8,8},{20,20,20,20,20},{64,64,64,64,64},
 {0,28,34,65,0},{0,65,34,28,0},{0,65,34,20,8},{8,20,34,65,0}
};
bool ST7567S_I2C::command(uint8_t value) {
  if (!address_) return false;
  wire_.beginTransmission(address_); wire_.write(0x00); wire_.write(value);
  bool ok = wire_.endTransmission() == 0;
  if (!ok) ++errors_;
  return ok;
}
bool ST7567S_I2C::begin(int sda, int scl, int reset, uint8_t address, uint32_t frequency) {
  ready_ = false; errors_ = 0; address_ = 0;
  if (address && (address < 0x3C || address > 0x3F)) return false;
  if (!wire_.begin(sda, scl, frequency)) return false;
  wire_.setTimeOut(50);
  if (reset >= 0) {
    pinMode(reset, OUTPUT); digitalWrite(reset, LOW); delay(20);
    digitalWrite(reset, HIGH); delay(100);
  }
  for (uint8_t a = (address ? address : 0x3C); a <= (address ? address : 0x3F); ++a) {
    wire_.beginTransmission(a);
    if (wire_.endTransmission() == 0) { address_ = a; break; }
  }
  if (!address_) return false;
  command(0xE2); delay(20); command(0xAE); command(0xA2);
  command(0xA0); command(0xC8); // Panel-tested direction: no horizontal mirror
  command(0x40); command(0x24);
  command(0x2C); delay(50); command(0x2E); delay(50); command(0x2F); delay(50);
  setContrast(contrast_); command(0xA4); command(0xA6); command(0xAF);
  ready_ = errors_ == 0; clear();
  return ready_ && display();
}
bool ST7567S_I2C::display() {
  if (!ready_) return false;
  uint32_t before = errors_;
  for (int page = 0; page < 8; ++page) {
    command(0xB0 | page); command(0x10); command(0);
    for (int x = 0; x < 132; x += 16) {
      wire_.beginTransmission(address_); wire_.write(0x40);
      for (int col = x; col < min(x + 16, 132); ++col) {
        int source = col - offset_;
        wire_.write(source >= 0 && source < 128 ? buffer_[page * 128 + source] : uint8_t(0));
      }
      if (wire_.endTransmission() != 0) ++errors_;
    }
  }
  return before == errors_;
}
void ST7567S_I2C::clear(bool color) { memset(buffer_, color ? 255 : 0, sizeof(buffer_)); }
void ST7567S_I2C::drawPixel(int16_t x, int16_t y, bool color) {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
  uint8_t &v = buffer_[(y / 8) * WIDTH + x];
  if (color) v |= 1 << (y & 7); else v &= ~(1 << (y & 7));
}
bool ST7567S_I2C::getPixel(int16_t x, int16_t y) const {
  return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT &&
         (buffer_[(y / 8) * WIDTH + x] & (1 << (y & 7)));
}
void ST7567S_I2C::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool color) {
  int32_t x = x0, y = y0, dx = abs(int32_t(x1) - x0), dy = -abs(int32_t(y1) - y0);
  int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
  int32_t err = dx + dy;
  for (;;) {
    drawPixel(x, y, color);
    if (x == x1 && y == y1) break;
    int32_t twice = 2 * err;
    if (twice >= dy) { err += dy; x += sx; }
    if (twice <= dx) { err += dx; y += sy; }
  }
}
void ST7567S_I2C::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, bool color) {
  if (w <= 0 || h <= 0) return;
  int32_t right = min(int32_t(WIDTH), int32_t(x) + w);
  int32_t bottom = min(int32_t(HEIGHT), int32_t(y) + h);
  for (int row = max(int(y), 0); row < bottom; ++row)
    for (int col = max(int(x), 0); col < right; ++col) drawPixel(col, row, color);
}
void ST7567S_I2C::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, bool color) {
  if (w <= 0 || h <= 0) return;
  // Use clipped fills, avoiding overflow for rectangles outside the display.
  fillRect(x, y, w, 1, color); fillRect(x, y, 1, h, color);
  int32_t right = int32_t(x) + w - 1, bottom = int32_t(y) + h - 1;
  if (right >= 0 && right < WIDTH) fillRect(right, y, 1, h, color);
  if (bottom >= 0 && bottom < HEIGHT) fillRect(x, bottom, w, 1, color);
}
void ST7567S_I2C::drawCircle(int16_t cx, int16_t cy, int16_t r, bool color) {
  if (r < 0 || r > 1024) return;
  int32_t x = r, y = 0, err = 1 - r;
  while (x >= y) {
    const int32_t xs[] = {x,y,-y,-x,-x,-y,y,x};
    const int32_t ys[] = {y,x,x,y,-y,-x,-x,-y};
    for (int i = 0; i < 8; ++i) {
      int32_t px = cx + xs[i], py = cy + ys[i];
      if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) drawPixel(px,py,color);
    }
    ++y;
    if (err < 0) err += 2 * y + 1; else { --x; err += 2 * (y - x) + 1; }
  }
}
void ST7567S_I2C::fillCircle(int16_t cx, int16_t cy, int16_t r, bool color) {
  if (r < 0) return;
  int32_t rr = int32_t(r) * r;
  for (int y = 0; y < HEIGHT; ++y) for (int x = 0; x < WIDTH; ++x) {
    int32_t dx = int32_t(x) - cx, dy = int32_t(y) - cy;
    if (int64_t(dx)*dx + int64_t(dy)*dy <= rr) drawPixel(x,y,color);
  }
}
void ST7567S_I2C::drawBitmap(int16_t x, int16_t y, const uint8_t *bits,
                            int16_t w, int16_t h, bool color) {
  if (!bits || w <= 0 || h <= 0) return;
  const uint32_t stride = (uint32_t(w) + 7) / 8;
  for (int py = 0; py < HEIGHT; ++py) for (int px = 0; px < WIDTH; ++px) {
    int32_t sx = int32_t(px) - x, sy = int32_t(py) - y;
    if (sx >= 0 && sx < w && sy >= 0 && sy < h &&
        (bits[sy * stride + sx / 8] & (0x80 >> (sx & 7)))) drawPixel(px,py,color);
  }
}
void ST7567S_I2C::drawChar(int16_t x, int16_t y, char c, uint8_t size, bool color) {
  size = constrain(size, 1, 8);
  c = toupper(static_cast<unsigned char>(c)); // Lowercase renders as uppercase
  if (c == '\0') return;
  const char *p = strchr(chars, c);
  if (!p) { drawRect(x, y, 5 * size, 7 * size, color); return; }
  for (int col = 0; col < 5; ++col) for (int row = 0; row < 7; ++row)
    if (glyphs[p - chars][col] & (1 << row))
      fillRect(x + col * size, y + row * size, size, size, color);
}
size_t ST7567S_I2C::write(uint8_t c) {
  if (c == '\r') return 1;
  if (c == '\n') { cursorX_ = 0; cursorY_ += 8 * textSize_; return 1; }
  if (wrap_ && int32_t(cursorX_) + 6 * textSize_ > WIDTH) {
    cursorX_ = 0; cursorY_ += 8 * textSize_;
  }
  drawChar(cursorX_, cursorY_, c, textSize_, textColor_);
  cursorX_ += 6 * textSize_;
  return 1;
}
bool ST7567S_I2C::setContrast(uint8_t value) {
  contrast_ = min(value, uint8_t(63));
  bool a = command(0x81); bool b = command(contrast_); return a && b;
}
bool ST7567S_I2C::setInverted(bool inverted) { return command(inverted ? 0xA7 : 0xA6); }
bool ST7567S_I2C::setDisplayEnabled(bool enabled) { return command(enabled ? 0xAF : 0xAE); }
bool ST7567S_I2C::setFlipped(bool flipped) {
  bool a = command(flipped ? 0xA1 : 0xA0);
  bool b = command(flipped ? 0xC0 : 0xC8); return a && b;
}
