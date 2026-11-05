#include "ui/delta.h"

void delta_add(DeltaPopup *d, int applied, Uint32 now)
{
    if (applied == 0)
        return;
    if (!d->active)
        d->sum = 0;
    d->sum += applied;
    d->active = true;
    d->last = now;
}

bool delta_tick(DeltaPopup *d, Uint32 now)
{
    if (!d->active)
        return false;
    Uint32 age = now - d->last;
    if (age >= DELTA_HOLD_MS + DELTA_FADE_MS) {
        d->active = false;
        return true;
    }
    return age >= DELTA_HOLD_MS; /* fading: redraw every frame */
}

int delta_alpha(const DeltaPopup *d, Uint32 now)
{
    if (!d->active)
        return 0;
    Uint32 age = now - d->last;
    if (age >= DELTA_HOLD_MS + DELTA_FADE_MS)
        return 0;
    if (age <= DELTA_HOLD_MS)
        return 256;
    return 256 - (int)((age - DELTA_HOLD_MS) * 256 / DELTA_FADE_MS);
}
