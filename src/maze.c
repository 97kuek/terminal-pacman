#include "maze.h"

#define MAZE_MAX_WIDTH 64
#define MAZE_MAX_HEIGHT 32
#define MAZE_MAX_GHOSTS 8

typedef struct MazePoint {
    int x;
    int y;
} MazePoint;

static unsigned int maze_rng_next(unsigned int *state)
{
    unsigned int x = *state;

    if (x == 0U) {
        x = 0x6d2b79f5U;
    }
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static int maze_rng_percent(unsigned int *state, int threshold)
{
    return (int)(maze_rng_next(state) % 100U) < threshold;
}

static int maze_cell_index(int x, int y)
{
    return y * MAZE_MAX_WIDTH + x;
}

static int maze_is_taken(const unsigned char *taken, int x, int y)
{
    return taken[maze_cell_index(x, y)] != 0U;
}

static void maze_take(unsigned char *taken, int x, int y)
{
    taken[maze_cell_index(x, y)] = 1U;
}

static MazePoint maze_nearest_open(char grid[MAZE_MAX_HEIGHT][MAZE_MAX_WIDTH],
                                   const unsigned char *taken,
                                   int width, int height, int cx, int cy)
{
    MazePoint best;
    int best_d;
    int x;
    int y;

    best.x = -1;
    best.y = -1;
    best_d = 0x7fffffff;

    for (y = 1; y < height - 1; y++) {
        for (x = 1; x < width - 1; x++) {
            int dx;
            int dy;
            int d;

            if (grid[y][x] == '#' || maze_is_taken(taken, x, y)) {
                continue;
            }

            dx = x - cx;
            dy = y - cy;
            d = dx * dx + dy * dy;
            if (d < best_d) {
                best_d = d;
                best.x = x;
                best.y = y;
            }
        }
    }

    return best;
}

static int maze_carve_tunnel(char grid[MAZE_MAX_HEIGHT][MAZE_MAX_WIDTH],
                             int width, int height)
{
    int center;
    int best_row;
    int best_d;
    int y;

    center = height / 2;
    best_row = -1;
    best_d = 0x7fffffff;

    for (y = 1; y < height - 1; y++) {
        int d;

        if (grid[y][1] == '#' || grid[y][width - 2] == '#') {
            continue;
        }

        d = y - center;
        if (d < 0) {
            d = -d;
        }
        if (d < best_d) {
            best_d = d;
            best_row = y;
        }
    }

    if (best_row >= 0) {
        grid[best_row][0] = '.';
        grid[best_row][width - 1] = '.';
    }

    return best_row;
}

static int maze_validate(char grid[MAZE_MAX_HEIGHT][MAZE_MAX_WIDTH],
                         int width, int height, const MazeSpawns *spawns)
{
    unsigned char seen[MAZE_MAX_WIDTH * MAZE_MAX_HEIGHT];
    MazePoint queue[MAZE_MAX_WIDTH * MAZE_MAX_HEIGHT];
    int head;
    int tail;
    int i;
    int x;
    int y;

    for (i = 0; i < MAZE_MAX_WIDTH * MAZE_MAX_HEIGHT; i++) {
        seen[i] = 0U;
    }

    if (spawns->player_x < 0 || spawns->player_y < 0) {
        return 0;
    }

    head = 0;
    tail = 0;
    queue[tail].x = spawns->player_x;
    queue[tail].y = spawns->player_y;
    tail++;
    seen[maze_cell_index(spawns->player_x, spawns->player_y)] = 1U;

    while (head < tail) {
        static const int dirs[4][2] = {
            { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }
        };
        int d;

        x = queue[head].x;
        y = queue[head].y;
        head++;

        for (d = 0; d < 4; d++) {
            int nx = x + dirs[d][0];
            int ny = y + dirs[d][1];

            if (nx < 0 || nx >= width || ny < 0 || ny >= height) {
                continue;
            }
            if (grid[ny][nx] == '#' || seen[maze_cell_index(nx, ny)]) {
                continue;
            }

            seen[maze_cell_index(nx, ny)] = 1U;
            queue[tail].x = nx;
            queue[tail].y = ny;
            tail++;
        }
    }

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            if ((grid[y][x] == '.' || grid[y][x] == 'o') &&
                !seen[maze_cell_index(x, y)]) {
                return 0;
            }
        }
    }

    for (i = 0; i < spawns->ghost_count; i++) {
        if (!seen[maze_cell_index(spawns->ghost_x[i], spawns->ghost_y[i])]) {
            return 0;
        }
    }

    return 1;
}

static int maze_variant_index(int variant)
{
    int v = variant % 3;

    if (v < 0) {
        v += 3;
    }
    return v;
}

