#ifndef LC_UI_DELTA_H
#define LC_UI_DELTA_H

#include <stdbool.h>

#include <SDL/SDL.h>

/* The running change ("+3") shown next to a player's life while they press
 * buttons. It stays for DELTA_HOLD_MS after the last press, then fades over
 * DELTA_FADE_MS. The next press after it has gone starts a new total. */

#define DELTA_HOLD_MS 1000
#define DELTA_FADE_MS 500

typedef struct {
    bool active;
    int sum;     /* total change since the popup appeared */
    Uint32 last; /* SDL_GetTicks() of the last press */
} DeltaPopup;

/* Adds a change that was actually applied (0 does nothing). */
void delta_add(DeltaPopup *d, int applied, Uint32 now);

/* Ages the popup. Returns true if it needs redrawing (fading or just gone). */
bool delta_tick(DeltaPopup *d, Uint32 now);

/* How visible the popup is: 256 = fully, 0 = not shown. */
int delta_alpha(const DeltaPopup *d, Uint32 now);

#endif
