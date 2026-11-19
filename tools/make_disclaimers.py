#!/usr/bin/env python3
"""Start-up disclaimer pictures (English and Arabic) with the credits of
this Android edition added under the original text.
Output: content/images/disclaimer.jpg, content/images/disclaimer_ar.jpg

Project Owner & Creator: Muhamed Alzarqi
AI Programming Assistant: Claude (Anthropic)
Created through human-AI collaboration
"""
import io, os, subprocess
from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
UPSTREAM = 'b05dc19'
RED, WHITE, LIGHT, GOLD, GREY = (230, 0, 0), (255, 255, 255), (235, 235, 235), (255, 205, 80), (175, 175, 175)

def mont(size, weight):
    f = ImageFont.truetype(os.path.join(HERE, 'fonts', 'Montserrat.ttf'), size)
    try:
        f.set_variation_by_axes([weight])
    except Exception:
        pass
    return f

def lalezar(size):
    return ImageFont.truetype(os.path.join(HERE, 'fonts', 'Lalezar.ttf'), size,
                              layout_engine=ImageFont.Layout.RAQM)

def english():
    data = subprocess.run(['git', '-C', ROOT, 'show', f'{UPSTREAM}:content/images/disclaimer.jpg'],
                          check=True, capture_output=True).stdout
    im = Image.open(io.BytesIO(data)).convert('RGB')
    W, H = im.size
    d = ImageDraw.Draw(im)
    # thin line between the original text and our credits
    d.line((W * 0.30, 520, W * 0.70, 520), fill=(90, 90, 90), width=2)
    y = 540
    for text, font, col in [
        ('This Android edition is a fan-made project, not made by the original company.', mont(24, 500), LIGHT),
        ('Project Owner & Creator: Muhamed Alzarqi', mont(26, 700), GOLD),
        ('AI Programming Assistant: Claude (Anthropic)', mont(26, 700), GOLD),
        ('Created through human-AI collaboration', mont(20, 400), GREY),
    ]:
        d.text((W / 2, y), text, font=font, fill=col, anchor='mt')
        y += font.size * 1.45
    im.save(os.path.join(ROOT, 'content/images/disclaimer.jpg'), quality=92)

def arabic():
    W, H = 1280, 720
    im = Image.new('RGB', (W, H), (0, 0, 0)); d = ImageDraw.Draw(im)
    y = 150
    for text, size, col in [
        ('تنبيه', 104, RED),
        ('مطوّرو هذه اللعبة لا علاقة لهم بشركة PopCap أو EA.', 42, WHITE),
        ('«ZUMA HD» مشروع غير ربحي صُنع لأغراض تعليمية فقط.', 32, LIGHT),
        ('لا يهدف أصحاب هذا المشروع إلى تحقيق أي ربح منه.', 32, LIGHT),
        ('ولا يقصد المساهمون في «ZUMA HD» التعدّي على أي حقوق نشر أو علامات تجارية.', 32, LIGHT),
    ]:
        d.text((W / 2, y), text, font=lalezar(size), fill=col, anchor='mt', direction='rtl')
        y += size * (1.0 if size > 100 else 1.3)
    y += 14
    d.line((W * 0.30, y, W * 0.70, y), fill=(90, 90, 90), width=2)
    y += 22
    for text, size, col in [
        ('نسخة الأندرويد هذه مشروع من صنع المعجبين، وليست من الشركة الأصلية.', 30, LIGHT),
        ('صاحب المشروع ومنشئه: محمد الزرقي', 34, GOLD),
        ('مساعد البرمجة بالذكاء الاصطناعي: Claude من Anthropic', 34, GOLD),
        ('صُنع بالتعاون بين الإنسان والذكاء الاصطناعي', 26, GREY),
    ]:
        d.text((W / 2, y), text, font=lalezar(size), fill=col, anchor='mt', direction='rtl')
        y += size * 1.35
    im.save(os.path.join(ROOT, 'content/images/disclaimer_ar.jpg'), quality=92)

if __name__ == '__main__':
    english(); arabic(); print('ok')
