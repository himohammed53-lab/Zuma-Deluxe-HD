#!/usr/bin/env python3
"""Makes the Arabic copies of the game's button pictures.

The English words painted on the stones and wooden buttons are erased and
the Arabic word is painted in the same place, in the same style:
  carved   - stones of the main menu (gold letters cut into the stone)
  painted  - wooden buttons and the in-game MENU (yellow, dark outline)
  embossed - difficulty names on the pyramid (raised orange letters)

Output: content/images/menu_ar.png, content/images/gameinterface_ar.png
Needs: Pillow with libraqm, numpy, opencv (cv2).

Project Owner & Creator: Muhamed Alzarqi
AI Programming Assistant: Claude (Anthropic)
"""
import io, os, subprocess
import numpy as np
import cv2
from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
UPSTREAM = 'b05dc19'   # original pictures, before any change of ours
KUFI = os.path.join(HERE, 'fonts', 'ReemKufi.ttf')

def original(path):
    data = subprocess.run(['git', '-C', ROOT, 'show', f'{UPSTREAM}:{path}'],
                          check=True, capture_output=True).stdout
    return np.array(Image.open(io.BytesIO(data)).convert('RGBA'))

# atlas, x, y, w, h (one state), states, Arabic, style, lines
BUTTONS = [
    ('menu', 2724,  901, 245, 138, 3, 'مغامرة',        'carved',  1),
    ('menu', 2648,  776, 270, 125, 3, 'الحسابات',      'carved',  1),
    ('menu', 2563,  647, 299, 129, 3, 'الإعدادات',     'carved',  1),
    ('menu', 2924, 1038, 177, 190, 3, 'ألعاب\nأخرى',   'carved',  2),
    ('menu', 2917, 1226, 180, 211, 3, 'خروج',          'carved',  1),
    ('menu', 1280, 2160, 211,  84, 3, 'القائمة الرئيسية', 'painted', 1),
    ('menu', 1922, 2161, 141,  48, 3, 'التالي',        'painted', 1),
    ('menu', 1922, 2210, 157,  49, 3, 'العب',          'painted', 1),
    ('menu', 1924, 2260, 139,  48, 3, 'رجوع',          'painted', 1),
    ('menu', 1608, 2336, 328,  63, 3, 'أرنب',          'embossed', 1),
    ('menu', 1538, 2399, 258,  49, 3, 'نسر',           'embossed', 1),
    ('menu', 1485, 2448, 205,  43, 3, 'جاكوار',        'embossed', 1),
    ('menu', 1439, 2491, 159,  54, 3, 'إله الشمس',     'embossed', 1),
    ('hud',   925,   15, 118,  37, 3, 'القائمة',       'painted', 1),
]

def letter_mask(rgb, style):
    """Old (English) letters: gold / yellow / orange pixels."""
    hsv = cv2.cvtColor(rgb, cv2.COLOR_RGB2HSV)
    H, S, V = hsv[:, :, 0], hsv[:, :, 1], hsv[:, :, 2]
    if style == 'carved':
        m = (H >= 12) & (H <= 38) & (S > 90) & (V > 120)
    elif style == 'painted':
        m = (H >= 14) & (H <= 40) & (S > 110) & (V > 170)
    else:
        m = (H >= 8) & (H <= 30) & (S > 70) & (S < 235) & (V > 150)
    return m.astype(np.uint8)

def inner_band(h, w, style):
    b = np.zeros((h, w), np.uint8)
    if style == 'carved':
        b[int(h * .08):int(h * .80), int(w * .08):int(w * .90)] = 1
    elif style == 'painted':
        b[int(h * .12):int(h * .88), int(w * .07):int(w * .93)] = 1
    else:
        b[int(h * .22):int(h * .90), int(w * .20):int(w * .80)] = 1
    return b

