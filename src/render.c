#include "render.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "platform.h"

/* Each map cell is drawn this many terminal columns wide so the maze fills the
 * window and keeps a roughly square aspect ratio (terminal cells are tall). */
#define SCALE_X 2
#define BOARD_COLS (MAP_WIDTH * SCALE_X)

/* ANSI SGR colors. */
#define C_RESET   "\x1b[0m"
#define C_WALL    "\x1b[38;5;33m"   /* bright blue maze */
#define C_PELLET  "\x1b[38;5;223m"  /* warm dim dots */
#define C_POWER   "\x1b[1;38;5;215m"/* glowing power pellet */
#define C_PLAYER  "\x1b[1;38;5;226m"/* bright yellow */
#define C_GHOST   "\x1b[1;38;5;196m"/* red ghost */
#define C_FRIGHT  "\x1b[1;38;5;39m" /* edible ghost, blue */
#define C_BLINK   "\x1b[1;38;5;231m"/* power ending, white flash */
#define C_FRUIT   "\x1b[1;38;5;201m"/* bonus fruit, magenta */
#define C_STASIS  "\x1b[1;38;5;51m" /* frozen ghosts, icy cyan */
#define C_METER   "\x1b[1;38;5;45m" /* charge meter fill */
#define C_DIM     "\x1b[38;5;238m"  /* empty meter cells */
#define C_TITLE   "\x1b[1;38;5;51m"
#define C_LABEL   "\x1b[38;5;245m"
#define C_VALUE   "\x1b[1;38;5;231m"
#define C_GOOD    "\x1b[1;38;5;46m"
#define C_WARN    "\x1b[1;38;5;208m"
#define C_POPUP   "\x1b[1;38;5;213m"

static char g_frame[1 << 16];
static size_t g_len;
static int g_mono = 0;

void render_set_mono(int mono)
{
    g_mono = mono ? 1 : 0;
}

/* Remove SGR colour sequences (ESC [ ... m) in place, keeping cursor/clear
 * escapes (H/J/K) so the layout survives. Used by --mono. */
static void strip_colors(void)
{
    size_t r = 0;
    size_t w = 0;

    while (r < g_len) {
        if (g_frame[r] == 0x1b && r + 1 < g_len && g_frame[r + 1] == '[') {
            size_t k = r + 2;

            while (k < g_len &&
                   !((g_frame[k] >= 'A' && g_frame[k] <= 'Z') ||
                     (g_frame[k] >= 'a' && g_frame[k] <= 'z'))) {
                k++;
            }
            if (k < g_len && g_frame[k] == 'm') {
                r = k + 1; /* drop the whole colour sequence */
                continue;
            }
        }
        g_frame[w++] = g_frame[r++];
    }
    g_len = w;
    g_frame[w] = '\0';
}

static void buf_reset(void)
{
    g_len = 0;
    g_frame[0] = '\0';
}

static void buf_put(const char *s)
{
    size_t n = strlen(s);
    size_t avail;

    if (g_len > sizeof(g_frame) - 1) {
        g_len = sizeof(g_frame) - 1;
    }

    avail = sizeof(g_frame) - 1 - g_len;
    if (n > avail) {
        n = avail; /* truncate rather than overflow */
    }

    memcpy(g_frame + g_len, s, n);
    g_len += n;
    g_frame[g_len] = '\0';
}

static void buf_putf(const char *fmt, ...)
{
    char tmp[256];
    va_list args;

    va_start(args, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);
    buf_put(tmp);
}

static void buf_spaces(int count)
{
    int i;

    for (i = 0; i < count; i++) {
        buf_put(" ");
    }
}

/* Clear to end of line, reset color, newline, then re-pad for the next row. */
static void line_break(int left_pad)
{
    buf_put(C_RESET "\x1b[K\r\n");
    buf_spaces(left_pad);
}

static char ghost_glyph(const Game *game, int x, int y, const char **color)
{
    int i;

    for (i = 0; i < GHOST_COUNT; i++) {
        char glyph;

        if (game->ghosts[i].pos.x != x || game->ghosts[i].pos.y != y) {
            continue;
        }
        if (game_power_is_blinking(game)) {
            *color = C_BLINK;
            glyph = 'g';
        } else if (game_ghosts_are_frightened(game)) {
            *color = C_FRIGHT;
            glyph = 'g';
        } else {
            *color = C_GHOST;
            glyph = 'G';
        }
        if (game->stasis_ticks > 0) {
            *color = C_STASIS; /* frozen by a stasis pulse */
        }
        return glyph;
    }

    *color = NULL;
    return '\0';
}

