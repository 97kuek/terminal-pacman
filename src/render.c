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
    int level_percent = 0;

    if (game->initial_pellets > 0) {
        level_percent = ((game->initial_pellets - game->pellets_remaining) * 100) /
                        game->initial_pellets;
    }

    platform_clear_screen();

    printf("+-----------------------------------------------------------+\n");
    printf("| TERMINAL PAC-MAN                                          |\n");
    printf("+-----------------------------------------------------------+\n");
    printf(" Stage %d/%d: %-16s  Score %6d  High %6d  Lives %d\n",
           game->level_index + 1,
           game->level_count,
           game->level_name,
           game->score,
           game->high_score,
           game->lives);
    printf(" Goal: eat every pellet, avoid G, use o to hunt ghosts. Progress %3d%%\n",
           level_percent);
    printf(" Pellets %3d/%3d  State %-9s  Power %3d  Ghost interval %d\n",
           game->pellets_remaining,
           game->initial_pellets,
           game_state_label(game->state),
           game->power_ticks,
           game_ghost_move_interval(game));
    printf(" Legend: C you  G ghost  g edible ghost  . pellet  o power  # wall\n");
    printf(" Keys: WASD/arrows move | P pause | R restart after result | Q quit\n\n");

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
        printf("\nREADY %d: choose a direction and prepare to move.\n", (game->countdown_ticks + 9) / 10);
    } else if (game->state == GAME_PAUSED) {
        printf("\nPAUSED: press P to resume.\n");
    } else if (game->state == GAME_WON) {
        printf("\nCLEAR: all stages completed. Press R to restart or Q to quit.\n");
    } else if (game->state == GAME_OVER) {
        printf("\nGAME OVER: no lives remaining. Press R to restart or Q to quit.\n");
    } else if (game->state == GAME_QUIT) {
        printf("\nQuit requested.\n");
    } else if (game_ghosts_are_frightened(game)) {
        if (game_power_is_blinking(game)) {
            printf("\nPOWER ENDING: blinking ghosts will become dangerous soon.\n");
        } else {
            printf("\nPOWER MODE: ghosts shown as g can be eaten for bonus points.\n");
        }
    } else {
        printf("\nRUNNING: clear the wide maze and watch the ghost routes.\n");
    }

    platform_finish_frame();
    fflush(stdout);
}
