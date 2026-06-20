#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pathfind.h"

#define POWER_TICKS 80
#define POWER_BLINK_TICKS 25
#define READY_TICKS 20
#define GHOST_EAT_SCORE 200
#define GHOST_COMBO_MAX 4
#define POPUP_TICKS 14
#define FRUIT_TICKS 90
#define FRUIT_SPAWNS 2
#define CLYDE_RANGE 8        /* shy ghost backs off inside this distance */
#define CHARGE_PER_POWER 6   /* bonus charge from a power pellet */
#define STASIS_TICKS 14      /* how long a pulse freezes the ghosts */
#define DEATH_TICKS 16       /* length of the death animation */
#define CLEAR_BONUS_PER_LIFE 200 /* stage-clear bonus per remaining life */
#define SCORE_FILE "terminal-pacman.score"
#define LEVEL_COUNT 3

static const int FRUIT_VALUES[LEVEL_COUNT] = {100, 300, 500};

/* Per-difficulty tuning. interval_delta is added to the base ghost tempo
 * (higher = slower ghosts); chase/scatter set the wave lengths; charge_max
 * sets how many pellets fill the stasis pulse. */
typedef struct DifficultySettings {
    int interval_delta;
    int chase_ticks;
    int scatter_ticks;
    int charge_max;
    const char *label;
} DifficultySettings;

static const DifficultySettings DIFFICULTY[3] = {
    { +1, 170, 60, 28, "Easy" },
    {  0, 220, 45, 40, "Normal" },
    { -1, 300, 30, 52, "Hard" }
};

static const DifficultySettings *diff_of(const Game *game)
{
    int d = game->difficulty;

    if (d < DIFF_EASY || d > DIFF_HARD) {
        d = DIFF_NORMAL;
    }
    return &DIFFICULTY[d];
}

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
        "#############################################",
        "#o.........................................o#",
        "#.###.###.###.###.###.###.###.#.#.###.###.###",
        "#.###.###.###.....###.........#.#.###.###.###",
        "#.###.###.###.###.###.###.###.#.#.###.###.###",
        "#...........................................#",
        "#.#.#.#.#.#.#.###.###.###.###.#.#.###.###.###",
        "#.....#.#.........###.###.###.#.#.###.###.###",
        "#.#.#.#.#.#.#.###.###G###.###.#.#.###.###.###",
        "...........................G.................",
        "#.#.#.#.#.###.###G###.#.#.###.###.###.#.#.###",
        "#.#.#.....###.###.###.G...###.###.....#.#.###",
        "#.#.#.#.#.###.###.###.#.#.###.###.###.#.#.###",
        "#...........................................#",
        "#.###.###.#.#.#.#.#.#.###.###.###.###.#.#.###",
        "#.....###.........#.#.###.................###",
        "#.###.###.#.#.#.#.#.#.###.###.###.###.#.#.###",
        "#.....................C....................o#",
        "#.###.###.###.###.###.###.###.###.###.###.###",
        "#o###.###.###.###.###.###.###.###.###.###.###",
        "#############################################"
    },
    {
        "#############################################",
        "#o.........................................o#",
        "#..#.##..#.##.##.##.##.##..#.##..#.##.##.##.#",
        "#..#.......##.##.##.##.##..#.##..#.##.##.##.#",
        "#..#.##..#.##.##.##.##.##..#.##..#.##.##.##.#",
        "#...........................................#",
        "#.##.##.##.##.##..#.##.##.##.##.##.##.##.##.#",
        "#.##.##.##.##.....#.......##.##.##.##.##.##.#",
        "#.##.##.##.##.##..#.##G##.##.##.##.##.##.##.#",
        "#..........................G................#",
        "..##.##.##.##.##.G#..#.##.##..#.##..#..#.##..",
        "#.##....##.##.##..#..#.##.##....##..#..#.##.#",
        "#.##.##.##.##.##..#..#G##.##..#.##..#..#.##.#",
        "#...........................................#",
        "#.##.##.##.##.##.##.##.##.##.##.##.##.##..#.#",
        "#.##.##.##.##.##.##.##.##....##....##.....#.#",
        "#.##.##.##.##.##.##.##.##.##.##.##.##.##..#.#",
        "#...........................................#",
        "#.##.##.##.##.##.##.##C##.##.##.##.##.##.##.#",
        "#o##.##.##.##.##.##.##.##.##.##.##.##.##.##o#",
        "#############################################"
    },
    {
        "#############################################",
        "#o.........................................o#",
        "#.###.#.#.###.#.#.###.###.###.###.....#.#.###",
        "#.###.#.#.###.#.#.###.###.###.###.#.#.#.#.###",
        "#...........................................#",
        "#.###.###.###.#.#.........###.............###",
        "#.###.###.###.#.#.###.#.#.###.#.#.#.#.###.###",
        "#...........................................#",
        "#.............###.....G...........###.###.###",
        "#.###.#.#.###.###.#.#.#.#.#.#.#.#.###.###.###",
        ".................G.........G.................",
        "#.###.....###.#.#.#.#.###.....#.#.###.....###",
        "#.###.#.#.###.#.#.#.#G###.###.#.#.###.#.#.###",
        "#...........................................#",
        "#.###.###.#.#.###.#.#.###.........#.#.###.###",
        "#.###.###.#.#.###.#.#.###.#.#.###.#.#.###.###",
        "#...........................................#",
        "#.###.###.###.###.###.###.........#.#.....###",
        "#.###.###.###.###.###C###.#.#.###.#.#.###.###",
        "#o.........................................o#",
        "#############################################"
    }
};

