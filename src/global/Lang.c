#include "Lang.h"
#include "Engine.h"
#include "Enums.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Project Owner & Creator: Muhamed Alzarqi
   AI Programming Assistant: Claude (Anthropic) */

#define LANG_MAX_TEXTS 256

typedef struct {
    char style;          /* 'K' Kufi, 'C' clear, 'A' clear + always shown */
    char en[64];
    int  enLen;
    bool prefix;         /* English ended with '*' */
    char file[16];
    int  w, h, baseline, alefH;
    SDL_Texture* tex;    /* loaded the first time it is drawn */
    bool failed;
} LangText;

static LangText s_texts[LANG_MAX_TEXTS];
static int  s_textsLen = 0;
static int  s_lang = LANG_EN;
static bool s_button = false;
static bool s_busy = false;   /* drawing the non-Arabic rest of a text */
static struct _Engine* s_engine = NULL;

/* Pictures with painted English words, and their Arabic copies */
static const int   s_swapTex[]  = { TEX_MENU, TEX_GAME_HUD };
static const char* s_swapFile[] = { "menu_ar.png", "gameinterface_ar.png" };
#define LANG_SWAPS 2
static SDL_Texture* s_texEn[LANG_SWAPS];
static SDL_Texture* s_texAr[LANG_SWAPS];
static bool s_texArFailed[LANG_SWAPS];

/* Where the capital letters of each game font sit (font pixels):
   top of the letters and their height. Same order as filesFonts. */
static const float s_capTop[] = { 2, 1, 0, 0, 6, 3, 7, 17, 7 };
static const float s_capH[]   = { 12, 9, 12, 12, 8, 15, 77, 21, 26 };
/* Colour of the letters of fonts that are already coloured */
static const Uint8 s_tint[][3] = {
    {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255},
    {255,255,255}, {249,220,79}, {236,216,134}, {213,212,188},
};
#define LANG_FONTS ((int)(sizeof(s_capH) / sizeof(s_capH[0])))


static bool Lang_ArLoad(Engine* engine);

void Lang_Init(Engine* engine) {
    s_engine = engine;
    s_textsLen = 0;

    char* base = Engine_GetBasePath();
    char path[300];
    SDL_snprintf(path, sizeof(path), "%slang/ar/index.txt", base ? base : "");
    SDL_free(base);

    FILE* f = fopen(path, "r");
    if (!f)
        return;

    char line[512];
    while (fgets(line, sizeof(line), f) && s_textsLen < LANG_MAX_TEXTS) {
        line[strcspn(line, "\r\n")] = 0;
        char* parts[7];
        int n = 0;
        char* p = line;
        parts[n++] = p;
        while (n < 7 && (p = strchr(p, '\t'))) {
            *p++ = 0;
            parts[n++] = p;
        }
        if (n < 7)
            continue;

        LangText* t = &s_texts[s_textsLen];
        memset(t, 0, sizeof(*t));
        t->style = parts[0][0];
        SDL_strlcpy(t->en, parts[1], sizeof(t->en));
        t->enLen = (int)strlen(t->en);
        if (t->enLen > 0 && t->en[t->enLen - 1] == '*') {
            t->prefix = true;
            t->en[--t->enLen] = 0;
        }
        SDL_strlcpy(t->file, parts[2], sizeof(t->file));
        t->w = atoi(parts[3]);
        t->h = atoi(parts[4]);
        t->baseline = atoi(parts[5]);
        t->alefH = atoi(parts[6]);
        s_textsLen++;
    }
    fclose(f);
}

int Lang_Get(void) { return s_lang; }

void Lang_ApplyTextures(Engine* engine) {
    if (!engine || !engine->textures)
        return;

    for (int i = 0; i < LANG_SWAPS; i++) {
        int id = s_swapTex[i];
        if (id >= (int)engine->texturesLen)
            continue;
        if (!s_texEn[i])
            s_texEn[i] = engine->textures[id];

        if (s_lang == LANG_AR && !s_texAr[i] && !s_texArFailed[i]) {
            char* base = Engine_GetBasePath();
            char path[300];
            SDL_snprintf(path, sizeof(path), "%simages/%s", base ? base : "", s_swapFile[i]);
            SDL_free(base);
            s_texAr[i] = Engine_TextureLoad(engine, path);
            s_texArFailed[i] = (s_texAr[i] == NULL);
        }

        engine->textures[id] = (s_lang == LANG_AR && s_texAr[i]) ? s_texAr[i] : s_texEn[i];
    }

    /* Arabic: load every text picture now, so nothing is read from
       storage in the middle of a level (that caused a small hitch) */
    if (s_lang == LANG_AR) {
        char* base = Engine_GetBasePath();
        for (int i = 0; i < s_textsLen; i++) {
            LangText* t = &s_texts[i];
            if (t->tex || t->failed)
                continue;
            char path[300];
            SDL_snprintf(path, sizeof(path), "%slang/ar/%s", base ? base : "", t->file);
            t->tex = Engine_TextureLoad(engine, path);
            t->failed = (t->tex == NULL);
        }
        SDL_free(base);
        Lang_ArLoad(engine);
    }
}

