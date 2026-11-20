#include "MenuMgr.h"
#include "../global/Lang.h"


void Button_Init(Button* btn) {
    btn->state = BTN_IDLE;
    btn->clicked = false;
    btn->pressed = false;

    btn->textureID = TEX_MENU;

    btn->soundHoverID       = -1;
    btn->soundHoverSfxID    = -1;
    btn->sndSfxPitch        = 0.0;
    btn->soundClickID       = SND_BUTTON1;
    btn->deadBorder = 0;

    btn->withText = false;

    btn->tRect.x = 0;
    btn->tRect.y = 532;
    btn->tRect.w = 180;
    btn->tRect.h = 72;

    btn->xscale = 1;
    btn->textScale = 1.0f;
    btn->drawW = 0;
    btn->hitPadX = 0;
    btn->hitPadY = 0;

    btn->pos.x = 0;
    btn->pos.y = 0;
}


void Button_SetText(Button* btn, const char* str) {
    strcpy(btn->text, str);
    btn->withText = true;
}


void Button_Update(Engine *engine, Button* btn) {
    int mx, my;
    Engine_GetMousePos(engine, &mx, &my);

    int bWidth = btn->drawW > 0 ? btn->drawW
               : btn->tRect.w + (btn->tRect.w / 3) * (btn->xscale-1);

    if (mx > btn->pos.x + btn->deadBorder - btn->hitPadX && 
        my > btn->pos.y + btn->deadBorder - btn->hitPadY &&
        mx < btn->pos.x+bWidth - btn->deadBorder + btn->hitPadX &&
        my < btn->pos.y+btn->tRect.h - btn->deadBorder + btn->hitPadY) {
            if (Engine_IsMouseDown()) {
                if (btn->state != BTN_CLICKED) {
                    btn->state = BTN_CLICKED;
                    Engine_PlaySound(engine, btn->soundClickID);
                }
            } else {
                btn->clicked = (btn->state == BTN_CLICKED);

                if (Engine_IsTouch()) {
                    /* a finger doesn't hover: no lit-up button after a tap */
                    btn->state = BTN_IDLE;
                    return;
                }

                if (btn->state != BTN_HOVER) {
                    if (btn->soundHoverSfxID != -1) {
                        Engine_PlaySoundSfxPitch(engine, btn->soundHoverSfxID, btn->sndSfxPitch);
                    } else if (btn->soundHoverID != -1) {
                        Engine_PlaySound(engine, btn->soundHoverID);
                    }
                }

                btn->state = BTN_HOVER;
            }
    } else {
        btn->state = BTN_IDLE;
        btn->clicked = false;
    }
}


int Button_IsClicked(Button* btn) {
    return btn->clicked;
}


void Button_Draw(Engine *engine, Button* btn) {
    SDL_Texture* tex = Engine_GetTextureSDL(engine, btn->textureID);

    SDL_Rect dstRect = {
        btn->pos.x, 
        btn->pos.y, 
        btn->tRect.w,
        btn->tRect.h,
    };

    SDL_Rect srcRect = btn->tRect;
    if (btn->xscale == 1) {
        srcRect = btn->tRect;

        if (btn->state == BTN_HOVER)
            srcRect.x = btn->tRect.x + btn->tRect.w;
        else if (btn->state == BTN_CLICKED || btn->pressed)
            srcRect.x = btn->tRect.x + 2 * btn->tRect.w;
        else 
            srcRect.x = btn->tRect.x;

        SDL_RenderCopy(
            engine->render, 
            tex,
            &srcRect,
            &dstRect
        );
    } else {
        srcRect.w = btn->tRect.w / 3;
        dstRect.w = srcRect.w;
        int dx = 0;
        for (int i = 0; i < 2+btn->xscale; i++) {
            if (i == 0) dx = 0;
            else if (i == 2+btn->xscale-1) dx = 2;
            else dx = 1;


            if (btn->state == BTN_HOVER)
                srcRect.x = srcRect.w*dx + btn->tRect.x + btn->tRect.w;
            else if (btn->state == BTN_CLICKED || btn->pressed)
                srcRect.x = srcRect.w*dx + btn->tRect.x + 2 * btn->tRect.w;
            else 
                srcRect.x = srcRect.w*dx + btn->tRect.x;

            /* drawW: the middle part is stretched to reach that width */
            int segW = srcRect.w;
            if (btn->drawW > 0 && dx == 1)
                dstRect.w = (btn->drawW - 2 * srcRect.w) / btn->xscale;
            else
                dstRect.w = segW;
            SDL_RenderCopy(
                engine->render, 
                tex,
                &srcRect,
                &dstRect
            );

            dstRect.x += dstRect.w;
        }
    }

    // DrawText
    if (btn->withText) {
        int bWidth = btn->drawW > 0 ? btn->drawW
                   : btn->tRect.w + (btn->tRect.w / 3) * btn->xscale;
        int cx = btn->pos.x + bWidth / 2 - (btn->drawW > 0 ? (Lang_Get() == LANG_AR ? -8 : 4) : 24);

        int dx = 0;
        int dy = 0;

        SDL_Color color = {255, 255, 255, 255};
        if (btn->state == BTN_IDLE) 
            color.b = 0;

        if (btn->state == BTN_CLICKED) {
            dx = -2;
            dy = +2;
        }

        Lang_SetButtonStyle(true);
        Engine_DrawTextExtScale(
            engine, 
            btn->text,
            FONT_CANCUN_10,
            btn->textScale,
            color,
            true,
            true,
            cx + dx,
            btn->pos.y + 18 + (1.0f - btn->textScale) * 14 + dy
        );
        Lang_SetButtonStyle(false);
    }

}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void CheckBox_Init(CheckBox* cb) {
    cb->checked = false;
    cb->clicked = false;

    cb->pos.x = 0;
    cb->pos.y = 0;
}


void CheckBox_Update(Engine *engine, CheckBox* cb) {
    int mx, my;
    Engine_GetMousePos(engine, &mx, &my);

    if (mx > cb->pos.x && 
        my > cb->pos.y &&
        mx < cb->pos.x+CHECK_BOX_WIDTH &&
        my < cb->pos.y+CHECK_BOX_HEIGHT) {
            if (Engine_IsMouseDown()) {
                    if (!cb->clicked) {
                        Engine_PlaySound(engine, SND_BUTTON2);
                        cb->checked = !cb->checked;
                        cb->clicked = true;
                    }
            } else {
                cb->clicked = false;
            }
    } 
}


int CheckBox_IsClicked(CheckBox* cb) {
    return cb->clicked;
}


int CheckBox_IsChecked(CheckBox* cb) {
    return cb->checked;
}


