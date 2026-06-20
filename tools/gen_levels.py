#!/usr/bin/env python3
"""Generate dense, narrow-corridor Pac-Man mazes.

Walls are placed as solid blocks separated by 1-wide corridors. Corridors are
built as full horizontal rows + full vertical columns, so they always cross and
the whole maze is guaranteed connected. We validate that with a flood fill
before emitting anything. Output: levels/stageN.txt and a C array for game.c.
"""
import random
from collections import deque

W, H = 45, 21

# (label, open rows, open cols, carve seed, connector probability)
STAGES = [
    ("Stage 1", range(1, H - 1, 4), range(1, W - 1, 4), 11, 0.30),
    ("Stage 2", range(1, H - 1, 4), range(1, W - 1, 3), 22, 0.22),
    ("Stage 3", range(1, H - 1, 3), range(1, W - 1, 4), 33, 0.35),
]


def build(open_rows, open_cols, seed, prob):
    open_rows = sorted(set(open_rows))
    open_cols = sorted(set(open_cols))
    open_row_set = set(open_rows)
    open_col_set = set(open_cols)
    grid = [["#"] * W for _ in range(H)]
    for y in range(1, H - 1):
        for x in range(1, W - 1):
            if y in open_row_set or x in open_col_set:
                grid[y][x] = "."

    # Carve extra shortcuts through wall blocks to break up the grid. We only
    # ever connect two existing corridors (open a full middle row or column of a
    # block), so connectivity is preserved while the layout gains variety.
    rng = random.Random(seed)
    for ra, rb in zip(open_rows, open_rows[1:]):
        for ca, cb in zip(open_cols, open_cols[1:]):
            if rng.random() < prob:  # horizontal shortcut
                my = (ra + rb) // 2
                for x in range(ca, cb + 1):
                    grid[my][x] = "."
            if rng.random() < prob:  # vertical shortcut
                mx = (ca + cb) // 2
                for y in range(ra, rb + 1):
                    grid[y][mx] = "."
    return grid


def open_cells(grid):
    return {(x, y) for y in range(H) for x in range(W) if grid[y][x] != "#"}


def reachable(grid, start):
    seen = {start}
    q = deque([start])
    while q:
        x, y = q.popleft()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nx, ny = x + dx, y + dy
            if 0 <= nx < W and 0 <= ny < H and grid[ny][nx] != "#" and (nx, ny) not in seen:
                seen.add((nx, ny))
                q.append((nx, ny))
    return seen


def nearest_open(grid, cx, cy, taken):
    best = None
    best_d = 1e9
    for y in range(1, H - 1):
        for x in range(1, W - 1):
            if grid[y][x] == "#" or (x, y) in taken:
                continue
            d = (x - cx) ** 2 + (y - cy) ** 2
            if d < best_d:
                best_d = d
                best = (x, y)
    return best


def decorate(grid):
    taken = set()
    # power pellets near the four corners of the open area
    for cx, cy in ((1, 1), (W - 2, 1), (1, H - 2), (W - 2, H - 2)):
        p = nearest_open(grid, cx, cy, taken)
        grid[p[1]][p[0]] = "o"
        taken.add(p)
    # ghosts near center (keep in sync with GHOST_COUNT in game.h)
    ghosts = []
    for cx, cy in (
        (W // 2, H // 2 - 2),
        (W // 2 - 5, H // 2),
        (W // 2 + 5, H // 2),
        (W // 2, H // 2 + 2),
    ):
        g = nearest_open(grid, cx, cy, taken)
        grid[g[1]][g[0]] = "G"
        taken.add(g)
        ghosts.append(g)
    # player spawn near bottom center
    p = nearest_open(grid, W // 2, H - 3, taken)
    grid[p[1]][p[0]] = "C"
    taken.add(p)
    return p, ghosts


def carve_tunnel(grid):
    """Open the left/right border on one central corridor row. Movement wraps
    horizontally, so this becomes a warp tunnel. Both mouths stay reachable
    from the interior, so connectivity is unaffected."""
    cy = H // 2
    candidates = [
        y for y in range(1, H - 1)
        if grid[y][1] != "#" and grid[y][W - 2] != "#"
    ]
    if not candidates:
        return None
    row = min(candidates, key=lambda y: abs(y - cy))
    grid[row][0] = "."
    grid[row][W - 1] = "."
    return row


def main():
    import os

    os.makedirs("levels", exist_ok=True)
    c_blocks = []
    for i, (name, rows, cols, seed, prob) in enumerate(STAGES, start=1):
        grid = build(rows, cols, seed, prob)
        player, ghosts = decorate(grid)
        tunnel_row = carve_tunnel(grid)

        # validate: every non-wall cell reachable from the player spawn
        all_open = open_cells(grid)
        reach = reachable(grid, player)
        missing = all_open - reach
        assert not missing, f"{name}: {len(missing)} unreachable cells e.g. {list(missing)[:5]}"
        for g in ghosts:
            assert g in reach, f"{name}: ghost {g} unreachable"

        lines = ["".join(r) for r in grid]
        with open(f"levels/stage{i}.txt", "w", newline="\n") as f:
            f.write("\n".join(lines) + "\n")

        pellets = sum(row.count(".") for row in lines)
        powers = sum(row.count("o") for row in lines)
        print(f"{name}: {pellets} pellets, {powers} power, player {player}, "
              f"ghosts {ghosts}, tunnel row {tunnel_row}")

        body = ",\n".join('        "' + ln + '"' for ln in lines)
        c_blocks.append("    {\n" + body + "\n    }")

    with open("tools/builtin_levels.inc", "w", newline="\n") as f:
        f.write("static const char *BUILTIN_LEVELS[LEVEL_COUNT][MAP_HEIGHT] = {\n")
        f.write(",\n".join(c_blocks))
        f.write("\n};\n")
    print("wrote tools/builtin_levels.inc")


if __name__ == "__main__":
    main()