int maze_generate(char *out, int stride, int width, int height,
                  unsigned int seed, int variant, int ghost_count,
                  MazeSpawns *spawns)
{
    static const int row_steps[3] = { 4, 4, 3 };
    static const int col_steps[3] = { 4, 3, 4 };
    static const int carve_thresholds[3] = { 30, 22, 35 };
    static const MazePoint ghost_targets[MAZE_MAX_GHOSTS] = {
        { 0, -2 }, { -5, 0 }, { 5, 0 }, { 0, 2 },
        { -3, -1 }, { 3, -1 }, { -3, 1 }, { 3, 1 }
    };
    char grid[MAZE_MAX_HEIGHT][MAZE_MAX_WIDTH];
    unsigned char taken[MAZE_MAX_WIDTH * MAZE_MAX_HEIGHT];
    int open_rows[MAZE_MAX_HEIGHT];
    int open_cols[MAZE_MAX_WIDTH];
    int open_row_count;
    int open_col_count;
    int row_step;
    int col_step;
    int threshold;
    unsigned int rng;
    int v;
    int i;
    int x;
    int y;

    if (out == 0 || spawns == 0 || width < 3 || height < 3 ||
        width > MAZE_MAX_WIDTH || height > MAZE_MAX_HEIGHT ||
        stride < width + 1 || ghost_count < 0 ||
        ghost_count > MAZE_MAX_GHOSTS) {
        return 0;
    }

    spawns->player_x = -1;
    spawns->player_y = -1;
    spawns->ghost_count = 0;
    spawns->tunnel_row = -1;
    for (i = 0; i < MAZE_MAX_GHOSTS; i++) {
        spawns->ghost_x[i] = -1;
        spawns->ghost_y[i] = -1;
    }

    for (i = 0; i < MAZE_MAX_WIDTH * MAZE_MAX_HEIGHT; i++) {
        taken[i] = 0U;
    }

    v = maze_variant_index(variant);
    row_step = row_steps[v];
    col_step = col_steps[v];
    threshold = carve_thresholds[v];

    open_row_count = 0;
    for (y = 1; y < height - 1; y += row_step) {
        open_rows[open_row_count] = y;
        open_row_count++;
    }

    open_col_count = 0;
    for (x = 1; x < width - 1; x += col_step) {
        open_cols[open_col_count] = x;
        open_col_count++;
    }

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int is_open_row = 0;
            int is_open_col = 0;

            if (x == 0 || x == width - 1 || y == 0 || y == height - 1) {
                grid[y][x] = '#';
                continue;
            }

            for (i = 0; i < open_row_count; i++) {
                if (open_rows[i] == y) {
                    is_open_row = 1;
                    break;
                }
            }
            for (i = 0; i < open_col_count; i++) {
                if (open_cols[i] == x) {
                    is_open_col = 1;
                    break;
                }
            }

            grid[y][x] = (is_open_row || is_open_col) ? '.' : '#';
        }
    }

    rng = seed;
    if (rng == 0U) {
        rng = 0x6d2b79f5U;
    }

    for (y = 0; y < open_row_count - 1; y++) {
        int ra = open_rows[y];
        int rb = open_rows[y + 1];

        for (x = 0; x < open_col_count - 1; x++) {
            int ca = open_cols[x];
            int cb = open_cols[x + 1];

            if (maze_rng_percent(&rng, threshold)) {
                int my = (ra + rb) / 2;
                int cx;

                for (cx = ca; cx <= cb; cx++) {
                    grid[my][cx] = '.';
                }
            }
            if (maze_rng_percent(&rng, threshold)) {
                int mx = (ca + cb) / 2;
                int ry;

                for (ry = ra; ry <= rb; ry++) {
                    grid[ry][mx] = '.';
                }
            }
        }
    }

    {
        static const MazePoint power_targets[4] = {
            { 1, 1 }, { -2, 1 }, { 1, -2 }, { -2, -2 }
        };

        for (i = 0; i < 4; i++) {
            int cx = power_targets[i].x >= 0 ? power_targets[i].x :
                     width + power_targets[i].x;
            int cy = power_targets[i].y >= 0 ? power_targets[i].y :
                     height + power_targets[i].y;
            MazePoint p = maze_nearest_open(grid, taken, width, height, cx, cy);

            if (p.x < 0) {
                return 0;
            }
            grid[p.y][p.x] = 'o';
            maze_take(taken, p.x, p.y);
        }
    }

    for (i = 0; i < ghost_count; i++) {
        MazePoint g = maze_nearest_open(grid, taken, width, height,
                                        width / 2 + ghost_targets[i].x,
                                        height / 2 + ghost_targets[i].y);

        if (g.x < 0) {
            return 0;
        }
        grid[g.y][g.x] = ' ';
        maze_take(taken, g.x, g.y);
        spawns->ghost_x[i] = g.x;
        spawns->ghost_y[i] = g.y;
        spawns->ghost_count++;
    }

    {
        MazePoint p = maze_nearest_open(grid, taken, width, height,
                                        width / 2, height - 3);

        if (p.x < 0) {
            return 0;
        }
        grid[p.y][p.x] = ' ';
        maze_take(taken, p.x, p.y);
        spawns->player_x = p.x;
        spawns->player_y = p.y;
    }

    spawns->tunnel_row = maze_carve_tunnel(grid, width, height);

    if (!maze_validate(grid, width, height, spawns)) {
        return 0;
    }

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            out[y * stride + x] = grid[y][x];
        }
        out[y * stride + width] = '\0';
    }

    return 1;
}
