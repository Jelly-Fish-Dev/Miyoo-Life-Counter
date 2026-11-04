#include "ui/screen_counter.h"

#include <stdio.h>
#include <string.h>

#include "platform/log.h"
#include "ui/palette.h"
#include "ui/theme.h"
#include "version.h"

#define P_LEFT  0 /* P1, upside down, D-pad */
#define P_RIGHT 1 /* P2, face buttons */

bool screen_counter_init(CounterScreen *s, Gfx *gfx, const int colors[GAME_PLAYERS])
{
    memset(s, 0, sizeof *s);
    game_init(&s->game, START_LIFE_STANDARD);
    for (int i = 0; i < GAME_PLAYERS; i++)
        s->game.players[i].color = colors[i];
    s->panel = gfx_new_surface(gfx, PANEL_SIZE, PANEL_SIZE);
    return s->panel != NULL;
}

void screen_counter_free(CounterScreen *s)
{
    if (s->panel)
        SDL_FreeSurface(s->panel);
    s->panel = NULL;
}

/* Changes a player's life and adds the change to their running total. */
static void change(CounterScreen *s, int player, int delta, Uint32 now)
{
    int before = s->game.players[player].life;
    game_add(&s->game, player, delta);
    int applied = s->game.players[player].life - before; /* less than delta if clamped */
    if (applied == 0)
        return;

    DeltaPopup *d = &s->delta[player];
    if (!d->active)
        d->sum = 0;
    d->sum += applied;
    d->active = true;
    d->last = now;
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
        game_toggle_start(g);
        clear_deltas(s);
        LOG_INFO("starting life %d", g->start_life);
        break;
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
    for (int i = 0; i < GAME_PLAYERS; i++) {
        DeltaPopup *d = &s->delta[i];
        if (!d->active)
            continue;
        Uint32 age = now - d->last;
        if (age >= DELTA_HOLD_MS + DELTA_FADE_MS) {
            d->active = false; /* faded out: the next press starts a new total */
            redraw = true;
        } else if (age >= DELTA_HOLD_MS) {
            redraw = true; /* fading: redraw every frame */
        }
    }
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
    SDL_Color text = dead ? C_DANGER : palette_text_on(base);

    SDL_Rect all = {0, 0, PANEL_SIZE, PANEL_SIZE};
    gfx_fill_on(p, &all, fill);
    gfx_border_on(p, &all, PANEL_BORDER, dead ? base : C_RULE);

    char num[16];
    snprintf(num, sizeof num, "%d", pl->life);
    TTF_Font *font = strlen(num) >= 3 ? gfx->font_large : gfx->font_huge;
    gfx_digits_centered_on(p, font, num, PANEL_SIZE / 2, PANEL_SIZE / 2 - 10, text);

    char label[8];
    snprintf(label, sizeof label, "P%d", player + 1);
    gfx_text_on(p, gfx->font_small, label, PANEL_SIZE / 2, PANEL_SIZE - 40, text, 0,
                ALIGN_CENTER);
}

static Uint8 mix(Uint8 a, Uint8 b, int t256)
{
    return (Uint8)(a + ((b - a) * t256) / 256);
}

/* The running "+3" / "-2" for a player, below their square from their side.
 * Fades by blending towards the background (SDL 1.2 can't fade per-pixel-alpha text). */
static void draw_delta(CounterScreen *s, Gfx *gfx, int player, Uint32 now)
{
    const DeltaPopup *d = &s->delta[player];
    if (!d->active)
        return;

    Uint32 age = now - d->last;
    int t256 = 256; /* 256 = fully visible, 0 = gone */
    if (age >= DELTA_HOLD_MS + DELTA_FADE_MS)
        return;
    if (age > DELTA_HOLD_MS)
        t256 = 256 - (int)((age - DELTA_HOLD_MS) * 256 / DELTA_FADE_MS);

    SDL_Color full = d->sum >= 0 ? C_WHITE : C_DANGER;
    SDL_Color c = {mix(C_BG.r, full.r, t256), mix(C_BG.g, full.g, t256),
                   mix(C_BG.b, full.b, t256), 0};

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
    char hint[64];
    snprintf(hint, sizeof hint, "START reset  SELECT %d/%d  MENU exit", START_LIFE_STANDARD,
             START_LIFE_COMMANDER);
    int fy = SCREEN_H - 30;
    gfx_text(gfx, gfx->font_small, hint, PANEL_LEFT_X, fy, C_DIM, 0, ALIGN_LEFT);
    gfx_text(gfx, gfx->font_small, LC_VERSION, PANEL_RIGHT_X + PANEL_SIZE, fy, C_DIM, 0,
             ALIGN_RIGHT);

    for (int i = 0; i < GAME_PLAYERS; i++)
        draw_delta(s, gfx, i, now);

    gfx_present(gfx);
}
