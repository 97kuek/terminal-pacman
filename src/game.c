#include "game.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define POWER_TICKS 80
#define POWER_BLINK_TICKS 25
#define READY_TICKS 20
#define GHOST_EAT_SCORE 200
#define SCORE_FILE "terminal-pacman.score"
#define LEVEL_COUNT 3

static const char *LEVEL_FILES[LEVEL_COUNT] = {
    "levels/stage1.txt",
    "levels/stage2.txt",
    "levels/stage3.txt"
};

static const char *LEVEL_NAMES[LEVEL_COUNT] = {
    "Stage 1",
    "Stage 2",
    "Stage 3"
};

static const char *BUILTIN_LEVELS[LEVEL_COUNT][MAP_HEIGHT] = {
    {
        "###########################################################",
        "#.o.....................................................o.#",
        "#.........................................................#",
        "#........#.......................................#........#",
        "#....######################.....######################....#",
        "#........#.......................................#........#",
        "#........#..........#.................#..........#........#",
        "#........#..........#.................#..........#........#",
        "#........#....#############.....#############....#........#",
        "#........#..........#.................#..........#........#",
        "#............................G............................#",
        "#..........................G...G..........................#",
        "#.........................................................#",
        "#........#..........#.................#..........#........#",
        "#........#....#############.....#############....#........#",
        "#........#..........#.................#..........#........#",
        "#........#..........#.................#..........#........#",
        "#........#...................C...................#........#",
        "#....######################.....######################....#",
        "#........#.......................................#........#",
        "#.........................................................#",
        "#.o.....................................................o.#",
        "###########################################################"
    },
    {
        "###########################################################",
        "#.o..........................o..........................o.#",
        "#.........................................................#",
        "#....#############.....#############.....#############....#",
        "#...........#................#................#...........#",
        "#...........#................#................#...........#",
        "#....####...#...#########....#....#########...#...####....#",
        "#...............#.........................#...............#",
        "#...............#...........G.............#...............#",
        "#....####...#...#########.........#########...#...####....#",
        "#...........#................G................#...........#",
        "#...........#.................................#...........#",
        "#....####...#...#########.........#########...#...####....#",
        "#...............#...........G.............#...............#",
        "#...............#.........................#...............#",
        "#....####...#...#########....#....#########...#...####....#",
        "#...........#................#................#...........#",
        "#...........#................#................#...........#",
        "#....#############.....#############.....#############....#",
        "#............................C............................#",
        "#.........................................................#",
        "#.o..........................o..........................o.#",
        "###########################################################"
    },
    {
        "###########################################################",
        "#.o.....................................................o.#",
        "#.........................................................#",
        "#....#####################.......#####################....#",
        "#....#...............................................#....#",
        "#....#.......................#.......................#....#",
        "#....#.......................#.......................#....#",
        "#....#....################...#...################....#....#",
        "#....#........#..............#..............#........#....#",
        "#....#........#..............#..............#........#....#",
        "#............................G............................#",
        "#.............o............G...G............o.............#",
        "#.........................................................#",
        "#....#........#..............#..............#........#....#",
        "#....#........#..............#..............#........#....#",
        "#....#....################...#...################....#....#",
        "#....#.......................#.......................#....#",
        "#....#.......................C.......................#....#",
        "#....#...............................................#....#",
        "#....#####################.......#####################....#",
        "#.........................................................#",
        "#.o.....................................................o.#",
        "###########################################################"
    }
};

static const Position DEFAULT_PLAYER_SPAWN = {29, 17};
static const Position DEFAULT_GHOST_SPAWNS[GHOST_COUNT] = {
    {29, 10},
    {27, 11},
    {31, 11}
};

