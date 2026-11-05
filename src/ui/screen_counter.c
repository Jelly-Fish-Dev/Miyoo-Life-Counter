#include "ui/screen_counter.h"

#include <stdio.h>
#include <string.h>

#include "mui/color.h"
#include "mui/log.h"
#include "ui/glyphs.h"
#include "ui/palette.h"
#include "ui/theme.h"
#include "version.h"

#define P_LEFT  0 /* P1, upside down, D-pad */
#define P_RIGHT 1 /* P2, face buttons */

/* Distance from a player's screen edge to the top of their hint line. */
#define HINT_EDGE 28

/* Control hints, written from each player's own point of view (they must match
 * screen_counter_handle). P2: X top +1, B bottom -1, A right +5, Y left -5.
 * P1 sits across the table, so device DOWN is their up, device LEFT their right. */
#define HINT_P2 "{X}+1  {B}−1  {A}+5  {Y}−5"
#define HINT_P1 "{UP}+1  {DOWN}−1  {RIGHT}+5  {LEFT}−5"

bool screen_counter_init(CounterScreen *s, Gfx *gfx, const int colors[GAME_PLAYERS],
                         int start_life)
{
    memset(s, 0, sizeof *s);
    game_init(&s->game, GAME_PLAYERS, start_life);
    for (int i = 0; i < GAME_PLAYERS; i++)
        s->game.players[i].color = colors[i];
    s->font_huge = gfx_font(gfx, FONT_SIZE_HUGE);
    s->font_large = gfx_font(gfx, FONT_SIZE_LARGE);
    if (!s->font_huge || !s->font_large)
        return false;
    s->panel = gfx_new_surface(gfx, PANEL_SIZE, PANEL_SIZE);
    s->glyphs = glyph_font_open(GLYPH_SIZE_SMALL); /* optional */
    return s->panel != NULL;
}

void screen_counter_free(CounterScreen *s)
{
    if (s->panel)
        SDL_FreeSurface(s->panel);
    s->panel = NULL;
    if (s->glyphs)
        TTF_CloseFont(s->glyphs);
    s->glyphs = NULL;
}

/* Changes a player's life and adds the change to their running total. */
static void change(CounterScreen *s, int player, int delta, Uint32 now)
{
    int before = s->game.players[player].life;
    game_add(&s->game, player, delta);
    /* Less than delta if the total was clamped. */
    delta_add(&s->delta[player], s->game.players[player].life - before, now);
}

static void clear_deltas(CounterScreen *s)
{
    memset(s->delta, 0, sizeof s->delta);
}

ScreenResult screen_counter_handle(CounterScreen *s, Button btn, Uint32 now)
{
    Game *g = &s->game;
    switch (btn) {
    /* Right player, normal orientation. */
    case BTN_X: change(s, P_RIGHT, +1, now); break;
    case BTN_B: change(s, P_RIGHT, -1, now); break;
    case BTN_A: change(s, P_RIGHT, +5, now); break;
    case BTN_Y: change(s, P_RIGHT, -5, now); break;

    /* Left player sits across the table, so directions are from their side. */
    case BTN_DOWN:  change(s, P_LEFT, +1, now); break;
    case BTN_UP:    change(s, P_LEFT, -1, now); break;
    case BTN_LEFT:  change(s, P_LEFT, +5, now); break;
    case BTN_RIGHT: change(s, P_LEFT, -5, now); break;

    case BTN_START:
        game_reset(g);
        clear_deltas(s);
        LOG_INFO("reset to %d", g->start_life);
        break;
    case BTN_SELECT:
        return SCREEN_DONE; /* back to the mode menu */
    case BTN_MENU:
        return SCREEN_EXIT;
    default:
        break;
    }
    return SCREEN_OK;
}

bool screen_counter_tick(CounterScreen *s, Uint32 now)
{
    bool redraw = false;
    for (int i = 0; i < GAME_PLAYERS; i++)
        if (delta_tick(&s->delta[i], now))
            redraw = true;
    return redraw;
}

