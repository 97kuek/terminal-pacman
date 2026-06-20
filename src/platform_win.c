#ifdef _WIN32

#include "platform.h"

#include <conio.h>
#include <windows.h>

int platform_init(void)
{
    return 1;
}

void platform_shutdown(void)
{
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
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    DWORD cells;
    DWORD written;
    COORD home = {0, 0};

    if (console == INVALID_HANDLE_VALUE || !GetConsoleScreenBufferInfo(console, &info)) {
        return;
    }

    cells = (DWORD)info.dwSize.X * (DWORD)info.dwSize.Y;
    FillConsoleOutputCharacterA(console, ' ', cells, home, &written);
    FillConsoleOutputAttribute(console, info.wAttributes, cells, home, &written);
    SetConsoleCursorPosition(console, home);
}

void platform_sleep_ms(int milliseconds)
{
    Sleep((DWORD)milliseconds);
}

#endif