static const Position DEFAULT_PLAYER_SPAWN = {22, 17};
static const Position DEFAULT_GHOST_SPAWNS[GHOST_COUNT] = {
    {22, 8},
    {17, 10},
    {27, 10},
    {22, 12}
};

static const GhostBehavior GHOST_BEHAVIORS[GHOST_COUNT] = {
    GHOST_CHASER,
    GHOST_AMBUSH,
    GHOST_FLANK,
    GHOST_SHY
};

/* Home corners each ghost retreats to during AI_SCATTER. */
static const Position GHOST_SCATTER_CORNERS[GHOST_COUNT] = {
    {MAP_WIDTH - 3, 2},
    {2, 2},
    {MAP_WIDTH - 3, MAP_HEIGHT - 3},
    {2, MAP_HEIGHT - 3}
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

/* Wrap horizontally so warp tunnels (open cells on the left/right border)
 * connect. Non-tunnel border cells are walls, so wrapping there stays blocked. */
static Position wrap_position(Position pos)
{
    if (pos.x < 0) {
        pos.x = MAP_WIDTH - 1;
    } else if (pos.x >= MAP_WIDTH) {
        pos.x = 0;
    }
    return pos;
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
    game->player.desired_dir = DIR_NONE;

    game->stasis_ticks = 0;

    for (i = 0; i < GHOST_COUNT; i++) {
        game->ghosts[i].pos = game->ghosts[i].spawn;
        game->ghosts[i].dir = DIR_NONE;
        game->ghosts[i].behavior = GHOST_BEHAVIORS[i];
        game->ghosts[i].scatter = GHOST_SCATTER_CORNERS[i];
    }
}

static void add_charge(Game *game, int amount)
{
    game->charge += amount;
    if (game->charge > game->charge_max) {
        game->charge = game->charge_max;
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

static Position find_fruit_home(const Game *game);
static void set_popup(Game *game, Position pos, int value);

static void load_level(Game *game, int level_index)
{
    if (!load_level_file(game, level_index)) {
        apply_level_lines(game, BUILTIN_LEVELS[level_index], LEVEL_NAMES[level_index]);
    }

    game->level_index = level_index;
    game->level_count = LEVEL_COUNT;
    game->power_ticks = 0;
    game->tick = 0;
    game->ai_mode = AI_SCATTER;
    game->ai_timer = diff_of(game)->scatter_ticks;
    game->ghost_combo = 0;
    game->popup_ticks = 0;
    game->fruit_ticks = 0;
    game->fruit_spawns_left = FRUIT_SPAWNS;

    count_pellets(game);
    reset_actors(game);
    game->fruit_home = find_fruit_home(game);
    start_ready(game);
}

static void advance_or_win(Game *game)
{
    int bonus = game->lives * CLEAR_BONUS_PER_LIFE;

    if (bonus > 0) {
        game->score += bonus;
        set_popup(game, game->player.pos, bonus);
    }
    update_high_score(game);
    platform_play(SND_CLEAR);

    if (game->level_index + 1 < game->level_count) {
        load_level(game, game->level_index + 1);
        return;
    }

    game->state = GAME_WON;
}

static void set_popup(Game *game, Position pos, int value)
{
    game->popup_pos = pos;
    game->popup_value = value;
    game->popup_ticks = POPUP_TICKS;
}

static int is_spawn_cell(const Game *game, int x, int y)
{
    int i;

    if (game->player.spawn.x == x && game->player.spawn.y == y) {
        return 1;
    }
    for (i = 0; i < GHOST_COUNT; i++) {
        if (game->ghosts[i].spawn.x == x && game->ghosts[i].spawn.y == y) {
            return 1;
        }
    }
    return 0;
}

/* Pick a reachable open cell near the middle for bonus fruit to appear on. */
static Position find_fruit_home(const Game *game)
{
    int cx = MAP_WIDTH / 2;
    int cy = MAP_HEIGHT / 2;
    int radius;

    for (radius = 0; radius < MAP_WIDTH + MAP_HEIGHT; radius++) {
        int dx;
        int dy;

        for (dy = -radius; dy <= radius; dy++) {
            for (dx = -radius; dx <= radius; dx++) {
                int x = cx + dx;
                int y = cy + dy;
                Position p;

                if (x < 1 || x >= MAP_WIDTH - 1 || y < 1 || y >= MAP_HEIGHT - 1) {
                    continue;
                }
                if (game->map[y][x] == '#' || is_spawn_cell(game, x, y)) {
                    continue;
                }
                p.x = x;
                p.y = y;
                return p;
            }
        }
    }

    return game->player.spawn;
}

static void maybe_spawn_fruit(Game *game)
{
    int collected = game->initial_pellets - game->pellets_remaining;

    if (game->fruit_ticks > 0 || game->fruit_spawns_left <= 0 || game->initial_pellets <= 0) {
        return;
    }

    if ((game->fruit_spawns_left == 2 && collected >= game->initial_pellets / 4) ||
        (game->fruit_spawns_left == 1 && collected >= (game->initial_pellets * 3) / 4)) {
        game->fruit_pos = game->fruit_home;
        game->fruit_value = FRUIT_VALUES[game->level_index];
        game->fruit_ticks = FRUIT_TICKS;
        game->fruit_spawns_left--;
    }
}

static void collect_fruit(Game *game)
{
    if (game->fruit_ticks > 0 &&
        actors_overlap(game->player.pos, game->fruit_pos)) {
        game->score += game->fruit_value;
        set_popup(game, game->fruit_pos, game->fruit_value);
        game->fruit_ticks = 0;
        platform_play(SND_FRUIT);
        update_high_score(game);
    }
}

static void collect_pellet(Game *game)
{
    char *tile = &game->map[game->player.pos.y][game->player.pos.x];

    if (*tile == '.') {
        *tile = ' ';
        game->score += 10;
        game->pellets_remaining--;
        add_charge(game, 1);
        platform_play(SND_PELLET);
        update_high_score(game);
    } else if (*tile == 'o') {
        *tile = ' ';
        game->score += 50;
        game->pellets_remaining--;
        game->power_ticks = POWER_TICKS;
        game->ghost_combo = 0; /* fresh combo window each power pellet */
        add_charge(game, CHARGE_PER_POWER);
        platform_play(SND_POWER);
        update_high_score(game);
    }

    maybe_spawn_fruit(game);

    if (game->pellets_remaining <= 0) {
        advance_or_win(game);
    }
}

/* Start the death animation; the actual life loss happens when it ends. */
static void begin_dying(Game *game)
{
    game->power_ticks = 0;
    game->stasis_ticks = 0;
    game->dying_ticks = DEATH_TICKS;
    game->state = GAME_DYING;
    platform_play(SND_DEATH);
}

static void finish_dying(Game *game)
{
    game->lives--;
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
            int points;

            if (game->ghost_combo < GHOST_COMBO_MAX) {
                game->ghost_combo++;
            }
            points = GHOST_EAT_SCORE * (1 << (game->ghost_combo - 1));
            game->score += points;
            set_popup(game, game->ghosts[i].pos, points);
            platform_play(SND_EAT_GHOST);
            update_high_score(game);
            reset_ghost(&game->ghosts[i]);
        } else {
            begin_dying(game);
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
    Actor *player = &game->player;
    Position next;

    /* Buffered turn: if a queued direction is now possible, adopt it. This
     * lets the player line up a turn slightly before reaching the junction. */
    if (player->desired_dir != DIR_NONE) {
        next = wrap_position(step_position(player->pos, player->desired_dir));
        if (!is_wall(game, next)) {
            player->dir = player->desired_dir;
        }
    }

    if (player->dir == DIR_NONE) {
        return;
    }

    next = wrap_position(step_position(game->player.pos, player->dir));
    if (!is_wall(game, next)) {
        player->pos = next;
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

static int manhattan(Position a, Position b)
{
    int dx = a.x - b.x;
    int dy = a.y - b.y;

    return (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
}

static Direction delta_to_direction(int dx, int dy)
{
    if (dy < 0) {
        return DIR_UP;
    }
    if (dy > 0) {
        return DIR_DOWN;
    }
    if (dx < 0) {
        return DIR_LEFT;
    }
    if (dx > 0) {
        return DIR_RIGHT;
    }
    return DIR_NONE;
}

/* Snap an arbitrary target (which may be a wall or off-map) onto the closest
 * walkable cell so A* always has a reachable goal. */
static Position nearest_open_cell(const Game *game, Position target)
{
    int radius;

    if (target.x < 1) {
        target.x = 1;
    }
    if (target.x > MAP_WIDTH - 2) {
        target.x = MAP_WIDTH - 2;
    }
    if (target.y < 1) {
        target.y = 1;
    }
    if (target.y > MAP_HEIGHT - 2) {
        target.y = MAP_HEIGHT - 2;
    }

    for (radius = 0; radius < MAP_WIDTH + MAP_HEIGHT; radius++) {
        int dx;
        int dy;

        for (dy = -radius; dy <= radius; dy++) {
            for (dx = -radius; dx <= radius; dx++) {
                Position p;

                p.x = target.x + dx;
                p.y = target.y + dy;
                if (!is_wall(game, p)) {
                    return p;
                }
            }
        }
    }

    return game->player.spawn;
}

/* First A* step from the ghost toward a (snapped) target cell. */
static Direction astar_direction(const Game *game, Position from, Position target)
{
    PathPoint start;
    PathPoint goal;
    PathPoint next;
    Position open = nearest_open_cell(game, target);

    start.x = from.x;
    start.y = from.y;
    goal.x = open.x;
    goal.y = open.y;

    if (!astar_next(&game->map[0][0], MAP_WIDTH + 1, MAP_WIDTH, MAP_HEIGHT,
                    start, goal, &next)) {
        return DIR_NONE;
    }

    return delta_to_direction(next.x - from.x, next.y - from.y);
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

/* Inky-style flank: aim past the player using the chaser as a pivot, so this
 * ghost tends to cut off escape routes instead of trailing the pack. */
static Position flank_target(const Game *game)
{
    Position ahead = game->player.pos;
    Position blinky = game->ghosts[0].pos;
    Position target;
    int i;

    for (i = 0; i < 2; i++) {
        Position next = step_position(ahead, game->player.dir);

        if (game->player.dir == DIR_NONE || is_wall(game, next)) {
            break;
        }
        ahead = next;
    }

    target.x = 2 * ahead.x - blinky.x;
    target.y = 2 * ahead.y - blinky.y;
    return target;
}

static Position chase_target(const Game *game, const Actor *ghost)
{
    switch (ghost->behavior) {
    case GHOST_AMBUSH:
        return ambush_target(game);
    case GHOST_FLANK:
        return flank_target(game);
    case GHOST_SHY:
        if (manhattan(ghost->pos, game->player.pos) > CLYDE_RANGE) {
            return game->player.pos;
        }
        return ghost->scatter;
    case GHOST_CHASER:
    default:
        return game->player.pos;
    }
}

static int ghost_speed(const Game *game, const Actor *ghost)
{
    int interval = game_ghost_move_interval(game) + diff_of(game)->interval_delta;

    if (interval < 1) {
        interval = 1;
    }
    if (game->power_ticks > 0) {
        return interval + 1; /* sluggish while edible */
    }
    if (ghost->behavior == GHOST_CHASER &&
        game->pellets_remaining * 4 < game->initial_pellets) {
        interval -= 1; /* Cruise Elroy: relentless late in the stage */
    }
    if (interval < 1) {
        interval = 1;
    }
    return interval;
}

static void move_ghost(Game *game, Actor *ghost)
{
    Direction dir;

    if (game->power_ticks > 0) {
        dir = choose_frightened_direction(game, ghost);
    } else {
        Position target = (game->ai_mode == AI_SCATTER)
                              ? ghost->scatter
                              : chase_target(game, ghost);

        dir = astar_direction(game, ghost->pos, target);
        if (dir == DIR_NONE) {
            dir = random_valid_direction(game, ghost);
        }
    }

    ghost->dir = dir;
    if (dir != DIR_NONE) {
        ghost->pos = wrap_position(step_position(ghost->pos, dir));
    }
}

/* Reset score/lives/charge for a fresh run and load the starting stage. Keeps
 * the already-chosen difficulty, start level, and loaded high score. */
static void start_new_game(Game *game)
{
    game->score = 0;
    game->lives = 3;
    game->charge = 0;
    game->charge_max = diff_of(game)->charge_max;
    load_level(game, game->start_level);
}

void game_init(Game *game)
{
    int high_score = load_high_score();

    memset(game, 0, sizeof(*game));

    game->high_score = high_score;
    game->level_count = LEVEL_COUNT;
    game->difficulty = DIFF_NORMAL;
    game->menu_index = DIFF_NORMAL;
    game->start_level = 0;

    start_new_game(game);
}

void game_configure(Game *game, int difficulty, int start_level)
{
    if (difficulty < DIFF_EASY) {
        difficulty = DIFF_EASY;
    }
    if (difficulty > DIFF_HARD) {
        difficulty = DIFF_HARD;
    }
    if (start_level < 0) {
        start_level = 0;
    }
    if (start_level >= LEVEL_COUNT) {
        start_level = LEVEL_COUNT - 1;
    }

    game->difficulty = difficulty;
    game->menu_index = difficulty;
    game->start_level = start_level;
    start_new_game(game);
}

void game_show_menu(Game *game)
{
    game->state = GAME_MENU;
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

    if (game->state == GAME_MENU) {
        if (input == INPUT_UP) {
            game->menu_index = (game->menu_index + DIFF_HARD) % (DIFF_HARD + 1);
        } else if (input == INPUT_DOWN) {
            game->menu_index = (game->menu_index + 1) % (DIFF_HARD + 1);
        } else if (input == INPUT_PULSE || input == INPUT_RESTART) {
            game->difficulty = game->menu_index;
            start_new_game(game);
        }
        return;
    }

    if (input == INPUT_RESTART &&
        (game->state == GAME_WON || game->state == GAME_OVER || game->state == GAME_QUIT)) {
        start_new_game(game);
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

    if (input == INPUT_PULSE) {
        /* Stasis pulse: spend a full charge meter to freeze every ghost. */
        if (game->state == GAME_RUNNING && game->charge >= game->charge_max) {
            game->charge = 0;
            game->stasis_ticks = STASIS_TICKS;
            platform_play(SND_PULSE);
        }
        return;
    }

    if (game->state != GAME_RUNNING && game->state != GAME_READY) {
        return;
    }

    dir = input_to_direction(input);
    if (dir != DIR_NONE) {
        game->player.desired_dir = dir;
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

    if (game->state == GAME_DYING) {
        if (game->popup_ticks > 0) {
            game->popup_ticks--;
        }
        if (game->dying_ticks > 0) {
            game->dying_ticks--;
        }
        if (game->dying_ticks <= 0) {
            finish_dying(game);
        }
        return;
    }

    if (game->state != GAME_RUNNING) {
        return;
    }

    game->tick++;

    if (game->popup_ticks > 0) {
        game->popup_ticks--;
    }
    if (game->fruit_ticks > 0) {
        game->fruit_ticks--;
    }
    if (game->stasis_ticks > 0) {
        game->stasis_ticks--;
    }

    if (game->power_ticks > 0) {
        game->power_ticks--;
        if (game->power_ticks == 0) {
            game->ghost_combo = 0;
        }
    }

    move_player(game);
    collect_pellet(game);
    collect_fruit(game);

    if (game->state != GAME_RUNNING) {
        return;
    }

    resolve_collisions(game);

    if (game->state != GAME_RUNNING) {
        return;
    }

    /* Advance the scatter/chase wave (frozen while ghosts are frightened). */
    if (game->power_ticks == 0) {
        if (game->ai_timer > 0) {
            game->ai_timer--;
        }
        if (game->ai_timer <= 0) {
            game->ai_mode = (game->ai_mode == AI_CHASE) ? AI_SCATTER : AI_CHASE;
            game->ai_timer = (game->ai_mode == AI_CHASE)
                                 ? diff_of(game)->chase_ticks
                                 : diff_of(game)->scatter_ticks;
        }
    }

    /* Each ghost moves on its own cadence, so the pack arrives in waves.
     * A stasis pulse freezes every ghost in place until it wears off. */
    if (game->stasis_ticks == 0) {
        for (i = 0; i < GHOST_COUNT; i++) {
            if (game->tick % ghost_speed(game, &game->ghosts[i]) == 0) {
                move_ghost(game, &game->ghosts[i]);
            }
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

int game_charge_percent(const Game *game)
{
    int percent;

    if (game->charge_max <= 0) {
        return 0;
    }
    percent = (game->charge * 100) / game->charge_max;
    return percent > 100 ? 100 : percent;
}

int game_charge_is_ready(const Game *game)
{
    return game->charge_max > 0 && game->charge >= game->charge_max;
}

int game_power_is_blinking(const Game *game)
{
    return game->power_ticks > 0 &&
           game->power_ticks <= POWER_BLINK_TICKS &&
           (game->power_ticks / 4) % 2 == 0;
}

/* Base ghost tempo (ticks per step). Per-ghost modifiers (frightened slowdown,
 * Cruise Elroy speed-up) are applied in ghost_speed(). */
int game_ghost_move_interval(const Game *game)
{
    int collected;
    int interval = 3;

    if (game->initial_pellets <= 0) {
        return 2;
    }

    collected = game->initial_pellets - game->pellets_remaining;

    if (collected > game->initial_pellets / 3) {
        interval = 2;
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
    case GAME_DYING:
        return "DYING";
    case GAME_WON:
        return "YOU WIN";
    case GAME_OVER:
        return "GAME OVER";
    case GAME_QUIT:
        return "QUIT";
    case GAME_MENU:
        return "MENU";
    default:
        return "UNKNOWN";
    }
}

const char *game_difficulty_label(int difficulty)
{
    if (difficulty < DIFF_EASY || difficulty > DIFF_HARD) {
        difficulty = DIFF_NORMAL;
    }
    return DIFFICULTY[difficulty].label;
}
