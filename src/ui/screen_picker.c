#include "ui/screen_picker.h"

#include <stdio.h>
#include <string.h>

#include "mui/log.h"
#include "ui/glyphs.h"
#include "ui/palette.h"
#include "ui/theme.h"

#define P_LEFT  0
#define P_RIGHT 1

/* Swatch grid inside a PANEL_SIZE square. */
#define SWATCH     52
#define SWATCH_GAP 8
#define GRID_W     (PALETTE_COLS * SWATCH + (PALETTE_COLS - 1) * SWATCH_GAP)
#define GRID_X     ((PANEL_SIZE - GRID_W) / 2)
#define GRID_Y     46
#define PALETTE_ROWS (PALETTE_SIZE / PALETTE_COLS)

bool screen_picker_init(PickerScreen *s, Gfx *gfx)
{
    memset(s, 0, sizeof *s);
    s->sel[P_LEFT] = PALETTE_DEFAULT_P1;
    s->sel[P_RIGHT] = PALETTE_DEFAULT_P2;
    s->panel = gfx_new_surface(gfx, PANEL_SIZE, PANEL_SIZE);
    s->glyphs = glyph_font_open(GLYPH_SIZE_SMALL); /* optional */
    return s->panel != NULL;
}

void screen_picker_free(PickerScreen *s)
{
    if (s->panel)
        SDL_FreeSurface(s->panel);
    s->panel = NULL;
    if (s->glyphs)
        TTF_CloseFont(s->glyphs);
    s->glyphs = NULL;
}

/* Moves a player's cursor by (dc, dr) in their own view of the grid, wrapping. */
static void move(PickerScreen *s, int player, int dc, int dr)
{
    if (s->ready[player])
        return;
    int col = s->sel[player] % PALETTE_COLS, row = s->sel[player] / PALETTE_COLS;
    col = (col + dc + PALETTE_COLS) % PALETTE_COLS;
    row = (row + dr + PALETTE_ROWS) % PALETTE_ROWS;
    s->sel[player] = row * PALETTE_COLS + col;
}

static void toggle_ready(PickerScreen *s, int player, Uint32 now)
{
    s->ready[player] = !s->ready[player];
    LOG_INFO("P%d %s colour %d", player + 1, s->ready[player] ? "picked" : "unpicked",
             s->sel[player]);
    bool both = s->ready[P_LEFT] && s->ready[P_RIGHT];
    s->both_ready_at = both ? (now ? now : 1) : 0;
}

ScreenResult screen_picker_handle(PickerScreen *s, Button btn, Uint32 now)
{
    switch (btn) {
    /* P1 is upside down: every D-pad direction is reversed. */
    case BTN_UP:    move(s, P_LEFT, 0, +1); break;
    case BTN_DOWN:  move(s, P_LEFT, 0, -1); break;
    case BTN_LEFT:  move(s, P_LEFT, +1, 0); break;
    case BTN_RIGHT: move(s, P_LEFT, -1, 0); break;
    case BTN_SELECT: toggle_ready(s, P_LEFT, now); break;

    /* P2 uses the face-button diamond as a D-pad. */
    case BTN_X: move(s, P_RIGHT, 0, -1); break;
    case BTN_B: move(s, P_RIGHT, 0, +1); break;
    case BTN_Y: move(s, P_RIGHT, -1, 0); break;
    case BTN_A: move(s, P_RIGHT, +1, 0); break;
    case BTN_START: toggle_ready(s, P_RIGHT, now); break;

    case BTN_MENU: return SCREEN_EXIT;
    default: break;
    }
    return SCREEN_OK;
}

ScreenResult screen_picker_tick(PickerScreen *s, Uint32 now)
{
    if (s->both_ready_at && now - s->both_ready_at >= PICKER_DONE_DELAY_MS)
        return SCREEN_DONE;
    return SCREEN_OK;
}

/* Draws one player's picker into s->panel (upright). */
static void draw_panel(PickerScreen *s, Gfx *gfx, int player)
{
    SDL_Surface *p = s->panel;
    SDL_Rect all = {0, 0, PANEL_SIZE, PANEL_SIZE};
    gfx_fill_on(p, &all, C_PANEL);
    gfx_border_on(p, &all, PANEL_BORDER, s->ready[player] ? PALETTE[s->sel[player]] : C_RULE);

    char title[32];
    snprintf(title, sizeof title, s->ready[player] ? "P%d ready!" : "P%d pick a colour",
             player + 1);
    gfx_text_on(p, gfx->font_small, title, PANEL_SIZE / 2, 12,
                s->ready[player] ? C_ACCENT : C_WHITE, 0, ALIGN_CENTER);

    for (int i = 0; i < PALETTE_SIZE; i++) {
        int x = GRID_X + (i % PALETTE_COLS) * (SWATCH + SWATCH_GAP);
        int y = GRID_Y + (i / PALETTE_COLS) * (SWATCH + SWATCH_GAP);
        if (i == s->sel[player]) {
            SDL_Rect ring = {(Sint16)(x - 4), (Sint16)(y - 4), SWATCH + 8, SWATCH + 8};
            gfx_border_on(p, &ring, 3, s->ready[player] ? C_ACCENT : C_WHITE);
        }
        SDL_Rect sw = {(Sint16)x, (Sint16)y, SWATCH, SWATCH};
        gfx_fill_on(p, &sw, PALETTE[i]);
    }
}

void screen_picker_render(PickerScreen *s, Gfx *gfx)
{
    gfx_fill(gfx, NULL, C_BG);

    draw_panel(s, gfx, P_LEFT);
    gfx_rotate180(s->panel);
    gfx_blit(gfx, s->panel, PANEL_LEFT_X, PANEL_Y);

    draw_panel(s, gfx, P_RIGHT);
    gfx_blit(gfx, s->panel, PANEL_RIGHT_X, PANEL_Y);

    /* Each player's hint sits on their side of the table: P1's above the
     * panels (upside down), P2's below. */
    int lx = PANEL_LEFT_X + PANEL_SIZE / 2, rx = PANEL_RIGHT_X + PANEL_SIZE / 2;
    glyph_hint_rot180(gfx, gfx->font_small, s->glyphs, "{DPAD} move   SELECT pick", lx,
                      PANEL_Y - 40, C_DIM);
    glyph_hint(gfx, gfx->font_small, s->glyphs, "{X}{B}{Y}{A} move   START pick", rx,
               PANEL_Y + PANEL_SIZE + 20, C_DIM);

    gfx_present(gfx);
}
