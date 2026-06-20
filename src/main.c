#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include "game.h"
#include "platform.h"
#include "render.h"

#define DEFAULT_FRAME_MS 100

static volatile sig_atomic_t interrupted = 0;

static void handle_signal(int signal_number)
{
    (void)signal_number;
    interrupted = 1;
}

static void print_usage(const char *prog)
{
    printf("Usage: %s [options]\n", prog);
    printf("  --mode NAME         classic | endless | timeattack (skips the menu)\n");
    printf("  --level N           start on stage N (1-%d, Classic only)\n", 3);
    printf("  --difficulty NAME   easy | normal | hard (skips the menu)\n");
    printf("  --speed MS          frame interval in milliseconds (default %d)\n",
           DEFAULT_FRAME_MS);
    printf("  --no-sound          disable sound effects\n");
    printf("  --mono              disable colour (plain monochrome output)\n");
    printf("  --help              show this help and exit\n");
}

static int parse_difficulty(const char *name, int *out)
{
    if (strcmp(name, "easy") == 0) {
        *out = DIFF_EASY;
    } else if (strcmp(name, "normal") == 0) {
        *out = DIFF_NORMAL;
    } else if (strcmp(name, "hard") == 0) {
        *out = DIFF_HARD;
    } else {
        return 0;
    }
    return 1;
}

static int parse_mode(const char *name, int *out)
{
    if (strcmp(name, "classic") == 0) {
        *out = MODE_CLASSIC;
    } else if (strcmp(name, "endless") == 0) {
        *out = MODE_ENDLESS;
    } else if (strcmp(name, "timeattack") == 0) {
        *out = MODE_TIMEATTACK;
    } else {
        return 0;
    }
    return 1;
}

int main(int argc, char **argv)
{
    Game game;
    int frame_ms = DEFAULT_FRAME_MS;
    int sound = 1;
    int mono = 0;
    int mode = MODE_CLASSIC;
    int difficulty = DIFF_NORMAL;
    int start_level = 0;
    int configured = 0; /* CLI chose mode/difficulty/level -> skip the start menu */
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--no-sound") == 0) {
            sound = 0;
        } else if (strcmp(argv[i], "--mono") == 0) {
            mono = 1;
        } else if (strcmp(argv[i], "--level") == 0 && i + 1 < argc) {
            start_level = atoi(argv[++i]) - 1;
            configured = 1;
        } else if (strcmp(argv[i], "--speed") == 0 && i + 1 < argc) {
            frame_ms = atoi(argv[++i]);
            if (frame_ms < 10) {
                frame_ms = 10;
            }
        } else if (strcmp(argv[i], "--difficulty") == 0 && i + 1 < argc) {
            if (!parse_difficulty(argv[++i], &difficulty)) {
                fprintf(stderr, "Unknown difficulty: %s\n", argv[i]);
                return 1;
            }
            configured = 1;
        } else if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            if (!parse_mode(argv[++i], &mode)) {
                fprintf(stderr, "Unknown mode: %s\n", argv[i]);
                return 1;
            }
            configured = 1;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    srand((unsigned int)time(NULL));
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    if (!platform_init()) {
        fprintf(stderr, "Failed to initialize terminal input.\n");
        return 1;
    }

    platform_set_sound_enabled(sound);
    render_set_mono(mono);

    game_init(&game);
    if (configured) {
        game_configure(&game, mode, difficulty, start_level);
    } else {
        game_show_menu(&game);
    }

    while (!game_is_finished(&game) && !interrupted) {
        InputKey input = platform_poll_input();
        game_handle_input(&game, input);
        game_update(&game);
        render_game(&game);
        platform_sleep_ms(frame_ms);
    }

    if (interrupted && !game_is_finished(&game)) {
        game.state = GAME_QUIT;
    }

    render_game(&game);
    platform_shutdown();

    return 0;
}
