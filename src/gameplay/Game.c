#include "Game.h"
#include "../global/Lang.h"
#include "Level.h"

void constrain(int* a, int min, int max) {
    if (*a < min)
        *a = max;
    else if (*a > max)
        *a = min;
}

LevelMgr levelMgr;

void Game_Init(LevelMgr *levelMgr, Engine *engine, Game* game, int lvlID, int difficulty) {
    game->stageID = 0;
    game->score = 0;
    game->lives = 0;
    game->isLosed = false;
    game->isWon = false;
    game->goNextLevel = false;
    /* first power-up after 12-20 s of play */
    game->powerTimer = randInt(6 * 60, 10 * 60);   /* first after 6-10 s */
    game->slowTime = game->reverseTime = game->accuracyTime = 0;
    Bullet_speedMul = 1.0f;
    game->retryLevel = false;
    game->isOutroEnded = false;
    game->isIntroEnded = false;
    game->totalCoins = 0;
    game->headerFontScale = 0;
    game->subHeaderFontScale = 0;
    game->difficulty = difficulty;

    Stage* stg = LevelMgr_GetStage(levelMgr, game->stageID);

    game->lvlID = lvlID; 
    constrain(&game->lvlID, 0, stg->levelsLen-1);

    game->lvl = LevelMgr_GetLevelFromStage(levelMgr, game->stageID, game->lvlID);
    Level_Load(levelMgr, engine, game->lvl);

    game->graphics = LevelMgr_GetLevelGraphics(levelMgr, game->lvl);
    switch(difficulty) {
        case 0:
            game->settings = &levelMgr->settings[4];
            game->settings->ballColors = 4;
            game->settings->partTime = 70;
            game->settings->ballStartCount = 40;
            break;
        case 1:
            game->settings = &levelMgr->settings[8];
            game->settings->ballColors = 5;
            game->settings->partTime = 100;
            game->settings->ballStartCount = 50;
            break;
        case 2:
            game->settings = &levelMgr->settings[13];
            game->settings->ballColors = 6;
            game->settings->repeatChance = 25;
            game->settings->partTime = 120;
            game->settings->ballStartCount = 60;
            break;
        case 3:
            game->settings = &levelMgr->settings[24];
            game->settings->ballColors = 6;
            game->settings->partTime = 150;
            game->settings->ballStartCount = 60;
            break;
    }

    if (levelMgr->bestTime[game->lvlID][game->difficulty] != 0)
        game->settings->partTime = levelMgr->bestTime[game->lvlID][game->difficulty];
    
    Frog_Init(&game->frog, (game->graphics->frogPos.x + 106) * engine->scale_x, 
        game->graphics->frogPos.y * engine->scale_y);

    BallChain_Init(&game->chain, game->lvl, game->settings);

    for(int i = 0; i < BULLETS_ARR_MAX_LEN; i++)
        game->bullets.bullets[i].onScreen = 0;
    game->bullets.len = 0;

    for(int i = 0; i < MAX_MSGS_ARRAY_LEN; i++)
        game->msgs.msgs[i].onScreen = 0;
    game->msgs.len = 0;

    for(int i = 0; i < MAX_MSGS_ARRAY_LEN; i++)
        game->particles.prtcls[i].onScreen = 0;
    game->particles.len = 0;

    game->DEBUG = false;
    game->isRunning = true;
    game->isFirstTime = true;
    game->chain.isGenerating = true;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    Treasure treasure;

    treasure.isActive   = false;
    treasure.isBlinking = false;
    treasure.isFading   = false;
    treasure.time       = clock();

    SDL_Rect animTreasureRect = { 340, 0, 54, 54 };
    Animation_Init(&treasure.anim, TEX_GAME_OBJECTS, animTreasureRect);
    Animation_Set(&treasure.anim, 0, 30, 0.25);

    game->treasure = treasure;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    game->lightTrailPitch = 0;


    SDL_Rect animFinishRect = {629, 132, 132, 132};
    Animation_Init(&game->finishAnim, TEX_GAME_OBJECTS, animFinishRect);
    Animation_Set(&game->finishAnim, 0, 12, 0);

    game->finishPos.x = (game->lvl->spiralStart.x + 104)*engine->scale_x;
    game->finishPos.y = game->lvl->spiralStart.y*engine->scale_y;
    for (int i = 0; i < game->lvl->spiralLen; i++) {
        game->finishPos.x += game->lvl->spiral[i].dx * engine->scale_x;
        game->finishPos.y += game->lvl->spiral[i].dy * engine->scale_y;
    }
    game->finishAngle = 0;

    /* Level 1 (spiral): the skull sits in the middle of the round hole
       of the picture, its mouth turned toward the end of the groove */
    if (strcmp(game->graphics->id, "spiral") == 0) {
        float endX = game->finishPos.x, endY = game->finishPos.y;
        /* centre of the round hole, measured on the level picture */
        game->finishPos.x = 481;
        game->finishPos.y = 325;
        (void)endX; (void)endY;
        /* direction the balls come in (last part of the path); the mouth
           (bottom of the skull picture) faces them */
        float vx = 0, vy = 0;
        for (int i = game->lvl->spiralLen - 40; i < game->lvl->spiralLen; i++) {
            if (i < 0) continue;
            vx += game->lvl->spiral[i].dx;
            vy += game->lvl->spiral[i].dy;
        }
        game->finishAngle = (float)(atan2(-vy, -vx) * 180.0 / M_PI) - 90.0f;
    }

    game->scoreRect.x = 773;
    game->scoreRect.y = 28;
    game->scoreRect.w = 0;
    game->scoreRect.h = 28;

    Engine_PlaySound(engine, SND_CHANT1);
    Engine_PlayMusic(engine, MUS_GAME);

    Button_Init(&game->btnMenu);
    game->btnMenu.textureID = TEX_GAME_HUD;
    game->btnMenu.tRect.x = 925;
    game->btnMenu.tRect.y = 15;
    game->btnMenu.tRect.w = 118;
    game->btnMenu.tRect.h = 37;

    game->btnMenu.pos.x = 1128;
    game->btnMenu.pos.y = 4;

    // Menus
    game->dbMenu = NULL;
    game->dbStats = NULL;
    game->dbGameOver = NULL;
}