void Lang_Set(int lang) {
    s_lang = lang;
    Lang_ApplyTextures(s_engine);
}
void Lang_SetButtonStyle(bool on) { s_button = on; }

/* Finds the translation of str (exact text, or a prefix of it) */
static LangText* Lang_Find(const char* str, const char** rest) {
    char want = s_button ? 'K' : 'C';
    LangText* best = NULL;

    for (int pass = 0; pass < 2 && !best; pass++) {
        for (int i = 0; i < s_textsLen; i++) {
            LangText* t = &s_texts[i];
            if (s_lang != LANG_AR && t->style != 'A')
                continue;
            if (pass == 0 && t->style != want && !(want == 'C' && t->style == 'A'))
                continue;

            if (!t->prefix && strcmp(str, t->en) == 0) {
                *rest = NULL;
                return t;
            }
            if (t->prefix && strncmp(str, t->en, t->enLen) == 0 && str[t->enLen]) {
                if (!best || t->enLen > best->enLen) {
                    best = t;
                    *rest = str + t->enLen;
                }
            }
        }
    }
    return best;
}

static float Lang_Zoom(Engine* engine, LangText* t, int fontID, float scale) {
    int f = (fontID >= 0 && fontID < LANG_FONTS) ? fontID : 1;
    float capH = s_capH[f] * engine->scale_y * scale;
    bool small = s_capH[f] * engine->scale_y < 25;   /* by the font, not the zoom */
    /* Kufi letters are short: bigger, above all on the small button text.
       The clear font is tall: same size as the English letters. */
    bool big = s_capH[f] > 50;   /* the huge LEVEL title in the game */
    float size = (t->style == 'K') ? (small ? 1.7f : 0.9f) : (small ? 1.0f : (big ? 0.42f : 1.1f));
    if (t->style != 'K' && fontID == FONT_CANCUN_12)
        size = 0.72f;   /* results window: rows close together */
    return capH * size / (t->alefH > 0 ? t->alefH : 1);
}

static bool Lang_Available(void) {
    return s_textsLen > 0;
}

static float Lang_ArRun(Engine* engine, const char* str, int fontID, float scale,
                        float x, float y, bool draw);
bool Lang_HasArabic(const char* str);

bool Lang_TextWidth(Engine* engine, const char* str, int fontID, int* width) {
    if (str[0] && Lang_HasArabic(str)) {    /* a name typed in Arabic */
        *width = (int)Lang_ArRun(engine, str, fontID, 1.0f, 0, 0, false);
        return true;
    }
    if (s_busy || !Lang_Available() || !str[0])
        return false;

    if (strchr(str, '\n')) {
        if (s_lang != LANG_AR)
            return false;
        int maxW = 0;
        char line[256];
        const char* p = str;
        while (*p) {
            size_t n = strcspn(p, "\n");
            if (n >= sizeof(line)) n = sizeof(line) - 1;
            memcpy(line, p, n); line[n] = 0;
            int w = Engine_GetTextWidth(engine, line, fontID);
            if (w > maxW) maxW = w;
            p += n; if (*p == '\n') p++;
        }
        *width = maxW;
        return true;
    }

    const char* rest = NULL;
    LangText* t = Lang_Find(str, &rest);
    if (!t)
        return false;

    int f2 = (fontID >= 0 && fontID < LANG_FONTS) ? fontID : 1;
    float wk = 1.0f; (void)f2;
    int w = (int)(t->w * Lang_Zoom(engine, t, fontID, 1.0f) * wk);
    if (rest) {
        s_busy = true;
        w += Engine_GetTextWidth(engine, rest, fontID) + (int)(8 * engine->scale_x);
        s_busy = false;
    }
    *width = w;
    return true;
}

