#ifndef LANG_H
#define LANG_H

/* Arabic language for Zuma HD.
   The Arabic texts are pictures made by tools/make_arabic.py
   (content/lang/ar). When the language is Arabic, every text the game
   draws that has a translation is drawn as its Arabic picture instead.

   Project Owner & Creator: Muhamed Alzarqi
   AI Programming Assistant: Claude (Anthropic) */

#include <stdbool.h>

struct _Engine;

enum { LANG_EN = 0, LANG_AR = 1 };

void Lang_Init(struct _Engine* engine);
int  Lang_Get(void);
void Lang_Set(int lang);
/* Use the Arabic (or English) button pictures; call after loading them */
void Lang_ApplyTextures(struct _Engine* engine);

/* Buttons and titles use the Kufi font, other texts the clear font */
void Lang_SetButtonStyle(bool on);

/* Hooks used by the engine's text functions; return true when handled */
bool Lang_DrawText(struct _Engine* engine, const char* str, int fontID,
                   float scale, float x, float y);
bool Lang_TextWidth(struct _Engine* engine, const char* str, int fontID,
                    int* width);

/* Names typed in Arabic */
bool Lang_HasArabic(const char* str);
bool Lang_IsArabicCp(int cp);
/* width of a text with the given game font (uses the engine of Lang_Init) */
int  Lang_StrWidth(const char* str, int fontID);

#endif