static bool _Game_FrogControl(Engine *engine, Game* game, int mouseClicked) {
    bool isShooted = 0;

    /* Level finished (end explosions / game over): touches do nothing.
       Shooting here picked a new ball color from an empty chain and the
       game froze forever. */
    if (game->isWon || game->isLosed)
        return false;

    switch (mouseClicked) {
        case 1:
            if (!isShooted && !game->chain.isEndReached && !game->isFirstTime)
                isShooted = true;
            break;
        case 2: {
            char t = game->frog.color;
            game->frog.color = game->frog.nextColor;
            game->frog.nextColor = t;
            Engine_PlaySound(engine, SND_BUTTON2);
            break;
        }
    }

    return isShooted;
}

static bool _Game_isEnd(LevelMgr *levelMgr, Engine *engine, Game* game, int* inMenu) {
    if (!game->isLosed && !game->isWon)
        return false;

    if (game->isWon && !game->isOutroEnded) {
        Game_UpdateOutro(engine, game);
        return false;
    }

    if (game->dbStats) {
        DialogueBox_Update(engine, game->dbStats);
        int resBtn = DialogueBox_GetBtn(game->dbStats);
        if (resBtn == 1 || resBtn == 2) {
            game->goNextLevel = (resBtn == 2);
            DialogueBox_Destroy(game->dbStats);
            game->dbStats = NULL;

            if (game->score > levelMgr->bestScore[game->lvlID][game->difficulty])
                levelMgr->bestScore[game->lvlID][game->difficulty] = game->score;

            if (game->time < levelMgr->bestTime[game->lvlID][game->difficulty] ||
                levelMgr->bestTime[game->lvlID][game->difficulty] == 0)
                    levelMgr->bestTime[game->lvlID][game->difficulty] = game->time;

            LevelMgr_SaveProgress(levelMgr);

            *inMenu = 1;
            Level_Free(game->lvl);
        }
    }

    if (game->dbGameOver) {
        DialogueBox_Update(engine, game->dbGameOver);
        int goBtn = DialogueBox_GetBtn(game->dbGameOver);
        if (goBtn == 1 || goBtn == 2) {
            game->retryLevel = (goBtn == 1);
            DialogueBox_Destroy(game->dbGameOver);
            game->dbGameOver = NULL;
            *inMenu = 1;
            Level_Free(game->lvl);
        }
    }

    return true;
}

static bool _Game_HandleMenu(Engine *engine, Game* game, int* inMenu) {
    if (!game->dbMenu)
        return false;
    
    DialogueBox_UpdateMenu(engine, game->dbMenu);

    switch (DialogueBox_GetBtn(game->dbMenu)) {
        case 1:
            DialogueBox_Destroy(game->dbMenu);
            game->dbMenu = NULL;
            break;
        case 2:
            DialogueBox_Destroy(game->dbMenu);
            game->dbMenu = NULL;
            *inMenu = 1;
            Level_Free(game->lvl);
            break;
    }

    return true;
}

/* ---- Power-ups, like the original Zuma: now and then one ball of the
   chain carries a power-up for a while. Exploding it with balls of
   its colour starts the power. ---- */
