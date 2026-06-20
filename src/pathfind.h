#ifndef TERMINAL_PACMAN_PATHFIND_H
#define TERMINAL_PACMAN_PATHFIND_H

typedef struct PathPoint { int x; int y; } PathPoint;

/* Compute an A* shortest path from start to goal over a row-major grid.
   The cell at (x, y) is map[y * stride + x]; '#' is a wall, anything else is open.
   Movement is 4-directional with unit cost; the heuristic is Manhattan distance.
   On success, writes the FIRST step (the cell to move into from start) to *next and
   returns 1. Returns 0 if start/goal are out of range, goal is a wall, or no path
   exists (in which case *next is left unchanged). If start == goal, writes start to
   *next and returns 1. */
int astar_next(const char *map, int stride, int width, int height,
               PathPoint start, PathPoint goal, PathPoint *next);

#endif
