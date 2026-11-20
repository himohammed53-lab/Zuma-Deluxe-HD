#include "Application.h"

#include "global/Consts.h"
#include "global/Engine.h"
#include "global/Enums.h"
#include "global/Lang.h"


#include "gameplay/Game.h"
#include "menu/MenuMgr.h"
#include <stdio.h>
#include <time.h>
#ifdef __ANDROID__
#include <unistd.h>
#endif

#define MAX_FPS 60
#define FRAME_DELAY 1000 / MAX_FPS

Engine engine;

typedef struct {
  Game game;
  MenuMgr menuMgr;

  int curLvl;
  int curDifficulty;

  int inMenu;
  int mouseClicked;

  uint32_t frameStart;
  int frameTime;

  bool isRunning;
} App;

static int ApplicationZuma_LoadResources(LevelMgr *levelMgr, Engine *engine) {
  if (!Engine_LoadSettings(engine))
    return 9;

  if (!Engine_TexturesLoad(engine, filesTextures, TEXTURES_COUNT)) {
    return 3;
  }
  Lang_ApplyTextures(engine);   /* Arabic button pictures if Arabic */
  if (!Engine_FontsLoad(engine, filesFonts, FONTS_COUNT))
    return 4;
  const int soundCopies = 4;
  const int sfxCopies = 1;

  if (!Engine_SoundsLoad(engine, filesSounds, SOUNDS_COUNT, soundCopies))
    return 5;
  if (!Engine_SoundsSfxLoad(engine, filesSoundsSfx, SOUNDS_SFX_COUNT, sfxCopies))
    return 6;
  if (!Engine_MusicLoad(engine, fileMusic))
    return 7;

  if (!LevelMgr_LoadLevels(levelMgr, "levels.xml"))
    return 8;

  return 0;
}

static int ApplicationZuma_ShowStartupImage(Engine *engine) {
  char *basePath = Engine_GetBasePath();

  char resPath[100];
  Engine_LoadSettings(engine);   /* to know the language already */
  sprintf(resPath, "%simages/%s", basePath,
          Lang_Get() == LANG_AR ? "disclaimer_ar.jpg" : "disclaimer.jpg");

  SDL_free(basePath);

  SDL_Texture *texDisc = Engine_TextureLoad(engine, resPath);
  if (!texDisc)
    return 10;

  /* Draw it on every screen buffer (phones use 2-3), always on black,
     so no random pixels show in the side areas */
  SDL_SetRenderDrawColor(engine->render, 0, 0, 0, 255);
  for (int i = 0; i < 3; i++) {
    SDL_RenderClear(engine->render);
    SDL_RenderCopy(engine->render, texDisc, NULL, NULL);
    SDL_RenderPresent(engine->render);
  }
  SDL_Delay(2000);
  return 0;
}

/* Touch: drag the finger to aim (like moving the mouse),
   lift it to shoot at that spot. Tapping the frog swaps the balls
   without turning it. e.button.x/y are already in game coordinates. */
#define FROG_TAP_RADIUS 95

static bool s_deadTouch = false;   /* finger on the black side bars or a button */

/* The in-game MENU button (top right): a tap there must press the
   button, never shoot or turn the frog. A bit bigger than the picture. */
static int ApplicationZuma_IsOnGameButton(App *app, int x, int y) {
  if (app->inMenu)
    return 0;
  Button* b = &app->game.btnMenu;
  return x >= b->pos.x - 25 && x <= b->pos.x + b->tRect.w + 25 &&
         y >= b->pos.y - 25 && y <= b->pos.y + b->tRect.h + 25;
}

static int ApplicationZuma_IsOnFrog(App *app, Engine *engine) {
  if (app->inMenu)
    return 0;

  int x, y;
  Engine_GetMousePos(engine, &x, &y);   /* same mapping as aiming */

  float dx = x - app->game.frog.x;
  float dy = y - app->game.frog.y;
  return dx * dx + dy * dy <= FROG_TAP_RADIUS * FROG_TAP_RADIUS;
}

static void ApplicationZuma_HandleEvents(App *app, Engine *engine) {
  app->mouseClicked = 0;
  Engine_InputBeginFrame();

  SDL_Event e;
  while (SDL_PollEvent(&e) != 0) {
    switch (e.type) {
    case SDL_QUIT:
      exit(0);
      break;
    case SDL_TEXTINPUT:
      Engine_TextInputAppend(e.text.text);
      break;
    case SDL_KEYDOWN:
      if (e.key.keysym.sym == SDLK_BACKSPACE)
        Engine_TextInputBackspace();
      else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER)
        Engine_TextInputEnter();
      break;
    case SDL_MOUSEMOTION:
      Engine_SetTouch(e.motion.which == SDL_TOUCH_MOUSEID);
      break;
    case SDL_MOUSEBUTTONDOWN:
      Engine_SetTouch(e.button.which == SDL_TOUCH_MOUSEID);
      if (e.button.button == SDL_BUTTON_LEFT &&
          !(e.button.which == SDL_TOUCH_MOUSEID &&
            (e.button.x < 0 || e.button.x >= WINDOW_WIDTH)))
        Engine_InputPointerDown();

      if (e.button.which == SDL_TOUCH_MOUSEID) {
        if (e.button.button == SDL_BUTTON_LEFT) {
          /* black bars at the sides: dead, the finger does nothing */
          s_deadTouch = (e.button.x < 0 || e.button.x >= WINDOW_WIDTH) ||
                        ApplicationZuma_IsOnGameButton(app, e.button.x, e.button.y) ||
                        (!app->inMenu && e.button.y < 46);   /* top bar: score, lives, MENU */
          /* finger put on the frog = swap balls on release, frog stays still */
          Engine_SetAimFrozen(s_deadTouch || ApplicationZuma_IsOnFrog(app, engine));
        }
        break; /* touch shoots on release, see below */
      }

      switch (e.button.button) {
      case SDL_BUTTON_LEFT:
        app->mouseClicked = 1;
        break;
      case SDL_BUTTON_RIGHT:
        app->mouseClicked = 2;
        break;
      }
      break;
    case SDL_MOUSEBUTTONUP:
      if (e.button.button == SDL_BUTTON_LEFT)
        Engine_InputPointerUp();

      if (e.button.which == SDL_TOUCH_MOUSEID &&
          e.button.button == SDL_BUTTON_LEFT) {
        if (!s_deadTouch)
          app->mouseClicked = Engine_IsAimFrozen() ? 2 : 1;
        s_deadTouch = false;
        Engine_SetAimFrozen(false);
      }
      break;
    }
  }
}

