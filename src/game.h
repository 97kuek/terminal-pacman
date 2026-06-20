#ifndef TERMINAL_PACMAN_GAME_H
#define TERMINAL_PACMAN_GAME_H

#include "platform.h"

#define MAP_WIDTH 27
#define MAP_HEIGHT 15
#define GHOST_COUNT 3

typedef enum Direction {
    DIR_NONE = 0,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

typedef enum GameState {
    GAME_RUNNING = 0,
    GAME_READY,
    GAME_PAUSED,
    GAME_WON,
    GAME_OVER,
    GAME_QUIT
} GameState;

typedef enum GhostBehavior {
    GHOST_RANDOM = 0,
    GHOST_CHASER,
    GHOST_AMBUSH
} GhostBehavior;

typedef struct Position {
    int x;
    int y;
} Position;

typedef struct Actor {
    Position pos;
    Position spawn;
    Direction dir;
    GhostBehavior behavior;
} Actor;

typedef struct Game {
    char map[MAP_HEIGHT][MAP_WIDTH + 1];
    Actor player;
    Actor ghosts[GHOST_COUNT];
    int score;
    int lives;
    int pellets_remaining;
    int initial_pellets;
    int power_ticks;
    int countdown_ticks;
    int tick;
    GameState state;
} Game;

void game_init(Game *game);
void game_handle_input(Game *game, InputKey input);
void game_update(Game *game);
int game_is_finished(const Game *game);
int game_ghosts_are_frightened(const Game *game);
int game_ghost_move_interval(const Game *game);

const char *game_state_label(GameState state);

#endif
