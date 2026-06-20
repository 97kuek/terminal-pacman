$ErrorActionPreference = "Stop"

$buildDir = "build"
$target = Join-Path $buildDir "test.exe"
$sources = @(
    "tests/test_game.c",
    "src/game.c",
    "src/pathfind.c"
)

if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

$gcc = Get-Command gcc -ErrorAction SilentlyContinue
if (-not $gcc) {
    throw "gcc was not found. Install MinGW-w64 or build with another C compiler."
}

& $gcc.Source -std=c99 -Wall -Wextra -pedantic -O2 @sources -o $target
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

& $target
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}
