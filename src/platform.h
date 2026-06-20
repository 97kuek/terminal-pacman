#ifndef TERMINAL_PACMAN_PLATFORM_H
#define TERMINAL_PACMAN_PLATFORM_H

typedef enum InputKey {
    INPUT_NONE = 0,
    INPUT_UP,
    INPUT_DOWN,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_RESTART,
    INPUT_PAUSE,
    INPUT_QUIT
} InputKey;

int platform_init(void);
void platform_shutdown(void);
InputKey platform_poll_input(void);
void platform_clear_screen(void);
void platform_finish_frame(void);
void platform_sleep_ms(int milliseconds);

#endif
