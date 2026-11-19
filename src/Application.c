#include "Application.h"

#include "global/Consts.h"
#include "global/Engine.h"
#include "global/Enums.h"


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
  sprintf(resPath, "%simages/disclaimer.jpg", basePath);

  SDL_free(basePath);

  SDL_Texture *texDisc = Engine_TextureLoad(engine, resPath);
  if (!texDisc)
    return 10;

  SDL_RenderCopy(engine->render, texDisc, NULL, NULL);
  SDL_RenderPresent(engine->render);
  SDL_Delay(2000);
  return 0;
}

/* Touch: drag the finger to aim (like moving the mouse),
   lift it to shoot at that spot. Tapping the frog swaps the balls. */
#define FROG_TAP_RADIUS 70

static int ApplicationZuma_IsOnFrog(App *app, int x, int y) {
  if (app->inMenu)
    return 0;

  float dx = x - app->game.frog.x;
  float dy = y - app->game.frog.y;
  return dx * dx + dy * dy <= FROG_TAP_RADIUS * FROG_TAP_RADIUS;
}

static void ApplicationZuma_HandleEvents(App *app) {
  app->mouseClicked = 0;
  Engine_InputBeginFrame();

  SDL_Event e;
  while (SDL_PollEvent(&e) != 0) {
    switch (e.type) {
    case SDL_QUIT:
      exit(0);
      break;
    case SDL_MOUSEBUTTONDOWN:
      if (e.button.button == SDL_BUTTON_LEFT)
        Engine_InputPointerDown();

      if (e.button.which == SDL_TOUCH_MOUSEID)
        break; /* touch shoots on release, see below */

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
        /* e.button.x/y are already in game coordinates */
        app->mouseClicked =
            ApplicationZuma_IsOnFrog(app, e.button.x, e.button.y) ? 2 : 1;
      }
      break;
    }
  }
}

static void ApplicationZuma_Update(LevelMgr *levelMgr, Engine *engine, App *app) {
  SDL_RenderClear(engine->render);

  if (app->inMenu) {
    MenuMgr_Update(engine, &app->menuMgr);

    if (app->menuMgr.roomID == MR_GAME && app->inMenu) {
      app->inMenu = 0;
      Game_Init(levelMgr, engine, &app->game, app->curLvl, app->curDifficulty);
    }

    MenuMgr_Draw(levelMgr, engine, &app->menuMgr);
  } else {
    Game_Update(levelMgr, engine, &app->game, &app->inMenu, app->mouseClicked);
    if (app->inMenu) {
      MenuMgr_Set(engine, &app->menuMgr, MR_GAUNTLET);
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
    ApplicationZuma_HandleEvents(&app);

    ApplicationZuma_Update(&levelMgr, &engine, &app);

    Uint64 now = SDL_GetPerformanceCounter();
    if (now < nextFrame) {
      Uint32 ms = (Uint32)((nextFrame - now) * 1000 / freq);
      if (ms > 1)
        SDL_Delay(ms - 1);
      while (SDL_GetPerformanceCounter() < nextFrame) {
        /* wait the last few microseconds */
      }
      nextFrame += period;
    } else if (now - nextFrame > period * 4) {
      nextFrame = now + period; /* was paused or very slow: resync */
    } else {
      nextFrame += period;
    }
  }

  LevelMgr_Free(&levelMgr);
  Engine_Destroy(&engine);

  return 0;
}