static void _Game_UpdatePowerUps(Engine *engine, Game* game) {
    BallChain* ch = &game->chain;

    /* a power-up was exploded */
    switch (ch->lastBonus) {
        case BONUS_ACCURACY:
            game->accuracyTime = 10 * 60;   /* 10 s */
            Engine_PlaySound(engine, SND_ACCURACY3);
            break;
        case BONUS_REVERSE:
            game->reverseTime = 150;   /* 2.5 s */
            game->slowTime = 0;
            Engine_PlaySound(engine, SND_REVERSE1);
            break;
        case BONUS_SLOW:
            game->slowTime = 7 * 60;       /* 7 s */
            Engine_PlaySound(engine, SND_SLOWDOWN1);
            break;
        case BONUS_BOMB:
            game->frog.hasBomb = true;   /* next shot is a bomb */
            Engine_PlaySound(engine, SND_JEWELAPPEAR);
            break;
        default:
            break;
    }
    ch->lastBonus = BONUS_NONE;

    /* the fuse sizzles while the bomb is in the mouth */
    static int fuseTimer = 0;
    if (game->frog.hasBomb) {
        if (--fuseTimer <= 0) {
            Engine_PlaySound(engine, SND_FUSE);
            fuseTimer = 118;  /* the sound is 2 s: replay just before it ends */
        }
    } else {
        if (fuseTimer > 0)
            Engine_StopSound(engine, SND_FUSE);
        fuseTimer = 0;
    }

    if (ch->bombScore) {
        game->score += ch->bombScore;
        ch->bombScore = 0;
    }

    if (game->reverseTime > 0) game->reverseTime--;
    if (game->slowTime > 0) game->slowTime--;
    if (game->accuracyTime > 0) game->accuracyTime--;
    Bullet_speedMul = game->accuracyTime > 0 ? 1.7f : 1.0f;

    /* power-ups on balls fade away after a while (blinking at the end) */
    bool anyBonus = false;
    for (int i = 0; i < ch->len; i++) {
        Ball* b = &ch->balls[i];
        if (!b->bonus)
            continue;
        anyBonus = true;
        if (--b->bonusTime <= 0) {
            b->bonus = BONUS_NONE;
            Ball_InitAnim(b);
        } else if (b->bonusTime < BONUS_BLINK_FRAMES) {
            b->anim.color.a = ((b->bonusTime / 8) % 2) ? 255 : 110;
        }
    }

    /* a new one now and then, never at the start of a level */
    if (game->isFirstTime || game->isWon || game->isLosed || ch->isEndReached)
        return;
    if (anyBonus || --game->powerTimer > 0)
        return;

    game->powerTimer = randInt(8 * 60, 14 * 60);   /* then every 8-14 s */
    if (ch->len < 8)
        return;

    for (int tries = 0; tries < 20; tries++) {
        int i = randInt(0, ch->len - 1);
        Ball* b = &ch->balls[i];
        if (b->isExploding || b->inTunnel || b->startAnim || b->pos < 60)
            continue;
        b->bonus = randInt(BONUS_ACCURACY, BONUS_SLOW);
        b->bonusTime = BONUS_LIFE_FRAMES;
        Ball_InitAnim(b);
        Engine_PlaySound(engine, SND_JEWELAPPEAR);
        break;
    }
}

static void _Game_GenerateChain(Engine *engine, Game* game) {
    game->chain.speed = game->settings->ballSpd;

    /* power-ups change the speed for a while */
    if (game->reverseTime > 0) {
        game->chain.speed = -1.0f;   /* gentle roll back */
    } else if (game->slowTime > 0) {
        game->chain.speed = game->settings->ballSpd * 0.35f;
    }

    if (!game->chain.isGenerating)
        return;

    if (game->isFirstTime && game->chain.len < game->settings->ballStartCount) {
        if (game->chain.balls[game->chain.len-1].pos >= 0.5) {
            BallChain_Append(&game->chain, game->lvl, game->settings);
            game->chain.balls[game->chain.len-1].spd = game->chain.balls[game->chain.len-2].spd;
        }
    } else {
        if (game->chain.balls[game->chain.len-1].pos >= BALLS_CHAIN_PAD) {
            BallChain_Append(&game->chain, game->lvl, game->settings);
            game->chain.balls[game->chain.len-1].spd = game->chain.balls[game->chain.len-2].spd;
            game->chain.balls[game->chain.len-1].startAnim = true;
            Ball_InitAnim(&game->chain.balls[game->chain.len-1]);
        } else if (game->chain.len == 0) {
            BallChain_Append(&game->chain, game->lvl, game->settings);
        }

        if (game->isFirstTime) {
            Engine_StopSound(engine, SND_ROLLING);
        }
        game->isFirstTime = false;
    }

}

static void _Game_HandleGameWon(Engine *engine, Game* game) {
    if (game->chain.isEndReached || 
        game->chain.len != 0 || 
        game->score <= game->settings->gaugeScore ||
        game->isWon
    ) {
        return;
    }

    Engine_PlayMusic(engine, MUS_WIN);

    game->dbStats = DialogueBox_CreateResults(game->lvlID + 1 < LEVELS_COUNT);

    char buff[8];
    sprintf(buff, "%d", game->score);
    DialogueBox_SetTextStr(game->dbStats, 
        DB_RES_TEXT_POINTS, buff);
    
    sprintf(buff, "%d", game->totalCoins);
    DialogueBox_SetTextStr(game->dbStats, 
        DB_RES_TEXT_COINS, buff);

    sprintf(buff, "%d", game->chain.totalCombos);
    DialogueBox_SetTextStr(game->dbStats, 
        DB_RES_TEXT_COMBOS, buff);

    sprintf(buff, "%d", game->chain.maxChainBonus);
    DialogueBox_SetTextStr(game->dbStats, 
        DB_RES_TEXT_MAX_CHAIN, buff);

    sprintf(buff, "%d", game->chain.maxCombo);
    DialogueBox_SetTextStr(game->dbStats, 
        DB_RES_TEXT_MAX_COMBO, buff);

    game->time = ((float)clock())/CLOCKS_PER_SEC - game->time;
    sprintf(buff, "%d:%02d", 
        game->time / 60, 
        game->time % 60);
    DialogueBox_SetTextStr(game->dbStats, 
        DB_RES_TEXT_YOUR_TIME, buff);

    if (game->time <= game->settings->partTime) {
        game->dbStats->texts[DB_RES_TEXT_YOUR_TIME].color.r = 0;
        game->dbStats->texts[DB_RES_TEXT_YOUR_TIME].color.g = 255;
        game->dbStats->texts[DB_RES_TEXT_YOUR_TIME].color.b = 0;
    }


    sprintf(buff, "%d:%02d", 
        game->settings->partTime / 60, 
        game->settings->partTime % 60);
    DialogueBox_SetTextStr(game->dbStats, 
        DB_RES_TEXT_ACE_TIME, buff);

    game->isWon = true;

}

