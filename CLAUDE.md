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

テスト（純粋ロジック + A*）:

```sh
make test                          # Linux / macOS
powershell -File build_tests.ps1   # Windows
```

CI（`.github/workflows/ci.yml`）が push / PR ごとに Windows / Linux / macOS でビルドとテストを実行する。

起動オプション: `--level N` / `--difficulty easy|normal|hard` / `--speed MS` / `--no-sound` / `--mono` / `--help`。`--level` か `--difficulty` を渡すと起動メニューを省略。

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
| `src/maze.h` / `src/maze.c` | 迷路ジェネレータ（`gen_levels.py` の C 移植、連結性検証つき）。Endless / Time Attack のランタイム生成に使用 |
| `src/qghost.h` / `src/qghost.c` | 表型オンライン Q 学習コア（ε-greedy・Q 更新）。Endless の学習ゴースト（ghost 0、マゼンタ表示）が使用 |
| `src/qfeatures.h` / `src/qfeatures.c` | 学習器の状態符号化・合法判定・報酬（ゲームとオフライン学習ツールで共有） |
| `src/qtable_data.h` | `tools/qsim.c` が書き出す学習済み Q テーブル。Endless 開始時にウォームスタートで読み込む |
| `src/render.h` / `src/render.c` | 1 フレームを 1 バッファに組み立て、ANSI カラー・横2倍スケール・中央寄せして `platform_present` で一括出力 |
| `src/platform.h` | 端末/OS API の境界（入力・VT初期化・一括描画・端末サイズ・サウンド・スリープ） |
| `src/platform_win.c` | Windows 実装（`_kbhit`/`_getch`、VT 有効化、`WriteConsoleA`、別スレッド `Beep`） |
| `src/platform_unix.c` | Unix 実装（`termios`/`select`、ANSI、`write`、端末ベル） |
| `levels/stage*.txt` | ステージ定義（読み込み優先、失敗時は `game.c` の内蔵配列にフォールバック）。左右端の開口がワープトンネル |
| `tools/gen_levels.py` | 迷路ジェネレータ（連結性検証＋トンネル開口つき）。`levels/*.txt` と内蔵配列を生成 |
| `tools/qsim.c` | 学習器のオフライン学習・評価（`make qsim`）。捕獲率を計測し `src/qtable_data.h` を出力 |
| `tools/gen_banner.py` | メニューの ASCII バナーを等幅で組み立て（`render.c` の `MENU_BANNER`） |
| `tests/test_game.c` | 単体テスト（A*・迷路生成・Q 学習・モード・純粋ロジック）。`platform_play` をスタブしてリンク |
| `build_tests.ps1` / `Makefile (test)` | テストのビルド・実行 |
| `.github/workflows/ci.yml` | Windows / Linux / macOS の CI |

## ゲームの主要要素（現状）

- モード: Classic（3ステージ）/ Endless（ローグライト・無限生成・ランプ）/ Time Attack（120秒）。起動メニューまたは `--mode` で選択。ハイスコアはモード別（`high_scores[MODE_COUNT]`）。設計は [docs/MODES_DESIGN.md](docs/MODES_DESIGN.md)。
- 迷路: 45x21、密で狭い通路。Classic は `levels/*.txt`、Endless/Time Attack は `maze_generate` でランタイム生成。
- 敵: 4 体。性格分け（直線追跡 / 待ち伏せ / 挟み込み / 臆病）。追跡経路は A*。
- 散開 / 追跡ウェーブ（`SCATTER_TICKS` / `CHASE_TICKS`）。終盤は追跡型が加速（Cruise Elroy）。
- 連続捕食ボーナス（200→400→800→1600）、ボーナスフルーツ `F`、スコアポップアップ。
- **スタシス・パルス（本作独自）**: ペレットでチャージし、満タンで `Space` を押すと全敵を一定時間凍結。
- ターン先行入力、左右ワープトンネル、ミス時の死亡演出。
- 難易度（Easy/Normal/Hard、`game.c` の `DIFFICULTY` テーブル）で敵速度・ウェーブ長・チャージ量を切替。起動メニューまたは CLI で選択。
- ステージクリアボーナス（残ライフ依存）。
- 効果音: 外部ライブラリ非依存（Windows は別スレッド `Beep`、Unix は端末ベル）。`--no-sound` / `--mono` で無効化可。

設計判断の詳細は [docs/enemy-ai.md](docs/eneymy-ai.md)、仕様は [SPEC.md](SPEC.md)

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

- 難易度別の調整: `src/game.c` の `DIFFICULTY[]` テーブル（敵速度差・ウェーブ長・チャージ量・ラベル）。
- テンポ・能力: `src/game.c` 冒頭（`POWER_TICKS`, `CLYDE_RANGE`, `STASIS_TICKS`, `DEATH_TICKS`, `CLEAR_BONUS_PER_LIFE`, `FRUIT_*`, `GHOST_COMBO_MAX`）。
- フレーム間隔: `src/main.c` の `FRAME_MS`。
- 効果音の音程: `src/platform_win.c` の `platform_play`。
- 配色: `src/render.c` の `C_*` マクロ（256色 SGR）。

## 規約・注意

- 外部ライブラリを足さない（標準 C と小さな OS 別アダプタで解決する）。
- 表示は ASCII グリフ + ANSI カラー。新しいグリフや Unicode を足すときは SPEC を更新する。
- 仕様・挙動・ビルド手順を変えたら、対応するドキュメント（SPEC / ROADMAP / AI_DESIGN / README / 本ファイル）も更新する。
- `codex exec` で codex と分担する場合、codex はサンドボックスの別 OS ユーザーで動くため、後で git が "dubious ownership" を警告することがある（`git config --global --add safe.directory <repo>` で解消）。
