#include "qfeatures.h"

#include <stddef.h>

static int qfeat_bucket(int d)
{
    if (d < -3) {
        return 0;
    }
    if (d < 0) {
        return 1;
    }
    if (d == 0) {
        return 2;
    }
    if (d <= 3) {
        return 3;
    }
    return 4;
}

int qfeat_state(int ghost_x, int ghost_y, int player_x, int player_y, int player_dir)
{
    int dxb = qfeat_bucket(player_x - ghost_x);
    int dyb = qfeat_bucket(player_y - ghost_y);

    if (player_dir < 0 || player_dir > 4) {
        player_dir = 0;
    }
    return (dxb * 5 + dyb) * 5 + player_dir; /* < 125 */
}

void qfeat_action_delta(int action, int *dx, int *dy)
{
    int tx = 0;
    int ty = 0;

    switch (action) {
    case 0: /* up */
        ty = -1;
        break;
    case 1: /* down */
        ty = 1;
        break;
    case 2: /* left */
        tx = -1;
        break;
    case 3: /* right */
        tx = 1;
        break;
    default:
        break;
    }
    if (dx != NULL) {
        *dx = tx;
    }
    if (dy != NULL) {
        *dy = ty;
    }
}

int qfeat_wrap_x(int x, int width)
{
    if (x < 0) {
        return width - 1;
    }
    if (x >= width) {
        return 0;
    }
    return x;
}

void qfeat_valid(const char *map, int stride, int width, int height,
                 int x, int y, unsigned char valid[QFEAT_ACTIONS])
{
    int a;

    for (a = 0; a < QFEAT_ACTIONS; a++) {
        int dx;
        int dy;
        int nx;
        int ny;

        qfeat_action_delta(a, &dx, &dy);
        nx = qfeat_wrap_x(x + dx, width);
        ny = y + dy;
        if (ny < 0 || ny >= height || map[ny * stride + nx] == '#') {
            valid[a] = 0;
        } else {
            valid[a] = 1;
        }
    }
}

float qfeat_reward(int old_gx, int old_gy, int new_gx, int new_gy,
                   int player_x, int player_y)
{
    int old_d = (old_gx - player_x < 0 ? player_x - old_gx : old_gx - player_x) +
                (old_gy - player_y < 0 ? player_y - old_gy : old_gy - player_y);
    int new_d = (new_gx - player_x < 0 ? player_x - new_gx : new_gx - player_x) +
                (new_gy - player_y < 0 ? player_y - new_gy : new_gy - player_y);
    float reward = (float)(old_d - new_d);

    if (new_gx == player_x && new_gy == player_y) {
        reward += 5.0f;
    }
    return reward;
}