static bool _Game_IsGameLose(LevelMgr *levelMgr, Engine *engine, Game* game) {
    if (!(game->chain.len == 0 && game->chain.isEndReached))
        return false;


    if (game->lives >= 1) {
        int lives = game->lives - 1;
        Engine_PlaySound(engine, SND_CHANT14);
        Game_Init(levelMgr, engine, game, game->stageID, game->lvlID);
        game->lives = lives;
        return true;
    } 

    if (game->isLosed) 
        return true;

    Engine_PlaySound(engine, SND_CHANT8);
    Engine_PlayMusic(engine, MUS_GAME_OVER);

    game->dbGameOver = DialogueBox_CreateGameOver();

    char buff[8];
    
    sprintf(buff, "%d", game->totalCoins);
    DialogueBox_SetTextStr(game->dbGameOver, 
        DB_GO_TEXT_COINS, buff);

    sprintf(buff, "%d", game->chain.totalCombos);
    DialogueBox_SetTextStr(game->dbGameOver, 
        DB_GO_TEXT_COMBOS, buff);

    sprintf(buff, "%d", game->chain.maxChainBonus);
    DialogueBox_SetTextStr(game->dbGameOver, 
        DB_GO_TEXT_MAX_CHAIN, buff);

    sprintf(buff, "%d", game->chain.maxCombo);
    DialogueBox_SetTextStr(game->dbGameOver, 
        DB_GO_TEXT_MAX_COMBO, buff);

    game->time = ((float)clock())/CLOCKS_PER_SEC - game->time;
    sprintf(buff, "%d:%02d", game->time / 60, game->time % 60);
    DialogueBox_SetTextStr(game->dbGameOver, 
        DB_GO_TEXT_TOTAL_TIME, buff);

    game->isLosed = true;
    
    return false;

}

void Game_Update(LevelMgr *levelMgr, Engine *engine, Game* game, int* inMenu, int mouseClicked) {
    // Get mouse pos and rotate frog;
    if (!Engine_IsAimFrozen())   /* finger on the frog: don't turn */
        Engine_GetMousePos(engine, &game->mx, &game->my);
    Frog_Rotate(&game->frog, game->mx, game->my);

    if (!game->isIntroEnded) {
        Game_UpdateIntro(engine, game);
        return;
    }

    bool isShooted = _Game_FrogControl(engine, game, mouseClicked);

    if (_Game_isEnd(levelMgr, engine, game, inMenu)) return;
    if (_Game_HandleMenu(engine, game, inMenu)) return;

    if (game->reverseTime > 0 && !game->chain.isEndReached) {
        for (int i = 0; i < game->chain.len; i++)
            if (!game->chain.balls[i].isExploding && !game->chain.balls[i].isInserted)
                game->chain.balls[i].spd = -1.2f;
    }

    BallChain_Update(engine, &game->chain, game->lvl->spiral, 
        game->lvl->spiralLen, &game->score, &game->msgs);

    _Game_UpdatePowerUps(engine, game);
    
    if (game->chain.len > 0) {
        BulletsArr_UpdateOnScreenStatus(engine, &game->bullets, &game->chain.chainBonus);
        BulletsArr_Update(&game->bullets);
    }

    if (game->chain.isGenerating && game->score > game->settings->gaugeScore) {
        game->chain.isGenerating = 0;
        game->chain.isGlowing = 1;
        game->chain.glowTime = clock();
        Engine_PlaySound(engine, SND_CHANT4);
    }

    _Game_GenerateChain(engine, game);
    _Game_HandleGameWon(engine, game);
    if (_Game_IsGameLose(levelMgr, engine, game)) return;

    // UI Update
    Button_Update(engine, &game->btnMenu);
    if (Button_IsClicked(&game->btnMenu)) {
        game->dbMenu = DialogueBox_CreateMenu(engine);
    }

    if (isShooted && !(int)game->frog.shift && game->btnMenu.state != BTN_CLICKED &&
        !game->isWon && !game->isLosed) {
        BulletsArr_AddBullet(&game->bullets, &game->frog);
        Engine_PlaySound(engine, SND_FIREBALL1);

        game->frog.isShooted = 1;
        game->frog.color = game->frog.nextColor;
        game->frog.nextColor = randInt(0, game->chain.ballColors-1);
        while(!game->chain.colorInChain[game->frog.nextColor])
            game->frog.nextColor = randInt(0, game->chain.ballColors-1);
    }

    // Changing frog color if color not in chain
    if (!(game->isLosed || game->isWon || game->chain.isEndReached) 
        && (!game->chain.colorInChain[game->frog.nextColor] || !game->chain.colorInChain[game->frog.color])) {
        while(!game->chain.colorInChain[game->frog.nextColor])
            game->frog.nextColor = randInt(0, game->chain.ballColors-1);

        while(!game->chain.colorInChain[game->frog.color])
            game->frog.color = randInt(0, game->chain.ballColors-1);
    }


    if (game->chain.balls[BallChain_GetLastMovingBall(&game->chain)].pos > game->lvl->spiralLen * SLOWFACTOR_POS) {
        if (game->chain.isEndReached)
            game->chain.speed = ROLLING_TO_PIT_SPEED;
        else
            game->chain.speed = game->settings->ballSpd / game->settings->slowFactor;
    }
    else
        game->chain.speed = game->settings->ballSpd;

    /* power-ups win over the normal speed (this was set above and
       overwrote them before) */
    if (!game->chain.isEndReached) {
        if (game->reverseTime > 0)
            game->chain.speed = -1.2f;
        else if (game->slowTime > 0)
            game->chain.speed = game->chain.speed * 0.3f;
    }
    
    BulletsArr_CollideWithChainUpdate(engine, &game->bullets, &game->chain, game->lvl->spiral);
    Messages_Update(&game->msgs);
    Game_UpdateTreasure(engine, game);
}

