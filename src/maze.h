#ifndef TERMINAL_PACMAN_MAZE_H
#define TERMINAL_PACMAN_MAZE_H

typedef struct MazeSpawns {
    int player_x, player_y;
    int ghost_x[8], ghost_y[8];
    int ghost_count;     /* number actually placed */
    int tunnel_row;      /* row with the left/right warp opening, or -1 */
} MazeSpawns;

/* Generate a maze into `out` (row-major, out[y*stride + x], NUL-terminated rows).
   Writes '#'=wall, '.'=pellet, 'o'=power pellet, ' '=empty (player/ghost spawn cells
   are left as ' '; their coordinates are returned in `spawns`). The left/right border
   gets one warp-tunnel opening. width<=64, height<=32, ghost_count<=8.
   Deterministic for a given (seed, variant). Returns 1 if the maze is fully connected
   (every '.'/'o' and every spawn reachable from the player spawn), else 0. */
int maze_generate(char *out, int stride, int width, int height,
                  unsigned int seed, int variant, int ghost_count,
                  MazeSpawns *spawns);

#endif