/* Returns the color + the (already scaled) two-character cell rendering. */
static void cell_render(const Game *game, int x, int y, const char **color, char out[3])
{
    char ghost_color_glyph;
    const char *gcolor = NULL;
    char tile;

    if (game->player.pos.x == x && game->player.pos.y == y) {
        if (game->state == GAME_DYING) {
            /* blink between a yellow C and a red X while dying */
            int on = (game->dying_ticks / 2) % 2 == 0;
            *color = on ? C_PLAYER : C_GHOST;
            out[0] = on ? 'C' : 'X';
        } else {
            *color = C_PLAYER;
            out[0] = 'C';
        }
        out[1] = ' ';
        out[2] = '\0';
        return;
    }

    ghost_color_glyph = ghost_glyph(game, x, y, &gcolor);
    if (ghost_color_glyph != '\0') {
        *color = gcolor;
        out[0] = ghost_color_glyph;
        out[1] = ' ';
        out[2] = '\0';
        return;
    }

    if (game->fruit_ticks > 0 && game->fruit_pos.x == x && game->fruit_pos.y == y) {
        /* blink the fruit so it draws the eye */
        *color = (game->tick / 3) % 2 == 0 ? C_FRUIT : C_WARN;
        out[0] = 'F';
        out[1] = ' ';
        out[2] = '\0';
        return;
    }

    tile = game->map[y][x];
    switch (tile) {
    case '#':
        *color = C_WALL;
        out[0] = '#';
        out[1] = '#';
        break;
    case '.':
        *color = C_PELLET;
        out[0] = '.';
        out[1] = ' ';
        break;
    case 'o':
        *color = C_POWER;
        out[0] = 'o';
        out[1] = ' ';
        break;
    default:
        *color = NULL;
        out[0] = ' ';
        out[1] = ' ';
        break;
    }
    out[2] = '\0';
}

static void render_hud(const Game *game, int left_pad, int level_percent)
{
    buf_spaces(left_pad);
    buf_put(C_TITLE "  PAC-MAN  " C_RESET);
    if (game->mode == MODE_CLASSIC) {
        buf_putf(C_LABEL "  " C_VALUE "Classic" C_LABEL "  Stage " C_VALUE "%d/%d   "
                 C_WARN "[%s]", game->level_index + 1, game->level_count,
                 game_difficulty_label(game->difficulty));
    } else if (game->mode == MODE_ENDLESS) {
        buf_putf(C_LABEL "  " C_VALUE "Endless" C_LABEL "  Maze " C_VALUE "%d   "
                 C_WARN "[%s]", game->mazes_cleared + 1,
                 game_difficulty_label(game->difficulty));
    } else {
        buf_putf(C_LABEL "  " C_VALUE "Time Attack" C_LABEL "  Mazes " C_VALUE "%d   "
                 C_WARN "[%s]", game->mazes_cleared,
                 game_difficulty_label(game->difficulty));
    }
    line_break(left_pad);

    buf_putf(C_LABEL "Score " C_VALUE "%-6d  " C_LABEL "High " C_VALUE "%-6d  ",
             game->score, game->high_score);
    if (game->mode == MODE_TIMEATTACK) {
        int secs = (game->time_left + 9) / 10;
        const char *tcol = secs <= 10 ? C_GHOST : C_GOOD;
        buf_putf(C_LABEL "Time " "%s%d:%02d", tcol, secs / 60, secs % 60);
    } else {
        buf_putf(C_LABEL "Lives " C_GOOD "%d", game->lives);
    }
    buf_putf(C_LABEL "  Progress " C_VALUE "%d%%", level_percent);
    if (game->popup_ticks > 0 && (game->popup_ticks / 2) % 2 == 0) {
        buf_putf("   " C_POPUP "+%d!", game->popup_value);
    }
    line_break(left_pad);

    buf_putf(C_LABEL "Pellets " C_VALUE "%d/%d   " C_LABEL "State " C_VALUE "%-9s  "
             C_LABEL "Power " C_VALUE "%-3d",
             game->pellets_remaining, game->initial_pellets,
             game_state_label(game->state), game->power_ticks);
    line_break(left_pad);

    {
        int filled = game_charge_percent(game) * 12 / 100;
        int k;

        buf_put(C_LABEL "Pulse " C_DIM "[");
        for (k = 0; k < 12; k++) {
            buf_put(k < filled ? C_METER "|" : C_DIM ".");
        }
        buf_put(C_DIM "]  ");
        if (game_charge_is_ready(game)) {
            buf_put(C_STASIS "READY - press Space to freeze ghosts");
        } else {
            buf_put(C_LABEL "charging...");
        }
    }
    line_break(left_pad);

    buf_put(C_PLAYER "C" C_LABEL " you   " C_GHOST "G" C_LABEL " ghost   "
            C_FRIGHT "g" C_LABEL " edible   " C_PELLET "." C_LABEL " pellet   "
            C_POWER "o" C_LABEL " power   " C_FRUIT "F" C_LABEL " fruit   "
            C_WALL "#" C_LABEL " wall");
    line_break(left_pad);

    buf_put(C_LABEL "WASD/arrows move   Space pulse   P pause   R restart   Q quit");
    line_break(left_pad);
    line_break(left_pad);
}

