"""Generate OLED-style fonts from native WenQuanYi bitmaps (Pillow required).

Run from anywhere: python tools/generate_oled_fonts.py
12/16px preserve native bitmap pixels; 24px is an integer 2x enlargement of 12px.
Includes no grayscale thresholding, erosion, or fractional scaling.
"""
import sys
from functools import lru_cache
from pathlib import Path
from PIL import Image
import generate_fonts as generator

ROOT = Path(__file__).resolve().parents[1]


@lru_cache(maxsize=2)
def load_bdf(size):
    name = 'wenquanyi_9pt.bdf' if size == 12 else 'wenquanyi_12pt.bdf'
    path = ROOT / 'extras/font-source' / name
    glyphs = {}
    for block in path.read_text(encoding='utf-8').split('STARTCHAR ')[1:]:
        lines = [line.strip() for line in block.splitlines()]
        code = next(int(s.split()[1]) for s in lines if s.startswith('ENCODING '))
        w, h, dx, dy = next(tuple(map(int,s.split()[1:])) for s in lines if s.startswith('BBX '))
        start = lines.index('BITMAP') + 1
        rows = lines[start:start+h]
        if code < 0: continue
        image = Image.new('1', (size,size), 0)
        # Preserve bitmap proportions and align all glyphs on one baseline.
        # Native Chinese ink fits a size-1 tall region with a blank bottom row.
        top = size - 2 - dy - h
        if h <= size:
            top = max(0, min(top, size - h))
        rowbits = ((w+7)//8)*8
        for y, row in enumerate(rows):
            bits = int(row,16) if row else 0
            for x in range(w):
                px, py = dx+x, top+y
                if 0 <= px < size and 0 <= py < size and bits & (1 << (rowbits-1-x)):
                    image.putpixel((px,py),1)
        glyphs[code] = image
    return glyphs


def render(char, size, unused):
    source_size = 12 if size == 24 else size
    glyph = load_bdf(source_size).get(ord(char))
    if glyph is None: raise ValueError(f'Missing native glyph U+{ord(char):04X}: {char}')
    return glyph.resize((size,size), Image.Resampling.NEAREST) if size == 24 else glyph


if __name__ == '__main__':
    generator.render = render
    # Keep the same output format and checks as the original generator.
    sys.argv = [sys.argv[0], '--font', 'unused'] + sys.argv[1:]
    generator.main()
    for size in (12,16,24):
        path = ROOT / f'src/ST7567S_CN{size}.cpp'
        text = path.read_text(encoding='utf-8')
        text = text.replace('Generated subset of Noto Sans SC, SIL OFL 1.1.',
            'WenQuanYi Bitmap Song subset, GPL v2 with font embedding exception.')
        text = text.replace('extras/font-source/OFL.txt',
            'extras/font-source/WQY-README.txt and GPL-2.0.txt')
        path.write_text(text,encoding='utf-8')