void Game_Draw(LevelMgr *levelMgr, Engine *engine, Game* game) {
    Level_Draw(engine, game->lvl);
    Game_DrawFinish(engine, game);
    /* accuracy power-up: a long thin triangle of light from the ball in
       the frog's mouth, stopping at the first ball it reaches */
    if (game->accuracyTime > 0 && game->isIntroEnded && !game->isWon && !game->isLosed) {
        static const Uint8 pal[6][3] = { {0,254,253}, {0,250,68}, {253,253,6},
                                         {249,111,76}, {253,136,248}, {253,234,187} };
        const Uint8* c = pal[(int)game->frog.color % 6];
        float dx = (float)game->frog.forwardX, dy = (float)game->frog.forwardY;
        float sx = game->frog.x + dx * FROG_CUR_BALL_PAD;
        float sy = game->frog.y + dy * FROG_CUR_BALL_PAD;

        /* walk along the beam until a ball or the screen edge */
        float len = 0;
        for (len = 0; len < 1600; len += 4) {
            float px = sx + dx * len, py = sy + dy * len;
            if (px < 0 || px > WINDOW_WIDTH || py < 0 || py > WINDOW_HEIGHT)
                break;
            bool hit = false;
            for (int i = 0; i < game->chain.len && !hit; i++) {
                Ball* bl = &game->chain.balls[i];
                if (bl->inTunnel || bl->isExploding)
                    continue;
                float ex = bl->x - px, ey = bl->y - py;
                hit = ex * ex + ey * ey < 24 * 24;   /* touches the ball's edge */
            }
            if (hit)
                break;
        }

        /* the tip moves smoothly, so it doesn't jump back and forth
           when the beam just grazes a moving ball */
        static float s_beamLen = -1;
        if (s_beamLen < 0 || game->accuracyTime >= 10 * 60 - 1)
            s_beamLen = len;
        s_beamLen += (len - s_beamLen) * 0.35f;
        len = s_beamLen;

        /* steady light; fades out gently in the last second */
        float fade = game->accuracyTime < 60 ? game->accuracyTime / 60.0f : 1.0f;
        Uint8 alpha = (Uint8)(150 * fade);
        SDL_Color base = { c[0], c[1], c[2], alpha };
        SDL_Color tip  = { c[0], c[1], c[2], (Uint8)(alpha * 0.6f) };   /* visible up to the ball */
        float half = 14;   /* half width at the frog */
        SDL_Vertex v[3] = {
            { { sx - dy * half, sy + dx * half }, base, { 0, 0 } },
            { { sx + dy * half, sy - dx * half }, base, { 0, 0 } },
            { { sx + dx * len,  sy + dy * len  }, tip,  { 0, 0 } },
        };
        SDL_SetRenderDrawBlendMode(engine->render, SDL_BLENDMODE_ADD);
        SDL_RenderGeometry(engine->render, NULL, v, 3, NULL, 0);
        SDL_SetRenderDrawBlendMode(engine->render, SDL_BLENDMODE_BLEND);
    }

    Frog_Draw(engine, &game->frog, game->isIntroEnded);

    if (!game->isIntroEnded) {
        Game_DrawHUD(engine, game);
        Game_DrawIntro(engine, game);
        return;
    }

    if (game->isWon && !game->isOutroEnded) {
        Game_DrawHUD(engine, game);
        Game_DrawOutro(engine, game);
        return;
    }

    if (game->dbMenu || game->dbGameOver || game->dbStats) {
        Game_DrawHUD(engine, game);
        return;
    } 
    
    if (!game->graphics->textureTopLayerFile) {
        BallChain_Draw(engine, &game->chain, false);
    } else {
        BallChain_Draw(engine, &game->chain, false);
        Level_DrawTopLayer(engine, game->lvl);
        BallChain_Draw(engine, &game->chain, true);
    }



    BulletsArr_Draw(engine, &game->bullets);
    Messages_Draw(engine, &game->msgs);
    Game_DrawHUD(engine, game);

    if (game->treasure.isActive) {
        if (game->treasure.isBlinking) {
            int delta = (int)(((float)(clock() - game->treasure.time))/(CLOCKS_PER_SEC / TREASURE_BLINK_SPEED));
            if (delta % 2 == 0)
                Game_DrawTreasure(engine, game);
        } else
            Game_DrawTreasure(engine, game);
    }

    if (game->treasure.isFading) {
        game->treasure.anim.scale += TREASURE_FADING_SPEED;
        Game_DrawTreasure(engine, game);
    }

    if (game->DEBUG) {
        char buff[48];

        sprintf(buff, "isGlowing: %d", game->chain.isGlowing);
        Engine_DrawText(engine, buff, FONT_ARIAL_12, 600, 16);

        int w, h;
        SDL_GetWindowSize(engine->win, &w, &h);
        sprintf(buff, "Window_Size: (%d, %d)", w, h);
        Engine_DrawText(engine, buff, FONT_ARIAL_12, 16, 16);

        sprintf(buff, "Mouse_POS: (%d, %d)", game->mx, game->my);
        Engine_DrawText(engine, buff, FONT_ARIAL_12, 16, 32);
        sprintf(buff, "MAX_COMBO: %d", game->chain.maxCombo);
        Engine_DrawText(engine, buff, FONT_ARIAL_12, 16, 128);

        sprintf(buff, "BULLETS: ");
        Engine_DrawText(engine, buff, FONT_ARIAL_12, 240, 16);
        for (int i = 0; i < game->bullets.len; i++) {
            sprintf(buff, "    %d. (%lf, %lf, %d)", i + 1, 
                game->bullets.bullets[i].x, game->bullets.bullets[i].y, 
                game->bullets.bullets[i].onScreen);
            Engine_DrawText(engine, buff, FONT_ARIAL_12, 240, 16*(i+2));
        }

        Level_DrawDebug(levelMgr, engine, game->lvl);
    }
}

