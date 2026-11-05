#include "model/game.h"

/* Only the two-player game exists so far. */
const GameMode GAME_MODES[MODE_COUNT] = {
    [MODE_2P_20] = {"20 life · 2 player", 2, START_LIFE_STANDARD, true},
    [MODE_4P_40] = {"40 life · 4 player", 4, START_LIFE_COMMANDER, false},
};

void game_init(Game *game, int start_life)
{
    game->start_life = start_life;
    game_reset(game);
}

void game_reset(Game *game)
{
    for (int i = 0; i < GAME_PLAYERS; i++)
        game->players[i].life = game->start_life;
}

void game_add(Game *game, int player, int delta)
{
    if (player < 0 || player >= GAME_PLAYERS)
        return;
    int life = game->players[player].life + delta;
    if (life < LIFE_MIN) life = LIFE_MIN;
    if (life > LIFE_MAX) life = LIFE_MAX;
    game->players[player].life = life;
}