/* Draws one player's panel into s->panel (upright). */
static void draw_panel(CounterScreen *s, Gfx *gfx, int player)
{
    SDL_Surface *p = s->panel;
    const Player *pl = &s->game.players[player];
    SDL_Color base = PALETTE[pl->color];

    /* At 0 or below the panel goes dark with a red total; the colour stays as the border. */
    bool dead = pl->life <= 0;
    SDL_Color fill = dead ? C_PANEL : base;
    SDL_Color text = dead ? C_DANGER : color_text_on(base);

    SDL_Rect all = {0, 0, PANEL_SIZE, PANEL_SIZE};
    gfx_fill_on(p, &all, fill);
    gfx_border_on(p, &all, PANEL_BORDER, dead ? base : C_RULE);

    char num[16];
    snprintf(num, sizeof num, "%d", pl->life);
    TTF_Font *font = strlen(num) >= 3 ? s->font_large : s->font_huge;
    gfx_digits_centered_on(p, font, num, PANEL_SIZE / 2, PANEL_SIZE / 2 - 10, text);

    char label[8];
    snprintf(label, sizeof label, "P%d", player + 1);
    gfx_text_on(p, gfx->font_small, label, PANEL_SIZE / 2, PANEL_SIZE - 40, text, 0,
                ALIGN_CENTER);
}

/* The running "+3" / "-2" for a player, below their square from their side.
 * Fades by blending towards the background (SDL 1.2 can't fade per-pixel-alpha text). */
static void draw_delta(CounterScreen *s, Gfx *gfx, int player, Uint32 now)
{
    const DeltaPopup *d = &s->delta[player];
    int t256 = delta_alpha(d, now);
    if (t256 == 0)
        return;

    SDL_Color full = d->sum >= 0 ? C_WHITE : C_DANGER;
    SDL_Color c = color_mix(C_BG, full, t256);

    char txt[16];
    snprintf(txt, sizeof txt, "%+d", d->sum);
    TTF_Font *font = gfx->font_big;

    if (player == P_LEFT) {
        /* Upside down: "below" for P1 is above the square on the device. */
        int y = PANEL_Y - DELTA_GAP - TTF_FontHeight(font);
        gfx_text_rot180(gfx, font, txt, PANEL_LEFT_X + PANEL_SIZE / 2, y, c);
    } else {
        gfx_text(gfx, font, txt, PANEL_RIGHT_X + PANEL_SIZE / 2, PANEL_Y + PANEL_SIZE + DELTA_GAP,
                 c, 0, ALIGN_CENTER);
    }
}

void screen_counter_render(CounterScreen *s, Gfx *gfx, Uint32 now)
{
    gfx_fill(gfx, NULL, C_BG);

    draw_panel(s, gfx, P_LEFT);
    gfx_rotate180(s->panel);
    gfx_blit(gfx, s->panel, PANEL_LEFT_X, PANEL_Y);

    draw_panel(s, gfx, P_RIGHT);
    gfx_blit(gfx, s->panel, PANEL_RIGHT_X, PANEL_Y);

    /* Footer under the left square, readable from the right player's side. */
    int fy = SCREEN_H - HINT_EDGE;
    gfx_text(gfx, gfx->font_small, "START reset  SELECT menu  MENU exit", PANEL_LEFT_X, fy, C_DIM,
             0, ALIGN_LEFT);
    /* Top right is free (P1's side only has their hint and delta over the left square). */
    gfx_text(gfx, gfx->font_small, LC_VERSION, PANEL_RIGHT_X + PANEL_SIZE, 6, C_DIM, 0,
             ALIGN_RIGHT);

    /* Each player's controls at the screen edge on their side, under their
     * delta, flush with the outer edge of their square (P2's sits clear of the
     * footer). P1's is the mirror image of P2's, rotated to face them. */
    int w2 = glyph_hint_width(gfx->font_small, s->glyphs, HINT_P2);
    int w1 = glyph_hint_width(gfx->font_small, s->glyphs, HINT_P1);
    glyph_hint(gfx, gfx->font_small, s->glyphs, HINT_P2, PANEL_RIGHT_X + PANEL_SIZE - w2 / 2, fy,
               C_DIM);
    glyph_hint_rot180(gfx, gfx->font_small, s->glyphs, HINT_P1, PANEL_LEFT_X + w1 / 2,
                      HINT_EDGE - TTF_FontHeight(gfx->font_small), C_DIM);

    for (int i = 0; i < GAME_PLAYERS; i++)
        draw_delta(s, gfx, i, now);

    gfx_present(gfx);
}
