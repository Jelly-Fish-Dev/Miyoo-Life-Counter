#ifndef LC_MODEL_GAME_H
#define LC_MODEL_GAME_H

#include <stdbool.h>

/* Life totals for 2 to 4 players. Pure logic: no SDL, no I/O. */

#define GAME_MAX_PLAYERS 4
#define GAME_PLAYERS     2 /* the 2 player game (picker and counter screens) */
#define LIFE_MIN (-999)
#define LIFE_MAX 9999

#define START_LIFE_STANDARD  20
#define START_LIFE_COMMANDER 40

/* Game modes offered on the start screen. */
typedef enum { MODE_2P_20, MODE_4P_40, MODE_COUNT } GameModeId;

typedef struct {
    const char *name; /* "20 life · 2 player" */
    int players;
    int start_life;
    bool available; /* false = shown as "Coming soon" */
} GameMode;

extern const GameMode GAME_MODES[MODE_COUNT];

typedef struct {
    int life;
    int color; /* index into the UI palette */
} Player;

typedef struct {
    Player players[GAME_MAX_PLAYERS];
    int count; /* players in this game */
    int start_life;
} Game;

/* Sets the player count and start_life, and resets life totals.
 * Player colours are left alone. */
void game_init(Game *game, int count, int start_life);

/* Every player back to start_life. */
void game_reset(Game *game);

/* Adds delta to a player's life, clamped to LIFE_MIN..LIFE_MAX. */
void game_add(Game *game, int player, int delta);

#endif
