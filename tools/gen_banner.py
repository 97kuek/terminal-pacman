#!/usr/bin/env python3
"""Assemble the PAC-MAN menu banner from fixed-width letter blocks so every row
lines up. Prints the C array literal for src/render.c."""

P = [" ____  ", "|  _ \\ ", "| |_) |", "|  __/ ", "|_|    "]
A = ["    _    ", "   / \\   ", "  / _ \\  ", " / ___ \\ ", "/_/   \\_\\"]
C = ["  ____ ", " / ___|", "| |    ", "| |___ ", " \\____|"]
D = ["       ", "       ", " _____ ", "       ", "       "]
M = [" __  __ ", "|  \\/  |", "| |\\/| |", "| |  | |", "|_|  |_|"]
N = [" _   _ ", "| \\ | |", "|  \\| |", "| |\\  |", "|_| \\_|"]

letters = [P, A, C, D, M, A, N]
for L in letters:
    assert len({len(r) for r in L}) == 1, [len(r) for r in L]

rows = ["".join(L[r] for L in letters) for r in range(5)]
assert len({len(r) for r in rows}) == 1, [len(r) for r in rows]

print("width =", len(rows[0]))
print("static const char *MENU_BANNER[5] = {")
for i, r in enumerate(rows):
    esc = r.replace("\\", "\\\\").replace('"', '\\"')
    print('    "%s"%s' % (esc, "," if i < 4 else ""))
print("};")
print("---- preview ----")
for r in rows:
    print(r)