void Game_UpdateTreasure(Engine *engine, Game* game) {
    clock_t t = clock();
    float delta = ((float)(t - game->treasure.time)) / CLOCKS_PER_SEC;

    if (game->treasure.isActive && game->treasure.isBlinking) {
        if (delta > TREASURE_BLINK_TIME) {
            game->treasure.isActive = false;
            game->treasure.isBlinking = false;
            game->treasure.time = t;
            Engine_PlaySound(engine, SND_GEMVANISHES);
        }
    } else if (game->treasure.isActive && !game->treasure.isBlinking) {
        if (delta > TREASURE_LIFE_TIME) {
            game->treasure.isBlinking = true;
            game->treasure.time = t;
        }
    } else {
        if (delta > TREASURE_LIFE_TIME) {
            Engine_PlaySound(engine, SND_JEWELAPPEAR);
            game->treasure.isActive = true;
            game->treasure.pos = randInt(0, game->graphics->coinsLen - 1);
            game->treasure.time = t;
        }
        if (game->treasure.isFading) {
            if (game->treasure.anim.scale > TREASURE_MAX_SCALE)  {
                game->treasure.isFading = false;
                game->treasure.anim.color.a = 255;
                game->treasure.anim.scale = 1;
                game->treasure.anim.spd = 0.25;
            }
        }
    }
 
    if (game->treasure.isActive) {
        //Check bullets collide with treasure
        for (int i = 0; i < game->bullets.len; i++) {
            if (!game->bullets.bullets[i].onScreen) continue;
            float coinX = (game->graphics->coinsPos[game->treasure.pos].x + 104) * engine->scale_x;
            float coinY = game->graphics->coinsPos[game->treasure.pos].y * engine->scale_y;

            float dist = (game->bullets.bullets[i].x - coinX) * (game->bullets.bullets[i].x - coinX);
            dist += (game->bullets.bullets[i].y - coinY) * (game->bullets.bullets[i].y - coinY);

            if (dist < TREASURE_COLLIDE_DIST) {
                game->chain.chainBonus = 0;
                game->treasure.isActive = false;
                game->treasure.isBlinking = false;
                game->treasure.isFading = true;
                game->treasure.anim.spd = 1;
                game->treasure.time = t;
                game->bullets.bullets[i].onScreen = 0;
                game->score += 500;
                game->totalCoins++;
                char txt[MAX_MSG_LEN];
                sprintf(txt, "BONUS +500");
                Messages_NewMsg(&game->msgs, txt, coinX, coinY, FONT_CANCUN_FLOAT_14, 2);
                Engine_PlaySound(engine, SND_COINGRAB);
            }
        }
    }
}

void Game_DrawTreasure(Engine *engine, Game* game) {
    float x = (game->graphics->coinsPos[game->treasure.pos].x + 104) * engine->scale_x;
    float y = game->graphics->coinsPos[game->treasure.pos].y * engine->scale_y;

    if (game->treasure.isFading) {
        float rescale = (1 - game->treasure.anim.scale / TREASURE_MAX_SCALE);
        if (rescale > 0)
            game->treasure.anim.color.a = 255.0 * rescale;
        else
            game->treasure.anim.color.a = 0;
    }

    SDL_Color tc = game->treasure.anim.color;
    SDL_Color c = {0, 0, 0, game->treasure.anim.color.a / 2};
    game->treasure.anim.color = c;
    Animation_Draw(engine, &game->treasure.anim, x-12, y+16);

    game->treasure.anim.color = tc;
    Animation_Draw(engine, &game->treasure.anim, x, y);
}

void Game_DrawFinish(Engine *engine, Game* game) {
    Animation anim;
    SDL_Rect animFinishRect = {629, 0, 132, 132};
    Animation_Init(&anim, TEX_GAME_OBJECTS, animFinishRect);
    Animation_Set(&anim, 0, 1, 0);
    Animation_SetAngle(&anim, game->finishAngle);
    Animation_Draw(engine, &anim, game->finishPos.x, game->finishPos.y);

    float dist_to_finish = game->lvl->spiralLen - game->chain.balls[0].pos;
    int frame = 12 * ((game->lvl->spiralLen * PIT_OPEN_POS - dist_to_finish) / (game->lvl->spiralLen * PIT_OPEN_POS));
    if (dist_to_finish < game->lvl->spiralLen * PIT_OPEN_POS)
        Animation_SetFrame(&game->finishAnim, frame);

    Animation_SetAngle(&game->finishAnim, game->finishAngle);
    Animation_Draw(engine, &game->finishAnim, game->finishPos.x, game->finishPos.y);
}

