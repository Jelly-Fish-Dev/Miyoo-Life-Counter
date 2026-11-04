#ifndef LC_UI_SCREEN_PICKER_H
#define LC_UI_SCREEN_PICKER_H

#include <stdbool.h>

#include "model/game.h"
#include "platform/input.h"
#include "ui/gfx.h"
#include "ui/screen.h"

/* Colour picker: each player picks a colour from a 4x4 palette in their own
 * square. Same layout as the counter: the left square (P1) is upside down and
 * its D-pad directions are reversed to match what P1 sees.
 *
 *   P1 (left):  D-pad moves, SELECT picks (press again to change)
 *   P2 (right): X up, B down, Y left, A right, START picks
 *
 * When both have picked, the screen finishes after a short pause. */

#define PICKER_DONE_DELAY_MS 600

typedef struct {
    int sel[GAME_PLAYERS];    /* palette index under each cursor */
    bool ready[GAME_PLAYERS]; /* player has locked in sel */
    Uint32 both_ready_at;     /* SDL_GetTicks() when both became ready, 0 = not yet */
    SDL_Surface *panel;
} PickerScreen;

bool screen_picker_init(PickerScreen *s, Gfx *gfx);
void screen_picker_free(PickerScreen *s);

ScreenResult screen_picker_handle(PickerScreen *s, Button btn, Uint32 now);

/* SCREEN_DONE once both players have been ready for PICKER_DONE_DELAY_MS. */
ScreenResult screen_picker_tick(PickerScreen *s, Uint32 now);

void screen_picker_render(PickerScreen *s, Gfx *gfx);

#endif
