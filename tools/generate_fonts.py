"""Generate subset fonts and previews. Requires Pillow: python -m pip install pillow.

Usage: python tools/generate_fonts.py --font path/to/NotoSansSC.ttf
Optional: --chars path/to/utf8-text.txt (all non-ASCII characters are deduplicated).
"""
import argparse
from functools import lru_cache
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]


@lru_cache(maxsize=3)
def load_font(size, path):
    font = ImageFont.truetype(str(path), size)
    if hasattr(font, 'set_variation_by_axes'):
        font.set_variation_by_axes([400])
    return font


def render(char, size, path):
    font = load_font(size, path)
    image = Image.new('L', (size, size), 0)
    draw = ImageDraw.Draw(image)
    left, top, right, bottom = draw.textbbox((0, 0), char, font=font)
    # Center the ink in a fixed square; keep natural size and stroke thickness.
    draw.text(((size - (right-left)) // 2 - left,
               (size - (bottom-top)) // 2 - top), char, font=font, fill=255)
    return image.point(lambda v: 255 if v >= 128 else 0, mode='1')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--font', type=Path, required=True)
    parser.add_argument('--chars', type=Path, default=ROOT / 'extras/characters.txt')
    args = parser.parse_args()
    chars = sorted(set(c for c in args.chars.read_text(encoding='utf-8-sig') if ord(c) >= 128))
    if not chars:
        raise ValueError('No non-ASCII glyphs selected')
    if len(chars) > 65535:
        raise ValueError('Too many glyphs')
    header = '#pragma once\n#include "ST7567S_I2C.h"\n\n'
    preview = Image.new('RGB', (680, 390), 'white')
    pd = ImageDraw.Draw(preview)
    total = 0
    for size in (12, 16, 24):
        symbol = f'ST7567S_CN{size}'
        header += f'extern const ST7567S_ChineseFont {symbol};\n'
        text = ('// Generated subset of Noto Sans SC, SIL OFL 1.1.\n'
                '// Copyright and license: extras/font-source/OFL.txt\n'
                '// Modified bitmap subset: ST7567S LCD Project.\n'
                '#include "ST7567S_ChineseFonts.h"\n\n')
        text += 'namespace {\nconst uint32_t codes[] = {\n'
        text += ','.join(f'0x{ord(c):X}' for c in chars) + '\n};\nconst uint8_t data[] = {\n'
        for char in chars:
            bitmap = render(char, size, args.font)
            assert bitmap.getbbox() is not None, f'Empty glyph: {char}'
            packed = bitmap.tobytes()
            assert len(packed) == ((size + 7) // 8) * size
            text += f'  // U+{ord(char):04X} {char}\n  '
            text += ','.join(f'0x{v:02X}' for v in packed) + ',\n'
        text += f'}};\n}}\nconst ST7567S_ChineseFont {symbol} = {{{size}, {len(chars)}, codes, data}};\n'
        (ROOT / f'src/ST7567S_CN{size}.cpp').write_text(text, encoding='utf-8')
        used = len(chars) * (((size+7)//8)*size + 4)
        total += used
        print(f'{size}x{size}: {len(chars)} glyphs, bitmap+index {used} bytes')
    (ROOT / 'src/ST7567S_ChineseFonts.h').write_text(header, encoding='utf-8')
    # Preview the actual monochrome pixels at 4x nearest-neighbor scale.
    for row, size in enumerate((12, 16, 24)):
        pd.text((8, row*130 + 4), f'{size}x{size} pixels', fill='black')
        for i, c in enumerate('中文显示测试'):
            glyph = render(c, size, args.font).convert('L')
            tile = Image.new('RGB', glyph.size, 'white')
            tile.paste('black', (0, 0, size, size), glyph)
            preview.paste(tile.resize((size*4,size*4), Image.Resampling.NEAREST), (8+i*(size+2)*4, row*130+24))
    preview.save(ROOT / 'extras/chinese-font-preview.png')
    print(f'All three sizes bitmap+index: {total} bytes')


if __name__ == '__main__':
    main()
