#ifndef LC_UI_GFX_H
#define LC_UI_GFX_H

#include <stdbool.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

/* Display, fonts and drawing primitives. Screens draw into gfx->screen (or
 * into an off-screen surface from gfx_new_surface) and call gfx_present()
 * when a frame is done. */

typedef struct {
    SDL_Surface *video;  /* real display surface */
    SDL_Surface *screen; /* off-screen buffer screens draw into */
    TTF_Font *font_huge; /* life totals */
    TTF_Font *font_large; /* life totals with 3+ characters */
    TTF_Font *font_big;
    TTF_Font *font;
    TTF_Font *font_small;
} Gfx;

typedef enum { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT } TextAlign;

/* Opens the 640x480 display and loads the fonts. On failure, call gfx_quit(). */
bool gfx_init(Gfx *gfx);
void gfx_quit(Gfx *gfx);

void gfx_present(Gfx *gfx);

/* A w x h surface in the same pixel format as gfx->screen. Free with SDL_FreeSurface. */
SDL_Surface *gfx_new_surface(Gfx *gfx, int w, int h);

/* Rotates a 32bpp surface by 180 degrees in place. */
void gfx_rotate180(SDL_Surface *surf);

/* Fills rect (NULL = whole surface) with a colour. */
void gfx_fill_on(SDL_Surface *dst, const SDL_Rect *rect, SDL_Color color);
void gfx_fill(Gfx *gfx, const SDL_Rect *rect, SDL_Color color);

/* Draws a 1px-wide-per-unit border of thickness t just inside rect. */
void gfx_border_on(SDL_Surface *dst, const SDL_Rect *rect, int t, SDL_Color color);

/* Draws a surface at x,y. */
void gfx_blit_on(SDL_Surface *dst, SDL_Surface *surf, int x, int y);
void gfx_blit(Gfx *gfx, SDL_Surface *surf, int x, int y);

int gfx_text_width(TTF_Font *font, const char *text);

/* Draws text anchored at x (left / centre / right edge), top at y. If max_w > 0
 * and the text is wider, it is cut on a UTF-8 boundary and ends with "...". */
void gfx_text_on(SDL_Surface *dst, TTF_Font *font, const char *text, int x, int y,
                 SDL_Color color, int max_w, TextAlign align);
void gfx_text(Gfx *gfx, TTF_Font *font, const char *text, int x, int y, SDL_Color color,
              int max_w, TextAlign align);

/* Draws text rotated 180 degrees on gfx->screen, centred on x, with the
 * rotated text's top edge at y (for a reader across the table). */
void gfx_text_rot180(Gfx *gfx, TTF_Font *font, const char *text, int x, int y, SDL_Color color);

/* Draws a string of digits (and '-') centred horizontally on cx, with the
 * digits' visible height centred vertically on cy. */
void gfx_digits_centered_on(SDL_Surface *dst, TTF_Font *font, const char *text, int cx, int cy,
                            SDL_Color color);

#endif