static const GhostBehavior GHOST_BEHAVIORS[GHOST_COUNT] = {
    GHOST_RANDOM,
    GHOST_CHASER,
    GHOST_AMBUSH
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

static int is_pellet_tile(char tile)
{
    return tile == '.' || tile == 'o';
}

static int load_high_score(void)
{
    int score = 0;
    FILE *file = fopen(SCORE_FILE, "r");

    if (file == NULL) {
        return 0;
    }

    if (fscanf(file, "%d", &score) != 1 || score < 0) {
        score = 0;
    }

    fclose(file);
    return score;
}

static void save_high_score(int score)
{
    FILE *file = fopen(SCORE_FILE, "w");

    if (file == NULL) {
        return;
    }

    fprintf(file, "%d\n", score);
    fclose(file);
}

static void update_high_score(Game *game)
{
    if (game->score > game->high_score) {
        game->high_score = game->score;
        save_high_score(game->high_score);
    }
}

static void reset_actors(Game *game)
{
    int i;

    game->player.pos = game->player.spawn;
    game->player.dir = DIR_NONE;

    for (i = 0; i < GHOST_COUNT; i++) {
        game->ghosts[i].pos = game->ghosts[i].spawn;
        game->ghosts[i].dir = DIR_NONE;
        game->ghosts[i].behavior = GHOST_BEHAVIORS[i];
    }
}

static void reset_ghost(Actor *ghost)
{
    ghost->pos = ghost->spawn;
    ghost->dir = DIR_NONE;
}

static void start_ready(Game *game)
{
    game->countdown_ticks = READY_TICKS;
    game->state = GAME_READY;
}

static void count_pellets(Game *game)
{
    int x;
    int y;

    game->pellets_remaining = 0;

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            if (is_pellet_tile(game->map[y][x])) {
                game->pellets_remaining++;
            }
        }
    }

    game->initial_pellets = game->pellets_remaining;
}

static void strip_line_end(char *line)
{
    size_t length = strlen(line);

    while (length > 0 && (line[length - 1] == '\n' || line[length - 1] == '\r')) {
        line[length - 1] = '\0';
        length--;
    }
}

static void apply_level_lines(Game *game, const char *lines[MAP_HEIGHT], const char *name)
{
    int x;
    int y;
    int ghost_count = 0;

    game->player.spawn = DEFAULT_PLAYER_SPAWN;
    for (x = 0; x < GHOST_COUNT; x++) {
        game->ghosts[x].spawn = DEFAULT_GHOST_SPAWNS[x];
    }

    strncpy(game->level_name, name, sizeof(game->level_name) - 1);
    game->level_name[sizeof(game->level_name) - 1] = '\0';

    for (y = 0; y < MAP_HEIGHT; y++) {
        size_t length = strlen(lines[y]);

        for (x = 0; x < MAP_WIDTH; x++) {
            char tile = x < (int)length ? lines[y][x] : ' ';

            if (tile == 'C') {
                game->player.spawn.x = x;
                game->player.spawn.y = y;
                tile = ' ';
            } else if (tile == 'G') {
                if (ghost_count < GHOST_COUNT) {
                    game->ghosts[ghost_count].spawn.x = x;
                    game->ghosts[ghost_count].spawn.y = y;
                    ghost_count++;
                }
                tile = ' ';
            } else if (tile != '#' && tile != '.' && tile != 'o' && tile != ' ') {
                tile = ' ';
            }

            game->map[y][x] = tile;
        }
        game->map[y][MAP_WIDTH] = '\0';
    }
}

static int load_level_file(Game *game, int level_index)
{
    FILE *file;
    char buffers[MAP_HEIGHT][MAP_WIDTH + 8];
    const char *lines[MAP_HEIGHT];
    int y;

    file = fopen(LEVEL_FILES[level_index], "r");
    if (file == NULL) {
        return 0;
    }

    for (y = 0; y < MAP_HEIGHT; y++) {
        if (fgets(buffers[y], sizeof(buffers[y]), file) == NULL) {
            fclose(file);
            return 0;
        }
        strip_line_end(buffers[y]);
        lines[y] = buffers[y];
    }

    fclose(file);
    apply_level_lines(game, lines, LEVEL_NAMES[level_index]);
    return 1;
}

static void load_level(Game *game, int level_index)
{
    if (!load_level_file(game, level_index)) {
        apply_level_lines(game, BUILTIN_LEVELS[level_index], LEVEL_NAMES[level_index]);
    }

    game->level_index = level_index;
    game->level_count = LEVEL_COUNT;
    game->power_ticks = 0;
    game->tick = 0;

    count_pellets(game);
    reset_actors(game);
    start_ready(game);
}

static void advance_or_win(Game *game)
{
    update_high_score(game);

    if (game->level_index + 1 < game->level_count) {
        load_level(game, game->level_index + 1);
        return;
    }

    game->state = GAME_WON;
}

static void collect_pellet(Game *game)
{
    char *tile = &game->map[game->player.pos.y][game->player.pos.x];

    if (*tile == '.') {
        *tile = ' ';
        game->score += 10;
        game->pellets_remaining--;
        update_high_score(game);
    } else if (*tile == 'o') {
        *tile = ' ';
        game->score += 50;
        game->pellets_remaining--;
        game->power_ticks = POWER_TICKS;
        update_high_score(game);
    }

    if (game->pellets_remaining <= 0) {
        advance_or_win(game);
    }
}

