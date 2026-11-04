#include "model/game.h"

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

void game_toggle_start(Game *game)
{
    game_init(game, game->start_life == START_LIFE_STANDARD ? START_LIFE_COMMANDER
                                                             : START_LIFE_STANDARD);
}
