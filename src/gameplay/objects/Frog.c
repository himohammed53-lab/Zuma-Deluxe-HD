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

	double length = sqrt(vecX * vecX + vecY * vecY);
	if (length < 1)   /* finger exactly on the frog: keep the old direction */
		return;

	frog->rotation = (2 * M_PI - atan2(vecX, vecY)) * TO_DEGREES;
	vecX = vecX / length;
	vecY = vecY / length;

	frog->forwardX = vecX;
	frog->forwardY = vecY;
}

static void Frog_Disc(Engine* engine, float cx, float cy, float rad,
                      SDL_Color mid, SDL_Color edge) {
	SDL_Vertex v[3];
	for (int k = 0; k < 24; k++) {
		float a1 = k * 6.2832f / 24, a2 = (k + 1) * 6.2832f / 24;
		v[0] = (SDL_Vertex){ { cx, cy }, mid, { 0, 0 } };
		v[1] = (SDL_Vertex){ { cx + cosf(a1) * rad, cy + sinf(a1) * rad }, edge, { 0, 0 } };
		v[2] = (SDL_Vertex){ { cx + cosf(a2) * rad, cy + sinf(a2) * rad }, edge, { 0, 0 } };
		SDL_RenderGeometry(engine->render, NULL, v, 3, NULL, 0);
	}
}

/* A classic round black bomb with a short fuse and a flickering spark.
   (dx, dy): direction the frog looks; the fuse points that way. */
void Frog_DrawBomb(Engine* engine, float bx, float by, float dx, float dy, bool spark) {
	SDL_SetRenderDrawBlendMode(engine->render, SDL_BLENDMODE_BLEND);

	/* fuse cap (small dark grey block) */
	float cx = bx + dx * 19, cy = by + dy * 19;
	SDL_Color capC = { 85, 85, 90, 255 };
	SDL_Vertex q[6] = {
		{ { cx - dy * 6 - dx * 5, cy + dx * 6 - dy * 5 }, capC, {0,0} },
		{ { cx + dy * 6 - dx * 5, cy - dx * 6 - dy * 5 }, capC, {0,0} },
		{ { cx + dy * 6 + dx * 5, cy - dx * 6 + dy * 5 }, capC, {0,0} },
		{ { cx - dy * 6 - dx * 5, cy + dx * 6 - dy * 5 }, capC, {0,0} },
		{ { cx + dy * 6 + dx * 5, cy - dx * 6 + dy * 5 }, capC, {0,0} },
		{ { cx - dy * 6 + dx * 5, cy + dx * 6 + dy * 5 }, capC, {0,0} },
	};
	SDL_RenderGeometry(engine->render, NULL, q, 6, NULL, 0);

	/* fuse cord */
	float fx = cx + dx * 10 - dy * 4, fy = cy + dy * 10 + dx * 4;
	SDL_SetRenderDrawColor(engine->render, 150, 120, 80, 255);
	for (int o = -1; o <= 1; o++)
		SDL_RenderDrawLineF(engine->render, cx + dx * 4 + o * dy * 0.7f, cy + dy * 4 - o * dx * 0.7f,
		                    fx + o * dy * 0.7f, fy - o * dx * 0.7f);

	/* body: black ball with a soft shine */
	Frog_Disc(engine, bx, by, 20, (SDL_Color){ 55, 55, 62, 255 }, (SDL_Color){ 18, 18, 22, 255 });
	Frog_Disc(engine, bx - 7, by - 7, 7, (SDL_Color){ 170, 170, 185, 200 }, (SDL_Color){ 60, 60, 70, 0 });

	if (!spark)
		return;

	/* spark at the end of the fuse */
	SDL_SetRenderDrawBlendMode(engine->render, SDL_BLENDMODE_ADD);
	for (int k = 0; k < 9; k++) {
		float a = (float)(rand() % 628) / 100.0f;
		float len = 7 + rand() % 11;
		SDL_Color hot = { 255, 150, 20, 255 };
		SDL_Color tip = { 255, 80, 0, 0 };
		SDL_Vertex v[3] = {
			{ { fx - sinf(a) * 1.5f, fy + cosf(a) * 1.5f }, hot, { 0, 0 } },
			{ { fx + sinf(a) * 1.5f, fy - cosf(a) * 1.5f }, hot, { 0, 0 } },
			{ { fx + cosf(a) * len, fy + sinf(a) * len }, tip, { 0, 0 } },
		};
		SDL_RenderGeometry(engine->render, NULL, v, 3, NULL, 0);
	}
	Frog_Disc(engine, fx, fy, 7, (SDL_Color){ 255, 120, 0, 200 }, (SDL_Color){ 255, 90, 0, 0 });
	Frog_Disc(engine, fx, fy, 3.5f, (SDL_Color){ 255, 240, 170, 255 }, (SDL_Color){ 255, 160, 40, 0 });
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
	if (frog->hasBomb)
		Frog_DrawBomb(engine, ballX, ballY, (float)frog->forwardX, (float)frog->forwardY, true);
	else
		Animation_Draw(engine, &anim, ballX, ballY);

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