static void render_footer(const Game *game, int left_pad)
{
    line_break(left_pad);

    if (game->state == GAME_READY) {
        buf_putf(C_WARN "READY %d" C_LABEL "  pick a direction and go!",
                 (game->countdown_ticks + 9) / 10);
    } else if (game->state == GAME_PAUSED) {
        buf_put(C_WARN "PAUSED" C_LABEL "  press P to resume.");
    } else if (game->state == GAME_DYING) {
        buf_put(C_GHOST "OUCH!" C_LABEL "  caught by a ghost...");
    } else if (game->state == GAME_WON) {
        buf_put(C_GOOD "CLEAR!" C_LABEL "  all stages done. R restart / Q quit.");
    } else if (game->state == GAME_OVER) {
        if (game->mode == MODE_TIMEATTACK) {
            buf_put(C_WARN "TIME UP!" C_LABEL "  R restart / Q quit.");
        } else {
            buf_put(C_GHOST "GAME OVER" C_LABEL "  R restart / Q quit.");
        }
    } else if (game->state == GAME_QUIT) {
        buf_put(C_LABEL "Quit requested.");
    } else if (game->stasis_ticks > 0) {
        buf_put(C_STASIS "STASIS" C_LABEL "  ghosts are frozen - press your attack!");
    } else if (game_ghosts_are_frightened(game)) {
        if (game_power_is_blinking(game)) {
            buf_put(C_BLINK "POWER ENDING" C_LABEL "  ghosts turn dangerous soon!");
        } else {
            buf_put(C_FRIGHT "POWER MODE" C_LABEL "  chase the blue ghosts for bonus!");
        }
        if (game->ghost_combo > 0) {
            buf_putf(C_GOOD "   combo x%d", game->ghost_combo);
        }
    } else if (game->ai_mode == AI_CHASE) {
        buf_put(C_GHOST "CHASE" C_LABEL "  the ghosts are hunting - keep moving!");
    } else {
        buf_put(C_GOOD "SCATTER" C_LABEL "  ghosts retreat to their corners. Press on!");
    }
    buf_put(C_RESET "\x1b[K");
}

/* Big ASCII banner so the title reads large in a roomy terminal. */
static const char *MENU_BANNER[5] = {
    " ____   _    ____   __  __    _    _   _ ",
    "|  _ \\ / \\  / ___| |  \\/  |  / \\  | \\ | |",
    "| |_) / _ \\| |     | |\\/| | / _ \\ |  \\| |",
    "|  __/ ___ \\ |___  | |  | |/ ___ \\| |\\  |",
    "|_|  /_/   \\_\\____| |_|  |_/_/   \\_\\_| \\_|"
};

/* Horizontal 3-stop gradient (blue -> purple -> pink), like the gradient
 * wordmarks in Claude Code / Gemini CLI. t runs 0..1 across the banner. */
static void gradient_rgb(double t, int *r, int *g, int *b)
{
    static const int stop[3][3] = {
        { 80, 160, 255 }, { 150, 110, 230 }, { 235, 110, 160 }
    };
    int i;
    double seg;

    if (t < 0.0) {
        t = 0.0;
    }
    if (t > 1.0) {
        t = 1.0;
    }
    if (t < 0.5) {
        i = 0;
        seg = t / 0.5;
    } else {
        i = 1;
        seg = (t - 0.5) / 0.5;
    }
    *r = (int)(stop[i][0] + (stop[i + 1][0] - stop[i][0]) * seg);
    *g = (int)(stop[i][1] + (stop[i + 1][1] - stop[i][1]) * seg);
    *b = (int)(stop[i][2] + (stop[i + 1][2] - stop[i][2]) * seg);
}

/* Emit the banner with a per-column truecolor gradient. The caller pads the
 * first row; each later row is padded here via line_break(). */
static void buf_put_banner(int left_pad)
{
    int width = (int)strlen(MENU_BANNER[0]);
    int row;
    int col;

    for (row = 0; row < 5; row++) {
        const char *line = MENU_BANNER[row];
        int len = (int)strlen(line);

        if (row > 0) {
            line_break(left_pad);
        }
        for (col = 0; col < len; col++) {
            char ch[2];

            if (line[col] == ' ') {
                buf_put(" ");
                continue;
            }
            {
                int r;
                int g;
                int b;
                double t = width > 1 ? (double)col / (width - 1) : 0.0;

                gradient_rgb(t, &r, &g, &b);
                buf_putf("\x1b[1;38;2;%d;%d;%dm", r, g, b);
            }
            ch[0] = line[col];
            ch[1] = '\0';
            buf_put(ch);
        }
    }
}

