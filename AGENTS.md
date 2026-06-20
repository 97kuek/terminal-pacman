# Agent Instructions

This repository contains a portable C terminal game.

## Development Priorities

- Keep the first version simple, portable, and playable.
- Prefer standard C and small platform-specific adapters over third-party libraries.
- Keep Windows, Linux, and macOS behavior in sync.
- Use ASCII rendering unless the specification changes.
- Avoid unrelated refactors while implementing gameplay.

## Expected Workflow

- Read `SPEC.md` before changing gameplay behavior.
- Read `docs/IMPLEMENTATION_PLAN.md` before adding new modules.
- Keep platform-specific code isolated behind `src/platform.h`.
- Keep game rules independent from terminal rendering where practical.
- Update documentation when behavior or build commands change.

## Build And Test Notes

- Do not assume one operating system only.
- Prefer commands that work in the current shell.
- If adding tests later, keep them focused on pure game-state logic.

