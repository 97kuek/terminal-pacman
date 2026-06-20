#include "render.h"

#include <stdio.h>

#include "platform.h"

static char ghost_at(const Game *game, int x, int y)
{
    int i;

    for (i = 0; i < GHOST_COUNT; i++) {
        if (game->ghosts[i].pos.x == x && game->ghosts[i].pos.y == y) {
            if (game_power_is_blinking(game)) {
                return (game->tick / 4) % 2 == 0 ? 'g' : 'G';
            }
            return game_ghosts_are_frightened(game) ? 'g' : 'G';
        }
    }

    return '\0';
}

void render_game(const Game *game)
{
    int x;
    int y;
    char ghost;

    platform_clear_screen();

    printf("Level: %d/%d %s  Score: %d  High: %d  Lives: %d\n",
           game->level_index + 1,
           game->level_count,
           game->level_name,
           game->score,
           game->high_score,
           game->lives);
    printf("Pellets: %d  State: %s  Power: %d  Ghost speed: %d\n",
           game->pellets_remaining,
           game_state_label(game->state),
           game->power_ticks,
           game_ghost_move_interval(game));
    printf("Controls: WASD/arrows move, P pause, R restart after win/game over, Q quit\n\n");

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            ghost = ghost_at(game, x, y);
            if (game->player.pos.x == x && game->player.pos.y == y) {
                putchar('C');
            } else if (ghost != '\0') {
                putchar(ghost);
            } else {
                putchar(game->map[y][x]);
            }
        }
        putchar('\n');
    }

    if (game->state == GAME_READY) {
        printf("\nReady... %d\n", (game->countdown_ticks + 9) / 10);
    } else if (game->state == GAME_PAUSED) {
        printf("\nPaused. Press P to resume.\n");
    } else if (game->state == GAME_WON) {
        printf("\nAll pellets collected. You win. Press R to restart or Q to quit.\n");
    } else if (game->state == GAME_OVER) {
        printf("\nNo lives remaining. Game over. Press R to restart or Q to quit.\n");
    } else if (game->state == GAME_QUIT) {
        printf("\nQuit requested.\n");
    } else if (game_ghosts_are_frightened(game)) {
        if (game_power_is_blinking(game)) {
            printf("\nPower mode ending soon. Frightened ghosts are blinking.\n");
        } else {
            printf("\nPower mode: eat frightened ghosts while they are shown as g.\n");
        }
    }

    platform_finish_frame();
    fflush(stdout);
}
