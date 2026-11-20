#include "Frog.h"
#include <stdlib.h>
#include <math.h>

#define TO_DEGREES (180.0 / M_PI)

void Frog_Init(Frog* frog, float x, float y) {
	frog->nextColor = 1;
	frog->color = 0;
	frog->x = x;
	frog->y = y;

	frog->forwardX = 0;
	frog->forwardY = 1;
	frog->isShooted = 0;
	frog->hasBomb = 0;
	frog->shift = 0;
	frog->tongueShift = 0;

	SDL_Rect animEyesRect = {0, 162, 162, 162};
	Animation_Init(&frog->eyesAnim, TEX_FROG, animEyesRect);
	Animation_Set(&frog->eyesAnim, 0, 4, 0);
}

void Frog_Rotate(Frog* frog, int mx, int my) {
	int w, h;

	double vecX = mx - frog->x;
	double vecY = my - frog->y;

	frog->rotation = (2 * M_PI - atan2(vecX, vecY)) * TO_DEGREES;

	double length = sqrt(vecX * vecX + vecY * vecY);
	vecX = vecX / length;
	vecY = vecY / length;

	frog->forwardX = vecX;
	frog->forwardY = vecY;
}

/* small orange spark on the bomb's fuse, flickering */
static void Frog_DrawSpark(Engine *engine, Frog* frog, float bx, float by) {
	/* fuse on the side of the bomb, toward the outside of the mouth */
	float sx = bx + (float)frog->forwardX * 15 - (float)frog->forwardY * 12;
	float sy = by + (float)frog->forwardY * 15 + (float)frog->forwardX * 12;

	SDL_SetRenderDrawBlendMode(engine->render, SDL_BLENDMODE_ADD);
	for (int k = 0; k < 9; k++) {
		float a = (float)(rand() % 628) / 100.0f;
		float len = 8 + rand() % 12;
		float ex = sx + cosf(a) * len, ey = sy + sinf(a) * len;
		SDL_Color hot  = { 255, 150, 20, 255 };
		SDL_Color tip  = { 255, 80, 0, 0 };
		SDL_Vertex v[3] = {
			{ { sx - sinf(a) * 1.6f, sy + cosf(a) * 1.6f }, hot, { 0, 0 } },
			{ { sx + sinf(a) * 1.6f, sy - cosf(a) * 1.6f }, hot, { 0, 0 } },
			{ { ex, ey }, tip, { 0, 0 } },
		};
		SDL_RenderGeometry(engine->render, NULL, v, 3, NULL, 0);
	}
	/* round glowing core: orange outside, yellow-white inside */
	for (int layer = 0; layer < 2; layer++) {
		float rad = layer == 0 ? 7.0f : 3.5f;
		SDL_Color mid  = layer == 0 ? (SDL_Color){255, 120, 0, 200} : (SDL_Color){255, 240, 170, 255};
		SDL_Color edge = { 255, 90, 0, 0 };
		SDL_Vertex v[3];
		for (int k = 0; k < 12; k++) {
			float a1 = k * 6.2832f / 12, a2 = (k + 1) * 6.2832f / 12;
			v[0] = (SDL_Vertex){ { sx, sy }, mid, { 0, 0 } };
			v[1] = (SDL_Vertex){ { sx + cosf(a1) * rad, sy + sinf(a1) * rad }, edge, { 0, 0 } };
			v[2] = (SDL_Vertex){ { sx + cosf(a2) * rad, sy + sinf(a2) * rad }, edge, { 0, 0 } };
			SDL_RenderGeometry(engine->render, NULL, v, 3, NULL, 0);
		}
	}
	SDL_SetRenderDrawBlendMode(engine->render, SDL_BLENDMODE_BLEND);
}

