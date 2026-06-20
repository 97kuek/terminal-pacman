# Specification

## Goal

Implement a Pac-Man-like terminal game in C that runs on Windows, Linux, and macOS.

The initial version prioritizes portability, simple controls, readable source code, and a complete playable loop over visual complexity.

## Platform Support

- Windows
- Linux
- macOS

The game should avoid third-party libraries in the first version. Platform-specific terminal handling is allowed behind a small abstraction layer.

## Display

The first version uses ASCII-only rendering.

| Entity | Character |
| --- | --- |
| Wall | `#` |
| Pellet | `.` |
| Empty tile | space |
| Player | `C` |
| Ghost | `G` |

The game renders in a terminal using a fixed-size map. The screen includes the map and a compact status line with score, lives, and state.

## Controls

| Key | Action |
| --- | --- |
| `W` / Up arrow | Move up |
| `A` / Left arrow | Move left |
| `S` / Down arrow | Move down |
| `D` / Right arrow | Move right |
| `P` | Pause / resume |
| `Q` | Quit |

Letter controls are required. Arrow keys are preferred if they can be supported cleanly across platforms.

## Gameplay

- The player starts with 3 lives.
- Pellets increase the score when collected.
- Clearing all pellets wins the game.
- Touching a ghost costs one life and resets the player and ghosts to their spawn positions.
- Reaching 0 lives ends the game.
- Ghosts move randomly in the first version.
- Ghosts cannot pass through walls.
- The player cannot pass through walls.

## Game Loop

The game uses a fixed-timestep loop:

1. Poll input without blocking.
2. Update player direction.
3. Move the player.
4. Move ghosts on their own interval.
5. Resolve collisions.
6. Render the frame.
7. Sleep until the next tick.

## Portability Requirements

Terminal and input behavior should be isolated behind platform-specific functions:

- Initialize terminal mode.
- Restore terminal mode before exit.
- Poll for key input without blocking.
- Clear/redraw screen.
- Sleep for a small number of milliseconds.

Windows can use console APIs or `_kbhit` / `_getch`. Unix-like systems can use `termios`, `select`, and `usleep` or `nanosleep`.

## Non-Goals For First Version

- Unicode graphics
- Sound
- Menus beyond a simple start/end screen
- Configurable maps
- Save files
- Advanced ghost AI
- Networking

