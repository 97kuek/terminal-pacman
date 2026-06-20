#ifndef _WIN32

#include "platform.h"

#include <stdio.h>
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
        fputs("\033[?25h\n", stdout);
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
    case 'q':
    case 'Q':
        return INPUT_QUIT;
    default:
        return INPUT_NONE;
    }
}

void platform_clear_screen(void)
{
    fputs("\033[H", stdout);
}

void platform_finish_frame(void)
{
    fputs("\033[J", stdout);
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
