#include <stdio.h>
#include <string.h>

#include "../src/game.h"
#include "../src/pathfind.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { \
    ++checks; \
    if (cond) { \
        printf("PASS %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } else { \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        ++failures; \
    } \
} while (0)

void platform_play(SoundId sound)
{
    (void)sound;
}

static int manhattan(PathPoint a, PathPoint b)
{
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    if (dx < 0) {
        dx = -dx;
    }
    if (dy < 0) {
        dy = -dy;
    }
    return dx + dy;
}

static void test_astar(void)
{
    {
        const char map[] = ".....";
        PathPoint start = {0, 0};
        PathPoint goal = {4, 0};
        PathPoint next = {-1, -1};
        int start_dist = manhattan(start, goal);

        CHECK(astar_next(map, 5, 5, 1, start, goal, &next) == 1);
        CHECK(next.x == 1);
        CHECK(next.y == 0);
        CHECK(manhattan(next, goal) < start_dist);
    }

    {
        const char map[] =
            "..."
            "###"
            "...";
        PathPoint start = {0, 0};
        PathPoint goal = {0, 2};
        PathPoint next = {-1, -1};

        CHECK(astar_next(map, 3, 3, 3, start, goal, &next) == 0);
    }

    {
        const char map[] =
            "..."
            "..."
            "...";
        PathPoint start = {1, 1};
        PathPoint goal = {1, 1};
        PathPoint next = {-1, -1};

        CHECK(astar_next(map, 3, 3, 3, start, goal, &next) == 1);
        CHECK(next.x == start.x);
        CHECK(next.y == start.y);
    }

    {
        const char map[] =
            "..."
            ".#."
            "...";
        PathPoint start = {0, 0};
        PathPoint goal = {1, 1};
        PathPoint next = {-1, -1};

        CHECK(astar_next(map, 3, 3, 3, start, goal, &next) == 0);
    }
}

static void test_game_init(void)
{
    Game game;

    game_init(&game);

    CHECK(game.lives == 3);
    CHECK(game.initial_pellets > 0);
    CHECK(game.pellets_remaining == game.initial_pellets);
    CHECK(!game_is_finished(&game));
    CHECK(game_charge_percent(&game) == 0);
    CHECK(!game_charge_is_ready(&game));
}

static void test_game_state_label(void)
{
    CHECK(game_state_label(GAME_RUNNING) != NULL);
    CHECK(strcmp(game_state_label(GAME_RUNNING), "RUNNING") == 0);
    CHECK(game_state_label(GAME_READY) != NULL);
    CHECK(strcmp(game_state_label(GAME_READY), "READY") == 0);
    CHECK(game_state_label(GAME_PAUSED) != NULL);
    CHECK(strcmp(game_state_label(GAME_PAUSED), "PAUSED") == 0);
    CHECK(game_state_label(GAME_WON) != NULL);
    CHECK(strcmp(game_state_label(GAME_WON), "YOU WIN") == 0);
    CHECK(game_state_label(GAME_OVER) != NULL);
    CHECK(strcmp(game_state_label(GAME_OVER), "GAME OVER") == 0);
    CHECK(game_state_label(GAME_QUIT) != NULL);
    CHECK(strcmp(game_state_label(GAME_QUIT), "QUIT") == 0);
}

static void test_ghost_move_interval(void)
{
    Game game;

    game_init(&game);

    CHECK(game_ghost_move_interval(&game) >= 1);
}

static void test_update_smoke(void)
{
    Game game;
    int i;

    game_init(&game);
    for (i = 0; i < 60; ++i) {
        game_update(&game);
        CHECK(!game_is_finished(&game));
    }
}

int main(void)
{
    test_astar();
    test_game_init();
    test_game_state_label();
    test_ghost_move_interval();
    test_update_smoke();

    printf("%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
