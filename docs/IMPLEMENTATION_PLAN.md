# Implementation Plan

## Phase 1: Repository Foundation

- Done: Add project documentation.
- Done: Add cross-platform design notes.
- Done: Add license and agent instructions.
- Done: Initialize Git repository.
- Done: Push to existing GitHub repository.

## Phase 2: Build Skeleton

- Done: Create `src/` layout.
- Done: Add a small C build path for Windows, Linux, and macOS.
- Done: Add `Makefile` for Unix-like systems and compatible Make environments.
- Done: Add `build.ps1` for Windows with MinGW-w64 GCC.
- Done: Keep compiler flags strict enough to catch mistakes.

## Phase 3: Terminal Abstraction

Done: Create a small platform layer for:

- Non-blocking input.
- Terminal setup and teardown.
- Screen clearing or cursor repositioning.
- Sleeping by milliseconds.

Files:

- `src/platform.h`
- `src/platform_win.c`
- `src/platform_unix.c`

## Phase 4: Core Game Model

Done: Represent the game with simple C structs:

- Tile map.
- Player position and direction.
- Ghost positions and directions.
- Score.
- Lives.
- Game state.

Files:

- `src/game.h`
- `src/game.c`

## Phase 5: Rendering

Done: Render the current game state as ASCII.

The renderer should be dumb: it reads the game state and prints a frame. Game rules should stay out of rendering code.

Files:

- `src/render.h`
- `src/render.c`

## Phase 6: Gameplay Loop

Done: Implement:

- Input handling.
- Movement and collision against walls.
- Pellet collection.
- Random ghost movement.
- Life loss and reset.
- Win and game-over states.
- Pause and quit.

## Phase 7: Verification

Manual checks:

- Done: Build on Windows with MinGW-w64 GCC.
- Build on Linux or WSL.
- Build on macOS if available.
- Partially done: Terminal mode is restored on normal quit and handled SIGINT/SIGTERM paths.
- Pending manual runtime check: Confirm player cannot move through walls.
- Pending manual runtime check: Confirm all pellets can be collected.
- Pending manual runtime check: Confirm ghost collision reduces lives.

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

- Compiler support matrix: MinGW-w64 GCC is verified on Windows. GCC/Clang via `make` are intended for Linux/macOS.
- Arrow-key support is included in the first implementation.
- The first map is hard-coded in source.

## Follow-Up Candidates

- Add automated tests for pure game-state logic.
- Add optional map loading from text files.
- Add configurable tick speed and ghost speed.
- Add smarter ghost movement after the random baseline is stable.
