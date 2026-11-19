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
    bool small = capH < 25;
    /* Kufi letters are short: bigger, above all on the small button text.
       The clear font is tall: same size as the English letters. */
    float size = (t->style == 'K') ? (small ? 1.7f : 1.25f) : (small ? 1.0f : 1.1f);
    return capH * size / (t->alefH > 0 ? t->alefH : 1);
}

static bool Lang_Available(void) {
    return s_textsLen > 0;
}

bool Lang_TextWidth(Engine* engine, const char* str, int fontID, int* width) {
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

    int w = (int)(t->w * Lang_Zoom(engine, t, fontID, 1.0f));
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

    SDL_FRect dst = { ax, baselineY - t->baseline * k, t->w * k, t->h * k };

    /* same colour as the game would give its own letters */
    Uint8 r, g, b, a;
    SDL_GetTextureColorMod(font->texture, &r, &g, &b);
    SDL_GetTextureAlphaMod(font->texture, &a);
    SDL_SetTextureColorMod(t->tex, r * s_tint[f][0] / 255, g * s_tint[f][1] / 255, b * s_tint[f][2] / 255);
    SDL_SetTextureAlphaMod(t->tex, a);
    SDL_RenderCopyF(engine->render, t->tex, NULL, &dst);
    return true;
}