static void ApplicationZuma_Update(LevelMgr *levelMgr, Engine *engine, App *app) {
  SDL_SetRenderDrawColor(engine->render, 0, 0, 0, 255);
  SDL_RenderClear(engine->render);

  if (app->inMenu) {
    MenuMgr_Update(levelMgr, engine, &app->menuMgr);

    if (app->menuMgr.roomID == MR_GAME && app->inMenu) {
      app->inMenu = 0;
      Game_Init(levelMgr, engine, &app->game, app->curLvl, app->curDifficulty);
    }

    MenuMgr_Draw(levelMgr, engine, &app->menuMgr);
  } else {
    Game_Update(levelMgr, engine, &app->game, &app->inMenu, app->mouseClicked);
    if (app->inMenu) {
      bool won = app->game.isWon;
      int next = app->game.lvlID + 1;

      if (won && next < LEVELS_COUNT) {
        /* after a win the next (now open) level is selected */
        app->curLvl = next;

        if (app->game.goNextLevel) {
          /* NEXT LEVEL: play it right away */
          app->inMenu = 0;
          Engine_StopMusic(engine);
          Game_Init(levelMgr, engine, &app->game, app->curLvl, app->curDifficulty);
          SDL_RenderPresent(engine->render);
          return;
        }
      }

      if (!won && app->game.retryLevel) {
        /* TRY AGAIN: play the same level again */
        app->inMenu = 0;
        Engine_StopMusic(engine);
        Game_Init(levelMgr, engine, &app->game, app->game.lvlID, app->curDifficulty);
        SDL_RenderPresent(engine->render);
        return;
      }

      /* MAIN MENU in the results window goes to the main menu */
      MenuMgr_Set(engine, &app->menuMgr, won ? MR_MAIN : MR_GAUNTLET);
      return;
    }

    Game_Draw(levelMgr, engine, &app->game);
  }

  SDL_RenderPresent(engine->render);
}

int ApplicationZuma_Start(void) {
  Engine engine;
  LevelMgr levelMgr;

#ifdef __ANDROID__
  /* settings.txt and progress.bin are saved next to the game files */
  chdir(SDL_AndroidGetInternalStoragePath());
  SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif

  if (!Engine_Init(&engine))
    return 1;

  if (!Engine_CreateWindow(&engine, "Zuma HD. By GalaxyShad and s4lat", WINDOW_WIDTH, WINDOW_HEIGHT))
    return 2;

  ApplicationZuma_ShowStartupImage(&engine);
  ApplicationZuma_LoadResources(&levelMgr, &engine);

  srand(time(NULL));

  App app;
  app.curLvl = 0;
  app.curDifficulty = 0;
  app.inMenu = 1;
  app.isRunning = true;

  MenuMgr_Init(&app.menuMgr, &app.curLvl, &app.curDifficulty);
  MenuMgr_Set(&engine, &app.menuMgr, MR_MAIN);

  /* Steady 60 FPS: every frame has its own deadline, so the average
     stays at 60 even on 90/120 Hz screens. */
  const Uint64 freq = SDL_GetPerformanceFrequency();
  const Uint64 period = freq / MAX_FPS;
  Uint64 nextFrame = SDL_GetPerformanceCounter() + period;

  while (app.isRunning) {
    ApplicationZuma_HandleEvents(&app, &engine);

    ApplicationZuma_Update(&levelMgr, &engine, &app);

    Uint64 now = SDL_GetPerformanceCounter();

    /* Screen already runs at 60 Hz: V-Sync gives perfectly even frames,
       extra waiting would only add stutter. */
    SDL_DisplayMode mode;
    int displayIndex = SDL_GetWindowDisplayIndex(engine.win);
    if (displayIndex >= 0 && SDL_GetCurrentDisplayMode(displayIndex, &mode) == 0 &&
        mode.refresh_rate >= 58 && mode.refresh_rate <= 62) {
      nextFrame = now + period;
      continue;
    }

    if (now < nextFrame) {
      Uint32 ms = (Uint32)((nextFrame - now) * 1000 / freq);
      if (ms > 1)
        SDL_Delay(ms - 1);
      while (SDL_GetPerformanceCounter() < nextFrame) {
        /* wait the last few microseconds */
      }
      nextFrame += period;
    } else if (now - nextFrame > period) {
      /* a slow frame: start again from now instead of rushing several
         frames to catch up (rushing looks like a stutter) */
      nextFrame = now + period;
    } else {
      nextFrame += period;
    }
  }

  LevelMgr_Free(&levelMgr);
  Engine_Destroy(&engine);

  return 0;
}