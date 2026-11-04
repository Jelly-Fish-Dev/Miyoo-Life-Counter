#ifndef LC_UI_THEME_H
#define LC_UI_THEME_H

#include <SDL/SDL.h>

/* Screen size, layout, font and colours shared by all screens. */

#define SCREEN_W 640
#define SCREEN_H 480

/* Two square panels side by side, vertically centred. */
#define PANEL_SIZE   300
#define PANEL_Y      ((SCREEN_H - PANEL_SIZE) / 2)
#define PANEL_GAP    14
#define PANEL_LEFT_X ((SCREEN_W - 2 * PANEL_SIZE - PANEL_GAP) / 2)
#define PANEL_RIGHT_X (PANEL_LEFT_X + PANEL_SIZE + PANEL_GAP)
#define PANEL_BORDER 3

#define FONT_PATH       "res/fonts/Exo2-Bold.ttf"
#define FONT_SIZE_HUGE  150
#define FONT_SIZE_LARGE 104
#define FONT_SIZE_BIG   52
#define FONT_SIZE       26
#define FONT_SIZE_SMALL 19

/* text */
static const SDL_Color C_WHITE  = {240, 240, 245, 0};
static const SDL_Color C_DIM    = {140, 145, 160, 0};
static const SDL_Color C_DARK   = {20, 22, 30, 0};
static const SDL_Color C_ACCENT = {255, 196, 60, 0};
static const SDL_Color C_DANGER = {235, 80, 80, 0};

/* fills */
static const SDL_Color C_BG    = {22, 24, 34, 0};
static const SDL_Color C_PANEL = {32, 35, 48, 0};
static const SDL_Color C_RULE  = {60, 64, 84, 0};

#endif