def arabic_mask(text, lines):
    font = ImageFont.truetype(KUFI, 160, layout_engine=ImageFont.Layout.RAQM)
    try:
        font.set_variation_by_axes([700])
    except Exception:
        pass
    rows = text.split('\n')
    imgs = []
    for r in rows:
        im = Image.new('L', (160 * len(r) + 200, 320), 0)
        ImageDraw.Draw(im).text((40, 220), r, font=font, fill=255, anchor='ls',
                                direction='rtl')
        imgs.append(im.crop(im.getbbox()))
    if len(imgs) == 1:
        return np.array(imgs[0]).astype(np.float32) / 255
    gap = 30
    W = max(i.width for i in imgs); Ht = sum(i.height for i in imgs) + gap * (len(imgs) - 1)
    c = Image.new('L', (W, Ht), 0); y = 0
    for i in imgs:
        c.paste(i, ((W - i.width) // 2, y)); y += i.height + gap
    return np.array(c).astype(np.float32) / 255

def fit(mask, box_w, box_h, style):
    mh, mw = mask.shape
    k = min(box_w / mw, box_h / mh)
    w, h = max(1, int(mw * k)), max(1, int(mh * k))
    # Arabic words can be a little wider than tall boxes allow
    if w < box_w * 0.75 and style != 'carved':
        w = int(min(box_w * 0.9, w * 1.15))
    return cv2.resize(mask, (w, h), interpolation=cv2.INTER_AREA)

def gold_rows(rgb, m):
    """Average colour of the old letters, row by row (light part only)."""
    rows = []
    for y in range(rgb.shape[0]):
        p = rgb[y][m[y] > 0].astype(np.float32)
        if len(p):
            v = p.mean(1); p = p[v >= np.percentile(v, 35)]
        rows.append(p.mean(0) if len(p) else None)
    good = [r for r in rows if r is not None]
    if not good:
        return [np.array([230, 175, 50], np.float32)], 0
    first = next(i for i, r in enumerate(rows) if r is not None)
    return good, first

def paint_state(reg, style, text, lines, ref_mask=None, ref_box=None, seed=0):
    rgb = reg[:, :, :3].copy()
    h, w = rgb.shape[:2]
    # see-through pixels are black in the file: give them the plank colour
    # so erasing the letters doesn't pull black into the picture
    opaque = reg[:, :, 3] > 200
    if (~opaque).any() and opaque.any():
        rgb[~opaque] = rgb[opaque].mean(0).astype(np.uint8)
    if style == 'embossed':
        # the plank face is smooth: erase the whole text band of it
        band = inner_band(h, w, style) * opaque.astype(np.uint8)
        ys, xs = np.where(band > 0)
        box = (xs.min(), ys.min(), xs.max(), ys.max())
        good = [np.array([255, 214, 110], np.float32), np.array([240, 170, 60], np.float32),
                np.array([205, 120, 35], np.float32)]
        m = band
    else:
        m = letter_mask(rgb, style) * inner_band(h, w, style)
        if ref_mask is not None:
            m = ref_mask.copy()
        if m.sum() < 20:
            return reg, None, None
        ys, xs = np.where(m > 0)
        box = (xs.min(), ys.min(), xs.max(), ys.max()) if ref_box is None else ref_box
        good, first = gold_rows(rgb, m)
    ring = cv2.dilate(m, np.ones((3, 3), np.uint8)) - m
    outline = rgb[ring > 0].astype(np.float32).mean(0) * 0.45
    if style == 'embossed':
        outline = np.array([70, 30, 10], np.float32)

    # erase the English letters
    hole = cv2.dilate(m * 255, np.ones((3, 3), np.uint8),
                      iterations=3 if style == 'carved' else (1 if style == 'embossed' else 2))
    hole = hole * opaque.astype(np.uint8)
    smooth = cv2.inpaint(rgb, hole, 7, cv2.INPAINT_TELEA).astype(np.float32)
    f = rgb.astype(np.float32)
    if style == 'carved':
        detail = f - cv2.GaussianBlur(f, (0, 0), 3)
        src = np.zeros_like(detail)
        lo = min(h - 2, box[3] + 6)
        span = max(1, h - lo - 4)
        for y in range(h):
            src[y] = detail[lo + (y % span)]
        smooth = smooth + src * 0.9
    out = np.where((hole > 0)[:, :, None], smooth, f)

    # the Arabic word, fitted in the old text's box
    bx0, by0, bx1, by1 = box
    bw, bh = bx1 - bx0 + 1, by1 - by0 + 1
    a_src = arabic_mask(text, lines)
    if style == 'embossed':
        a_fit = fit(a_src, bw * 0.85, bh * 0.92, style)
    else:
        a_fit = fit(a_src, bw * 1.04, bh * 0.98, style)
    th, tw = a_fit.shape
    gx = int(bx0 + (bw - tw) / 2); gy = int(by0 + (bh - th) / 2)
    gx = max(0, min(w - tw, gx)); gy = max(0, min(h - th, gy))
    a = np.zeros((h, w), np.float32); a[gy:gy + th, gx:gx + tw] = a_fit
    a = np.clip(a * 1.2, 0, 1)

    # letter colours: same gradient as the English letters had
    grad = np.zeros((h, 3), np.float32)
    for y in range(h):
        t = (y - gy) / max(th, 1)
        i = int(np.clip(t * (len(good) - 1), 0, len(good) - 1))
        grad[y] = good[i]
    gold = np.repeat(grad[:, None, :], w, axis=1)

    if style == 'carved':
        wall = np.zeros_like(a)
        for k in range(1, 6):
            dy = int(round(k * 0.5)); t2 = np.zeros_like(a); t2[dy:, :-k] = a[:h - dy, k:]
            wall = np.maximum(wall, t2 * (1 - 0.08 * k))
        wall = np.clip(wall - a, 0, 1)
        deep = out * 0.28 + np.array([10, 25, 25], np.float32) * 0.3
        out = out * (1 - wall[:, :, None]) + deep * wall[:, :, None]
        edge = np.clip(cv2.dilate(a, np.ones((3, 3), np.uint8)) - a, 0, 1)
        out = out * (1 - 0.45 * edge[:, :, None])
        left = np.clip(a - np.pad(a, ((0, 0), (2, 0)))[:, :w], 0, 1)
        right = np.clip(a - np.pad(a, ((0, 0), (0, 2)))[:, 2:], 0, 1)
        gold = gold * (1 + 0.25 * left[:, :, None]) * (1 - 0.30 * right[:, :, None])
    else:
        thick = 2 if style == 'painted' else 2
        ol = cv2.dilate(a, np.ones((2 * thick + 1, 2 * thick + 1), np.uint8))
        ol = cv2.GaussianBlur(ol, (3, 3), 0)
        out = out * (1 - ol[:, :, None]) + outline[None, None, :] * ol[:, :, None]
        if style == 'embossed':
            top = np.clip(a - np.pad(a, ((2, 0), (0, 0)))[:h, :], 0, 1)
            bot = np.clip(a - np.pad(a, ((0, 2), (0, 0)))[2:, :], 0, 1)
            gold = gold * (1 + 0.30 * top[:, :, None]) * (1 - 0.30 * bot[:, :, None])
    out = out * (1 - a[:, :, None]) + np.clip(gold, 0, 255) * a[:, :, None]

    res = reg.copy()
    res[:, :, :3] = np.clip(out, 0, 255).astype(np.uint8)
    return res, m, box

def main():
    atlases = {
        'menu': ('content/images/menu.png', 'content/images/menu_ar.png'),
        'hud': ('content/images/gameinterface.png', 'content/images/gameinterface_ar.png'),
    }
    pics = {k: original(v[0]) for k, v in atlases.items()}

    for atlas, x, y, w, h, states, text, style, lines in BUTTONS:
        img = pics[atlas]
        ref_m = ref_b = None
        for s in range(states):
            X = x + s * w
            reg = img[y:y + h, X:X + w]
            if style == 'embossed' and s > 0:
                # glowing states: same letter place as the normal state
                new, _, _ = paint_state(reg, style, text, lines, ref_m, ref_b, s)
            else:
                new, m, b = paint_state(reg, style, text, lines, seed=s)
                if s == 0:
                    ref_m, ref_b = m, b
            img[y:y + h, X:X + w] = new

    for k, (_, out) in atlases.items():
        Image.fromarray(pics[k]).save(os.path.join(ROOT, out), optimize=True)
        print('wrote', out)

if __name__ == '__main__':
    main()
