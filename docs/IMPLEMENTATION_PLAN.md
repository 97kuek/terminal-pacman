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
- ランダムな敵移動
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
- 59 x 23 の広いステージ

## フェーズ 7: 検証

確認済み:

- Windows + MinGW-w64 GCC でビルドできる。
- 通常終了時に端末終了処理を通る。
- `SIGINT` / `SIGTERM` 時も通常終了ルートへ寄せる。

未確認または今後確認したい項目:

- Linux でのビルド
- macOS でのビルド
- 実機プレイでの壁抜けなし確認
- すべてのドットを収集できることの確認
- 敵接触時にライフが減ることの確認

## 現在のファイル構成

```text
terminal-pacman/
  AGENTS.md
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
  src/
    main.c
    game.c
    game.h
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
- 最初のマップはソースコード内に固定で持つ。
- 敵 AI は小マップに向く BFS ベースで実装する。
- ステージファイルを優先し、読み込み失敗時は内蔵ステージへフォールバックする。
- ハイスコアはカレントディレクトリの `terminal-pacman.score` に保存する。
- ステージは全画面プレイを前提に 59 x 23 とする。
- HUD は目的、凡例、進捗、状態、操作を常時表示する。

## 次の実装候補

優先度付きの改善案は [ROADMAP.md](ROADMAP.md) にまとめる。
敵 AI の設計判断は [AI_DESIGN.md](AI_DESIGN.md) にまとめる。
