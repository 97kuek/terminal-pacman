#ifdef _WIN32

#include "platform.h"

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

typedef struct BeepRequest {
    DWORD frequency;
    DWORD duration_ms;
} BeepRequest;

static DWORD WINAPI beep_thread_proc(LPVOID parameter)
{
    BeepRequest *request = (BeepRequest *)parameter;

    if (request != NULL) {
        Beep(request->frequency, request->duration_ms);
        free(request);
    }

    return 0;
}

int platform_init(void)
{
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    DWORD written;
    const char *init_sequence = "\x1b[?25l\x1b[2J\x1b[H";

    if (console != INVALID_HANDLE_VALUE && GetConsoleMode(console, &mode)) {
        mode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(console, mode);
    }

    SetConsoleOutputCP(CP_UTF8);
    if (console != INVALID_HANDLE_VALUE &&
        WriteConsoleA(console, init_sequence, (DWORD)strlen(init_sequence), &written, NULL)) {
        return 1;
    }

    fputs(init_sequence, stdout);
    fflush(stdout);
    return 1;
}

void platform_shutdown(void)
{
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;
    const char *shutdown_sequence = "\x1b[0m\x1b[?25h\n";

    if (console != INVALID_HANDLE_VALUE) {
        if (WriteConsoleA(console, shutdown_sequence, (DWORD)strlen(shutdown_sequence), &written, NULL)) {
            return;
        }
    }

    fputs(shutdown_sequence, stdout);
    fflush(stdout);
}

InputKey platform_poll_input(void)
{
    int ch;

    if (!_kbhit()) {
        return INPUT_NONE;
    }

    ch = _getch();
    if (ch == 0 || ch == 224) {
        ch = _getch();
        switch (ch) {
        case 72:
            return INPUT_UP;
        case 80:
            return INPUT_DOWN;
        case 75:
            return INPUT_LEFT;
        case 77:
            return INPUT_RIGHT;
        default:
            return INPUT_NONE;
        }
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

void platform_play(SoundId sound)
{
    BeepRequest *request;
    HANDLE thread;
    DWORD frequency;
    DWORD duration_ms;

    switch (sound) {
    case SND_PELLET:
        frequency = 988;
        duration_ms = 15;
        break;
    case SND_POWER:
        frequency = 440;
        duration_ms = 90;
        break;
    case SND_EAT_GHOST:
        frequency = 1245;
        duration_ms = 60;
        break;
    case SND_FRUIT:
        frequency = 1047;
        duration_ms = 70;
        break;
    case SND_PULSE:
        frequency = 196;
        duration_ms = 130;
        break;
    case SND_DEATH:
        frequency = 196;
        duration_ms = 280;
        break;
    case SND_CLEAR:
        frequency = 1319;
        duration_ms = 180;
        break;
    default:
        return;
    }

    request = (BeepRequest *)malloc(sizeof(*request));
    if (request == NULL) {
        return;
    }

    request->frequency = frequency;
    request->duration_ms = duration_ms;

    thread = CreateThread(NULL, 0, beep_thread_proc, request, 0, NULL);
    if (thread == NULL) {
        free(request);
        return;
    }

    CloseHandle(thread);
}

void platform_present(const char *frame)
{
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;
    size_t length;

    if (frame == NULL) {
        return;
    }

    length = strlen(frame);
    if (console != INVALID_HANDLE_VALUE) {
        if (WriteConsoleA(console, frame, (DWORD)length, &written, NULL)) {
            return;
        }
    }

    fwrite(frame, 1, length, stdout);
    fflush(stdout);
}

void platform_term_size(int *cols, int *rows)
{
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;

    if (console == INVALID_HANDLE_VALUE || !GetConsoleScreenBufferInfo(console, &info)) {
        return;
    }

    if (cols != NULL) {
        *cols = info.srWindow.Right - info.srWindow.Left + 1;
    }
    if (rows != NULL) {
        *rows = info.srWindow.Bottom - info.srWindow.Top + 1;
    }
}

void platform_sleep_ms(int milliseconds)
{
    Sleep((DWORD)milliseconds);
}

#endif