static void lose_life(Game *game)
{
    game->lives--;
    game->power_ticks = 0;
    update_high_score(game);

    if (game->lives <= 0) {
        game->state = GAME_OVER;
        return;
    }

    reset_actors(game);
    start_ready(game);
}

static void resolve_collisions(Game *game)
{
    int i;

    for (i = 0; i < GHOST_COUNT; i++) {
        if (!actors_overlap(game->player.pos, game->ghosts[i].pos)) {
            continue;
        }

        if (game->power_ticks > 0) {
            game->score += GHOST_EAT_SCORE;
            update_high_score(game);
            reset_ghost(&game->ghosts[i]);
        } else {
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

static Direction opposite_direction(Direction dir)
{
    switch (dir) {
    case DIR_UP:
        return DIR_DOWN;
    case DIR_DOWN:
        return DIR_UP;
    case DIR_LEFT:
        return DIR_RIGHT;
    case DIR_RIGHT:
        return DIR_LEFT;
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

static int collect_valid_directions(const Game *game, Position pos, Direction *choices)
{
    int count = 0;
    int i;

    for (i = DIR_UP; i <= DIR_RIGHT; i++) {
        Direction dir = (Direction)i;
        Position next = step_position(pos, dir);

        if (!is_wall(game, next)) {
            choices[count++] = dir;
        }
    }

    return count;
}

static Direction random_valid_direction(const Game *game, const Actor *actor)
{
    Direction choices[4];
    int count = collect_valid_directions(game, actor->pos, choices);

    if (count == 0) {
        return DIR_NONE;
    }

    return choices[rand() % count];
}

static int build_distance_map(const Game *game, Position target, int distance[MAP_HEIGHT][MAP_WIDTH])
{
    Position queue[MAP_HEIGHT * MAP_WIDTH];
    int head = 0;
    int tail = 0;
    int x;
    int y;

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            distance[y][x] = -1;
        }
    }

    if (is_wall(game, target)) {
        return 0;
    }

    distance[target.y][target.x] = 0;
    queue[tail++] = target;

    while (head < tail) {
        Position current = queue[head++];
        int i;

        for (i = DIR_UP; i <= DIR_RIGHT; i++) {
            Direction dir = (Direction)i;
            Position next = step_position(current, dir);

            if (is_wall(game, next) || distance[next.y][next.x] >= 0) {
                continue;
            }

            distance[next.y][next.x] = distance[current.y][current.x] + 1;
            queue[tail++] = next;
        }
    }

    return 1;
}

static Direction choose_path_direction(const Game *game, const Actor *actor, Position target)
{
    int distance[MAP_HEIGHT][MAP_WIDTH];
    Direction choices[4];
    Direction best_dirs[4];
    int choice_count;
    int best_count = 0;
    int best_distance = INT_MAX;
    int i;

    if (!build_distance_map(game, target, distance)) {
        return random_valid_direction(game, actor);
    }

    choice_count = collect_valid_directions(game, actor->pos, choices);
    for (i = 0; i < choice_count; i++) {
        Position next = step_position(actor->pos, choices[i]);
        int candidate = distance[next.y][next.x];

        if (candidate < 0) {
            continue;
        }

        if (candidate < best_distance) {
            best_distance = candidate;
            best_count = 0;
            best_dirs[best_count++] = choices[i];
        } else if (candidate == best_distance) {
            best_dirs[best_count++] = choices[i];
        }
    }

    if (best_count == 0) {
        return random_valid_direction(game, actor);
    }

    return best_dirs[rand() % best_count];
}

static Direction choose_frightened_direction(const Game *game, const Actor *actor)
{
    int distance[MAP_HEIGHT][MAP_WIDTH];
    Direction choices[4];
    Direction best_dirs[4];
    int choice_count;
    int best_count = 0;
    int best_distance = -1;
    int i;

    if (!build_distance_map(game, game->player.pos, distance)) {
        return random_valid_direction(game, actor);
    }

    choice_count = collect_valid_directions(game, actor->pos, choices);
    for (i = 0; i < choice_count; i++) {
        Position next = step_position(actor->pos, choices[i]);
        int candidate = distance[next.y][next.x];

        if (candidate < 0) {
            continue;
        }

        if (candidate > best_distance) {
            best_distance = candidate;
            best_count = 0;
            best_dirs[best_count++] = choices[i];
        } else if (candidate == best_distance) {
            best_dirs[best_count++] = choices[i];
        }
    }

    if (best_count == 0) {
        return random_valid_direction(game, actor);
    }

    return best_dirs[rand() % best_count];
}

static Position ambush_target(const Game *game)
{
    Position target = game->player.pos;
    int i;

    for (i = 0; i < 4; i++) {
        Position next = step_position(target, game->player.dir);

        if (game->player.dir == DIR_NONE || is_wall(game, next)) {
            break;
        }

        target = next;
    }

    return target;
}

static Direction choose_ambush_direction(const Game *game, const Actor *actor)
{
    Direction choices[4];
    int choice_count = collect_valid_directions(game, actor->pos, choices);

    if (choice_count <= 2 && actor->dir != DIR_NONE) {
        Position next = step_position(actor->pos, actor->dir);
        if (!is_wall(game, next)) {
            return actor->dir;
        }
    }

    if (choice_count > 1 && actor->dir != DIR_NONE) {
        Direction reverse = opposite_direction(actor->dir);
        int i;

        for (i = 0; i < choice_count; i++) {
            if (choices[i] != reverse) {
                return choose_path_direction(game, actor, ambush_target(game));
            }
        }
    }

    return choose_path_direction(game, actor, ambush_target(game));
}

static void move_ghost(Game *game, Actor *ghost)
{
    Direction dir;

    if (game->power_ticks > 0) {
        dir = choose_frightened_direction(game, ghost);
    } else if (ghost->behavior == GHOST_CHASER) {
        dir = choose_path_direction(game, ghost, game->player.pos);
    } else if (ghost->behavior == GHOST_AMBUSH) {
        dir = choose_ambush_direction(game, ghost);
    } else {
        dir = random_valid_direction(game, ghost);
    }

    ghost->dir = dir;
    if (dir != DIR_NONE) {
        ghost->pos = step_position(ghost->pos, dir);
    }
}

void game_init(Game *game)
{
    int high_score = load_high_score();

    memset(game, 0, sizeof(*game));

    game->score = 0;
    game->high_score = high_score;
    game->lives = 3;
    game->level_count = LEVEL_COUNT;

    load_level(game, 0);
}

void game_handle_input(Game *game, InputKey input)
{
    Direction dir;

    if (input == INPUT_NONE) {
        return;
    }

    if (input == INPUT_QUIT) {
        update_high_score(game);
        game->state = GAME_QUIT;
        return;
    }

    if (input == INPUT_RESTART &&
        (game->state == GAME_WON || game->state == GAME_OVER || game->state == GAME_QUIT)) {
        game_init(game);
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

    if (game->state != GAME_RUNNING && game->state != GAME_READY) {
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

    if (game->state == GAME_READY) {
        if (game->countdown_ticks > 0) {
            game->countdown_ticks--;
        }

        if (game->countdown_ticks <= 0) {
            game->state = GAME_RUNNING;
        }
        return;
    }

    if (game->state != GAME_RUNNING) {
        return;
    }

    game->tick++;

    if (game->power_ticks > 0) {
        game->power_ticks--;
    }

    move_player(game);
    collect_pellet(game);

    if (game->state != GAME_RUNNING) {
        return;
    }

    resolve_collisions(game);

    if (game->state != GAME_RUNNING) {
        return;
    }

    if (game->tick % game_ghost_move_interval(game) == 0) {
        for (i = 0; i < GHOST_COUNT; i++) {
            move_ghost(game, &game->ghosts[i]);
        }
        resolve_collisions(game);
    }
}

int game_is_finished(const Game *game)
{
    return game->state == GAME_QUIT;
}

int game_ghosts_are_frightened(const Game *game)
{
    return game->power_ticks > 0;
}

int game_power_is_blinking(const Game *game)
{
    return game->power_ticks > 0 &&
           game->power_ticks <= POWER_BLINK_TICKS &&
           (game->power_ticks / 4) % 2 == 0;
}

int game_ghost_move_interval(const Game *game)
{
    int collected;
    int interval = 4;

    if (game->initial_pellets <= 0) {
        return 3;
    }

    collected = game->initial_pellets - game->pellets_remaining;

    if (collected > game->initial_pellets / 3) {
        interval = 3;
    }

    if (collected > (game->initial_pellets * 2) / 3 || game->tick > 900) {
        interval = 2;
    }

    if (game->power_ticks > 0) {
        interval += 1;
    }

    return interval;
}

const char *game_state_label(GameState state)
{
    switch (state) {
    case GAME_RUNNING:
        return "RUNNING";
    case GAME_READY:
        return "READY";
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
