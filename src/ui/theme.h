#ifndef LC_UI_THEME_H
#define LC_UI_THEME_H

#include "mui/theme.h"

/* Life Counter's layout and font sizes on top of the shared Miyoo-UI theme
 * (screen size, font, C_WHITE / C_DIM / C_BG / C_PANEL / C_DANGER ...). */

/* Two square panels side by side, vertically centred. */
#define PANEL_SIZE   300
#define PANEL_Y      ((SCREEN_H - PANEL_SIZE) / 2)
#define PANEL_GAP    14
#define PANEL_LEFT_X ((SCREEN_W - 2 * PANEL_SIZE - PANEL_GAP) / 2)
#define PANEL_RIGHT_X (PANEL_LEFT_X + PANEL_SIZE + PANEL_GAP)
#define PANEL_BORDER 3

/* Extra sizes for the life totals, opened with gfx_font(). */
#define FONT_SIZE_HUGE  150
#define FONT_SIZE_LARGE 104

/* Button icons (ui/glyphs.h) next to font_small hint text. */
#define GLYPH_SIZE_SMALL 30

#endif