void Game_DrawHUD(Engine *engine, Game* game) {
    Animation anim;
    SDL_Rect animBorderRect = {0, 51, 1280, 730};
    Animation_Init(&anim, TEX_GAME_HUD, animBorderRect);
    Animation_Draw(engine, &anim, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 1);

    SDL_Color yellow = {255, 255, 0, 255};

    char buff[64];
    sprintf(buff, "%9d", game->score);
    Engine_DrawTextExt(engine, buff, FONT_CANCUN_12, yellow, 0, 0, 545, 6);

    if (Lang_Get() == LANG_AR)
        sprintf(buff, "%d-%d", game->stageID + 1, game->lvlID + 1);   /* small box */
    else
        sprintf(buff, "lvl%d-%d", game->stageID + 1, game->lvlID + 1);
    Engine_DrawTextExt(engine, buff, FONT_CANCUN_12, yellow, 0, 1, 460, 6);

    SDL_Rect animFrogRect = {40, 18, 40, 36};
    Animation_Init(&anim, TEX_GAME_HUD, animFrogRect);
    if (game->lives <= 3) {
        for (int i = 0; i < game->lives; i++) {
            anim.scale = 0.9;
            Animation_Draw(engine, &anim, 57 + (40 * i), 22);
        }
    } else {
        Animation_Draw(engine, &anim, 57, 22);
        sprintf(buff, "x%d", game->lives);
        Engine_DrawText(engine, buff, FONT_CANCUN_FLOAT_14, 77, 12);
    }

    if (game->score < game->settings->gaugeScore) {
        int targetW = 96 * game->score / game->settings->gaugeScore;
        if (game->scoreRect.w < targetW)
            game->scoreRect.w++;
    } else {
        game->scoreRect.y = 0;
        game->scoreRect.w = 96;
    }

    Animation_Init(&anim, TEX_GAME_HUD, game->scoreRect);
    Animation_Draw(engine, &anim, 820 - (96 - game->scoreRect.w) / 2, 16);

    Button_Draw(engine, &game->btnMenu);
    if (game->dbStats && game->isOutroEnded) DialogueBox_Draw(engine, game->dbStats);
    if (game->dbMenu) DialogueBox_Draw(engine, game->dbMenu);
    if (game->dbGameOver) DialogueBox_Draw(engine, game->dbGameOver);
}

void Game_UpdateOutro(Engine *engine, Game* game) {
    if (game->particles.len == 0) {
        SDL_Rect animExplRect = {528, 0, 100, 130};
        Animation_Init(&game->particles.anim, TEX_GAME_OBJECTS, animExplRect);
        game->particles.anim.color.b = 255;
        game->particles.anim.scale = 1.5;

        game->particles.x = game->chain.balls[0].x;
        game->particles.y = game->chain.balls[0].y;
        game->particles.pos = game->chain.balls[0].pos;
        game->particles.prevPos = game->chain.balls[0].pos;

        game->score += 100;
        Engine_PlaySound(engine, SND_ENDOFLEVELPOP1);
        Messages_NewMsg(&game->msgs, "+100", game->particles.x, game->particles.y, FONT_CANCUN_FLOAT_14, 2);
        Particles_NewParticle(&game->particles, game->particles.x, game->particles.y);
        return;
    }

    Messages_Update(&game->msgs);
    Particles_Update(&game->particles, EXPLOSION_LAST_FRAME, EXPLOSION_ANIM_SPEED);

    if (game->particles.pos > game->lvl->spiralLen) {
        for (int i = 0; i < game->particles.len; i++) {
            if (game->particles.prtcls[i].onScreen) return;
        }

        if (!game->isOutroEnded)
            Engine_PlaySound(engine, SND_CHANT2);
        game->isOutroEnded = true;
        game->particles.len = 0;

        char buff[8];
        sprintf(buff, "%d", game->score);
        DialogueBox_SetTextStr(game->dbStats, 
            DB_RES_TEXT_POINTS, buff);
        return;
    }

    if (game->particles.pos - game->particles.prevPos > EXPLOSIONS_PAD) {
        game->score += 100;
        Engine_PlaySound(engine, SND_ENDOFLEVELPOP1);
        Messages_NewMsg(&game->msgs, "+100", game->particles.x, game->particles.y, FONT_CANCUN_FLOAT_14, 2);
        Particles_NewParticle(&game->particles, game->particles.x, game->particles.y);
        game->particles.prevPos = game->particles.pos;
    }

    game->particles.x   += game->lvl->spiral[(int)game->particles.pos].dx * engine->scale_x * EXPLOSION_MOVE_SPEED;
    game->particles.y   += game->lvl->spiral[(int)game->particles.pos].dy * engine->scale_y * EXPLOSION_MOVE_SPEED;
    game->particles.pos += EXPLOSION_MOVE_SPEED;
}

void Game_DrawOutro(Engine *engine, Game* game) {
    Particles_Draw(engine, &game->particles);
    Messages_Draw(engine, &game->msgs);
    return;
}

