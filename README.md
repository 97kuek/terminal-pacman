# terminal-pacman

C 言語で実装した、ターミナル上で動くパックマン風ゲームです。

外部ランタイム依存なしで Windows / Linux / macOS で動く、ANSI カラー表示のターミナルゲームです（表示文字は ASCII グリフ、色付けに ANSI エスケープを使います）。

## 状態

プレイ可能版を実装済みです。A* ベースの敵 AI、効果音、独自要素のスタシス・パルスまで入っています。

関連ドキュメント:

- [SPEC.md](SPEC.md): ゲーム仕様
- [CLAUDE.md](CLAUDE.md): AI エージェント / 開発者向けの全体ガイド
- [docs/IMPLEMENTATION_PLAN.md](docs/IMPLEMENTATION_PLAN.md): 実装計画と進捗
- [docs/ROADMAP.md](docs/ROADMAP.md): 今後の改善案
- [docs/AI_DESIGN.md](docs/AI_DESIGN.md): 敵 AI の設計
- [CONTRIBUTING.md](CONTRIBUTING.md): 開発・貢献ガイド

## ビルド

### Windows

MinGW-w64 GCC を使います。

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
```

実行:

```powershell
.\build\terminal-pacman.exe
```

### Linux / macOS

```sh
make
./build/terminal-pacman
```

## テスト

純粋なゲームロジックと A* の単体テストがあります。

```sh
make test                 # Linux / macOS
powershell -File build_tests.ps1   # Windows
```

push / PR ごとに GitHub Actions が Windows / Linux / macOS でビルドとテストを実行します。

## 起動オプション

```text
--level N           ステージ N から開始（1-3、メニューを省略）
--difficulty NAME   easy | normal | hard（メニューを省略）
--speed MS          フレーム間隔（ミリ秒、既定 100）
--no-sound          効果音を無効化
--mono              色を使わないモノクロ表示
--help              ヘルプを表示
```

例: `./build/terminal-pacman --difficulty hard --no-sound`

オプションなしで起動すると、最初に難易度選択メニューが出ます（`W`/`S` か矢印で選び、`Space` で開始）。

## 操作方法

| キー | 操作 |
| --- | --- |
| `W` / 上矢印 | 上へ移動 |
| `A` / 左矢印 | 左へ移動 |
| `S` / 下矢印 | 下へ移動 |
| `D` / 右矢印 | 右へ移動 |
| `Space` | スタシス・パルス（チャージ満タン時に敵を凍結）／メニューで決定 |
| `P` | 一時停止 / 再開 |
| `R` | クリア / ゲームオーバー後にリスタート |
| `Q` | 終了 |

曲がりたい方向は交差点の手前で入力しておけます（先行入力）。

## 現在のゲーム内容

- ドットをすべて食べるとクリア
- パワーエサ `o` を食べると一定時間だけ敵を食べられる（終了前は点滅表示）
- 敵に触れるとライフが減少し、ライフが 0 でゲームオーバー
- 敵は 4 体。直線追跡型・待ち伏せ型・挟み込み型・臆病型に分かれ、追跡経路は **A***（[docs/AI_DESIGN.md](docs/AI_DESIGN.md)）
- 敵は **散開（コーナーへ退く）と追跡（プレイヤーへ収束）のウェーブ**を周期で切り替え、終盤は追跡型が加速する
- パワーエサ中に連続で敵を食べると得点が倍増（200 → 400 → 800 → 1600）
- ステージ進行に応じて**ボーナスフルーツ `F`** が一定時間出現
- 得点時に取得地点付近へスコアを点滅表示
- **スタシス・パルス（本作独自）**: ペレットでチャージし、満タンで `Space` を押すと全敵を一定時間凍結
- **ワープトンネル**: 左右端をつなぐ抜け道で緊急回避
- 難易度選択（Easy / Normal / Hard）とステージクリアボーナス
- ミス時の死亡演出（点滅と一拍の間）
- 主要イベントで**効果音**（外部ライブラリ非依存。Windows は `Beep`、Unix は端末ベル）
- ステージは `45 x 21`。各セルを横 2 文字に拡大し、端末サイズに合わせて中央寄せ表示
- 画面はカーソルを戻して 1 フレームを一括上書きし、ちらつきを抑える
- `levels/stage*.txt` から複数ステージを読み込み、順番に遊べる
- ハイスコアを `terminal-pacman.score` に保存する

## 画面サイズ

推奨サイズ:

- 横 95 文字以上
- 縦 32 行以上

画面が狭いと HUD とマップが折り返して見づらくなります。色が出ない場合は、ANSI / 仮想端末（VT）表示に対応した端末（Windows Terminal など）で実行してください。

## 次の方向性

優先度付きの改善案は [docs/ROADMAP.md](docs/ROADMAP.md) にまとめています。
