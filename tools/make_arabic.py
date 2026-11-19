#!/usr/bin/env python3
"""Renders the Arabic texts of tools/ar.tsv to content/lang/ar/*.png.

Each picture is white letters with a dark outline (the game tints it like
its own fonts). content/lang/ar/index.txt lists, per line:
  style  english  file  width  height  baseline  alefHeight
Needs Pillow with libraqm (Arabic shaping, right-to-left).

Project Owner & Creator: Muhamed Alzarqi
AI Programming Assistant: Claude (Anthropic)
"""
import os
from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
OUT = os.path.join(ROOT, 'content', 'lang', 'ar')
SIZE = 96
STROKE = 6

def load_font(style):
    if style == 'K':
        f = ImageFont.truetype(os.path.join(HERE, 'fonts', 'ReemKufi.ttf'), SIZE,
                               layout_engine=ImageFont.Layout.RAQM)
        try:
            f.set_variation_by_axes([700])
        except Exception:
            pass
        return f
    return ImageFont.truetype(os.path.join(HERE, 'fonts', 'Lalezar.ttf'), SIZE,
                              layout_engine=ImageFont.Layout.RAQM)

def main():
    os.makedirs(OUT, exist_ok=True)
    for f in os.listdir(OUT):
        if f.endswith('.png'):
            os.remove(os.path.join(OUT, f))

    fonts = {'K': load_font('K'), 'C': load_font('C'), 'A': load_font('C')}
    lines = []
    n = 0
    for raw in open(os.path.join(HERE, 'ar.tsv'), encoding='utf-8'):
        raw = raw.rstrip('\n')
        if not raw or raw.startswith('#'):
            continue
        style, en, ar = raw.split('\t')
        font = fonts[style]
        ascent, descent = font.getmetrics()
        pad = STROKE + 4
        H = ascent + descent + 2 * pad
        # alef height = the "capital letter" height of Arabic
        ab = font.getbbox('ا', direction='rtl')
        alef_h = ab[3] - ab[1]

        tmp = Image.new('LA', (SIZE * len(ar) + 200, H), (0, 0))
        d = ImageDraw.Draw(tmp)
        d.text((pad, pad + ascent), ar, font=font, anchor='ls', direction='rtl',
               fill=(255, 255), stroke_width=STROKE, stroke_fill=(40, 255))
        bbox = tmp.getbbox()
        x0, x1 = bbox[0], bbox[2]
        img = tmp.crop((x0, 0, x1, H))

        # white letters, dark brown outline
        rgba = Image.new('RGBA', img.size)
        px = img.load(); out = rgba.load()
        for y in range(img.size[1]):
            for x in range(img.size[0]):
                l, a = px[x, y]
                c = int(40 + (255 - 40) * max(0, l - 40) / 215)
                out[x, y] = (c, int(c * 0.93) if c < 250 else 255, int(c * 0.85) if c < 250 else 255, a)
        name = '%03d.png' % n
        rgba.save(os.path.join(OUT, name), optimize=True)
        lines.append('\t'.join([style, en, name, str(img.size[0]), str(H),
                                str(pad + ascent), str(alef_h)]))
        n += 1

    with open(os.path.join(OUT, 'index.txt'), 'w', encoding='utf-8') as f:
        f.write('\n'.join(lines) + '\n')
    print('rendered', n, 'texts')

if __name__ == '__main__':
    main()
