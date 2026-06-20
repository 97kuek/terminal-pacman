# terminal-pacman

A small Pac-Man-like game implemented in C for the terminal.

The first target is a portable ASCII version that runs on Windows, Linux, and macOS without third-party runtime dependencies.

## Status

Initial playable version implemented.

See [SPEC.md](SPEC.md) and [docs/IMPLEMENTATION_PLAN.md](docs/IMPLEMENTATION_PLAN.md).

## Build

### Windows

Use MinGW-w64 GCC:

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
```

Run:

```powershell
.\build\terminal-pacman.exe
```

### Linux / macOS

```sh
make
./build/terminal-pacman
```

## Controls

| Key | Action |
| --- | --- |
| `W` / Up arrow | Move up |
| `A` / Left arrow | Move left |
| `S` / Down arrow | Move down |
| `D` / Right arrow | Move right |
| `P` | Pause / resume |
| `Q` | Quit |