/* One selectable row: a label, then the options with the chosen one bracketed
 * and the active row marked with '>'. */
static void render_menu_row(const Game *game, const char *label, int active,
                            int selected, int count,
                            const char *(*name_of)(int))
{
    int i;

    buf_putf("%s%s %-11s", active ? C_VALUE : C_LABEL, active ? ">" : " ", label);
    for (i = 0; i < count; i++) {
        if (i == selected) {
            buf_putf("%s[%s] ", active ? C_GOOD : C_VALUE, name_of(i));
        } else {
            buf_putf(C_DIM "%s  ", name_of(i));
        }
    }
    (void)game;
}

static const char *menu_mode_name(int m) { return game_mode_label(m); }
static const char *menu_diff_name(int d) { return game_difficulty_label(d); }

static void render_menu(const Game *game, int cols, int rows)
{
    static const char *mode_desc[3] = {
        "three hand-built stages, win at the end",
        "one life - mazes never stop, ghosts ramp up",
        "120 seconds - rack up the highest score"
    };
    int banner_w = (int)strlen(MENU_BANNER[0]);
    int pad = (cols - banner_w) / 2;
    int top = (rows - 18) / 2;
    int i;
    int sel_mode = game->menu_mode;

    if (sel_mode < 0 || sel_mode >= MODE_COUNT) {
        sel_mode = 0;
    }
    if (pad < 0) {
        pad = 0;
    }
    if (top < 0) {
        top = 0;
    }

    buf_reset();
    buf_put("\x1b[H");
    for (i = 0; i < top; i++) {
        buf_put("\x1b[K\r\n");
    }

    /* Only the very first line is padded explicitly; every later line is
     * padded by the preceding line_break(). */
    buf_spaces(pad);
    buf_put_banner(pad);
    line_break(pad);
    line_break(pad);

    render_menu_row(game, "Mode:", game->menu_field == 0, sel_mode,
                    MODE_COUNT, menu_mode_name);
    line_break(pad);
    render_menu_row(game, "Difficulty:", game->menu_field == 1, game->menu_index,
                    DIFF_HARD + 1, menu_diff_name);
    line_break(pad);
    line_break(pad);

    buf_putf(C_LABEL "%s", mode_desc[sel_mode]);
    line_break(pad);
    line_break(pad);

    buf_put(C_LABEL "Up/Down switch row,  Left/Right change,  Space start,  Q quit");
    line_break(pad);
    line_break(pad);
    buf_putf(C_LABEL "High score (%s): " C_VALUE "%d",
             game_mode_label(sel_mode), game->high_scores[sel_mode]);
    line_break(pad);

    buf_put(C_RESET "\x1b[J");
}

void render_game(const Game *game)
{
    int x;
    int y;
    int cols = 80;
    int rows = 24;
    int left_pad;
    int top_pad;
    int content_rows;
    int level_percent = 0;
    const char *cur_color = NULL;

    if (game->initial_pellets > 0) {
        level_percent = ((game->initial_pellets - game->pellets_remaining) * 100) /
                        game->initial_pellets;
    }

    platform_term_size(&cols, &rows);

    if (game->state == GAME_MENU) {
        render_menu(game, cols, rows);
        if (g_mono) {
            strip_colors();
        }
        platform_present(g_frame);
        return;
    }

    left_pad = (cols - BOARD_COLS) / 2;
    if (left_pad < 0) {
        left_pad = 0;
    }

    content_rows = 8 /* hud */ + MAP_HEIGHT + 2 /* footer */;
    top_pad = (rows - content_rows) / 2;
    if (top_pad < 0) {
        top_pad = 0;
    }

    buf_reset();
    buf_put("\x1b[H");
    for (y = 0; y < top_pad; y++) {
        buf_put("\x1b[K\r\n");
    }

    render_hud(game, left_pad, level_percent);

    for (y = 0; y < MAP_HEIGHT; y++) {
        /* The preceding line_break already padded this row; adding more here
         * would double the indent and push the board off-centre. */
        cur_color = NULL;
        for (x = 0; x < MAP_WIDTH; x++) {
            const char *color = NULL;
            char cell[3];

            cell_render(game, x, y, &color, cell);
            if (color == NULL) {
                color = C_RESET;
            }
            if (color != cur_color) {
                buf_put(color);
                cur_color = color;
            }
            buf_put(cell);
        }
        line_break(left_pad);
    }

    render_footer(game, left_pad);

    /* Clear anything left below the content, then flush in a single write. */
    buf_put(C_RESET "\x1b[J");
    if (g_mono) {
        strip_colors();
    }
    platform_present(g_frame);
}
