#ifndef LC_UI_SCREEN_SPLASH_H
#define LC_UI_SCREEN_SPLASH_H

#include <stdbool.h>

#include "mui/input.h"
#include "mui/gfx.h"
#include "mui/screen.h"

/* Title screen shown at start-up. It ends after SPLASH_MS or on any button. */

#define SPLASH_MS 2500

typedef struct {
    Uint32 started; /* SDL_GetTicks() when shown */
} SplashScreen;

void screen_splash_init(SplashScreen *s);

ScreenResult screen_splash_handle(SplashScreen *s, Button btn);

/* SCREEN_DONE once the splash has been up for SPLASH_MS. */
ScreenResult screen_splash_tick(SplashScreen *s, Uint32 now);

void screen_splash_render(SplashScreen *s, Gfx *gfx);

#endif
