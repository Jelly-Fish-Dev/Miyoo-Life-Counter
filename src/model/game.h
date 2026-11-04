#ifndef LC_MODEL_GAME_H
#define LC_MODEL_GAME_H

/* Two-player life totals. Pure logic: no SDL, no I/O. */

#define GAME_PLAYERS 2
#define LIFE_MIN (-999)
#define LIFE_MAX 9999

#define START_LIFE_STANDARD  20
#define START_LIFE_COMMANDER 40

typedef struct {
    int life;
    int color; /* index into the UI palette, chosen in the picker */
} Player;

typedef struct {
    Player players[GAME_PLAYERS];
    int start_life;
} Game;

/* Sets start_life and resets life totals. Player colours are left alone. */
void game_init(Game *game, int start_life);

/* Every player back to start_life. */
void game_reset(Game *game);

/* Adds delta to a player's life, clamped to LIFE_MIN..LIFE_MAX. */
void game_add(Game *game, int player, int delta);

/* Switches start_life between 20 and 40, then resets. */
void game_toggle_start(Game *game);

#endif
