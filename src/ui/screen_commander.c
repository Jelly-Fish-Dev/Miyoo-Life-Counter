#include "ui/screen_commander.h"

#include <stdio.h>
#include <string.h>

#include "mui/color.h"
#include "mui/log.h"
#include "ui/glyphs.h"
#include "ui/palette.h"
#include "ui/theme.h"

/* 2 x 2 panels with a strip between the rows for the footer. */
#define CMD_W     306
#define CMD_H     212
#define CMD_GAP   8
#define CMD_X0    ((SCREEN_W - 2 * CMD_W - CMD_GAP) / 2)
#define CMD_TOP_Y 8
#define CMD_BOT_Y (SCREEN_H - 8 - CMD_H)

/* Player order: top left, top right, bottom left, bottom right (P1..P4). */
enum { TL, TR, BL, BR };

/* Each player's buttons, and their hint as they see it from their seat. The
 * top row sits across the table: device UP is their down and device LEFT
 * their right, so their hints show the mirrored icons. */
static const struct {
    Button plus, minus;
    const char *hint;
    bool flip; /* panel drawn upside down */
    int x, y;
} SEATS[CMD_PLAYERS] = {
    [TL] = {BTN_Y, BTN_LEFT,  "{A}+1  {RIGHT}−1", true,  CMD_X0,                 CMD_TOP_Y},
    [TR] = {BTN_X, BTN_UP,    "{B}+1  {DOWN}−1",  true,  CMD_X0 + CMD_W + CMD_GAP, CMD_TOP_Y},
    [BL] = {BTN_B, BTN_DOWN,  "{B}+1  {DOWN}−1",  false, CMD_X0,                 CMD_BOT_Y},
    [BR] = {BTN_A, BTN_RIGHT, "{A}+1  {RIGHT}−1", false, CMD_X0 + CMD_W + CMD_GAP, CMD_BOT_Y},
};

bool screen_commander_init(CommanderScreen *s, Gfx *gfx, int start_life)
{
    memset(s, 0, sizeof *s);
    game_init(&s->game, CMD_PLAYERS, start_life);
    for (int i = 0; i < CMD_PLAYERS; i++)
        s->game.players[i].color = PALETTE_DEFAULT_4P[i];
    s->font_huge = gfx_font(gfx, FONT_SIZE_HUGE);
    s->font_large = gfx_font(gfx, FONT_SIZE_LARGE);
    if (!s->font_huge || !s->font_large)
        return false;
    s->panel = gfx_new_surface(gfx, CMD_W, CMD_H);
    s->glyphs = glyph_font_open(GLYPH_SIZE_SMALL); /* optional */
    return s->panel != NULL;
}

void screen_commander_free(CommanderScreen *s)
{
    if (s->panel)
        SDL_FreeSurface(s->panel);
    s->panel = NULL;
    if (s->glyphs)
        TTF_CloseFont(s->glyphs);
    s->glyphs = NULL;
}

static void change(CommanderScreen *s, int player, int delta, Uint32 now)
{
    int before = s->game.players[player].life;
    game_add(&s->game, player, delta);
    delta_add(&s->delta[player], s->game.players[player].life - before, now);
}

ScreenResult screen_commander_handle(CommanderScreen *s, Button btn, Uint32 now)
{
    for (int i = 0; i < CMD_PLAYERS; i++) {
        if (btn == SEATS[i].plus) {
            change(s, i, +1, now);
            return SCREEN_OK;
        }
        if (btn == SEATS[i].minus) {
            change(s, i, -1, now);
            return SCREEN_OK;
        }
    }
    switch (btn) {
    case BTN_START:
        game_reset(&s->game);
        memset(s->delta, 0, sizeof s->delta);
        LOG_INFO("reset to %d", s->game.start_life);
        break;
    case BTN_SELECT:
        return SCREEN_DONE;
    case BTN_MENU:
        return SCREEN_EXIT;
    default:
        break;
    }
    return SCREEN_OK;
}

bool screen_commander_tick(CommanderScreen *s, Uint32 now)
{
    bool redraw = false;
    for (int i = 0; i < CMD_PLAYERS; i++)
        if (delta_tick(&s->delta[i], now))
            redraw = true;
    return redraw;
}

/* Draws one player's panel into s->panel as they see it (upright). */
static void draw_panel(CommanderScreen *s, Gfx *gfx, int player, Uint32 now)
{
    SDL_Surface *p = s->panel;
    const Player *pl = &s->game.players[player];
    SDL_Color base = PALETTE[pl->color];

    /* At 0 or below the panel goes dark with a red total; the colour stays as the border. */
    bool dead = pl->life <= 0;
    SDL_Color fill = dead ? C_PANEL : base;
    SDL_Color text = dead ? C_DANGER : color_text_on(base);

    SDL_Rect all = {0, 0, CMD_W, CMD_H};
    gfx_fill_on(p, &all, fill);
    gfx_border_on(p, &all, PANEL_BORDER, dead ? base : C_RULE);

    char buf[16];
    snprintf(buf, sizeof buf, "P%d", player + 1);
    gfx_text_on(p, gfx->font_small, buf, 12, 8, text, 0, ALIGN_LEFT);

    /* Running change in the top right corner, fading into the panel colour. */
    int t256 = delta_alpha(&s->delta[player], now);
    if (t256 > 0) {
        snprintf(buf, sizeof buf, "%+d", s->delta[player].sum);
        gfx_text_on(p, gfx->font, buf, CMD_W - 12, 4, color_mix(fill, text, t256), 0,
                    ALIGN_RIGHT);
    }

    snprintf(buf, sizeof buf, "%d", pl->life);
    TTF_Font *font = strlen(buf) >= 3 ? s->font_large : s->font_huge;
    gfx_digits_centered_on(p, font, buf, CMD_W / 2, CMD_H / 2 - 8, text);

    glyph_hint_on(p, gfx->font_small, s->glyphs, SEATS[player].hint, CMD_W / 2, CMD_H - 36,
                  text);
}

void screen_commander_render(CommanderScreen *s, Gfx *gfx, Uint32 now)
{
    gfx_fill(gfx, NULL, C_BG);

    for (int i = 0; i < CMD_PLAYERS; i++) {
        draw_panel(s, gfx, i, now);
        if (SEATS[i].flip)
            gfx_rotate180(s->panel);
        gfx_blit(gfx, s->panel, SEATS[i].x, SEATS[i].y);
    }

    /* Footer in the strip between the rows. */
    int strip = CMD_BOT_Y - (CMD_TOP_Y + CMD_H);
    int fy = CMD_TOP_Y + CMD_H + (strip - TTF_FontHeight(gfx->font_small)) / 2;
    gfx_text(gfx, gfx->font_small, "START reset  SELECT menu  MENU exit", SCREEN_W / 2, fy, C_DIM,
             0, ALIGN_CENTER);

    gfx_present(gfx);
}
