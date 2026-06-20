$ErrorActionPreference = "Stop"

$buildDir = "build"
$target = Join-Path $buildDir "terminal-pacman.exe"
$sources = @(
    "src/main.c",
    "src/game.c",
    "src/render.c",
    "src/pathfind.c",
    "src/maze.c",
    "src/qghost.c",
    "src/platform_win.c"
)

if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

$gcc = Get-Command gcc -ErrorAction SilentlyContinue
if (-not $gcc) {
    throw "gcc was not found. Install MinGW-w64 or build with another C compiler."
}

& $gcc.Source -std=c99 -Wall -Wextra -pedantic -O2 @sources -o $target

Write-Host "Built $target"

