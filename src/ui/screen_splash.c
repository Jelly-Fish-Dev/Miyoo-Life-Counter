#include "ui/screen_splash.h"

#include "mui/color.h"
#include "ui/palette.h"
#include "ui/theme.h"
#include "version.h"

#define MINI_SIZE 110
#define MINI_GAP  14
#define MINI_Y    190

void screen_splash_init(SplashScreen *s)
{
    s->started = SDL_GetTicks();
}

ScreenResult screen_splash_handle(SplashScreen *s, Button btn)
{
    (void)s;
    if (btn == BTN_MENU)
        return SCREEN_EXIT;
    return btn == BTN_NONE ? SCREEN_OK : SCREEN_DONE;
}

ScreenResult screen_splash_tick(SplashScreen *s, Uint32 now)
{
    return now - s->started >= SPLASH_MS ? SCREEN_DONE : SCREEN_OK;
}

/* A small life panel showing "20" in colour c, optionally upside down. */
static void draw_mini(Gfx *gfx, SDL_Surface *tmp, int x, SDL_Color c, bool flip)
{
    SDL_Rect all = {0, 0, MINI_SIZE, MINI_SIZE};
    gfx_fill_on(tmp, &all, c);
    gfx_border_on(tmp, &all, 2, C_RULE);
    gfx_digits_centered_on(tmp, gfx->font_big, "20", MINI_SIZE / 2, MINI_SIZE / 2,
                           color_text_on(c));
    if (flip)
        gfx_rotate180(tmp);
    gfx_blit(gfx, tmp, x, MINI_Y);
}

void screen_splash_render(SplashScreen *s, Gfx *gfx)
{
    (void)s;
    gfx_fill(gfx, NULL, C_BG);

    gfx_text(gfx, gfx->font, "Miyoo Mini", SCREEN_W / 2, 60, C_DIM, 0, ALIGN_CENTER);
    gfx_text(gfx, gfx->font_big, "Life Counter", SCREEN_W / 2, 92, C_WHITE, SCREEN_W - 40,
             ALIGN_CENTER);

    SDL_Surface *tmp = gfx_new_surface(gfx, MINI_SIZE, MINI_SIZE);
    if (tmp) {
        int x0 = (SCREEN_W - 2 * MINI_SIZE - MINI_GAP) / 2;
        draw_mini(gfx, tmp, x0, PALETTE[PALETTE_DEFAULT_P1], true);
        draw_mini(gfx, tmp, x0 + MINI_SIZE + MINI_GAP, PALETTE[PALETTE_DEFAULT_P2], false);
        SDL_FreeSurface(tmp);
    }

    gfx_text(gfx, gfx->font_small, "Press any button", SCREEN_W / 2, 360, C_ACCENT, 0,
             ALIGN_CENTER);
    gfx_text(gfx, gfx->font_small, LC_VERSION, SCREEN_W / 2, 440, C_DIM, 0, ALIGN_CENTER);

    gfx_present(gfx);
}
