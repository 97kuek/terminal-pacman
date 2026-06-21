#include <stdio.h>
#include <string.h>

#include "../src/game.h"
#include "../src/pathfind.h"
#include "../src/maze.h"
#include "../src/qghost.h"

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

/* ---- maze generator ---------------------------------------------------- */

#define MW MAP_WIDTH
#define MH MAP_HEIGHT
#define MSTRIDE (MAP_WIDTH + 1)

/* Flood fill over non-wall cells; mark reachable[]. */
static void maze_flood(const char *m, int sx, int sy, unsigned char *seen)
{
    int stack_x[MW * MH];
    int stack_y[MW * MH];
    int top = 0;

    stack_x[top] = sx;
    stack_y[top] = sy;
    seen[sy * MW + sx] = 1;
    top++;
    while (top > 0) {
        int dx4[4] = {1, -1, 0, 0};
        int dy4[4] = {0, 0, 1, -1};
        int k;
        int x;
        int y;

        --top;
        x = stack_x[top];
        y = stack_y[top];
        for (k = 0; k < 4; ++k) {
            int nx = x + dx4[k];
            int ny = y + dy4[k];

            if (nx < 0 || nx >= MW || ny < 0 || ny >= MH) {
                continue;
            }
            if (m[ny * MSTRIDE + nx] == '#' || seen[ny * MW + nx]) {
                continue;
            }
            seen[ny * MW + nx] = 1;
            stack_x[top] = nx;
            stack_y[top] = ny;
            top++;
        }
    }
}

static void test_maze(void)
{
    char a[MH * MSTRIDE];
    char b[MH * MSTRIDE];
    unsigned char seen[MW * MH];
    MazeSpawns sa;
    MazeSpawns sb;
    int x;
    int y;
    int unreachable = 0;
    int i;

    CHECK(maze_generate(a, MSTRIDE, MW, MH, 12345u, 0, GHOST_COUNT, &sa) == 1);
    /* deterministic for the same seed/variant */
    CHECK(maze_generate(b, MSTRIDE, MW, MH, 12345u, 0, GHOST_COUNT, &sb) == 1);
    CHECK(memcmp(a, b, sizeof(a)) == 0);
    /* spawns are in range and the right count */
    CHECK(sa.ghost_count == GHOST_COUNT);
    CHECK(sa.player_x > 0 && sa.player_x < MW - 1);
    CHECK(sa.player_y > 0 && sa.player_y < MH - 1);
    /* spawn cells are left empty (no pellet) */
    CHECK(a[sa.player_y * MSTRIDE + sa.player_x] == ' ');
    for (i = 0; i < sa.ghost_count; ++i) {
        CHECK(a[sa.ghost_y[i] * MSTRIDE + sa.ghost_x[i]] == ' ');
    }
    /* tunnel opening, if present, is open on both borders */
    if (sa.tunnel_row >= 0) {
        CHECK(a[sa.tunnel_row * MSTRIDE + 0] != '#');
        CHECK(a[sa.tunnel_row * MSTRIDE + (MW - 1)] != '#');
    }
    /* every pellet/power and ghost spawn is reachable from the player */
    memset(seen, 0, sizeof(seen));
    maze_flood(a, sa.player_x, sa.player_y, seen);
    for (y = 0; y < MH; ++y) {
        for (x = 0; x < MW; ++x) {
            char c = a[y * MSTRIDE + x];

            if ((c == '.' || c == 'o') && !seen[y * MW + x]) {
                unreachable++;
            }
        }
    }
    CHECK(unreachable == 0);
    for (i = 0; i < sa.ghost_count; ++i) {
        CHECK(seen[sa.ghost_y[i] * MW + sa.ghost_x[i]] == 1);
    }
}

/* ---- Q-learning core --------------------------------------------------- */

static void test_qghost(void)
{
    QGhost q;
    unsigned char only_left[QGHOST_ACTIONS] = {0, 0, 1, 0};
    unsigned char none[QGHOST_ACTIONS] = {0, 0, 0, 0};
    unsigned char all[QGHOST_ACTIONS] = {1, 1, 1, 1};
    int s;
    int a;

    qghost_init(&q, 7u);
    CHECK(q.steps == 0);
    for (s = 0; s < QGHOST_STATES; ++s) {
        for (a = 0; a < QGHOST_ACTIONS; ++a) {
            if (q.q[s][a] != 0.0f) {
                CHECK(q.q[s][a] == 0.0f);
            }
        }
    }
    /* the only legal action must be chosen */
    CHECK(qghost_select(&q, 3, only_left) == 2);
    /* no legal action -> -1 */
    CHECK(qghost_select(&q, 3, none) == -1);
    /* update math: from 0, reward 10, no next value -> Q = alpha*reward = 2.0 */
    qghost_init(&q, 7u);
    qghost_update(&q, 5, 1, 10.0f, 9, none);
    CHECK(q.q[5][1] > 1.99f && q.q[5][1] < 2.01f);
    /* out-of-range is a no-op */
    qghost_update(&q, QGHOST_STATES + 5, 0, 1.0f, 0, all);
    CHECK(qghost_select(&q, -1, all) == -1);
}

/* ---- modes ------------------------------------------------------------- */

static void test_modes(void)
{
    Game game;

    game_init(&game);

    game_configure(&game, MODE_CLASSIC, DIFF_NORMAL, 0);
    CHECK(game.mode == MODE_CLASSIC);
    CHECK(game.lives == 3);

    game_configure(&game, MODE_ENDLESS, DIFF_NORMAL, 0);
    CHECK(game.mode == MODE_ENDLESS);
    CHECK(game.lives == 1); /* roguelite */
    CHECK(game.initial_pellets > 0);

    game_configure(&game, MODE_TIMEATTACK, DIFF_HARD, 0);
    CHECK(game.mode == MODE_TIMEATTACK);
    CHECK(game.difficulty == DIFF_HARD);
    CHECK(game.time_left > 0);

    /* the Time Attack clock running out ends the run (checked before any
     * movement, so this is independent of the ghosts) */
    game.state = GAME_RUNNING;
    game.time_left = 1;
    game_update(&game);
    CHECK(game.state == GAME_OVER);

    CHECK(strcmp(game_mode_label(MODE_CLASSIC), "Classic") == 0);
    CHECK(strcmp(game_mode_label(MODE_ENDLESS), "Endless") == 0);
    CHECK(strcmp(game_mode_label(MODE_TIMEATTACK), "Time Attack") == 0);
}

int main(void)
{
    test_astar();
    test_game_init();
    test_game_state_label();
    test_ghost_move_interval();
    test_update_smoke();
    test_maze();
    test_qghost();
    test_modes();

    printf("%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
