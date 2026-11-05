#ifndef LC_UI_SCREEN_COUNTER_H
#define LC_UI_SCREEN_COUNTER_H

#include <stdbool.h>

#include "model/game.h"
#include "mui/input.h"
#include "mui/gfx.h"
#include "mui/screen.h"

/* The main screen: two square life panels side by side, each in its player's
 * colour. The left one (P1) is drawn upside down for the player across the table. */

/* The running change ("+3") shown below a player's square while they press buttons.
 * It stays for DELTA_HOLD_MS after the last press, then fades over DELTA_FADE_MS. */
#define DELTA_HOLD_MS 1000
#define DELTA_FADE_MS 500
#define DELTA_GAP     6 /* px between the square and the number */

typedef struct {
    bool active;
    int sum;     /* total change since the popup appeared */
    Uint32 last; /* SDL_GetTicks() of the last press */
} DeltaPopup;

typedef struct {
    Game game;
    DeltaPopup delta[GAME_PLAYERS];
    SDL_Surface *panel; /* off-screen PANEL_SIZE x PANEL_SIZE buffer, reused for both panels */
    TTF_Font *font_huge;  /* life totals (owned by Gfx) */
    TTF_Font *font_large; /* life totals with 3+ characters */
    TTF_Font *glyphs;     /* button icons for the hints (owned); NULL falls back to words */
} CounterScreen;

/* colors: palette index per player, from the picker. */
bool screen_counter_init(CounterScreen *s, Gfx *gfx, const int colors[GAME_PLAYERS]);
void screen_counter_free(CounterScreen *s);

ScreenResult screen_counter_handle(CounterScreen *s, Button btn, Uint32 now);

/* Ages the delta popups. Returns true if the screen needs redrawing (fading). */
bool screen_counter_tick(CounterScreen *s, Uint32 now);

void screen_counter_render(CounterScreen *s, Gfx *gfx, Uint32 now);

#endif
