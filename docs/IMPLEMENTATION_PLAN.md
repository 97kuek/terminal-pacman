# 実装計画

## フェーズ 1: リポジトリ基盤

- 完了: プロジェクトドキュメントを追加する。
- 完了: クロスプラットフォーム方針を整理する。
- 完了: ライセンスとエージェント向け指示を追加する。
- 完了: Git リポジトリを初期化する。
- 完了: 既存の GitHub リポジトリへ push する。

## フェーズ 2: ビルド基盤

- 完了: `src/` 構成を作成する。
- 完了: Windows / Linux / macOS 向けの小さな C ビルド手順を追加する。
- 完了: Unix 系環境向けに `Makefile` を追加する。
- 完了: Windows 向けに MinGW-w64 GCC 用の `build.ps1` を追加する。
- 完了: 警告を拾いやすいコンパイルオプションを設定する。

## フェーズ 3: 端末平台レイヤー

完了: 以下の処理を平台レイヤーとして分離した。

- ノンブロッキング入力
- 端末モードの初期化と復元
- 画面クリア
- ミリ秒単位の待機

対象ファイル:

- `src/platform.h`
- `src/platform_win.c`
- `src/platform_unix.c`

## フェーズ 4: ゲームモデル

完了: 以下の状態を C の構造体で表現した。

- タイルマップ
- プレイヤーの位置と向き
- 敵の位置と向き
- スコア
- ライフ
- ゲーム状態

対象ファイル:

- `src/game.h`
- `src/game.c`

## フェーズ 5: 描画

完了: 現在のゲーム状態を ASCII で描画する。

描画処理はゲームルールを持たず、ゲーム状態を読んで画面へ出力するだけにしている。

対象ファイル:

- `src/render.h`
- `src/render.c`

## フェーズ 6: ゲームループ

完了: 以下を実装した。

- 入力処理
- 壁との衝突を考慮した移動
- ドット取得
- 敵移動（初期はランダム／追跡／先読み。フェーズ 8 で A* ベースへ刷新）
- ライフ減少と位置リセット
- クリア、ゲームオーバー、終了状態
- 一時停止
- リスタート操作
- パワーエサ
- 敵ごとの移動方針
- ステージ開始前とミス後のカウントダウン
- ドット収集量に応じた難易度上昇
- ハイスコア保存
- マップ読み込み
- パワーエサ終了前の点滅表示
- 表示のちらつき軽減
- 複数ステージ
- 全画面向け HUD
- 広いステージ（初期は 59 x 23。フェーズ 8 で 45 x 21 に縮小）

## フェーズ 7: 検証

確認済み:

- Windows + MinGW-w64 GCC でビルドできる（`-Wall -Wextra -pedantic` 警告ゼロ）。
- 通常終了時に端末終了処理を通る。
- `SIGINT` / `SIGTERM` 時も通常終了ルートへ寄せる。

未確認または今後確認したい項目:

- Linux でのビルド
- macOS でのビルド
- 実機プレイでの壁抜けなし確認

## フェーズ 8: 表示・AI・演出の刷新

完了:

- 描画を 1 フレーム一括書き込みに変更し、ちらつきを解消（`platform_present`）。
- ANSI カラー化（Windows は VT 処理を有効化）。横 2 倍スケール＋端末サイズに応じた中央寄せ。
- 迷路を密で狭い通路に刷新し、寸法を 45 x 21 に縮小。生成と連結性検証を `tools/gen_levels.py` に分離。
- 敵を 4 体に増やし、A*（`src/pathfind.c`）ベースの性格分け（直線追跡・待ち伏せ・挟み込み・臆病）を実装。
- 散開／追跡ウェーブと、終盤の追跡型高速化（Cruise Elroy）。
- 連続捕食ボーナス、ボーナスフルーツ、スコアポップアップ。
- 効果音（外部ライブラリ非依存：Windows は別スレッド `Beep`、Unix は端末ベル）。
- スタシス・パルス（ペレットでチャージし、`Space` で敵を一定時間凍結する独自要素）。

## 現在のファイル構成

```text
terminal-pacman/
  AGENTS.md
  CLAUDE.md
  CONTRIBUTING.md
  LICENSE
  Makefile
  README.md
  SPEC.md
  build.ps1
  docs/
    IMPLEMENTATION_PLAN.md
    AI_DESIGN.md
    ROADMAP.md
  levels/
    stage1.txt
    stage2.txt
    stage3.txt
  tools/
    gen_levels.py
  src/
    main.c
    game.c
    game.h
    pathfind.c
    pathfind.h
    render.c
    render.h
    platform.h
    platform_win.c
    platform_unix.c
```

## 決定済み事項

- Windows では MinGW-w64 GCC でのビルドを確認済み。
- Linux / macOS では GCC または Clang + `make` を想定する。
- 矢印キー対応は初期実装に含める。
- マップは `levels/*.txt` を優先し、読み込み失敗時はソース内蔵の配列へフォールバックする。両者は `tools/gen_levels.py` で生成して同期させる。
- 敵 AI の追跡経路は A*（`src/pathfind.c`、最大 64x32）で求める。逃走は BFS 距離表を使う。
- ハイスコアはカレントディレクトリの `terminal-pacman.score` に保存する。
- ステージ寸法は 45 x 21（密で狭い通路）。
- 表示は ASCII グリフ + ANSI カラー。HUD は目的、凡例、進捗、状態、操作、チャージメーター、ウェーブ状態を表示する。
- サウンドは外部ライブラリを使わず OS 別アダプタで鳴らす。

## 次の実装候補

優先度付きの改善案は [ROADMAP.md](ROADMAP.md) にまとめる。
敵 AI の設計判断は [AI_DESIGN.md](AI_DESIGN.md) にまとめる。
