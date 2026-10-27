"""Build the GB2312 level-1 (3755 Hanzi) character list using Python's codec.

The extras are retained in characters-extra.txt, independently of the generated list.
Run before generate_fonts.py. No third-party dependencies.
"""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
hanzi = []
for high in range(0xB0, 0xD8):
    for low in range(0xA1, 0xFF):
        try:
            char = bytes((high, low)).decode('gb2312')
        except UnicodeDecodeError:
            continue
        assert '\u4e00' <= char <= '\u9fff'
        hanzi.append(char)
assert len(set(hanzi)) == 3755
extras = (root / 'extras/characters-extra.txt').read_text(encoding='utf-8-sig')
chars = sorted(set(hanzi) | {c for c in extras if ord(c) >= 128})
(root / 'extras/characters.txt').write_text(
    '\n'.join(''.join(chars[i:i+64]) for i in range(0, len(chars), 64)) + '\n',
    encoding='utf-8')
print(f'GB2312 level-1: {len(hanzi)}; total with extras: {len(chars)}')
