# Implementation Plan

## Phase 1: Repository Foundation

- Add project documentation.
- Add cross-platform design notes.
- Add license and agent instructions.
- Initialize Git repository.
- Create GitHub repository once authentication is available.

## Phase 2: Build Skeleton

- Create `src/` layout.
- Add a small C build path that works on Windows, Linux, and macOS.
- Prefer a simple `Makefile` for Unix-like systems.
- Add a Windows build script, likely `build.ps1`, for MSVC or MinGW.
- Keep compiler flags strict enough to catch mistakes.

## Phase 3: Terminal Abstraction

Create a small platform layer for:

- Non-blocking input.
- Terminal setup and teardown.
- Screen clearing or cursor repositioning.
- Sleeping by milliseconds.

Planned files:

- `src/platform.h`
- `src/platform_win.c`
- `src/platform_unix.c`

## Phase 4: Core Game Model

Represent the game with simple C structs:

- Tile map.
- Player position and direction.
- Ghost positions and directions.
- Score.
- Lives.
- Game state.

Planned files:

- `src/game.h`
- `src/game.c`

## Phase 5: Rendering

Render the current game state as ASCII.

The renderer should be dumb: it reads the game state and prints a frame. Game rules should stay out of rendering code.

Planned files:

- `src/render.h`
- `src/render.c`

## Phase 6: Gameplay Loop

Implement:

- Input handling.
- Movement and collision against walls.
- Pellet collection.
- Random ghost movement.
- Life loss and reset.
- Win and game-over states.
- Pause and quit.

## Phase 7: Verification

Manual checks:

- Build on Windows.
- Build on Linux or WSL.
- Build on macOS if available.
- Confirm terminal mode is restored after normal quit and interrupt paths where practical.
- Confirm player cannot move through walls.
- Confirm all pellets can be collected.
- Confirm ghost collision reduces lives.

Automated tests are optional for the first version, but pure game-rule functions should be kept testable so tests can be added later.

## Initial File Layout

```text
terminal-pacman/
  AGENTS.md
  LICENSE
  README.md
  SPEC.md
  docs/
    IMPLEMENTATION_PLAN.md
  src/
    main.c
    game.c
    game.h
    render.c
    render.h
    platform.h
    platform_win.c
    platform_unix.c
```

## Open Decisions

- Exact compiler support matrix: MSVC, MinGW, Clang, GCC.
- Whether to include arrow-key support in the first implementation or treat it as a follow-up.
- Whether the first map is hard-coded in source or loaded from a plain text file.