bool Lang_DrawText(Engine* engine, const char* str, int fontID,
                   float scale, float x, float y) {
    if (str[0] && fontID >= 0 && fontID < (int)engine->fontsLen && Lang_HasArabic(str)) {
        Lang_ArRun(engine, str, fontID, scale, x, y, true);
        return true;
    }
    if (s_busy || !Lang_Available() || !str[0] || fontID < 0 || fontID >= (int)engine->fontsLen)
        return false;

    Font* font = engine->fonts[fontID];

    /* several lines: each line on its own */
    if (strchr(str, '\n')) {
        if (s_lang != LANG_AR)
            return false;
        char line[256];
        const char* p = str;
        float ly = y;
        while (*p) {
            size_t n = strcspn(p, "\n");
            if (n >= sizeof(line)) n = sizeof(line) - 1;
            memcpy(line, p, n); line[n] = 0;
            Engine_DrawTextScale(engine, line, fontID, scale, x, ly);
            ly += font->rectList[0].h * engine->scale_y * scale;
            p += n; if (*p == '\n') p++;
        }
        return true;
    }

    const char* rest = NULL;
    LangText* t = Lang_Find(str, &rest);
    if (!t)
        return false;

    if (!t->tex && !t->failed) {
        char* base = Engine_GetBasePath();
        char path[300];
        SDL_snprintf(path, sizeof(path), "%slang/ar/%s", base ? base : "", t->file);
        SDL_free(base);
        t->tex = Engine_TextureLoad(engine, path);
        t->failed = (t->tex == NULL);
    }
    if (!t->tex)
        return false;

    /* the rest (numbers, names) on the left, Arabic on the right */
    float ax = x;
    if (rest) {
        s_busy = true;
        Engine_DrawTextScale(engine, rest, fontID, scale, x, y);
        ax += (Engine_GetTextWidth(engine, rest, fontID) + 8 * engine->scale_x) * scale;
        s_busy = false;
    }

    int f = fontID < LANG_FONTS ? fontID : 1;
    float k = Lang_Zoom(engine, t, fontID, scale);
    float capTop = s_capTop[f] * engine->scale_y * scale;
    float capH = s_capH[f] * engine->scale_y * scale;
    float baselineY = y + capTop + capH;

    float kx = k, ky = k;   /* same zoom both ways: natural shape */
    SDL_FRect dst = { ax, baselineY - t->baseline * ky, t->w * kx, t->h * ky };

    /* same colour as the game would give its own letters */
    Uint8 r, g, b, a;
    SDL_GetTextureColorMod(font->texture, &r, &g, &b);
    SDL_GetTextureAlphaMod(font->texture, &a);
    SDL_SetTextureColorMod(t->tex, r * s_tint[f][0] / 255, g * s_tint[f][1] / 255, b * s_tint[f][2] / 255);
    SDL_SetTextureAlphaMod(t->tex, a);
    SDL_RenderCopyF(engine->render, t->tex, NULL, &dst);
    return true;
}

/* ===================================================================
   Arabic names typed by the players: the letters are joined here
   (start / middle / end shapes) and drawn right to left from
   content/lang/ar/font.png (made by tools/make_arabic_font.py).
   =================================================================== */

#define AR_GLYPHS_MAX 200

typedef struct {
    int cp, form;
    SDL_Rect src;
    int left;
    float adv;
} ArGlyph;

static ArGlyph s_ar[AR_GLYPHS_MAX];
static int s_arLen = 0;
static int s_arBaseline = 0, s_arAlef = 1, s_arSpace = 10;
static SDL_Texture* s_arTex = NULL;
static bool s_arTried = false;

static bool Lang_ArLoad(Engine* engine) {
    if (s_arTried)
        return s_arTex != NULL;
    s_arTried = true;

    char* base = Engine_GetBasePath();
    char path[300];
    SDL_snprintf(path, sizeof(path), "%slang/ar/font.txt", base ? base : "");
    FILE* f = fopen(path, "r");
    if (!f) { SDL_free(base); return false; }

    int size, height;
    if (fscanf(f, "SIZE %d HEIGHT %d BASELINE %d ALEF %d SPACE %d",
               &size, &height, &s_arBaseline, &s_arAlef, &s_arSpace) != 5) {
        fclose(f); SDL_free(base); return false;
    }
    while (s_arLen < AR_GLYPHS_MAX) {
        ArGlyph* g = &s_ar[s_arLen];
        if (fscanf(f, "%d %d %d %d %d %d %d %f", &g->cp, &g->form, &g->src.x, &g->src.y,
                   &g->src.w, &g->src.h, &g->left, &g->adv) != 8)
            break;
        s_arLen++;
    }
    fclose(f);

    SDL_snprintf(path, sizeof(path), "%slang/ar/font.png", base ? base : "");
    SDL_free(base);
    s_arTex = Engine_TextureLoad(engine, path);
    return s_arTex != NULL;
}

/* next UTF-8 code point, moves *p */
static int Lang_NextCp(const char** p) {
    const unsigned char* s = (const unsigned char*)*p;
    int cp;
    if (s[0] < 0x80) { cp = s[0]; *p += 1; }
    else if ((s[0] & 0xE0) == 0xC0 && s[1]) { cp = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F); *p += 2; }
    else if ((s[0] & 0xF0) == 0xE0 && s[1] && s[2]) {
        cp = ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F); *p += 3;
    } else { cp = '?'; *p += 1; }
    return cp;
}

