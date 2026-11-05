#ifndef LC_UI_SCREEN_COMMANDER_H
#define LC_UI_SCREEN_COMMANDER_H

#include <stdbool.h>

#include "model/game.h"
#include "mui/input.h"
#include "mui/gfx.h"
#include "mui/screen.h"
#include "ui/delta.h"

/* Commander: four players at 40 life in a 2 x 2 grid. The top row sits across
 * the table, so its panels are drawn upside down. Each player has one face
 * button (+1) and one D-pad direction (-1), on the side of the device nearest
 * their panel:
 *
 *   top left      Y +1   LEFT  -1        top right     X +1   UP    -1
 *   bottom left   B +1   DOWN  -1        bottom right  A +1   RIGHT -1
 *
 * START resets everyone, SELECT goes back to the mode menu, MENU exits. */

#define CMD_PLAYERS 4

typedef struct {
    Game game;
    DeltaPopup delta[CMD_PLAYERS];
    SDL_Surface *panel;   /* off-screen buffer, reused for every panel */
    TTF_Font *font_huge;  /* life totals (owned by Gfx) */
    TTF_Font *font_large; /* life totals with 3+ characters */
    TTF_Font *glyphs;     /* button icons (owned); NULL falls back to words */
} CommanderScreen;

bool screen_commander_init(CommanderScreen *s, Gfx *gfx, int start_life);
void screen_commander_free(CommanderScreen *s);

/* SCREEN_DONE = back to the mode menu. */
ScreenResult screen_commander_handle(CommanderScreen *s, Button btn, Uint32 now);

/* Ages the delta popups. Returns true if the screen needs redrawing. */
bool screen_commander_tick(CommanderScreen *s, Uint32 now);

void screen_commander_render(CommanderScreen *s, Gfx *gfx, Uint32 now);

#endif
