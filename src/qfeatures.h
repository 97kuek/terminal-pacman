#ifndef TERMINAL_PACMAN_QFEATURES_H
#define TERMINAL_PACMAN_QFEATURES_H

/* Shared feature/reward helpers for the online Q-learning ghost. Kept free of
 * the Game type so the in-game learner and the offline trainer (tools/qsim.c)
 * use exactly the same encoding. Actions: 0=up, 1=down, 2=left, 3=right. */

#define QFEAT_ACTIONS 4

/* Discrete learner state: relative player-offset buckets (5 x 5) times the
 * player's heading (0=none, 1=up, 2=down, 3=left, 4=right). Range [0, 125). */
int qfeat_state(int ghost_x, int ghost_y, int player_x, int player_y, int player_dir);

/* The (dx, dy) grid step for an action. */
void qfeat_action_delta(int action, int *dx, int *dy);

/* Horizontal wrap for warp tunnels on a width-`width` grid. */
int qfeat_wrap_x(int x, int width);

/* valid[a] = 1 if action a from (x, y) lands on a non-wall cell (x wraps).
 * map is row-major: map[y * stride + x]; '#' is a wall. */
void qfeat_valid(const char *map, int stride, int width, int height,
                 int x, int y, unsigned char valid[QFEAT_ACTIONS]);

/* Reward after a learner move: how much closer it got to the player in
 * Manhattan distance, plus a bonus for landing on the player. */
float qfeat_reward(int old_gx, int old_gy, int new_gx, int new_gy,
                   int player_x, int player_y);

#endif