void Game_UpdateIntro(Engine *engine, Game* game) {
    if (game->particles.len == 0) {
        SDL_Rect animSparkleRect = {732, 2253, 30, 30};
        Animation_Init(&game->particles.anim, TEX_GAME_OBJECTS, animSparkleRect);
        game->particles.anim.color.b = 0;
        game->particles.anim.scale = 1.5;

        game->particles.x = (game->lvl->spiralStart.x + 104) * engine->scale_x;
        game->particles.y = game->lvl->spiralStart.y * engine->scale_y;
        game->particles.pos = 0;
        game->particles.prevPos = 0;
        Particles_NewParticle(&game->particles, game->particles.x, game->particles.y);

        game->lightTrailTime = ((float)clock())/CLOCKS_PER_SEC;
        return;
    }

    Particles_Update(&game->particles, SPARKLE_LAST_FRAME, SPARKLES_ANIM_SPEED);

    float distToEnd = game->particles.pos / (float)game->lvl->spiralLen;
    
    if (distToEnd > 0.8  && game->headerFontScale > 0)
        game->headerFontScale -= 0.1;
    else if (distToEnd > 0.1 && game->headerFontScale < 1.0) 
        game->headerFontScale += 0.1;

    if (distToEnd > 0.75  && game->subHeaderFontScale > 0) {
        game->subHeaderFontScale -= 0.1;
    } else if (distToEnd > 0.15 &&  game->subHeaderFontScale < 1.0)
        game->subHeaderFontScale += 0.1;

    if (game->particles.pos > game->lvl->spiralLen) {
        for (int i = 0; i < game->particles.len; i++) {
            if (game->particles.prtcls[i].onScreen) return;
        }

        Engine_PlaySound(engine, SND_ROLLING);
        game->isIntroEnded = true;
        game->particles.len = 0;
        game->time = ((float)clock())/CLOCKS_PER_SEC;
        return;
    }

    if (game->particles.pos - game->particles.prevPos > SPARKLES_PAD) {
        Particles_NewParticle(&game->particles, game->particles.x, game->particles.y);
        game->particles.prevPos = game->particles.pos;
    }
    
    game->particles.x += game->lvl->spiral[(int)game->particles.pos].dx * engine->scale_x * SPARKLES_MOVE_SPEED;
    game->particles.y += game->lvl->spiral[(int)game->particles.pos].dy * engine->scale_y * SPARKLES_MOVE_SPEED;
    game->particles.pos +=  SPARKLES_MOVE_SPEED;

    float currTime = ((float)clock())/CLOCKS_PER_SEC;

    if (currTime - game->lightTrailTime > 0.3) {
        Engine_PlaySoundSfxPitch(engine, SND_FX_LIGHTTRAIL2, game->lightTrailPitch);

        game->lightTrailTime = currTime;

        game->lightTrailPitch -= 1;
    }

}

void Game_DrawIntro(Engine *engine, Game* game) {
    Particles_Draw(engine, &game->particles);

    char buff[16];
    SDL_Color color = {255, 255, 255, 255};

    sprintf(buff, "%s", game->graphics->dispName);
    /* the level name never comes before the title */
    float subScale = game->subHeaderFontScale < game->headerFontScale ?
                     game->subHeaderFontScale : game->headerFontScale;
    Engine_DrawTextExtScale(engine, buff, FONT_NATIVE_ALIEN_EXT_16, subScale, color, false, true, 
        WINDOW_WIDTH - 250, WINDOW_HEIGHT - 95);

    if (Lang_Get() == LANG_AR) {
        /* Arabic: the word "LEVEL" and a smaller level number on its left,
           higher up so it doesn't cover the level name */
        float s = game->headerFontScale;
        float y = WINDOW_HEIGHT - 245;
        float ns = s * 0.55f;
        char num[8];
        sprintf(num, "%d", game->lvlID + 1);
        int ww = (int)(Engine_GetTextWidth(engine, "LEVEL", FONT_NATIVE_ALIEN_48) * s);
        int nw = (int)(Engine_GetTextWidth(engine, num, FONT_NATIVE_ALIEN_48) * ns);
        int gap = (int)(18 * s);
        float cx = WINDOW_WIDTH - 250;
        float left = cx - (ww + gap + nw) / 2.0f;
        if (left + ww + gap + nw > WINDOW_WIDTH - 30)
            left = WINDOW_WIDTH - 30 - (ww + gap + nw);
        float capBottom = 84 * engine->scale_y;   /* top + height of the capitals */
        Engine_DrawTextExtScale(engine, num, FONT_NATIVE_ALIEN_48, ns, color, false, false,
            left, y + capBottom * (s - ns));
        Engine_DrawTextExtScale(engine, "LEVEL", FONT_NATIVE_ALIEN_48, s, color, false, false,
            left + nw + gap, y);
        return;
    }
    sprintf(buff, "LEVEL %d-%d", game->lvlID + 1, game->difficulty + 1);
    /* keep the whole title on the screen (Arabic is wider) */
    float hx = WINDOW_WIDTH - 250;
    int hw = (int)(Engine_GetTextWidth(engine, buff, FONT_NATIVE_ALIEN_48) * game->headerFontScale);
    if (hx + hw / 2 > WINDOW_WIDTH - 30)
        hx = WINDOW_WIDTH - 30 - hw / 2;
    Engine_DrawTextExtScale(engine, buff, FONT_NATIVE_ALIEN_48, game->headerFontScale, color, false, true, 
        hx, WINDOW_HEIGHT - 200);
    return;
}