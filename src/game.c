#include "game.h"

#include <stdlib.h>
#include <string.h>

static const char *INITIAL_MAP[MAP_HEIGHT] = {
    "###########################",
    "#............#............#",
    "#.####.#####.#.#####.####.#",
    "#.#  #.#   #.#.#   #.#  #.#",
    "#.####.#####.#.#####.####.#",
    "#.........................#",
    "#.####.#.#########.#.####.#",
    "#......#.....#.....#......#",
    "######.##### # #####.######",
    "#............ ............#",
    "#.####.#####.#.#####.####.#",
    "#.#  #.......C.......#  #.#",
    "#.####.#.#########.#.####.#",
    "#......#...........#......#",
    "###########################"
};

static const Position PLAYER_SPAWN = {13, 11};
static const Position GHOST_SPAWNS[GHOST_COUNT] = {
    {13, 7},
    {12, 9},
    {14, 9}
};

static Position step_position(Position pos, Direction dir)
{
    switch (dir) {
    case DIR_UP:
        pos.y -= 1;
        break;
    case DIR_DOWN:
        pos.y += 1;
        break;
    case DIR_LEFT:
        pos.x -= 1;
        break;
    case DIR_RIGHT:
        pos.x += 1;
        break;
    default:
        break;
    }

    return pos;
}

static int is_inside(Position pos)
{
    return pos.x >= 0 && pos.x < MAP_WIDTH && pos.y >= 0 && pos.y < MAP_HEIGHT;
}

static int is_wall(const Game *game, Position pos)
{
    if (!is_inside(pos)) {
        return 1;
    }

    return game->map[pos.y][pos.x] == '#';
}

static int actors_overlap(Position a, Position b)
{
    return a.x == b.x && a.y == b.y;
}

static void reset_actors(Game *game)
{
    int i;

    game->player.pos = PLAYER_SPAWN;
    game->player.spawn = PLAYER_SPAWN;
    game->player.dir = DIR_NONE;

    for (i = 0; i < GHOST_COUNT; i++) {
        game->ghosts[i].pos = GHOST_SPAWNS[i];
        game->ghosts[i].spawn = GHOST_SPAWNS[i];
        game->ghosts[i].dir = DIR_NONE;
    }
}

static void count_pellets(Game *game)
{
    int x;
    int y;

    game->pellets_remaining = 0;

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            if (game->map[y][x] == '.') {
                game->pellets_remaining++;
            }
        }
    }
}

static void clean_spawn_markers(Game *game)
{
    int x;
    int y;

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            if (game->map[y][x] == 'C' || game->map[y][x] == 'G') {
                game->map[y][x] = ' ';
            }
        }
    }
}

static void collect_pellet(Game *game)
{
    char *tile = &game->map[game->player.pos.y][game->player.pos.x];

    if (*tile == '.') {
        *tile = ' ';
        game->score += 10;
        game->pellets_remaining--;

        if (game->pellets_remaining <= 0) {
            game->state = GAME_WON;
        }
    }
}

static void lose_life(Game *game)
{
    game->lives--;

    if (game->lives <= 0) {
        game->state = GAME_OVER;
        return;
    }

    reset_actors(game);
}

static void resolve_collisions(Game *game)
{
    int i;

    for (i = 0; i < GHOST_COUNT; i++) {
        if (actors_overlap(game->player.pos, game->ghosts[i].pos)) {
            lose_life(game);
            return;
        }
    }
}

static Direction input_to_direction(InputKey input)
{
    switch (input) {
    case INPUT_UP:
        return DIR_UP;
    case INPUT_DOWN:
        return DIR_DOWN;
    case INPUT_LEFT:
        return DIR_LEFT;
    case INPUT_RIGHT:
        return DIR_RIGHT;
    default:
        return DIR_NONE;
    }
}

static void move_player(Game *game)
{
    Position next;

    if (game->player.dir == DIR_NONE) {
        return;
    }

    next = step_position(game->player.pos, game->player.dir);
    if (!is_wall(game, next)) {
        game->player.pos = next;
    }
}

static void move_ghost(Game *game, Actor *ghost)
{
    Direction choices[4];
    int count = 0;
    int i;

    for (i = DIR_UP; i <= DIR_RIGHT; i++) {
        Direction dir = (Direction)i;
        Position next = step_position(ghost->pos, dir);

        if (!is_wall(game, next)) {
            choices[count++] = dir;
        }
    }

    if (count == 0) {
        ghost->dir = DIR_NONE;
        return;
    }

    ghost->dir = choices[rand() % count];
    ghost->pos = step_position(ghost->pos, ghost->dir);
}

void game_init(Game *game)
{
    int y;

    memset(game, 0, sizeof(*game));

    for (y = 0; y < MAP_HEIGHT; y++) {
        strcpy(game->map[y], INITIAL_MAP[y]);
    }

    clean_spawn_markers(game);
    count_pellets(game);
    reset_actors(game);

    game->score = 0;
    game->lives = 3;
    game->tick = 0;
    game->state = GAME_RUNNING;
}

void game_handle_input(Game *game, InputKey input)
{
    Direction dir;

    if (input == INPUT_NONE) {
        return;
    }

    if (input == INPUT_QUIT) {
        game->state = GAME_QUIT;
        return;
    }

    if (input == INPUT_PAUSE) {
        if (game->state == GAME_RUNNING) {
            game->state = GAME_PAUSED;
        } else if (game->state == GAME_PAUSED) {
            game->state = GAME_RUNNING;
        }
        return;
    }

    if (game->state != GAME_RUNNING) {
        return;
    }

    dir = input_to_direction(input);
    if (dir != DIR_NONE) {
        game->player.dir = dir;
    }
}

void game_update(Game *game)
{
    int i;

    if (game->state != GAME_RUNNING) {
        return;
    }

    game->tick++;

    move_player(game);
    collect_pellet(game);
    resolve_collisions(game);

    if (game->state != GAME_RUNNING) {
        return;
    }

    if (game->tick % 3 == 0) {
        for (i = 0; i < GHOST_COUNT; i++) {
            move_ghost(game, &game->ghosts[i]);
        }
        resolve_collisions(game);
    }
}

int game_is_finished(const Game *game)
{
    return game->state == GAME_WON ||
           game->state == GAME_OVER ||
           game->state == GAME_QUIT;
}

const char *game_state_label(GameState state)
{
    switch (state) {
    case GAME_RUNNING:
        return "RUNNING";
    case GAME_PAUSED:
        return "PAUSED";
    case GAME_WON:
        return "YOU WIN";
    case GAME_OVER:
        return "GAME OVER";
    case GAME_QUIT:
        return "QUIT";
    default:
        return "UNKNOWN";
    }
}
