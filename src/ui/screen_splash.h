#ifndef LC_UI_SCREEN_SPLASH_H
#define LC_UI_SCREEN_SPLASH_H

#include <stdbool.h>

#include "model/game.h"
#include "mui/input.h"
#include "mui/gfx.h"
#include "mui/screen.h"

/* Title screen and mode choice: one card per GAME_MODES entry.
 *
 *   D-pad LEFT / RIGHT   choose a mode
 *   A or START           start it (modes that aren't available yet do nothing)
 *   MENU                 exit
 *
 * SCREEN_DONE means a mode was picked; read it from mode. */

typedef struct {
    GameModeId mode;  /* highlighted card */
    TTF_Font *glyphs; /* button icons for the hint; NULL falls back to words */
} SplashScreen;

void screen_splash_init(SplashScreen *s, Gfx *gfx);
void screen_splash_free(SplashScreen *s);

ScreenResult screen_splash_handle(SplashScreen *s, Button btn);

void screen_splash_render(SplashScreen *s, Gfx *gfx);

#endif
