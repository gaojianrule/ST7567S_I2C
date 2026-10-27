# 当前字体来源与生成（1.3.0）

本版 12/16px 使用 U8g2 的 wqy 字体同源 BDF，保留原生点阵，不做矢量栅格化或阈值减细。
24px 是原生 12px 字形的 2 倍最近邻放大；不是 wqy 官方提供的 24px 字体。

- U8g2 参考：https://github.com/olikraus/u8g2/wiki/fntgrpwqy
- BDF 来源：https://github.com/larryli/u8g2_wqy/tree/master/bdf
- 完整 12px：https://raw.githubusercontent.com/larryli/u8g2_wqy/master/bdf/wenquanyi_9pt.bdf
- 完整 16px：https://raw.githubusercontent.com/larryli/u8g2_wqy/master/bdf/wenquanyi_12pt.bdf

随包 BDF 仅保留本库 3763 个字形，保留上游版权注释与每字原始点阵；可用 tools/generate_oled_fonts.py 重新生成。
同一字号共用基线，符号如遇边界只做整体位置微调，避免裁掉像素。

Copyright (C)2004-2010 WenQuanYi Project, Board of Trustees and Qianqian Fang.
Font license: GPL v2 with font embedding exception. See WQY-README.txt and GPL-2.0.txt.
The font embedding exception in WQY-README.txt is extended to these modified subsets and enlarged versions.
Modified bitmap subset names: ST7567S_CN12, ST7567S_CN16, ST7567S_CN24.
WQY-LICENSE.txt 是上游 u8g2_wqy 仓库的 MIT 许可；字体本身遵循上述 GPL v2 加字体嵌入例外。

## 可选的旧版 Noto 生成器

工具 generate_fonts.py 仍可生成旧版矢量栅格化字体（字重 400，阈值 128），不会用于本版默认字形。
- https://github.com/google/fonts/tree/main/ofl/notosanssc
- Font: https://raw.githubusercontent.com/google/fonts/main/ofl/notosanssc/NotoSansSC%5Bwght%5D.ttf
Copyright 2014-2021 Adobe (http://www.adobe.com/), with Reserved Font Name 'Source'.
Licensed under SIL Open Font License 1.1; full text in OFL.txt.
原始 TTF 不包含在 ZIP 内，补字通常不需要它。
