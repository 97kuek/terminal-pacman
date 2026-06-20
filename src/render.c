#include "render.h"

#include <stdio.h>

#include "platform.h"

static int ghost_at(const Game *game, int x, int y)
{
    int i;

    for (i = 0; i < GHOST_COUNT; i++) {
        if (game->ghosts[i].pos.x == x && game->ghosts[i].pos.y == y) {
            return 1;
        }
    }

    return 0;
}

void render_game(const Game *game)
{
    int x;
    int y;

    platform_clear_screen();

    printf("Score: %d  Lives: %d  Pellets: %d  State: %s\n",
           game->score,
           game->lives,
           game->pellets_remaining,
           game_state_label(game->state));
    printf("Controls: WASD/arrows move, P pause, Q quit\n\n");

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            if (game->player.pos.x == x && game->player.pos.y == y) {
                putchar('C');
            } else if (ghost_at(game, x, y)) {
                putchar('G');
            } else {
                putchar(game->map[y][x]);
            }
        }
        putchar('\n');
    }

    if (game->state == GAME_PAUSED) {
        printf("\nPaused. Press P to resume.\n");
    } else if (game->state == GAME_WON) {
        printf("\nAll pellets collected. You win.\n");
    } else if (game->state == GAME_OVER) {
        printf("\nNo lives remaining. Game over.\n");
    } else if (game->state == GAME_QUIT) {
        printf("\nQuit requested.\n");
    }

    fflush(stdout);
}

