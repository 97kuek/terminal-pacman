#ifndef _WIN32

#include "platform.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

static struct termios original_terminal;
static int terminal_configured = 0;

static int read_byte_nonblocking(void)
{
    unsigned char ch;
    fd_set set;
    struct timeval timeout;

    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    if (select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout) <= 0) {
        return -1;
    }

    if (read(STDIN_FILENO, &ch, 1) != 1) {
        return -1;
    }

    return (int)ch;
}

int platform_init(void)
{
    struct termios raw;

    if (tcgetattr(STDIN_FILENO, &original_terminal) != 0) {
        return 0;
    }

    raw = original_terminal;
    raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
        return 0;
    }

    fputs("\033[?25l\033[H\033[J", stdout);
    fflush(stdout);
    terminal_configured = 1;
    return 1;
}

void platform_shutdown(void)
{
    if (terminal_configured) {
        fputs("\x1b[0m\x1b[?25h\n", stdout);
        fflush(stdout);
        tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal);
        terminal_configured = 0;
    }
}

InputKey platform_poll_input(void)
{
    int ch = read_byte_nonblocking();

    if (ch < 0) {
        return INPUT_NONE;
    }

    if (ch == 27) {
        int second = read_byte_nonblocking();
        int third = read_byte_nonblocking();

        if (second == '[') {
            switch (third) {
            case 'A':
                return INPUT_UP;
            case 'B':
                return INPUT_DOWN;
            case 'C':
                return INPUT_RIGHT;
            case 'D':
                return INPUT_LEFT;
            default:
                return INPUT_NONE;
            }
        }

        return INPUT_NONE;
    }

    switch (ch) {
    case 'w':
    case 'W':
        return INPUT_UP;
    case 's':
    case 'S':
        return INPUT_DOWN;
    case 'a':
    case 'A':
        return INPUT_LEFT;
    case 'd':
    case 'D':
        return INPUT_RIGHT;
    case 'r':
    case 'R':
        return INPUT_RESTART;
    case 'p':
    case 'P':
        return INPUT_PAUSE;
    case ' ':
        return INPUT_PULSE;
    case 'q':
    case 'Q':
        return INPUT_QUIT;
    default:
        return INPUT_NONE;
    }
}

static int g_sound_enabled = 1;

void platform_set_sound_enabled(int enabled)
{
    g_sound_enabled = enabled ? 1 : 0;
}

void platform_play(SoundId sound)
{
    (void)sound;
    if (!g_sound_enabled) {
        return;
    }
    fputc('\a', stdout);
    fflush(stdout);
}

void platform_present(const char *frame)
{
    size_t length;
    size_t offset = 0;

    if (frame == NULL) {
        return;
    }

    length = strlen(frame);
    while (offset < length) {
        ssize_t written = write(STDOUT_FILENO, frame + offset, length - offset);

        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        if (written == 0) {
            break;
        }

        offset += (size_t)written;
    }
}

void platform_term_size(int *cols, int *rows)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != 0) {
        return;
    }

    if (cols != NULL) {
        *cols = ws.ws_col;
    }
    if (rows != NULL) {
        *rows = ws.ws_row;
    }
}

void platform_sleep_ms(int milliseconds)
{
    struct timespec delay;

    if (milliseconds < 0) {
        milliseconds = 0;
    }

    delay.tv_sec = milliseconds / 1000;
    delay.tv_nsec = (long)(milliseconds % 1000) * 1000000L;
    nanosleep(&delay, NULL);
}

#endif