void CheckBox_Draw(Engine *engine, CheckBox* cb) {
    SDL_Texture* texture = Engine_GetTextureSDL(engine, TEX_MENU);

    SDL_Rect srcRect = {
        CHECK_BOX_WIDTH * !cb->checked, 
        462, 
        CHECK_BOX_WIDTH, 
        CHECK_BOX_HEIGHT
    };

    SDL_Rect dstRect = {
        cb->pos.x,
        cb->pos.y,
        CHECK_BOX_WIDTH,
        CHECK_BOX_HEIGHT
    };

    SDL_RenderCopy(
        engine->render,
        texture,
        &srcRect,
        &dstRect
    );
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

void Slider_Init(Slider* s, float* val) {
    s->pos.x = 0;
    s->pos.y = 0;

    s->drag = false;

    s->value = val;
    s->maxValue = 1.0f;

    s->thumbPos = (SLIDER_THUMB_MAX * (*s->value)) / s->maxValue;
}


void Slider_Update(Engine *engine, Slider* s) {
    int mx, my;
    Engine_GetMousePos(engine, &mx, &my);

    if (mx > s->pos.x + s->thumbPos && 
        my > s->pos.y &&
        mx < s->pos.x + s->thumbPos + SLIDER_THUMB_WIDTH &&
        my < s->pos.y+SLIDER_HEIGHT) {
            if (Engine_IsMouseDown()) {
                    s->drag = true;
            } else {
                s->drag = false;
            }
    }

    if (s->drag) {
         if (!Engine_IsMouseDown())
                    s->drag = false;

        s->thumbPos = mx - s->pos.x - SLIDER_THUMB_WIDTH / 2;

        if (s->thumbPos < 0)
            s->thumbPos = 0;
        if (s->thumbPos > SLIDER_THUMB_MAX)
            s->thumbPos = SLIDER_THUMB_MAX;
        
        *s->value = (s->maxValue * s->thumbPos) / SLIDER_THUMB_MAX;
    }
}


int Slider_IsDragging(Slider* s) {
    return s->drag;
}


void Slider_Draw(Engine *engine, Slider* s) {
    SDL_Texture* texture = Engine_GetTextureSDL(engine, TEX_MENU);

    SDL_Rect srcRect = {0, 365,  307, 49};
    SDL_Rect dstRect = {s->pos.x, s->pos.y, 307, 49};

    SDL_RenderCopy(
        engine->render,
        texture,
        &srcRect,
        &dstRect
    );

    // Thumb
    SDL_Rect srcSlRect = {0, 414,  61, 45};
    SDL_Rect dstSlRect = {
        s->pos.x + s->thumbPos, 
        s->pos.y, 
        61, 
        49
    };

    SDL_RenderCopy(
        engine->render,
        texture,
        &srcSlRect,
        &dstSlRect
    );
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

void DialogueBoxText_Init(DialogueBoxText* dbt) {
    dbt->FontID = FONT_CANCUN_10;
    strcpy(dbt->str, "ERROR");

    dbt->pos.x = 0;
    dbt->pos.y = 0;

    dbt->color.a = 255;
    dbt->color.r = 255;
    dbt->color.g = 255;
    dbt->color.b = 0;
}


DialogueBox* DialogueBox_Create() {
    DialogueBox* db = malloc(sizeof(DialogueBox));
    if (!db)
        return NULL;

    db->drawGodHead = false;
    db->animation  = false;
    db->exitButton = 0;
    db->pressedBtn = 0;

    db->sizeW = 3;
    db->sizeH = 3;

    db->pos.x = WINDOW_WIDTH  / 2;
    db->pos.y = WINDOW_HEIGHT / 2;

    db->startY = db->pos.y;

    strcpy(db->caption, "test");

    db->textsLen = 0;
    db->texts = NULL;

    db->buttonsLen = 0;
    db->buttons = NULL;

    db->chBoxesLen = 0;
    db->chBoxes = NULL;

    db->slidersLen = 0;
    db->sliders = NULL;

    db->langBtn = 0;
    db->langLabel = 0;
    db->captionScale = 1.0f;

    return db;
}


void DialogueBox_AddButton(DialogueBox* db, Button* btn) {
    Button* ptr = realloc(
        db->buttons, 
        sizeof(Button) * (db->buttonsLen+1)
    );

    if (!ptr)
        return;

    db->buttons = ptr;
    db->buttons[db->buttonsLen] = *btn;
    db->buttons[db->buttonsLen].pos.x = db->pos.x + btn->pos.x;
    db->buttons[db->buttonsLen].pos.y = db->pos.y + btn->pos.y;
    db->buttonsLen++;
}


void DialogueBox_AddCheckBox(DialogueBox* db, CheckBox* cb) {
    CheckBox* ptr = realloc(
        db->chBoxes, 
        sizeof(CheckBox) * (db->chBoxesLen+1)
    );

    if (!ptr)
        return;

    db->chBoxes = ptr;
    db->chBoxes[db->chBoxesLen] = *cb;
    db->chBoxes[db->chBoxesLen].pos.x = db->pos.x + cb->pos.x;
    db->chBoxes[db->chBoxesLen].pos.y = db->pos.y + cb->pos.y;
    db->chBoxesLen++;
}


void DialogueBox_AddSlider(DialogueBox* db, Slider* s) {
    Slider* ptr = realloc(
        db->sliders, 
        sizeof(Slider) * (db->slidersLen+1)
    );

    if (!ptr)
        return;

    db->sliders = ptr;
    db->sliders[db->slidersLen] = *s;
    db->sliders[db->slidersLen].pos.x = db->pos.x + s->pos.x;
    db->sliders[db->slidersLen].pos.y = db->pos.y + s->pos.y;
    db->slidersLen++;
}


void DialogueBox_AddText(DialogueBox* db, DialogueBoxText* t) {
    DialogueBoxText* ptr = realloc(
        db->texts, 
        sizeof(DialogueBoxText) * (db->textsLen+1)
    );

    if (!ptr)
        return;

    db->texts = ptr;
    db->texts[db->textsLen] = *t;

    db->texts[db->textsLen].pos.x = db->pos.x + t->pos.x;
    db->texts[db->textsLen].pos.y = db->pos.y + t->pos.y;

    db->textsLen++;
}


void DialogueBox_SetTextStr(DialogueBox* db, int textID, const char* str) {
    if (textID < 0 || textID >= db->textsLen)
        return;

    strcpy(db->texts[textID].str, str);
}


void DialogueBox_Animate(DialogueBox* db) {
    db->animation = true;

    int dy = WINDOW_HEIGHT + (121 * db->sizeH) / 2;
    db->pos.y += dy;

    for (int i = 0; i < db->buttonsLen; i++) 
        db->buttons[i].pos.y += dy;
    for (int i = 0; i < db->chBoxesLen; i++)
        db->chBoxes[i].pos.y += dy;
    for (int i = 0; i < db->slidersLen; i++)
        db->sliders[i].pos.y += dy;
    for (int i = 0; i < db->textsLen; i++)
        db->texts[i].pos.y += dy;
}


int DialogueBox_GetBtn(DialogueBox* db) {
    int btn = db->pressedBtn;
    db->pressedBtn = 0;
    return btn;
}


void DialogueBox_Update(Engine *engine, DialogueBox* db) {

    if (db->animation)
        return;

    for (int i = 0; i < db->buttonsLen; i++) {
        Button_Update(engine, &db->buttons[i]);

        if (Button_IsClicked(&db->buttons[i]))
            db->pressedBtn = i+1;
    }

    for (int i = 0; i < db->chBoxesLen; i++)
        CheckBox_Update(engine, &db->chBoxes[i]);

    for (int i = 0; i < db->slidersLen; i++)
        Slider_Update(engine, &db->sliders[i]);
}


void DialogueBox_Draw(Engine *engine, DialogueBox* db) {

    if (db->animation) {
        if (db->pos.y > db->startY) {
            int spd = 0;
            if (DB_ANIM_SPD < db->pos.y-db->startY) {
                spd = DB_ANIM_SPD;
            } else {
                spd = db->pos.y-db->startY;
            }
            
            db->pos.y -= spd;
            for (int i = 0; i < db->buttonsLen; i++) 
                db->buttons[i].pos.y -= spd;
            for (int i = 0; i < db->chBoxesLen; i++)
                db->chBoxes[i].pos.y -= spd;
            for (int i = 0; i < db->slidersLen; i++)
                db->sliders[i].pos.y -= spd;
            for (int i = 0; i < db->textsLen; i++)
                db->texts[i].pos.y -= spd;
        } else {
            db->animation = false;
        }
    }

    SDL_Texture* texture = Engine_GetTextureSDL(engine, TEX_MENU);

    SDL_Point dPos;
    dPos.x = db->pos.x - (121 * db->sizeW) / 2;
    dPos.y = db->pos.y - (121 * db->sizeH) / 2;

    SDL_Rect srcBgRect = {0, 0, 121, 121};
    SDL_Rect dstBgRect = {0, 0, 121, 121};
    for (int i = 0; i < db->sizeH; i++) {
        if (i == 0)
            srcBgRect.y = 0;
        else if (i == db->sizeH-1)
            srcBgRect.y = 242;
        else 
            srcBgRect.y = 121;

        for (int j = 0; j < db->sizeW; j++) {
            if (j == 0)
                srcBgRect.x = 0;
            else if (j == db->sizeW-1)
                srcBgRect.x = 242;
            else 
                srcBgRect.x = 121;

            dstBgRect.x = dPos.x + 121 * j;
            dstBgRect.y = dPos.y + 121 * i;

            SDL_RenderCopy(
                engine->render,
                texture,
                &srcBgRect,
                &dstBgRect
            );
        }
    }

    // God Head
    if (db->drawGodHead) {
        SDL_Rect srcRect = {363, 0, 234, 210};
        SDL_Rect dstRect = {
            db->pos.x - 117, 
            dPos.y - 112, 
            234, 
            210
        };

        SDL_RenderCopy(
            engine->render,
            texture,
            &srcRect,
            &dstRect
        );
    }

    // Caption
    SDL_Color cColor = {255, 255, 255, 255};
    int cDy = 32;
    if (db->drawGodHead)
        cDy = 72;
    Lang_SetButtonStyle(true);
    Engine_DrawTextExtScale(
        engine, 
        db->caption, 
        FONT_NATIVE_ALIEN_EXT_18,
        Lang_Get() == LANG_AR ? 1.0f : db->captionScale,  /* Arabic: own size */
        cColor,
        false, 
        true,
        db->pos.x,
        dPos.y + cDy + (Lang_Get() == LANG_AR ? 0 : (1.0f - db->captionScale) * 30)
    );
    Lang_SetButtonStyle(false);

    for (int i = 0; i < db->buttonsLen; i++)
        Button_Draw(engine, &db->buttons[i]);

    for (int i = 0; i < db->chBoxesLen; i++) 
        CheckBox_Draw(engine, &db->chBoxes[i]);

    for (int i = 0; i < db->slidersLen; i++)
        Slider_Draw(engine, &db->sliders[i]);

    for (int i = 0; i < db->textsLen; i++) {
        if (db->texts[i].color.a == 0)
            continue;   /* invisible: drawn elsewhere */
        Engine_DrawTextExt(
            engine, 
            db->texts[i].str,
            db->texts[i].FontID,
            db->texts[i].color,
            true,
            false,
            db->texts[i].pos.x,
            db->texts[i].pos.y
        );
    }
}


void DialogueBox_Destroy(DialogueBox* db) {
    db->buttonsLen = 0;
    free(db->buttons);
    db->chBoxesLen = 0;
    free(db->chBoxes);
    db->slidersLen = 0;
    free(db->sliders);
    db->textsLen = 0;
    free(db->texts);

    free(db);
    db = NULL;
}


/* Word + two language buttons, centred in the window.
   English: LANGUAGE [ENGLISH][ARABIC]   Arabic: [ENGLISH][ARABIC] (word) */
/* Phones have no FULLSCREEN box: the language row moves down into
   the middle of the free space */
#if defined(__ANDROID__) || defined(ZT_PHONE)
#define LANG_ROW_DOWN 36
#else
#define LANG_ROW_DOWN 0
#endif

#define DB_OPTIONS_LABEL_X (-56 - 170)   /* left edge of MUSIC / SOUNDS VOLUME */
#define DB_OPTIONS_SLIDER_X (-56)

/* Places the options window's labels, sliders and language buttons.
   English: labels on the left, sliders on the right.
   Arabic: the same, mirrored (labels on the right, right-aligned). */
static void DialogueBox_LayoutOptions(Engine *engine, DialogueBox* db) {
    if (db->langBtn <= 0 || db->buttonsLen < db->langBtn + 2)
        return;

    bool ar = Lang_Get() == LANG_AR;
    const int btnW = 150, gap = -10, labelGap = 10;   /* drawW 150 */
    int left  = db->pos.x + DB_OPTIONS_LABEL_X;       /* English labels start here */
    int right = db->pos.x - DB_OPTIONS_LABEL_X;       /* Arabic labels end here */

    for (int i = 0; i < db->slidersLen && i < 2; i++)
        db->sliders[i].pos.x = ar ? db->pos.x - DB_OPTIONS_SLIDER_X - SLIDER_WIDTH
                                  : db->pos.x + DB_OPTIONS_SLIDER_X;

    /* the texts of the two sliders and the LANGUAGE word */
    int ids[3] = { 0, 1, db->langLabel };
    for (int k = 0; k < 3; k++) {
        DialogueBoxText* t = &db->texts[ids[k]];
        int w = Engine_GetTextWidth(engine, t->str, t->FontID);
        t->pos.x = ar ? right - w : left;
    }

    DialogueBoxText* lab = &db->texts[db->langLabel];
    int labelW = Engine_GetTextWidth(engine, lab->str, lab->FontID);
    Button* en = &db->buttons[db->langBtn];
    Button* arb = &db->buttons[db->langBtn + 1];
    /* Kufi words are wider: smaller so they fit the short buttons */
    en->textScale = arb->textScale = ar ? 0.75f : 0.95f;
    if (ar) {
        /* [ENGLISH][ARABIC] LANGUAGE  (mirror of the English row) */
        arb->pos.x = lab->pos.x - labelGap - btnW;
        en->pos.x  = arb->pos.x - gap - btnW;
    } else {
        en->pos.x  = left + labelW + labelGap;
        arb->pos.x = en->pos.x + btnW + gap;
    }
}

DialogueBox* DialogueBox_CreateOptions(Engine *engine) {
    DialogueBox* db = DialogueBox_Create();
    db->sizeW = 5;
    db->sizeH = 4;
    db->exitButton = 1;
    strcpy(db->caption, "OPTIONS");

    Slider slMus;
    Slider_Init(&slMus, &engine->volMus);
    slMus.pos.x = -56;
    slMus.pos.y = -128;
    DialogueBox_AddSlider(db, &slMus);

    DialogueBoxText tMus;
    DialogueBoxText_Init(&tMus);
    tMus.pos.x = slMus.pos.x - 170;
    tMus.pos.y = slMus.pos.y + 10;
    strcpy(tMus.str, "MUSIC VOLUME");
    DialogueBox_AddText(db, &tMus);

    Slider slSnd;
    Slider_Init(&slSnd, &engine->volSnd);
    slSnd.pos.x = slMus.pos.x;
    slSnd.pos.y = slMus.pos.y + 64;
    DialogueBox_AddSlider(db, &slSnd);

    DialogueBoxText tSnd;
    DialogueBoxText_Init(&tSnd);
    tSnd.pos.x = slSnd.pos.x - 170;
    tSnd.pos.y = slSnd.pos.y + 10;
    strcpy(tSnd.str, "SOUNDS VOLUME");
    DialogueBox_AddText(db, &tSnd);

#if !defined(__ANDROID__) && !defined(ZT_PHONE)
    CheckBox cbFullscr;
    CheckBox_Init(&cbFullscr);
    cbFullscr.checked = engine->fullScr;
    cbFullscr.pos.x = -236;
    cbFullscr.pos.y = slSnd.pos.y + 112;
    DialogueBox_AddCheckBox(db, &cbFullscr);

    DialogueBoxText tFullscr;
    DialogueBoxText_Init(&tFullscr);
    tFullscr.pos.x = cbFullscr.pos.x + 80;
    tFullscr.pos.y = cbFullscr.pos.y + 24;
    strcpy(tFullscr.str, "FULLSCREEN");
    DialogueBox_AddText(db, &tFullscr);
#endif

    /* Language: two buttons, the chosen one stays pressed.
       "LANGUAGE" and "ARABIC" are always shown in Arabic too. */
    DialogueBoxText tLang;
    DialogueBoxText_Init(&tLang);
    tLang.pos.x = -258;
    tLang.pos.y = slSnd.pos.y + 70 + LANG_ROW_DOWN;
    strcpy(tLang.str, "LANGUAGE");
    db->langLabel = db->textsLen;
    DialogueBox_AddText(db, &tLang);

    Button bDone;
    Button_Init(&bDone);
    Button_SetText(&bDone, "DONE");
    bDone.xscale = 6;
    bDone.pos.x = -236;
    bDone.pos.y = 120;
    DialogueBox_AddButton(db, &bDone);

    db->captionScale = 0.8f;   /* OPTIONS title a bit smaller */

    /* after DONE, so the other buttons keep their numbers */
    db->langBtn = db->buttonsLen;


    const char* langs[2] = { "ENGLISH", "ARABIC" };
    for (int i = 0; i < 2; i++) {
        Button b;
        Button_Init(&b);
        Button_SetText(&b, langs[i]);
        b.xscale = 2;      /* segmented button, stretched to drawW */
        b.drawW = 150;
        b.deadBorder = 10; /* the two buttons touch: no shared tap area */
        b.textScale = 0.8f;
        b.pos.x = 0;   /* placed by DialogueBox_LayoutOptions */
        b.pos.y = slSnd.pos.y + 52 + LANG_ROW_DOWN;
        b.pressed = (Lang_Get() == (i == 0 ? LANG_EN : LANG_AR));
        DialogueBox_AddButton(db, &b);
    }
    DialogueBox_LayoutOptions(engine, db);

    return db;
}


void DialogueBox_UpdateOptions(Engine *engine, DialogueBox* db) {
    DialogueBox_Update(engine, db);

    if (db->langBtn > 0 && db->buttonsLen >= db->langBtn + 2) {
        for (int i = 0; i < 2; i++) {
            if (Button_IsClicked(&db->buttons[db->langBtn + i])) {
                Lang_Set(i == 0 ? LANG_EN : LANG_AR);
                Engine_SaveSettings(engine);
                DialogueBox_LayoutOptions(engine, db);
            }
        }
        db->buttons[db->langBtn].pressed     = (Lang_Get() == LANG_EN);
        db->buttons[db->langBtn + 1].pressed = (Lang_Get() == LANG_AR);
        /* on a phone the "mouse" stays where the finger was: no hover look */
        for (int i = 0; i < 2; i++)
            if (db->buttons[db->langBtn + i].state == BTN_HOVER)
                db->buttons[db->langBtn + i].state = BTN_IDLE;
    }

#if !defined(__ANDROID__) && !defined(ZT_PHONE)
    if (CheckBox_IsClicked(&db->chBoxes[0])) {
        if (CheckBox_IsChecked(&db->chBoxes[0])) {
            SDL_SetWindowFullscreen(
                engine->win, SDL_WINDOW_FULLSCREEN);
            engine->fullScr = true;
        } else {
            SDL_SetWindowFullscreen(engine->win, 0);
            engine->fullScr = false;
        }
    }
#endif

    if (Slider_IsDragging(&db->sliders[0])) {
        Engine_UpdateMusicVolume(engine);
    }

    if (Slider_IsDragging(&db->sliders[1])) {
        Engine_UpdateSoundVolume(engine);
    }
}


DialogueBox* DialogueBox_CreateMenu(Engine *engine) {
    DialogueBox* db = DialogueBox_CreateOptions(engine);
    strcpy(db->caption, "mENU");
    /* the language is changed from the main menu only */
    if (db->langBtn > 0) {
        db->buttonsLen = db->langBtn;
        db->langBtn = 0;
        for (int i = 0; i < db->textsLen; i++)
            if (strcmp(db->texts[i].str, "LANGUAGE") == 0)
                db->texts[i].color.a = 0;
    }

    strcpy(db->buttons[0].text, "BACK TO GAME");

    Button bMMenu;
    Button_Init(&bMMenu);
    Button_SetText(&bMMenu, "RETURN TO MAIN MENU");
    bMMenu.xscale = db->buttons[0].xscale;
    bMMenu.pos.x = -236;
    bMMenu.pos.y = 48;
    DialogueBox_AddButton(db, &bMMenu);

    return db;
}


void DialogueBox_UpdateMenu(Engine *engine, DialogueBox* db) {
    DialogueBox_UpdateOptions(engine, db);
}


DialogueBox* DialogueBox_CreateResults(bool hasNextLevel) {
    DialogueBox* db = DialogueBox_Create();
    strcpy(db->caption, "STATS");
    db->sizeW = 6;
    db->sizeH = 4;
    db->drawGodHead = true;

    // Param Names
    DialogueBoxText tParamNames[7];
    for (int i = 0; i < 7; i++) {
        DialogueBoxText_Init(&tParamNames[i]);
        tParamNames[i].FontID = FONT_CANCUN_12;
        if (i < 5) {
            tParamNames[i].pos.x = -262;
            tParamNames[i].pos.y = -100 + (28 * i);
        } else {
            tParamNames[i].pos.x = 32;
            tParamNames[i].pos.y = -100 + (28 * (i-5));
        }
    }

    strcpy(tParamNames[0].str, "POINTS");
    strcpy(tParamNames[1].str, "COMBOS");
    strcpy(tParamNames[2].str, "COINS");
    strcpy(tParamNames[3].str, "MAX CHAIN");
    strcpy(tParamNames[4].str, "MAX COMBO");
    strcpy(tParamNames[5].str, "YOUR TIME");
    strcpy(tParamNames[6].str, "ACE TIME");

    for (int i = 0; i < 7; i++)
        DialogueBox_AddText(db, &tParamNames[i]);

    // Params
    DialogueBoxText tParam[7];
    for (int i = 0; i < 7; i++) {
        DialogueBoxText_Init(&tParam[i]);
        tParam[i].FontID = FONT_CANCUN_12;
        tParam[i].color.g = 152;
        tParam[i].pos.x = tParamNames[i].pos.x + 185;
        tParam[i].pos.y = tParamNames[i].pos.y;
        strcpy(tParam[i].str, "NULL");

        DialogueBox_AddText(db, &tParam[i]);
    }

    // Buttons: MAIN MENU (left) and NEXT level (right)
    Button bOk;
    Button_Init(&bOk);
    Button_SetText(&bOk, "MAIN MENU");
    bOk.xscale = hasNextLevel ? 3 : 8;
    bOk.pos.x = -297;
    bOk.pos.y = 120;
    DialogueBox_AddButton(db, &bOk);

    if (hasNextLevel) {
        Button bNext;
        Button_Init(&bNext);
        Button_SetText(&bNext, "NEXT");
        bNext.xscale = 3;
        bNext.pos.x = -297 + 300;
        bNext.pos.y = 120;
        DialogueBox_AddButton(db, &bNext);
    }

    DialogueBox_Animate(db);

    return db;
}


DialogueBox* DialogueBox_CreateGameOver() {
    DialogueBox* db = DialogueBox_Create();
    strcpy(db->caption, "GAmE OvER");
    db->sizeW = 6;
    db->sizeH = 4;
    db->drawGodHead = true;

    // Param Names
    DialogueBoxText tParamNames[5];
    for (int i = 0; i < 5; i++) {
        DialogueBoxText_Init(&tParamNames[i]);
        tParamNames[i].FontID = FONT_CANCUN_12;
        if (i < 3) {
            tParamNames[i].pos.x = -262;
            tParamNames[i].pos.y = -100 + (28 * i);
        } else {
            tParamNames[i].pos.x = 32;
            tParamNames[i].pos.y = -100 + (28 * (i-3));
        }
    }

    strcpy(tParamNames[0].str, "TIME");
    strcpy(tParamNames[1].str, "COMBOS");
    strcpy(tParamNames[2].str, "COINS");
    strcpy(tParamNames[3].str, "MAX CHAIN");
    strcpy(tParamNames[4].str, "MAX COMBO");

    for (int i = 0; i < 5; i++)
        DialogueBox_AddText(db, &tParamNames[i]);

    // Params
    DialogueBoxText tParam[5];
    for (int i = 0; i < 5; i++) {
        DialogueBoxText_Init(&tParam[i]);
        tParam[i].FontID = FONT_CANCUN_12;
        tParam[i].color.g = 152;
        tParam[i].pos.x = tParamNames[i].pos.x + 185;
        tParam[i].pos.y = tParamNames[i].pos.y;
        strcpy(tParam[i].str, "NULL");

        DialogueBox_AddText(db, &tParam[i]);
    }

    // Button
    /* LEAVE (left) and TRY AGAIN (right); TRY AGAIN stays button 1 */
    Button bRetry;
    Button_Init(&bRetry);
    Button_SetText(&bRetry, "TRY AGAIN");
    bRetry.xscale = 3;
    bRetry.pos.x = -297 + 300;   /* right */
    bRetry.pos.y = 120;
    DialogueBox_AddButton(db, &bRetry);

    Button bLeave;
    Button_Init(&bLeave);
    Button_SetText(&bLeave, "LEAVE");
    bLeave.xscale = 3;
    bLeave.pos.x = -297;         /* left */
    bLeave.pos.y = 120;
    DialogueBox_AddButton(db, &bLeave);

    DialogueBox_Animate(db);

    return db;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

Menu* Menu_Create() {
    Menu* menu = malloc(sizeof(Menu));

    menu->haveSky = false;

    menu->buttons = NULL;
    menu->buttonsLen = 0;

    menu->pressedBtn = 0;

    return menu;
}


void Menu_AddButton(Menu* menu, Button* btn) {
    Button* ptr = realloc(
        menu->buttons, 
        sizeof(Button) * (menu->buttonsLen+1));

    if (!ptr)
        return;

    menu->buttons = ptr;
    menu->buttons[menu->buttonsLen] = *btn;
    menu->buttonsLen++;
}


void Menu_SetBgRect(Menu* menu, SDL_Rect rect) {
    menu->trBg = rect;
}


void Menu_SetSkyRect(Menu* menu, SDL_Rect rect) {
    menu->haveSky = true;

    menu->trBgSky = rect;
}


int Menu_GetButton(Menu* menu) {
    int btn = menu->pressedBtn;
    menu->pressedBtn = 0;
    return btn;
}


void Menu_Update(Engine *engine, Menu* menu) {
    for (int i = 0; i < menu->buttonsLen; i++) {
        Button_Update(engine, &menu->buttons[i]);

        if (Button_IsClicked(&menu->buttons[i]))
            menu->pressedBtn = i+1;
    }
}


void Menu_Draw(Engine *engine, Menu* menu) {
    SDL_Texture* tex = Engine_GetTextureSDL(engine, TEX_MENU);

    if (menu->haveSky) {
        static int skyPos = 0;
        skyPos += SKY_SPD;

        skyPos %= 1280;

        SDL_Rect dtrSky = {
            skyPos, 
            0, 
            menu->trBgSky.w,
            menu->trBgSky.h
        };

        /* leave out the 2 outer columns of the sky picture: they are dark
           and showed as a black line moving across the sky */
        SDL_Rect srcSky = menu->trBgSky;
        srcSky.x += 2;
        srcSky.w -= 4;

        SDL_RenderCopy(engine->render, tex, &srcSky, &dtrSky);

        dtrSky.x = skyPos-1279;
        SDL_RenderCopy(engine->render, tex, &srcSky, &dtrSky);
    }

    SDL_Rect dtrBg = {0, 0, 1280, 720};
    SDL_RenderCopy(
        engine->render, 
        tex,
        &menu->trBg,
        &dtrBg
    );

    for (int i = 0; i < menu->buttonsLen; i++) {
        Button_Draw(engine, &menu->buttons[i]);
    }

}


void Menu_Destroy(Menu* menu) {
    menu->buttonsLen = 0;
    free(menu->buttons);
    free(menu);
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

void MenuMgr_Init(MenuMgr* m, int* curLvl, int* curDiff) {
    m->roomID = MR_MAIN;
    m->room = NULL;
    m->dbOptions = NULL;
    m->inDialog = false;
    m->curLvl = curLvl;
    m->curDiff = curDiff;

    m->profilesReady = false;
    m->dbPlayers = NULL;
    m->dbNewPlayer = NULL;
    m->dbDelete = NULL;
    m->selSlot = -1;
    m->rowsLen = 0;
}


void MenuMgr_Set(Engine *engine, MenuMgr* m, int room) {
    if (m->room != NULL) {
        Menu_Destroy(m->room);
    }

    m->roomID = room;
    if (m->roomID == MR_GAME) {
        m->room = NULL;
        return;
    }

    m->room = Menu_Create();
    if (m->roomID == MR_MAIN) {
        Engine_PlayMusic(engine, MUS_MAIN_MENU);

        SDL_Rect mBgRect = {1282, 720, 1280, 720};
        Menu_SetBgRect(m->room, mBgRect);
        SDL_Rect mSkyRect = {0, 720, 1280, 250};
        Menu_SetSkyRect(m->room, mSkyRect);

        Button btnAdv, btnGaunt, btnOptions, btnMore, btnQuit;
        Button_Init(&btnAdv);
        Button_Init(&btnGaunt);
        Button_Init(&btnOptions);
        Button_Init(&btnMore);
        Button_Init(&btnQuit);

        SDL_Rect rBtnAdv = {2724, 901, 245, 138};
        SDL_Rect rBtnGaunt = {2648, 776, 270, 125};
        SDL_Rect rBtnOptions = {2563, 647, 299, 129};
        SDL_Rect rBtnMore = {2924, 1038, 177, 190};
        SDL_Rect rBtnQuit = {2917, 1226, 180, 211};

        btnAdv.tRect = rBtnAdv;
        btnAdv.soundHoverSfxID = SND_FX_CHORAL1;
        btnAdv.pos.x = 840;
        btnAdv.pos.y = 95;
        btnAdv.sndSfxPitch = 12;
        btnAdv.deadBorder = 16;

        btnGaunt.tRect = rBtnGaunt;
        btnGaunt.soundHoverSfxID = SND_FX_CHORAL1;
        btnGaunt.pos.x = 816;
        btnGaunt.pos.y = 230;
        btnGaunt.sndSfxPitch = 10;
        btnGaunt.deadBorder = 16;

        btnOptions.tRect = rBtnOptions;
        btnOptions.soundHoverSfxID = SND_FX_CHORAL1;
        btnOptions.pos.x = 790;
        btnOptions.pos.y = 354;
        btnOptions.sndSfxPitch = 8;
        btnOptions.deadBorder = 16;

        btnMore.tRect = rBtnMore;
        btnMore.soundHoverSfxID = SND_FX_CHORAL1;
        btnMore.pos.x = 750;
        btnMore.pos.y = 452;
        btnMore.sndSfxPitch = 4;
        btnMore.deadBorder = 16;

        btnQuit.tRect = rBtnQuit;
        btnQuit.soundHoverSfxID = SND_FX_CHORAL1;
        btnQuit.pos.x = 902;
        btnQuit.pos.y = 470;
        btnQuit.sndSfxPitch = -2;
        btnQuit.deadBorder = 16;

        Menu_AddButton(m->room, &btnAdv);
        Menu_AddButton(m->room, &btnGaunt);
        Menu_AddButton(m->room, &btnOptions);
        Menu_AddButton(m->room, &btnMore);
        Menu_AddButton(m->room, &btnQuit);

    } else if (m->roomID == MR_GAUNTLET) {
        Engine_PlayMusic(engine, MUS_GAUNTLET);

        SDL_Rect mBgRect = {1282, 1440, 1280, 720};
        Menu_SetBgRect(m->room, mBgRect);
        SDL_Rect mSkyRect = {0, 1442, 1280, 250};
        Menu_SetSkyRect(m->room, mSkyRect);

        Button btnMainMenu, btnBack, btnPlay, btnNext;

        Button_Init(&btnMainMenu);
        SDL_Rect rBtnMainMenu = {1280, 2160, 211, 84};
        btnMainMenu.tRect = rBtnMainMenu;
        btnMainMenu.pos.x = 172;
        btnMainMenu.pos.y = 632;
        Menu_AddButton(m->room, &btnMainMenu);

        Button_Init(&btnBack);
        SDL_Rect rBtnBack = {1924, 2260, 139, 48};
        btnBack.tRect = rBtnBack;
        btnBack.pos.x = 426;
        btnBack.pos.y = 667;
        btnBack.hitPadX = 5;    /* bigger tap area, up to the next button */
        btnBack.hitPadY = 30;   /* and up/down for fingers */
        Menu_AddButton(m->room, &btnBack);

        Button_Init(&btnPlay);
        SDL_Rect rBtnPlay = {1922, 2210, 157, 49};
        btnPlay.tRect = rBtnPlay;
        btnPlay.pos.x = 576;
        btnPlay.pos.y = 667;
        btnPlay.hitPadX = 5;    /* bigger tap area, up to the next button */
        btnPlay.hitPadY = 30;   /* and up/down for fingers */
        Menu_AddButton(m->room, &btnPlay);

        Button_Init(&btnNext);
        SDL_Rect rBtnNext = {1922, 2161, 141, 48};
        btnNext.tRect = rBtnNext;
        btnNext.pos.x = 745;
        btnNext.pos.y = 667;
        btnNext.hitPadX = 5;    /* bigger tap area, up to the next button */
        btnNext.hitPadY = 30;   /* and up/down for fingers */
        Menu_AddButton(m->room, &btnNext);

        // Difficulty
        Button btnRabbit, btnEagle, btnJaguar, btnSunGod;
        Button_Init(&btnRabbit);
        SDL_Rect rBtnRabbit = {1280+328, 2336, 328, 63};
        btnRabbit.tRect = rBtnRabbit;
        btnRabbit.pos.x = 470;
        btnRabbit.pos.y = 270;
        Menu_AddButton(m->room, &btnRabbit);

        Button_Init(&btnEagle);
        SDL_Rect rBtnEagle = {1280+258, 2399, 258, 49};
        btnEagle.tRect = rBtnEagle;
        btnEagle.pos.x = 504;
        btnEagle.pos.y = 196;
        Menu_AddButton(m->room, &btnEagle);

        Button_Init(&btnJaguar);
        SDL_Rect rBtnJaguar = {1280+205, 2448, 205, 43};
        btnJaguar.tRect = rBtnJaguar;
        btnJaguar.pos.x = 530;
        btnJaguar.pos.y = 124;
        Menu_AddButton(m->room, &btnJaguar);

        Button_Init(&btnSunGod);
        SDL_Rect rBtnSunGod = {1280+159, 2491, 159, 54};
        btnSunGod.tRect = rBtnSunGod;
        btnSunGod.pos.x = 554;
        btnSunGod.pos.y = 45;
        Menu_AddButton(m->room, &btnSunGod);
    }

}



/////////////////////////////////////////////////////////////////////////
// ==== Player accounts (offline, like the original Zuma) ==== //
// Project Owner & Creator: Muhamed Alzarqi
// AI Programming Assistant: Claude (Anthropic)

#define NEWPLAYER_TEXT_NAME 1
#define NEWPLAYER_TEXT_ERROR 2

static void MenuMgr_UsePlayer(LevelMgr *levelMgr, MenuMgr* m, int slot) {
    m->profiles.current = slot;
    Profiles_Save(&m->profiles);

    char file[32];
    if (slot >= 0) {
        Profiles_ProgressFile(slot, file, sizeof(file));
        LevelMgr_UseProgressFile(levelMgr, file);
    } else {
        LevelMgr_UseProgressFile(levelMgr, "");
    }
    *m->curLvl = 0;   /* start from level 1 of this player */
}

static void MenuMgr_UpdateInDialog(MenuMgr* m) {
    m->inDialog = m->dbOptions || m->dbPlayers || m->dbNewPlayer || m->dbDelete;
}

static DialogueBox* MenuMgr_CreatePlayersDialog(MenuMgr* m) {
    DialogueBox* db = DialogueBox_Create();
    db->sizeW = 6;
    db->sizeH = 5;
    strcpy(db->caption, "PlAYERS");

    m->rowsLen = 0;
    for (int i = 0; i < PROFILES_MAX; i++) {
        if (!m->profiles.names[i][0])
            continue;

        DialogueBoxText t;
        DialogueBoxText_Init(&t);
        t.FontID = FONT_NATIVE_ALIEN_EXT_16;
        t.color.a = 0;   /* drawn by MenuMgr_DrawProfiles instead */
        t.pos.x = -230;
        t.pos.y = -205 + m->rowsLen * 58;
        SDL_strlcpy(t.str, m->profiles.names[i], sizeof(t.str));
        DialogueBox_AddText(db, &t);

        m->rowSlot[m->rowsLen++] = i;
    }

    const char* labels[3] = { "OK", "NEW", "DELETE" };
    for (int i = 0; i < 3; i++) {
        Button b;
        Button_Init(&b);
        Button_SetText(&b, labels[i]);
        b.pos.x = -290 + i * 200;
        b.pos.y = 170;
        DialogueBox_AddButton(db, &b);
    }

    return db;
}

static DialogueBox* MenuMgr_CreateNewPlayerDialog(bool canCancel) {
    DialogueBox* db = DialogueBox_Create();
    db->sizeW = 6;
    db->sizeH = 3;
    /* high on the screen, so the phone keyboard doesn't cover it */
    db->pos.y = 190;
    db->startY = db->pos.y;
    strcpy(db->caption, "NEW PlAYER");

    DialogueBoxText tAsk;
    DialogueBoxText_Init(&tAsk);
    tAsk.pos.x = -260;
    tAsk.pos.y = -100;
    strcpy(tAsk.str, "ENTER YOUR NAME:");
    DialogueBox_AddText(db, &tAsk);

    DialogueBoxText tName;
    DialogueBoxText_Init(&tName);
    tName.FontID = FONT_NATIVE_ALIEN_EXT_16;
    tName.color.b = 255;   /* white */
    tName.pos.x = -260;
    tName.pos.y = -62;
    strcpy(tName.str, "");   /* the name itself is drawn in MenuMgr_DrawProfiles */
    DialogueBox_AddText(db, &tName);

    DialogueBoxText tErr;
    DialogueBoxText_Init(&tErr);
    tErr.color.g = 80;
    tErr.color.b = 80;
    tErr.pos.x = -260;
    tErr.pos.y = -18;
    strcpy(tErr.str, "");
    DialogueBox_AddText(db, &tErr);

    Button bOk;
    Button_Init(&bOk);
    Button_SetText(&bOk, "OK");
    bOk.pos.x = canCancel ? -200 : -90;
    bOk.pos.y = 30;
    DialogueBox_AddButton(db, &bOk);

    if (canCancel) {
        Button bCancel;
        Button_Init(&bCancel);
        Button_SetText(&bCancel, "CANCEL");
        bCancel.pos.x = 20;
        bCancel.pos.y = 30;
        DialogueBox_AddButton(db, &bCancel);
    }

    return db;
}

static DialogueBox* MenuMgr_CreateDeleteDialog(const char* name) {
    DialogueBox* db = DialogueBox_Create();
    db->sizeW = 6;
    db->sizeH = 3;
    strcpy(db->caption, "DElETE");

    DialogueBoxText tn;
    DialogueBoxText_Init(&tn);
    tn.FontID = FONT_NATIVE_ALIEN_EXT_16;
    tn.color.b = 255;
    tn.pos.x = -260;
    tn.pos.y = -118;
    SDL_strlcpy(tn.str, name, sizeof(tn.str));
    if (Lang_HasArabic(name))
        tn.pos.x = 260 - Lang_StrWidth(name, tn.FontID);
    DialogueBox_AddText(db, &tn);

    DialogueBoxText t;
    DialogueBoxText_Init(&t);
    t.pos.x = -260;
    t.pos.y = -50;
    strcpy(t.str, "DELETE THIS ACCOUNT");
    DialogueBox_AddText(db, &t);

    DialogueBoxText t2;
    DialogueBoxText_Init(&t2);
    t2.pos.x = -260;
    t2.pos.y = -18;
    strcpy(t2.str, "AND ALL ITS PROGRESS?");
    DialogueBox_AddText(db, &t2);

    Button bYes;
    Button_Init(&bYes);
    Button_SetText(&bYes, "YES");
    bYes.pos.x = -200;
    bYes.pos.y = 20;
    DialogueBox_AddButton(db, &bYes);

    Button bNo;
    Button_Init(&bNo);
    Button_SetText(&bNo, "NO");
    bNo.pos.x = 20;
    bNo.pos.y = 20;
    DialogueBox_AddButton(db, &bNo);

    return db;
}

static void MenuMgr_OpenNewPlayer(MenuMgr* m) {
    m->dbNewPlayer = MenuMgr_CreateNewPlayerDialog(Profiles_Count(&m->profiles) > 0);
    Engine_TextInputStart(PROFILE_NAME_MAX);
}

static void MenuMgr_CloseDialog(DialogueBox** db) {
    if (*db) {
        DialogueBox_Destroy(*db);
        *db = NULL;
    }
}

static bool MenuMgr_ClickedIn(Engine *engine, int x1, int y1, int x2, int y2) {
    if (!Engine_WasClicked())
        return false;
    int mx, my;
    Engine_GetMousePos(engine, &mx, &my);
    return mx >= x1 && mx <= x2 && my >= y1 && my <= y2;
}

void MenuMgr_UpdateProfiles(LevelMgr *levelMgr, Engine *engine, MenuMgr* m) {
    if (!m->profilesReady) {
        Profiles_Load(&m->profiles);
        MenuMgr_UsePlayer(levelMgr, m, m->profiles.current);
        m->profilesReady = true;
    }

    if (m->roomID != MR_MAIN)
        return;

    /* First start (or every player deleted): ask for a name */
    if (Profiles_Count(&m->profiles) == 0 && !m->dbNewPlayer && !m->dbOptions) {
        MenuMgr_CloseDialog(&m->dbPlayers);
        MenuMgr_CloseDialog(&m->dbDelete);
        MenuMgr_OpenNewPlayer(m);
    }


    if (m->dbNewPlayer) {
        DialogueBox* db = m->dbNewPlayer;
        DialogueBox_Update(engine, db);

        /* tap on the name line: show the keyboard again */
        int fx = db->pos.x - 280, fy = db->pos.y - 70;
        if (MenuMgr_ClickedIn(engine, fx, fy, fx + 560, fy + 50))
            SDL_StartTextInput();

        int btn = DialogueBox_GetBtn(db);
        bool enter = Engine_TextInputTakeEnter();

        if (btn == 1 || enter) {
            char name[PROFILE_NAME_BYTES];
            SDL_strlcpy(name, Engine_TextInputGet(), sizeof(name));
            int len = (int)strlen(name);
            while (len > 0 && name[len - 1] == ' ')
                name[--len] = 0;

            int slot = Profiles_Add(&m->profiles, name);
            if (slot >= 0) {
                Engine_TextInputStop();
                MenuMgr_CloseDialog(&m->dbNewPlayer);
                MenuMgr_UsePlayer(levelMgr, m, slot);
            } else if (!name[0]) {
                DialogueBox_SetTextStr(db, NEWPLAYER_TEXT_ERROR, "PLEASE TYPE A NAME");
            } else {
                DialogueBox_SetTextStr(db, NEWPLAYER_TEXT_ERROR, "THIS NAME IS ALREADY USED");
            }
        } else if (btn == 2) {   /* CANCEL */
            Engine_TextInputStop();
            MenuMgr_CloseDialog(&m->dbNewPlayer);
            m->dbPlayers = MenuMgr_CreatePlayersDialog(m);
        }
    }
    else if (m->dbDelete) {
        DialogueBox_Update(engine, m->dbDelete);
        int btn = DialogueBox_GetBtn(m->dbDelete);
        if (btn == 1 || btn == 2) {
            MenuMgr_CloseDialog(&m->dbDelete);
            if (btn == 1) {
                int before = m->profiles.current;
                Profiles_Delete(&m->profiles, m->selSlot);
                Profiles_Save(&m->profiles);
                if (m->profiles.current != before)
                    MenuMgr_UsePlayer(levelMgr, m, m->profiles.current);
                m->selSlot = m->profiles.current;
            }
            if (Profiles_Count(&m->profiles) > 0)
                m->dbPlayers = MenuMgr_CreatePlayersDialog(m);
        }
    }
    else if (m->dbPlayers) {
        DialogueBox* db = m->dbPlayers;
        DialogueBox_Update(engine, db);

        /* tap a name to select it */
        for (int r = 0; r < m->rowsLen; r++) {
            int y = db->texts[r].pos.y;
            if (MenuMgr_ClickedIn(engine, db->pos.x - 300, y, db->pos.x + 300, y + 60)) {
                m->selSlot = m->rowSlot[r];
                Engine_PlaySound(engine, SND_BUTTON2);
            }
        }

        switch (DialogueBox_GetBtn(db)) {
            case 1:   /* OK */
                MenuMgr_CloseDialog(&m->dbPlayers);
                if (m->selSlot >= 0 && m->selSlot != m->profiles.current)
                    MenuMgr_UsePlayer(levelMgr, m, m->selSlot);
                break;
            case 2:   /* NEW */
                if (Profiles_Count(&m->profiles) < PROFILES_MAX) {
                    MenuMgr_CloseDialog(&m->dbPlayers);
                    MenuMgr_OpenNewPlayer(m);
                }
                break;
            case 3:   /* DELETE */
                if (m->selSlot >= 0 && m->profiles.names[m->selSlot][0]) {
                    MenuMgr_CloseDialog(&m->dbPlayers);
                    m->dbDelete = MenuMgr_CreateDeleteDialog(m->profiles.names[m->selSlot]);
                }
                break;
        }
    }

    MenuMgr_UpdateInDialog(m);
}

void MenuMgr_DrawWelcome(Engine *engine, MenuMgr* m) {
    (void)m;
    SDL_Color white = {255, 255, 255, 255};
    /* Arabic: Kufi, a bit smaller so it sits nicely in the sky */
    bool ar = Lang_Get() == LANG_AR;
    if (ar) {
        Engine_DrawTextExtScale(engine, "WELCOME TO ZUMA HD!", FONT_NATIVE_ALIEN_EXT_16,
            0.8f, white, false, true, 640, 6);
        return;
    }

    /* English: the font's space is very wide, so put the words
       closer together by hand */
    const char* words[] = { "WELCOME", "TO", "ZUMA", "HD!" };
    const int gap = 14;
    int total = 0;
    for (int i = 0; i < 4; i++)
        total += Engine_GetTextWidth(engine, words[i], FONT_NATIVE_ALIEN_EXT_16) + (i ? gap : 0);
    int x = 640 - total / 2;
    for (int i = 0; i < 4; i++) {
        Engine_DrawTextExt(engine, words[i], FONT_NATIVE_ALIEN_EXT_16, white, false, false, x, 0);
        x += Engine_GetTextWidth(engine, words[i], FONT_NATIVE_ALIEN_EXT_16) + gap;
    }
}

void MenuMgr_DrawProfiles(Engine *engine, MenuMgr* m) {
    if (m->roomID != MR_MAIN)
        return;
    if (m->dbPlayers) {
        DialogueBox* db = m->dbPlayers;
        DialogueBox_Draw(engine, db);

        /* every name is a bar you can tap, with an arrow on the right;
           the chosen one is darker with a gold frame */
        for (int r = 0; r < m->rowsLen && !db->animation; r++) {
            bool sel = m->rowSlot[r] == m->selSlot;
            SDL_Rect bar = { db->pos.x - 270, db->texts[r].pos.y + 10, 540, 48 };

            SDL_SetRenderDrawBlendMode(engine->render, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(engine->render, 0, 0, 0, sel ? 120 : 45);
            SDL_RenderFillRect(engine->render, &bar);
            if (sel) {
                SDL_SetRenderDrawColor(engine->render, 255, 220, 0, 255);
                SDL_RenderDrawRect(engine->render, &bar);
            }

            /* Arabic names sit on the right, English on the left;
               the arrow is on the other side, pointing at the name */
            bool arName = Lang_HasArabic(db->texts[r].str);
            float ay = bar.y + bar.h / 2.0f;
            float ax = arName ? bar.x + 18 : bar.x + bar.w - 18;
            float tip = arName ? ax + 18 : ax - 18;
            SDL_Color ac = sel ? (SDL_Color){255, 230, 60, 255} : (SDL_Color){255, 230, 60, 150};
            SDL_Vertex v[3] = {
                { { ax,  ay - 12 }, ac, { 0, 0 } },
                { { ax,  ay + 12 }, ac, { 0, 0 } },
                { { tip, ay      }, ac, { 0, 0 } },
            };
            SDL_RenderGeometry(engine->render, NULL, v, 3, NULL, 0);

            SDL_Color tc = sel ? (SDL_Color){255, 255, 255, 255} : (SDL_Color){255, 255, 0, 255};
            /* a bit smaller, centred in the bar */
            float tx = db->texts[r].pos.x + 6;
            float ty = db->texts[r].pos.y + 2;
            if (arName) {
                int w = (int)(Engine_GetTextWidth(engine, db->texts[r].str, db->texts[r].FontID) * 0.82f);
                tx = bar.x + bar.w - 24 - w;
                ty -= 13;   /* room for letters going below the line (ya) */
            }
            Engine_DrawTextExtScale(engine, db->texts[r].str, db->texts[r].FontID, 0.82f, tc,
                true, false, tx, ty);
        }
    }

    if (m->dbNewPlayer) {
        DialogueBox* db = m->dbNewPlayer;
        DialogueBox_Draw(engine, db);

        if (!db->animation) {
            /* name box */
            SDL_Rect box = { db->pos.x - 280, db->pos.y - 70, 560, 48 };
            SDL_SetRenderDrawBlendMode(engine->render, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(engine->render, 0, 0, 0, 150);
            SDL_RenderFillRect(engine->render, &box);
            SDL_SetRenderDrawColor(engine->render, 255, 220, 0, 255);
            SDL_RenderDrawRect(engine->render, &box);

            const char* name = Engine_TextInputGet();
            bool blink = (SDL_GetTicks() / 400) % 2;
            SDL_Color white = {255, 255, 255, 255};
            if (Lang_HasArabic(name)) {
                /* Arabic: written from the right, cursor on its left */
                int w = Engine_GetTextWidth(engine, name, FONT_NATIVE_ALIEN_EXT_16);
                float tx = box.x + box.w - 14 - w;
                Engine_DrawTextExt(engine, name, FONT_NATIVE_ALIEN_EXT_16, white, true, false,
                    tx, box.y - 27);   /* room for letters below the line (ya) */
                if (blink)
                    Engine_DrawTextExt(engine, "_", FONT_NATIVE_ALIEN_EXT_16, white, true, false,
                        tx - 26, box.y - 14);
            } else {
                char shown[110];
                SDL_snprintf(shown, sizeof(shown), "%s%s", name, blink ? "_" : "");
                /* this font draws ~20px below its y, so lift it into the box */
                Engine_DrawTextExt(engine, shown, FONT_NATIVE_ALIEN_EXT_16, white, true, false,
                    box.x + 14, box.y - 14);
            }
        }
    }
    if (m->dbDelete)    DialogueBox_Draw(engine, m->dbDelete);
}

void MenuMgr_Update(LevelMgr *levelMgr, Engine *engine, MenuMgr* m) {
    if (!m)
        return;
    
    if (!m->profilesReady || m->roomID == MR_MAIN)
        MenuMgr_UpdateProfiles(levelMgr, engine, m);

    if (!m->inDialog)
        Menu_Update(engine, m->room);

    if (m->roomID == MR_MAIN && !m->inDialog) {
        switch(Menu_GetButton(m->room)) {
            case 1: MenuMgr_Set(engine, m, MR_GAUNTLET); break;
            case 2:   /* this stone (was GAUNTLET, did nothing) = ACCOUNTS */
                m->selSlot = m->profiles.current;
                m->dbPlayers = MenuMgr_CreatePlayersDialog(m);
                m->inDialog = true;
                break;
            case 3: 
                m->dbOptions = DialogueBox_CreateOptions(engine);
                m->inDialog = true;
                break;
            case 4: break;          /* TODO NEVER GONNA GIVE YOU UP */
            case 5: exit(0); break; 
        }
    } else if (m->roomID == MR_GAUNTLET) {
        switch(Menu_GetButton(m->room)) {
            case 1: MenuMgr_Set(engine, m, MR_MAIN); return; break;
            case 2: (*m->curLvl)--; break;
            case 3:
                if (!LevelMgr_IsLevelUnlocked(levelMgr, *m->curLvl))
                    break; /* locked: win the level before it first */
                MenuMgr_Set(engine, m, MR_GAME); Engine_StopMusic(engine); return; break;
            case 4: (*m->curLvl)++; break;

            case 5: (*m->curDiff) = 0; break;
            case 6: (*m->curDiff) = 1; break;
            case 7: (*m->curDiff) = 2; break;
            case 8: (*m->curDiff) = 3; break;
        }

        if (*m->curLvl >= LEVELS_COUNT)
            *m->curLvl = 0;
        else if (*m->curLvl < 0)
            *m->curLvl = LEVELS_COUNT-1;

        for (int i = 4; i < 8; i++) {
            if (i == 4+(*m->curDiff))
                m->room->buttons[i].pressed = true;
            else
                m->room->buttons[i].pressed = false;
        }
    }

    if (m->dbOptions) {
        DialogueBox_UpdateOptions(engine, m->dbOptions);

        if (DialogueBox_GetBtn(m->dbOptions) == 1) {
            Engine_SaveSettings(engine);
            DialogueBox_Destroy(m->dbOptions);
            m->dbOptions = NULL;
            m->inDialog = false;
        }
    }
}


void MenuMgr_Draw(LevelMgr *levelMgr, Engine *engine, MenuMgr* m) {
    if (m->roomID == MR_GAME) 
        return;

    if (m->roomID == MR_GAUNTLET) {
        bool locked = !LevelMgr_IsLevelUnlocked(levelMgr, *m->curLvl);
        SDL_Texture* thumb = Engine_GetTextureSDL(engine, 4+(*m->curLvl));

        if (locked && thumb)
            SDL_SetTextureColorMod(thumb, 70, 70, 70);

        Engine_DrawTexture(engine, 4+(*m->curLvl), WINDOW_WIDTH / 2+16, 500);

        if (locked && thumb) {
            SDL_SetTextureColorMod(thumb, 255, 255, 255);

            SDL_Color color = {255, 210, 0, 255};
            Lang_SetButtonStyle(true);
            Engine_DrawTextExt(engine, "LOCKED", FONT_NATIVE_ALIEN_48, color,
                true, true, WINDOW_WIDTH / 2 + (Lang_Get() == LANG_AR ? -4 : 16),
                Lang_Get() == LANG_AR ? 432 : 476);
            Lang_SetButtonStyle(false);
        }
    }
    
    Menu_Draw(engine, m->room);
    if (m->dbOptions)
        DialogueBox_Draw(engine, m->dbOptions);

    if (m->roomID == MR_MAIN) {
        SDL_Texture* tempTex = Engine_GetTextureSDL(engine, TEX_MENU);

        SDL_Rect srcSunRect = {2563, 315, 130, 138};
        SDL_Rect dstSunRect = {158, 16, 130, 138};

        SDL_RenderCopy(engine->render, 
            tempTex, &srcSunRect, &dstSunRect);

        SDL_Rect srcGlowRect = {2563, 0, 315, 315};
        SDL_Rect dstGlowRect = {68, -68, 315, 315};

        SDL_SetTextureAlphaMod(tempTex, 152);
        SDL_SetTextureColorMod(tempTex, 255, 192, 0);

        SDL_RenderCopy(engine->render, 
            tempTex, &srcGlowRect, &dstGlowRect);

        SDL_SetTextureAlphaMod(tempTex, 255);
        SDL_SetTextureColorMod(tempTex, 255, 255, 255);

        MenuMgr_DrawWelcome(engine, m);


    } else if (m->roomID == MR_GAUNTLET) {
        char buff[32];
        SDL_Color color = {255, 255, 255, 255};

        sprintf(buff, "BEST SCORE:");
        Engine_DrawTextExt(engine, buff, FONT_CANCUN_10, color, true,
            false, 924, 632);

        sprintf(buff, "%d", levelMgr->bestScore[*m->curLvl][*m->curDiff]);
        color.b = 0;
        Engine_DrawTextExt(engine, buff, FONT_CANCUN_10, color, true,
            false, 1140, 632);

        sprintf(buff, "BEST TIME:");
        color.b = 255;
        Engine_DrawTextExt(engine, buff, FONT_CANCUN_10, color, true,
            false, 924, 686);

        int min, sec;
        min = levelMgr->bestTime[*m->curLvl][*m->curDiff] / 60;
        sec = levelMgr->bestTime[*m->curLvl][*m->curDiff] % 60;
        sprintf(buff, "%d:%02d", min, sec);
        color.b = 0;
        Engine_DrawTextExt(engine, buff, FONT_CANCUN_10, color, true,
            false, 1140, 686);
    }

    MenuMgr_DrawProfiles(engine, m);
}
