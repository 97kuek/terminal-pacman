#ifndef TERMINAL_PACMAN_GAME_H
#define TERMINAL_PACMAN_GAME_H

#include "platform.h"

#define MAP_WIDTH 45
#define MAP_HEIGHT 21
#define GHOST_COUNT 4

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
    GHOST_CHASER,  /* Blinky: A* straight at the player */
    GHOST_AMBUSH,  /* Pinky: A* to a few tiles ahead of the player */
    GHOST_FLANK,   /* Inky: A* to a cell mirrored through the chaser */
    GHOST_SHY      /* Clyde: chases when far, retreats to its corner when close */
} GhostBehavior;

typedef enum AiMode {
    AI_SCATTER = 0, /* ghosts head for their home corners (a breather) */
    AI_CHASE        /* ghosts converge on the player (the pressure) */
} AiMode;

typedef struct Position {
    int x;
    int y;
} Position;

typedef struct Actor {
    Position pos;
    Position spawn;
    Position scatter; /* home corner used in AI_SCATTER mode */
    Direction dir;
    GhostBehavior behavior;
} Actor;

typedef struct Game {
    char map[MAP_HEIGHT][MAP_WIDTH + 1];
    char level_name[64];
    Actor player;
    Actor ghosts[GHOST_COUNT];
    int score;
    int high_score;
    int lives;
    int pellets_remaining;
    int initial_pellets;
    int power_ticks;
    int countdown_ticks;
    int level_index;
    int level_count;
    int tick;
    int ai_mode;          /* AiMode: scatter/chase wave the ghosts follow */
    int ai_timer;         /* ticks left in the current wave */
    int charge;           /* stasis-pulse meter, filled by eating pellets */
    int stasis_ticks;     /* >0 while ghosts are frozen by a pulse */
    int ghost_combo;      /* ghosts eaten in the current power window */
    int popup_ticks;      /* >0 while a floating score popup is shown */
    int popup_value;      /* points earned by the latest pickup */
    Position popup_pos;   /* board cell the popup is anchored to */
    Position fruit_home;  /* where bonus fruit appears */
    Position fruit_pos;   /* current fruit cell (valid while fruit_ticks > 0) */
    int fruit_ticks;      /* >0 while bonus fruit is on the board */
    int fruit_value;      /* points for eating the current fruit */
    int fruit_spawns_left;/* remaining fruit spawns this stage */
    GameState state;
} Game;

void game_init(Game *game);
void game_handle_input(Game *game, InputKey input);
void game_update(Game *game);
int game_is_finished(const Game *game);
int game_ghosts_are_frightened(const Game *game);
int game_power_is_blinking(const Game *game);
int game_ghost_move_interval(const Game *game);
int game_charge_percent(const Game *game);
int game_charge_is_ready(const Game *game);

const char *game_state_label(GameState state);

#endif
