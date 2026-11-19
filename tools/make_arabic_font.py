#!/usr/bin/env python3
"""Arabic letters for names typed by the players (they can't be pictures
made in advance). Every letter is drawn in its 4 joined shapes, plus the
lam-alef ligatures, into content/lang/ar/font.png; content/lang/ar/font.txt
lists them:  codepoint form x y w h left advance
(form: 0 alone, 1 start, 2 middle, 3 end; codepoint 0xFEFB.. = lam-alef)
The game joins the letters itself (src/global/Lang.c).

Project Owner & Creator: Muhamed Alzarqi
AI Programming Assistant: Claude (Anthropic)
"""
import os
from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(os.path.dirname(HERE), 'content', 'lang', 'ar')
SIZE = 64
ZWJ = '‍'

DUAL = 'بتثجحخسشصضطظعغفقكلمنهيئى'
RIGHT = 'اأإآدذرزوؤة'
NONE = 'ء'
LAMALEF = {0xFEFB: 'لا', 0xFEF7: 'لأ', 0xFEF9: 'لإ', 0xFEF5: 'لآ'}

def main():
    font = ImageFont.truetype(os.path.join(HERE, 'fonts', 'Lalezar.ttf'), SIZE,
                              layout_engine=ImageFont.Layout.RAQM)
    ascent, descent = font.getmetrics()
    H = ascent + descent + 16
    base = ascent + 8
    items = []   # (cp, form, text)
    for ch in DUAL + RIGHT + NONE:
        forms = [0, 3] if ch in RIGHT else ([0] if ch in NONE else [0, 1, 2, 3])
        for f in forms:
            t = {0: ch, 1: ch + ZWJ, 2: ZWJ + ch + ZWJ, 3: ZWJ + ch}[f]
            items.append((ord(ch), f, t))
    for cp, t in LAMALEF.items():
        items.append((cp, 0, t))
        items.append((cp, 3, ZWJ + t))

    glyphs = []
    for cp, f, t in items:
        adv = font.getlength(t, direction='rtl')
        W = int(adv) + 2 * SIZE
        im = Image.new('L', (W, H), 0)
        R = W - SIZE   # right end of the pen
        ImageDraw.Draw(im).text((R, base), t, font=font, anchor='rs',
                                direction='rtl', fill=255)
        bb = im.getbbox() or (R, base, R + 1, base + 1)
        glyphs.append((cp, f, im.crop((bb[0], 0, bb[2], H)), bb[0] - R, adv))

    # atlas
    cols = 16
    cw = max(g[2].width for g in glyphs) + 2
    rows = (len(glyphs) + cols - 1) // cols
    atlas = Image.new('RGBA', (cols * cw, rows * H), (255, 255, 255, 0))
    lines = ['SIZE %d HEIGHT %d BASELINE %d ALEF %d SPACE %d' % (
        SIZE, H, base, font.getbbox('ا')[3] - font.getbbox('ا')[1],
        int(font.getlength(' ')))]
    for i, (cp, f, g, left, adv) in enumerate(glyphs):
        x, y = (i % cols) * cw, (i // cols) * H
        white = Image.new('RGBA', g.size, (255, 255, 255, 255))
        white.putalpha(g)
        atlas.paste(white, (x, y))
        lines.append('%d %d %d %d %d %d %d %.1f' % (cp, f, x, y, g.width, H, left, adv))
    os.makedirs(OUT, exist_ok=True)
    atlas.save(os.path.join(OUT, 'font.png'), optimize=True)
    with open(os.path.join(OUT, 'font.txt'), 'w') as fh:
        fh.write('\n'.join(lines) + '\n')
    print(len(glyphs), 'glyphs', atlas.size)

if __name__ == '__main__':
    main()
