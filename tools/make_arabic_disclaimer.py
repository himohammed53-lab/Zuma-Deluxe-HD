#!/usr/bin/env python3
"""Arabic copy of the start-up disclaimer: content/images/disclaimer_ar.jpg
Project Owner & Creator: Muhamed Alzarqi / AI Programming Assistant: Claude (Anthropic)"""
import os
from PIL import Image, ImageDraw, ImageFont
HERE = os.path.dirname(os.path.abspath(__file__)); ROOT = os.path.dirname(HERE)
src = Image.open(os.path.join(ROOT, 'content/images/disclaimer.jpg'))
W, H = src.size
im = Image.new('RGB', (W, H), (0, 0, 0)); d = ImageDraw.Draw(im)
def font(size):
    return ImageFont.truetype(os.path.join(HERE, 'fonts', 'Lalezar.ttf'), size,
                              layout_engine=ImageFont.Layout.RAQM)
s = W / 1280
lines = [
    ('تنبيه', 110, (230, 0, 0)),
    ('مطوّرو هذه اللعبة لا علاقة لهم بشركة PopCap أو EA.', 44, (255, 255, 255)),
    ('«ZUMA HD» مشروع غير ربحي صُنع لأغراض تعليمية فقط.', 34, (235, 235, 235)),
    ('لا يهدف أصحاب هذا المشروع إلى تحقيق أي ربح منه.', 34, (235, 235, 235)),
    ('ولا يقصد المساهمون في «ZUMA HD» التعدّي على أي حقوق نشر أو علامات تجارية.', 34, (235, 235, 235)),
]
y = H * 0.30
for text, size, col in lines:
    f = font(int(size * s))
    d.text((W / 2, y), text, font=f, fill=col, anchor='mt', direction='rtl')
    y += size * s * (1.25 if size < 100 else 1.05)
im.save(os.path.join(ROOT, 'content/images/disclaimer_ar.jpg'), quality=92)
print('ok')