void Frog_Draw(Engine *engine, Frog* frog, bool introEnd) {
	float frogX = frog->x;
	float frogY = frog->y;

	if (frog->isShooted) {
		frog->shift = FROG_SHIFT_ON_SHOOT;
		frog->tongueShift = FROG_TONGUE_SHIFT_ON_SHOOT;
	}

	if (frog->shift > 0) {
		frogX -= frog->forwardX * frog->shift;
		frogY -= frog->forwardY * frog->shift;
    	frog->shift -= FROG_REVERT_SPEED;
    }

    if (frog->tongueShift > 0)
    	frog->tongueShift -= FROG_REVERT_SPEED * (FROG_TONGUE_SHIFT_ON_SHOOT / FROG_SHIFT_ON_SHOOT);

	if (!introEnd)
		frog->tongueShift = 24;

    Animation anim;
    //Drawing Foundation
    SDL_Rect animFoundationRect = {162, 162, 162, 162};
	Animation_Init(&anim, TEX_FROG, animFoundationRect);
	Animation_SetAngle(&anim, frog->rotation);
	Animation_Draw(engine, &anim, frogX, frogY);

    //Drawing BotBodyLayer
	SDL_Rect animBodyRect = {0, 0, 162, 162};
	Animation_Init(&anim, TEX_FROG, animBodyRect);
	Animation_SetAngle(&anim, frog->rotation);
	Animation_Draw(engine, &anim, frogX, frogY);

	//Drawing Tongue
	SDL_Rect animTongueRect = {162, 0, 162, 162};
	Animation_Init(&anim, TEX_FROG, animTongueRect);
	Animation_SetAngle(&anim, frog->rotation);
	Animation_Draw(engine, &anim, frogX + frog->forwardX*(FROG_TOUNGUE_PAD-frog->tongueShift),
		 frogY + frog->forwardY*(FROG_TOUNGUE_PAD - frog->tongueShift));

	if (introEnd) {
	//Drawing  currentBall
    SDL_Rect animBallRect = {48 * frog->color, 0, 48, 48};
	if (frog->hasBomb) {   /* the bomb ball in the frog's mouth */
		SDL_Rect bombRect = {48 * 5, 2496, 48, 48};   /* grey ball, darkened below */
		animBallRect = bombRect;
	}
	Animation_Init(&anim, TEX_GAME_OBJECTS, animBallRect);
	Animation_Set(&anim, frog->hasBomb ? 0 : 24, frog->hasBomb ? 0 : 47, 0);
	Animation_SetAngle(&anim, frog->rotation);
	float ballX = frogX + frog->forwardX*(FROG_CUR_BALL_PAD-frog->tongueShift);
	float ballY = frogY + frog->forwardY*(FROG_CUR_BALL_PAD-frog->tongueShift);
	if (frog->hasBomb) {
		/* a black bomb */
		anim.color.r = anim.color.g = anim.color.b = 55;
	}
	Animation_Draw(engine, &anim, ballX, ballY);
	if (frog->hasBomb)
		Frog_DrawSpark(engine, frog, ballX, ballY);

	//Drawing  nextBall
    SDL_Rect animBallRect1 = {15 * frog->nextColor + 234, 633, 15, 15};
	Animation_Init(&anim, TEX_FROG, animBallRect1);
	Animation_Set(&anim, 0, 0, 0);
	Animation_SetAngle(&anim, frog->rotation);
	anim.scale = 1.6;
	if (!frog->isShooted)
		Animation_Draw(engine, &anim, frogX - frog->forwardX*FROG_NEXT_BALL_PAD, frogY - frog->forwardY*FROG_NEXT_BALL_PAD);
	}

	//Drawing TopBodyLayer
	anim.scale = 1;
	if (frog->isShooted) {
		Animation_Set(&frog->eyesAnim, 0, 3, FROG_BLINK_SPEED);
	}

	Animation_SetAngle(&frog->eyesAnim, frog->rotation);
	Animation_Draw(engine, &frog->eyesAnim, frogX, frogY);

	if (frog->eyesAnim.frame == 0) {
		Animation_Set(&frog->eyesAnim, 0, 3, 0);
		Animation_SetFrame(&frog->eyesAnim, 0);
	}

    frog->isShooted = 0;
}