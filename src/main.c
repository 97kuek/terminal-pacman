#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "game.h"
#include "platform.h"
#include "render.h"

#define FRAME_MS 100

static volatile sig_atomic_t interrupted = 0;

static void handle_signal(int signal_number)
{
    (void)signal_number;
    interrupted = 1;
}

int main(void)
{
    Game game;

    srand((unsigned int)time(NULL));
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    if (!platform_init()) {
        fprintf(stderr, "Failed to initialize terminal input.\n");
        return 1;
    }

    game_init(&game);

    while (!game_is_finished(&game) && !interrupted) {
        InputKey input = platform_poll_input();
        game_handle_input(&game, input);
        game_update(&game);
        render_game(&game);
        platform_sleep_ms(FRAME_MS);
    }

    if (interrupted && !game_is_finished(&game)) {
        game.state = GAME_QUIT;
    }

    render_game(&game);
    platform_shutdown();

    return game.state == GAME_QUIT ? 0 : 0;
}
