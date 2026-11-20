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
CARVED_GEOM = None
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
    ('menu', 1608, 2336, 328,  63, 3, 'سهل',           'embossed', 1),
    ('menu', 1538, 2399, 258,  49, 3, 'متوسط',         'embossed', 1),
    ('menu', 1485, 2448, 205,  43, 3, 'صعب',           'embossed', 1),
    ('menu', 1439, 2491, 159,  54, 3, 'خارق',          'embossed', 1),
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

# wooden buttons: part of the width that is the flat board (not the arrow)
body_box = {
    'رجوع': (0.20, 0.97),     # BACK: arrow on the left
    'التالي': (0.03, 0.78),   # NEXT: arrow on the right
    'العب': (0.04, 0.96),
    'القائمة الرئيسية': (0.05, 0.95),
    'القائمة': (0.05, 0.95),
}

# square stones: the face where the English words were, measured by hand
# on the normal state (top-left, top-right, bottom-right, bottom-left)
MANUAL_QUADS = {
    'خروج':        [(18, 40), (130, 68), (130, 126), (18, 98)],
    'ألعاب\nأخرى': [(26, 12), (134, 38), (134, 150), (26, 124)],
}

def quad_point(q, u, v):
    """Point at (u, v) in 0..1 inside the quad q (bilinear)."""
    tl, tr, br, bl = [np.array(p, np.float32) for p in q]
    top = tl + (tr - tl) * u; bot = bl + (br - bl) * u
    return top + (bot - top) * v

def text_lines(m):
    """Top and bottom lines of the old letters (they follow the stone)."""
    xs, tops, bots = [], [], []
    for x in range(m.shape[1]):
        ys = np.where(m[:, x] > 0)[0]
        if len(ys) > 3:
            xs.append(x); tops.append(ys.min()); bots.append(ys.max())
    xs = np.array(xs, np.float32)
    def robust(vals):
        vals = np.array(vals, np.float32)
        p = np.polyfit(xs, vals, 1)
        for _ in range(2):
            r = np.abs(np.polyval(p, xs) - vals)
            keep = r <= np.percentile(r, 60)
            p = np.polyfit(xs[keep], vals[keep], 1)
        p[0] = float(np.clip(p[0], -0.15, 0.15))
        p[1] = float(np.median(vals - p[0] * xs))
        return lambda x, p=p: float(np.polyval(p, x))
    return robust(tops), robust(bots)

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
    r = _paint_state(reg, style, text, lines, ref_mask, ref_box, seed)
    return r

def _paint_state(reg, style, text, lines, ref_mask=None, ref_box=None, seed=0):
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
    a = np.zeros((h, w), np.float32)
    if style == 'carved':
        # follow the stone: same slant and perspective as the English letters.
        # The shape is measured on the normal state and reused (moved) on the
        # others, so the word has the same size in all three.
        global CARVED_GEOM
        ys_m, xs_m = np.where(m > 0)
        cen = (xs_m.mean(), ys_m.mean())
        if seed == 0:
            CARVED_GEOM = (text_lines(m), (bx0, bx1), cen)
        (t0, b0), (gx0, gx1), c0 = CARVED_GEOM
        dx, dy = cen[0] - c0[0], cen[1] - c0[1]
        top_l = lambda x, t0=t0: t0(x - dx) + dy
        bot_l = lambda x, b0=b0: b0(x - dx) + dy
        bx0, bx1 = gx0 + dx, gx1 + dx
        bw = bx1 - bx0 + 1
        if text in MANUAL_QUADS:
            q = [(px + dx, py + dy) for px, py in MANUAL_QUADS[text]]
            qw = np.mean([q[1][0] - q[0][0], q[2][0] - q[3][0]])
            qh = np.mean([q[3][1] - q[0][1], q[2][1] - q[1][1]])
            ar = a_src.shape[1] / a_src.shape[0]
            # keep the word's shape inside the face
            if ar > qw / qh:
                uw, vh = 1.0, (qw / ar) / qh
            else:
                uw, vh = (qh * ar) / qw, 1.0
            u0, v0 = (1 - uw) / 2, (1 - vh) / 2
            dst = np.float32([quad_point(q, u0, v0), quad_point(q, u0 + uw, v0),
                              quad_point(q, u0 + uw, v0 + vh), quad_point(q, u0, v0 + vh)])
            sh, sw = a_src.shape
            P = cv2.getPerspectiveTransform(np.float32([[0, 0], [sw, 0], [sw, sh], [0, sh]]), dst)
            small = cv2.resize(a_src, (max(1, int(np.ptp(dst[:, 0]) * 2)), max(1, int(np.ptp(dst[:, 0]) * 2 * a_src.shape[0] / a_src.shape[1]))), interpolation=cv2.INTER_AREA)
            sh2, sw2 = small.shape
            P = cv2.getPerspectiveTransform(np.float32([[0, 0], [sw2, 0], [sw2, sh2], [0, sh2]]), dst)
            a = cv2.warpPerspective(small, P, (w, h), flags=cv2.INTER_LINEAR)
            gy, th = int(dst[:, 1].min()), int(dst[:, 1].max() - dst[:, 1].min())
            a = np.clip(a * 1.2, 0, 1)
            return finish(reg, out, a, gy, th, good, outline, style, h, w), m, box
        hav = np.mean([bot_l(x) - top_l(x) for x in (bx0, bx1)])
        ar = a_src.shape[1] / a_src.shape[0]
        tw = min(bw * 1.0, hav * 1.3 * ar)
        cx = (bx0 + bx1) / 2
        x0, x1 = cx - tw / 2, cx + tw / 2
        shrink = (hav - min(hav, tw / ar)) / 2
        src = cv2.resize(a_src, (int(tw), int(hav)), interpolation=cv2.INTER_AREA)
        # keep the word's shape: letters fill the height only as much as they need
        sh, sw = src.shape
        quad_src = np.float32([[0, 0], [sw, 0], [sw, sh], [0, sh]])
        quad_dst = np.float32([[x0, top_l(x0) + shrink], [x1, top_l(x1) + shrink],
                               [x1, bot_l(x1) - shrink], [x0, bot_l(x0) - shrink]])
        P = cv2.getPerspectiveTransform(quad_src, quad_dst)
        a = cv2.warpPerspective(src, P, (w, h), flags=cv2.INTER_LINEAR)
        gy, th = int(min(top_l(x0), top_l(x1))), int(hav)
    else:
        if style == 'embossed':
            small = {'متوسط': 0.72, 'صعب': 0.72}.get(text, 1.0)   # as big as سهل
            a_fit = fit(a_src, bw * 0.85 * small, bh * 0.92 * small, style)
            cx, cy = (bx0 + bx1) / 2, h * 0.47   # middle of the plank face
        else:
            body = body_box.get(text)
            bx0b, bx1b = (body[0] * w, body[1] * w) if body else (bx0, bx1)
            a_fit = fit(a_src, (bx1b - bx0b) * 0.80, bh * 0.78, style)
            cx, cy = (bx0b + bx1b) / 2, h * 0.5 - 1
        th, tw = a_fit.shape
        gx = int(cx - tw / 2); gy = int(cy - th / 2)
        gx = max(0, min(w - tw, gx)); gy = max(0, min(h - th, gy))
        a[gy:gy + th, gx:gx + tw] = a_fit
    a = np.clip(a * 1.2, 0, 1)
    return finish(reg, out, a, gy, th, good, outline, style, h, w), m, box

def finish(reg, out, a, gy, th, good, outline, style, h, w):
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
    return res

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
