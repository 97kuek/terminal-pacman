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
    INPUT_QUIT,
    INPUT_PULSE
} InputKey;

typedef enum SoundId {
    SND_PELLET = 0,
    SND_POWER,
    SND_EAT_GHOST,
    SND_FRUIT,
    SND_PULSE,
    SND_DEATH,
    SND_CLEAR
} SoundId;

int platform_init(void);
void platform_shutdown(void);
InputKey platform_poll_input(void);
void platform_play(SoundId sound);
void platform_set_sound_enabled(int enabled);
void platform_present(const char *frame);
void platform_term_size(int *cols, int *rows);
void platform_sleep_ms(int milliseconds);

#endif
