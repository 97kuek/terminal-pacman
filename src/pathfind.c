#include "pathfind.h"

#define PATHFIND_MAX_WIDTH 64
#define PATHFIND_MAX_HEIGHT 32
#define PATHFIND_MAX_CELLS (PATHFIND_MAX_WIDTH * PATHFIND_MAX_HEIGHT)
#define PATHFIND_WALL '#'
#define PATHFIND_INF 1000000

static int point_in_bounds(PathPoint point, int width, int height)
{
    return point.x >= 0 && point.x < width && point.y >= 0 && point.y < height;
}

static int point_index(PathPoint point, int width)
{
    return point.y * width + point.x;
}

static int map_index(PathPoint point, int stride)
{
    return point.y * stride + point.x;
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

static int is_wall(const char *map, int stride, PathPoint point)
{
    return map[map_index(point, stride)] == PATHFIND_WALL;
}

static PathPoint index_point(int index, int width)
{
    PathPoint point;

    point.x = index % width;
    point.y = index / width;

    return point;
}

int astar_next(const char *map, int stride, int width, int height,
               PathPoint start, PathPoint goal, PathPoint *next)
{
    int cell_count;
    int g_score[PATHFIND_MAX_CELLS];
    int came_from[PATHFIND_MAX_CELLS];
    unsigned char closed[PATHFIND_MAX_CELLS];
    unsigned char in_open[PATHFIND_MAX_CELLS];
    int open_set[PATHFIND_MAX_CELLS];
    int open_count;
    int start_index;
    int goal_index;
    int i;

    if (map == 0 || next == 0) {
        return 0;
    }
    if (width <= 0 || height <= 0 || width > PATHFIND_MAX_WIDTH ||
        height > PATHFIND_MAX_HEIGHT || stride < width) {
        return 0;
    }
    if (!point_in_bounds(start, width, height) ||
        !point_in_bounds(goal, width, height)) {
        return 0;
    }
    if (start.x == goal.x && start.y == goal.y) {
        *next = start;
        return 1;
    }
    if (is_wall(map, stride, goal)) {
        return 0;
    }

    cell_count = width * height;
    for (i = 0; i < cell_count; ++i) {
        g_score[i] = PATHFIND_INF;
        came_from[i] = -1;
        closed[i] = 0;
        in_open[i] = 0;
    }

    start_index = point_index(start, width);
    goal_index = point_index(goal, width);
    g_score[start_index] = 0;
    open_set[0] = start_index;
    open_count = 1;
    in_open[start_index] = 1;

    while (open_count > 0) {
        int best_open_pos = 0;
        int best_index = open_set[0];
        int best_f = g_score[best_index] + manhattan(index_point(best_index, width), goal);
        int current_index;
        PathPoint current;
        static const int neighbor_dx[4] = {0, 0, -1, 1};
        static const int neighbor_dy[4] = {-1, 1, 0, 0};
        int n;

        for (i = 1; i < open_count; ++i) {
            int candidate_index = open_set[i];
            int candidate_f = g_score[candidate_index] +
                              manhattan(index_point(candidate_index, width), goal);

            if (candidate_f < best_f ||
                (candidate_f == best_f && candidate_index < best_index)) {
                best_open_pos = i;
                best_index = candidate_index;
                best_f = candidate_f;
            }
        }

        current_index = best_index;
        open_set[best_open_pos] = open_set[open_count - 1];
        --open_count;
        in_open[current_index] = 0;

        if (current_index == goal_index) {
            int step_index = goal_index;

            while (came_from[step_index] != start_index) {
                step_index = came_from[step_index];
            }

            *next = index_point(step_index, width);
            return 1;
        }

        closed[current_index] = 1;
        current = index_point(current_index, width);

        for (n = 0; n < 4; ++n) {
            PathPoint neighbor;
            int neighbor_index;
            int tentative_g;

            neighbor.x = current.x + neighbor_dx[n];
            neighbor.y = current.y + neighbor_dy[n];

            if (!point_in_bounds(neighbor, width, height) ||
                is_wall(map, stride, neighbor)) {
                continue;
            }

            neighbor_index = point_index(neighbor, width);
            if (closed[neighbor_index]) {
                continue;
            }

            tentative_g = g_score[current_index] + 1;
            if (tentative_g >= g_score[neighbor_index]) {
                continue;
            }

            came_from[neighbor_index] = current_index;
            g_score[neighbor_index] = tentative_g;

            if (!in_open[neighbor_index]) {
                open_set[open_count] = neighbor_index;
                ++open_count;
                in_open[neighbor_index] = 1;
            }
        }
    }

    return 0;
}
