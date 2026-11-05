#include "ui/screen_splash.h"

#include <stdio.h>

#include "mui/color.h"
#include "mui/log.h"
#include "ui/glyphs.h"
#include "ui/palette.h"
#include "ui/theme.h"
#include "version.h"

/* Two mode cards side by side under the title. */
#define CARD_W   270
#define CARD_H   210
#define CARD_GAP 20
#define CARD_X0  ((SCREEN_W - 2 * CARD_W - CARD_GAP) / 2)
#define CARD_Y   150

/* Preview panels inside a card. */
#define MINI2_SIZE 100 /* 2 player: two side by side */
#define MINI2_GAP  10
#define MINI4_SIZE 52  /* 4 player: 2 x 2 */
#define MINI4_GAP  8
#define PREVIEW_Y  18  /* from the card's top */

#define HINT "{LEFTRIGHT} choose   {A} start   MENU exit"

void screen_splash_init(SplashScreen *s, Gfx *gfx)
{
    (void)gfx;
    s->mode = MODE_2P_20;
    s->glyphs = glyph_font_open(GLYPH_SIZE_SMALL); /* optional */
}

void screen_splash_free(SplashScreen *s)
{
    if (s->glyphs)
        TTF_CloseFont(s->glyphs);
    s->glyphs = NULL;
}

ScreenResult screen_splash_handle(SplashScreen *s, Button btn)
{
    switch (btn) {
    case BTN_LEFT:
        s->mode = (GameModeId)((s->mode + MODE_COUNT - 1) % MODE_COUNT);
        break;
    case BTN_RIGHT:
        s->mode = (GameModeId)((s->mode + 1) % MODE_COUNT);
        break;
    case BTN_A:
    case BTN_START:
        if (GAME_MODES[s->mode].available) {
            LOG_INFO("mode: %s", GAME_MODES[s->mode].name);
            return SCREEN_DONE;
        }
        break;
    case BTN_MENU:
        return SCREEN_EXIT;
    default:
        break;
    }
    return SCREEN_OK;
}

/* A small life panel showing the starting life, optionally upside down. */
static void draw_mini(Gfx *gfx, TTF_Font *font, int size, int x, int y, const char *life,
                      SDL_Color c, bool flip)
{
    SDL_Surface *tmp = gfx_new_surface(gfx, size, size);
    if (!tmp)
        return;
    SDL_Rect all = {0, 0, (Uint16)size, (Uint16)size};
    gfx_fill_on(tmp, &all, c);
    gfx_border_on(tmp, &all, 2, C_RULE);
    gfx_digits_centered_on(tmp, font, life, size / 2, size / 2, color_text_on(c));
    if (flip)
        gfx_rotate180(tmp);
    gfx_blit(gfx, tmp, x, y);
    SDL_FreeSurface(tmp);
}

/* One mode card: preview of the table layout, the mode name, and "Coming soon"
 * if it can't be played yet. Players across the table are drawn upside down. */
static void draw_card(SplashScreen *s, Gfx *gfx, GameModeId id, int x, int y)
{
    const GameMode *m = &GAME_MODES[id];
    bool sel = s->mode == id;

    SDL_Rect card = {(Sint16)x, (Sint16)y, CARD_W, CARD_H};
    gfx_fill(gfx, &card, C_PANEL);
    gfx_border_on(gfx->screen, &card, sel ? 3 : 2, sel ? C_ACCENT : C_RULE);

    char life[8];
    snprintf(life, sizeof life, "%d", m->start_life);
    int cx = x + CARD_W / 2;

    for (int i = 0; i < m->players && i < 4; i++) {
        int idx = m->players == 2 ? (i == 0 ? PALETTE_DEFAULT_P1 : PALETTE_DEFAULT_P2)
                                  : PALETTE_DEFAULT_4P[i];
        SDL_Color c = PALETTE[idx];
        if (!m->available)
            c = color_mix(C_PANEL, c, 110);
        if (m->players == 2) {
            int px = cx - MINI2_SIZE - MINI2_GAP / 2 + i * (MINI2_SIZE + MINI2_GAP);
            draw_mini(gfx, gfx->font_big, MINI2_SIZE, px, y + PREVIEW_Y, life, c, i == 0);
        } else {
            int col = i % 2, row = i / 2;
            int px = cx - MINI4_SIZE - MINI4_GAP / 2 + col * (MINI4_SIZE + MINI4_GAP);
            int py = y + PREVIEW_Y + row * (MINI4_SIZE + MINI4_GAP);
            draw_mini(gfx, gfx->font, MINI4_SIZE, px, py, life, c, row == 0);
        }
    }

    SDL_Color text = m->available ? C_WHITE : C_DIM;
    gfx_text(gfx, gfx->font, m->name, cx, y + 140, text, CARD_W - 16, ALIGN_CENTER);
    if (!m->available)
        gfx_text(gfx, gfx->font_small, "Coming soon", cx, y + 176, C_DIM, 0, ALIGN_CENTER);
}

void screen_splash_render(SplashScreen *s, Gfx *gfx)
{
    gfx_fill(gfx, NULL, C_BG);

    gfx_text(gfx, gfx->font, "Miyoo Mini", SCREEN_W / 2, 36, C_DIM, 0, ALIGN_CENTER);
    gfx_text(gfx, gfx->font_big, "Life Counter", SCREEN_W / 2, 66, C_WHITE, SCREEN_W - 40,
             ALIGN_CENTER);

    for (int i = 0; i < MODE_COUNT; i++)
        draw_card(s, gfx, (GameModeId)i, CARD_X0 + i * (CARD_W + CARD_GAP), CARD_Y);

    glyph_hint(gfx, gfx->font_small, s->glyphs, HINT, SCREEN_W / 2, CARD_Y + CARD_H + 26,
               C_ACCENT);
    gfx_text(gfx, gfx->font_small, LC_VERSION, SCREEN_W / 2, SCREEN_H - 32, C_DIM, 0,
             ALIGN_CENTER);

    gfx_present(gfx);
}
