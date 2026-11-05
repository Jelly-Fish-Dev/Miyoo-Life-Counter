#ifndef LC_UI_PALETTE_H
#define LC_UI_PALETTE_H

#include <SDL/SDL.h>

/* The 16 player colours, laid out as a 4x4 grid in the picker. */

#define PALETTE_SIZE 16
#define PALETTE_COLS 4

extern const SDL_Color PALETTE[PALETTE_SIZE];

#define PALETTE_DEFAULT_P1 4  /* red */
#define PALETTE_DEFAULT_P2 11 /* blue */

/* 4 player game, in screen order: top left, top right, bottom left, bottom right. */
extern const int PALETTE_DEFAULT_4P[4];

#endif
