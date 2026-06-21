# モード制への移行 設計・実装計画

ステージ進行型から **モード制**へ移行する。決定事項（ユーザー合意）:

- **タイムアタック**: 固定時間スコア型（制限時間内にスコア最大化）。
- **エンドレスのライフ**: ローグライト（1 ミスで終了）。
- **学習**: オンライン Q 学習（プレイ中にゴーストがプレイヤーの癖へ適応）。
- **迷路供給**: ジェネレータを C に移植して無限生成。
- 既存の 3 ステージは **Classic モード**として残す（おすすめ）。
- 難易度 Easy/Normal/Hard は各モードの基本パラメータとして併存（おすすめ）。

## モード仕様

| モード | 終了条件 | 迷路 | ライフ | スコア |
| --- | --- | --- | --- | --- |
| Classic | 3 ステージクリアで `GAME_WON` | `levels/*.txt`（既存） | 3 | 既存 |
| Endless | 1 ミスで `GAME_OVER`（ローグライト） | C 生成・食べ尽くすたび再生成、難易度逓増 | 1 | 累積（モード別ハイスコア） |
| Time Attack | 制限時間 0 で終了 | C 生成・再生成 | 実質無制限（捕獲＝短い復帰、時間は進む） | 累積 |

- Endless の難易度逓増: `mazes_cleared` に応じて敵速度・Elroy 閾値・scatter 短縮。
- Time Attack: 既定 120 秒（1200 tick）。捕獲時は短い READY を挟んで復帰（時間は止めない）。

## アーキテクチャ

### 新規モジュール `src/maze.{h,c}`（codex 担当）
`tools/gen_levels.py` のロジック（格子＋シード付きカービング＋トンネル＋スポーン＋BFS 連結検証）を C へ移植。ゲーム型に非依存。

```c
typedef struct MazeSpawns {
    int player_x, player_y;
    int ghost_x[8], ghost_y[8];
    int ghost_count;
    int tunnel_row;
} MazeSpawns;

/* out は height 行 × (width+1) ストライドの NUL 終端グリッド。'#','.','o',' ' を書く。
   左右端にトンネル開口を 1 本掘る。seed で RNG、variant で見た目を変える。
   連結（全 '.'/'o' とスポーンが到達可能）を保証できたら 1、できなければ 0。*/
int maze_generate(char *out, int stride, int width, int height,
                  unsigned int seed, int variant, int ghost_count,
                  MazeSpawns *spawns);
```

### ゲーム側（Claude 担当）
- `GameMode { MODE_CLASSIC, MODE_ENDLESS, MODE_TIMEATTACK }`。
- `Game` 追加: `mode`, `time_left`, `maze_seed`, `mazes_cleared`, `high_scores[3]`, メニュー用 `menu_field`/`menu_mode`。
- レベル供給の分岐: Classic は既存 `load_level`、Endless/TimeAttack は `maze_generate` を呼ぶ新 `load_generated_level`。
- 終了/クリア処理をモード別に（`advance_or_win` を分岐、Time Attack のタイマー、Endless の再生成＋ランプ）。
- メニュー UX: Up/Down で「モード/難易度」フィールド移動、Left/Right で値変更、Space で開始。
- スコア保存: `terminal-pacman.score` に 3 モード分（後方互換で 1〜3 整数を読む）。

## フェーズ

- **P1（完了）**: モード選択 UI・Classic/Endless/TimeAttack の枠組み・`src/maze.*`（codex）・モード別ハイスコア・難易度併存。Endless はスクリプト式ランプ。
- **P2（完了）**: オンライン Q 学習ゴースト（`src/qghost.*`、codex）。Endless の ghost 0 が学習者として A* の代わりに Q 方策で動く（マゼンタ表示）。
- **P3（任意）**: オフライン RL・モード別リーダーボード・スプリント型タイムアタック。

## 学習（P2）に関する設計メモ
- A* が追跡を最適化済みのため、Q 学習の価値は「プレイヤーの癖の先読み」。
- 状態（粗く）: ゴーストから見たプレイヤーの相対象限・プレイヤー進行方向・近傍の分岐。行動: 交差点での進行方向。
- 報酬: 「プレイヤーへの距離短縮 −（理不尽化を避けるため）過度な張り付きにペナルティ」。純 C の表型 Q で 1 プレイ内学習。
- 学習はオプトイン（Endless のみ）。Classic/TimeAttack は決定的 AI を維持。

## codex との分担
- **codex**: `src/maze.h` / `src/maze.c`（上記 API）。`build.ps1` / `Makefile` への追加はせず Claude 側で行う。テストは `tests/` に追加（後続）。
- **Claude**: `game.*` / `main.c` / `render.c` / ビルド配線 / メニュー / スコア / モード分岐。
