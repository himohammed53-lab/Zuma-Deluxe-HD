#ifndef ENGINE_H
#define ENGINE_H

#define SDL_MAIN_HANDLED

#include <SDL.h>
#include "sound/SoundEngine.h"
#include <stdio.h>
#include <stdbool.h>

#define STR_ERR_BUFFER_SIZE 128
#define STR_PATH_BUFFER_SIZE 48

#define TEXTURE_FOLDER "images"
#define MUSIC_FOLDER "music"
#define SOUND_FOLDER "sounds"
#define FONT_FOLDER "fonts"

#define MUSIC_FREQUENCY 44100

#define WINDOW_WIDTH 1280.0
#define WINDOW_HEIGHT 720.0

typedef struct _Font {
	SDL_Texture* texture;
	int charsLen;
	char* chars;
	int* widthList;
	SDL_Rect* rectList;
	SDL_Point* offsetList;

	int kerningPairsLen;
	struct { char ch[2]; } *kerningPairs;
	int* kerningValues;
} Font;

typedef struct _Engine {
    /* Graphics */
    SDL_Window* 	win;
	SDL_Renderer* 	render;
	SDL_Texture** 	textures;
	unsigned int 	texturesLen;

	float 			scale_x;
	float 			scale_y;

	Font** 			fonts;
	unsigned int 	fontsLen;

	bool fullScr;

	/* Scores */


    /* Sound and Music */
	float 			volMus;
	float 			volSnd;

	SoundEngine*	sound;		/* NULL when there is no audio device */
} Engine;

void Engine_PushError(const char*, const char*);
void Engine_PushErrorCode(const char*, int);
void Engine_PushErrorFile(const char*, const char*);
void Engine_PushErrorFileCode(const char*, int);

int Engine_Init(Engine*);
void Engine_Destroy(Engine*);

/* === Graphics === */
int Engine_CreateWindow(Engine*, const char*, int, int);
SDL_Texture* Engine_TextureLoad(Engine*, const char*);
int Engine_TexturesLoad(Engine*, const char**, int);
SDL_Texture* Engine_GetTextureSDL(Engine*, int);

/* Font */
Font* Engine_FontLoad(Engine*, const char*);
int Engine_FontsLoad(Engine*, const char**, int);
void Engine_FontFree(Font*);

/* Drawing */
void Engine_DrawTexture(Engine*, int, float, float);
void Engine_DrawTextureWithRot(Engine*, int, float, float, float);
void Engine_DrawTextureSDL(Engine*, SDL_Texture*, float, float);
void Engine_DrawTextScale(Engine*, const char*, int, float, float, float);
void Engine_DrawText(Engine*, const char*, int, float, float);
int Engine_GetTextWidth(Engine*, const char* str, int fontID);
void Engine_DrawTextExt(Engine*, const char* str, int fontID,
	SDL_Color color, bool drawShadow, bool center,
	float x, float y);
void Engine_DrawTextExtScale(Engine*, const char* str, int fontID,
	float scale, SDL_Color color, bool drawShadow, bool center,
	float x, float y);


/* === Sound and Music === */
int Engine_MusicLoad(Engine*, const char*);
int Engine_SoundsLoad(Engine*, const char**, int, int copiesPerSound);
int Engine_SoundsSfxLoad(Engine*, const char**, int, int copiesPerSound);

void Engine_PlayMusic(Engine*, int);
void Engine_StopMusic(Engine*);
void Engine_UpdateMusicVolume(Engine*);
void Engine_UpdateSoundVolume(Engine*);
void Engine_PlaySound(Engine*, int);
void Engine_StopSound(Engine*, int);
void Engine_PlaySoundSfxPitch(Engine*, int, float);
void Engine_StopSoundSfx(Engine*, int);


void Engine_GetMousePos(Engine *engine, int* mx, int* my);

/* Pointer (mouse / touch) state, updated by the app event loop.
   A press is always visible for at least one frame, so a quick tap
   on a phone still clicks menu buttons. */
void Engine_InputBeginFrame(void);
void Engine_InputPointerDown(void);
void Engine_InputPointerUp(void);
bool Engine_IsMouseDown(void);
bool Engine_WasClicked(void);
void Engine_SetTouch(bool touch);   /* last pointer was a finger */
bool Engine_IsTouch(void);   /* finger lifted / mouse released this frame */

/* Typing (player name). Only A-Z, 0-9 and spaces are kept. */
void Engine_TextInputStart(int maxLen);
void Engine_TextInputStop(void);
bool Engine_TextInputIsActive(void);
void Engine_TextInputAppend(const char* utf8);
void Engine_TextInputBackspace(void);
void Engine_TextInputEnter(void);
bool Engine_TextInputTakeEnter(void);
const char* Engine_TextInputGet(void);

/* While a finger is on the frog (to swap balls) the frog must not turn */
void Engine_SetAimFrozen(bool frozen);
bool Engine_IsAimFrozen(void);

/* Folder with the game content (images, sounds, levels...).
   Returned string must be freed with SDL_free(). */
char* Engine_GetBasePath(void);

int Engine_SaveSettings(Engine *engine);
int Engine_LoadSettings(Engine *engine);

int randInt(const int, const int);
float fsign(float x);
float lerp(float start, float end, float val);

#endif
