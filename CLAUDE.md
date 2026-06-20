# CLAUDE.md

このファイルは Claude Code / AI エージェントがこのリポジトリで作業するためのガイドです。人間向けの導入は [README.md](README.md)、開発フローは [CONTRIBUTING.md](CONTRIBUTING.md)、エージェント向け方針は [AGENTS.md](AGENTS.md) を参照してください。

## プロジェクト概要

外部ライブラリに依存しない C99 製のターミナル版パックマン。Windows / Linux / macOS で動く。表示は ASCII グリフ + ANSI カラー、固定間隔のゲームループで進行する。

## ビルドと実行

Windows（MinGW-w64 GCC）:

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
.\build\terminal-pacman.exe
```

Linux / macOS:

```sh
make
./build/terminal-pacman
```

- コンパイルは常に `-std=c99 -Wall -Wextra -pedantic` で**警告ゼロ**を保つ（`build.ps1` / `Makefile` 両方）。
- **再ビルド前に実行中の `terminal-pacman.exe` を必ず終了する**。起動したままだと Windows で exe がロックされ、リンク時に `Permission denied` になる（`Get-Process terminal-pacman | Stop-Process -Force`）。
- ハイスコアはカレントディレクトリの `terminal-pacman.score` に保存される（`.gitignore` 済み）。

## アーキテクチャ

責務分離が最重要の原則:

- **ゲームルールは描画・端末から独立**させる（`game.c`）。
- **OS 依存処理は `platform.h` の API の裏に閉じ込める**（`platform_win.c` / `platform_unix.c`）。
- **描画はゲーム状態を読んで出力するだけ**（`render.c`、ルールを持たない）。

| ファイル | 役割 |
| --- | --- |
| `src/main.c` | エントリポイント、固定間隔ゲームループ（`FRAME_MS`）、シグナル処理 |
| `src/game.h` / `src/game.c` | ゲーム状態と全ルール（移動・衝突・スコア・敵AI・ウェーブ・チャージ等）。`MAP_WIDTH` / `MAP_HEIGHT` / `GHOST_COUNT` もここ |
| `src/pathfind.h` / `src/pathfind.c` | 敵追跡用の A* 経路探索（`astar_next`）。汎用グリッド関数でゲーム型に非依存。**最大 64x32 セル** |
| `src/render.h` / `src/render.c` | 1 フレームを 1 バッファに組み立て、ANSI カラー・横2倍スケール・中央寄せして `platform_present` で一括出力 |
| `src/platform.h` | 端末/OS API の境界（入力・VT初期化・一括描画・端末サイズ・サウンド・スリープ） |
| `src/platform_win.c` | Windows 実装（`_kbhit`/`_getch`、VT 有効化、`WriteConsoleA`、別スレッド `Beep`） |
| `src/platform_unix.c` | Unix 実装（`termios`/`select`、ANSI、`write`、端末ベル） |
| `levels/stage*.txt` | ステージ定義（読み込み優先、失敗時は `game.c` の内蔵配列にフォールバック） |
| `tools/gen_levels.py` | 迷路ジェネレータ（連結性検証つき）。`levels/*.txt` と内蔵配列を生成 |

## ゲームの主要要素（現状）

- 迷路: 45x21、密で狭い通路。
- 敵: 4 体。性格分け（直線追跡 / 待ち伏せ / 挟み込み / 臆病）。追跡経路は A*。
- 散開 / 追跡ウェーブ（`SCATTER_TICKS` / `CHASE_TICKS`）。終盤は追跡型が加速（Cruise Elroy）。
- 連続捕食ボーナス（200→400→800→1600）、ボーナスフルーツ `F`、スコアポップアップ。
- **スタシス・パルス（本作独自）**: ペレットでチャージし、満タンで `Space` を押すと全敵を一定時間凍結。
- 効果音: 外部ライブラリ非依存（Windows は別スレッド `Beep`、Unix は端末ベル）。

設計判断の詳細は [docs/AI_DESIGN.md](docs/AI_DESIGN.md)、仕様は [SPEC.md](SPEC.md)、改善優先度は [docs/ROADMAP.md](docs/ROADMAP.md)。

## レベル（迷路）を変更するときの手順

`levels/*.txt`（読み込み優先）と `game.c` の `BUILTIN_LEVELS`（フォールバック）は**必ず同期**させる。手で両方を書き換えるとずれるので、ジェネレータを使う:

```sh
python tools/gen_levels.py          # levels/*.txt と tools/builtin_levels.inc を生成
```

`tools/builtin_levels.inc` の中身を `game.c` の `BUILTIN_LEVELS` 配列へ差し込む（この差し込みはコミット対象外の中間ファイル経由で行う）。マップ寸法を変えたら:

1. `tools/gen_levels.py` の `W, H` を変更。
2. `src/game.h` の `MAP_WIDTH` / `MAP_HEIGHT` を一致させる。
3. ペレットが全て到達可能であること（ジェネレータが flood-fill で自動検証）。
4. A* の上限 64x32 を超えないこと。

## 調整しやすい定数

- 難易度・テンポ・能力: `src/game.c` 冒頭（`POWER_TICKS`, `SCATTER_TICKS`, `CHASE_TICKS`, `CLYDE_RANGE`, `CHARGE_MAX`, `STASIS_TICKS`, `FRUIT_*`, `GHOST_COMBO_MAX`）。
- フレーム間隔: `src/main.c` の `FRAME_MS`。
- 効果音の音程: `src/platform_win.c` の `platform_play`。
- 配色: `src/render.c` の `C_*` マクロ（256色 SGR）。

## 規約・注意

- 外部ライブラリを足さない（標準 C と小さな OS 別アダプタで解決する）。
- 表示は ASCII グリフ + ANSI カラー。新しいグリフや Unicode を足すときは SPEC を更新する。
- 仕様・挙動・ビルド手順を変えたら、対応するドキュメント（SPEC / ROADMAP / AI_DESIGN / README / 本ファイル）も更新する。
- `codex exec` で codex と分担する場合、codex はサンドボックスの別 OS ユーザーで動くため、後で git が "dubious ownership" を警告することがある（`git config --global --add safe.directory <repo>` で解消）。