bool Lang_IsArabicCp(int cp) {
    return cp >= 0x0621 && cp <= 0x064A;
}

bool Lang_HasArabic(const char* str) {
    const char* p = str;
    while (*p)
        if (Lang_IsArabicCp(Lang_NextCp(&p)))
            return true;
    return false;
}

/* 'D' joins both sides, 'R' only to the letter before it, 'N' none */
static char Lang_JoinType(int cp) {
    static const int right[] = { 0x0627, 0x0623, 0x0625, 0x0622, 0x062F, 0x0630,
                                 0x0631, 0x0632, 0x0648, 0x0624, 0x0629 };
    if (!Lang_IsArabicCp(cp) || cp == 0x0621)
        return 'N';
    for (unsigned i = 0; i < sizeof(right) / sizeof(right[0]); i++)
        if (cp == right[i])
            return 'R';
    return 'D';
}

static int Lang_LamAlef(int alef) {
    switch (alef) {
        case 0x0627: return 0xFEFB;
        case 0x0623: return 0xFEF7;
        case 0x0625: return 0xFEF9;
        case 0x0622: return 0xFEF5;
    }
    return 0;
}

static ArGlyph* Lang_ArFind(int cp, int form) {
    for (int i = 0; i < s_arLen; i++)
        if (s_ar[i].cp == cp && s_ar[i].form == form)
            return &s_ar[i];
    if (form != 0)
        return Lang_ArFind(cp, form == 2 ? 3 : 0);
    return NULL;
}

/* Draws (draw = true) or measures an Arabic name. Returns its width. */
static float Lang_ArRun(Engine* engine, const char* str, int fontID, float scale,
                        float x, float y, bool draw) {
    if (!Lang_ArLoad(engine))
        return 0;

    int cps[128], n = 0;
    const char* p = str;
    while (*p && n < 128)
        cps[n++] = Lang_NextCp(&p);

    int f = (fontID >= 0 && fontID < LANG_FONTS) ? fontID : 1;
    float capH = s_capH[f] * engine->scale_y * scale;
    float k = capH * 0.78f / (s_arAlef > 0 ? s_arAlef : 1);   /* like the English capitals */

    /* shape: pick each letter's form */
    ArGlyph* gl[128]; int gn = 0; bool sp[128];
    for (int i = 0; i < n; i++) {
        int c = cps[i];
        bool prevJ = i > 0 && Lang_JoinType(cps[i - 1]) == 'D';
        sp[gn] = false;
        if (c == 0x0644 && i + 1 < n && Lang_LamAlef(cps[i + 1])) {
            gl[gn++] = Lang_ArFind(Lang_LamAlef(cps[i + 1]), prevJ ? 3 : 0);
            i++;
            continue;
        }
        char t = Lang_JoinType(c);
        if (t == 'N') {
            if (c == ' ') { sp[gn] = true; gl[gn++] = NULL; }
            else gl[gn++] = Lang_ArFind(c, 0);
            continue;
        }
        bool pj = prevJ;
        bool nj = t == 'D' && i + 1 < n && Lang_JoinType(cps[i + 1]) != 'N';
        int form = (pj && nj) ? 2 : pj ? 3 : nj ? 1 : 0;
        gl[gn++] = Lang_ArFind(c, form);
    }

    float total = 0;
    for (int i = 0; i < gn; i++)
        total += sp[i] ? s_arSpace : (gl[i] ? gl[i]->adv : 0);
    total *= k;
    if (!draw)
        return total;

    Font* font = engine->fonts[fontID];
    Uint8 r, g, b, a;
    SDL_GetTextureColorMod(font->texture, &r, &g, &b);
    SDL_GetTextureAlphaMod(font->texture, &a);
    SDL_SetTextureColorMod(s_arTex, r * s_tint[f][0] / 255, g * s_tint[f][1] / 255, b * s_tint[f][2] / 255);
    SDL_SetTextureAlphaMod(s_arTex, a);

    float baselineY = y + (s_capTop[f] + s_capH[f]) * engine->scale_y * scale;
    float pen = x + total;          /* right to left */
    for (int i = 0; i < gn; i++) {
        if (sp[i]) { pen -= s_arSpace * k; continue; }
        if (!gl[i]) continue;
        SDL_FRect dst = { pen + gl[i]->left * k, baselineY - s_arBaseline * k,
                          gl[i]->src.w * k, gl[i]->src.h * k };
        SDL_RenderCopyF(engine->render, s_arTex, &gl[i]->src, &dst);
        pen -= gl[i]->adv * k;
    }
    return total;
}

int Lang_StrWidth(const char* str, int fontID) {
    return s_engine ? Engine_GetTextWidth(s_engine, str, fontID) : 0;
}
