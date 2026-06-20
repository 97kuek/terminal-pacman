# terminal-pacman

C 言語で実装した、ターミナル上で動くパックマン風ゲームです。

外部ランタイム依存なしで Windows / Linux / macOS で動く、ASCII 表示の全画面向けターミナルゲームを目標にしています。

## 状態

初期プレイ可能版を実装済みです。

関連ドキュメント:

- [SPEC.md](SPEC.md): ゲーム仕様
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

## 操作方法

| キー | 操作 |
| --- | --- |
| `W` / 上矢印 | 上へ移動 |
| `A` / 左矢印 | 左へ移動 |
| `S` / 下矢印 | 下へ移動 |
| `D` / 右矢印 | 右へ移動 |
| `P` | 一時停止 / 再開 |
| `R` | クリア / ゲームオーバー後にリスタート |
| `Q` | 終了 |

## 現在のゲーム内容

- ドットをすべて食べるとクリア
- パワーエサ `o` を食べると一定時間だけ敵を食べられる
- パワーエサ終了前は敵表示が点滅する
- 敵に触れるとライフが減少
- ライフが 0 になるとゲームオーバー
- 敵はランダム型、追跡型、先読み型に分かれて移動
- ステージ開始時とミス後に短いカウントダウンが入る
- ドットを集めるほど敵の移動間隔が短くなる
- `levels/stage*.txt` からステージを読み込む
- 複数ステージを順番に遊べる
- ハイスコアを `terminal-pacman.score` に保存する
- 画面はカーソルを戻して上書きし、ちらつきを抑える
- ステージは `59 x 23` の広いマップ
- 画面上部に目的、凡例、進捗、状態、操作をまとめて表示する
- 画面表示は ASCII 文字のみ

## 画面サイズ

全画面に近いターミナルでのプレイを想定しています。

推奨サイズ:

- 横 80 文字以上
- 縦 35 行以上

画面が狭いと HUD とマップが折り返して見づらくなります。

## 次の方向性

ゲーム性をさらに上げるなら、次は以下の順で進めるのが現実的です。

1. ステージ選択
2. クリアボーナス
3. 敵 AI の難易度設定
4. Unicode 表示モード

詳細は [docs/ROADMAP.md](docs/ROADMAP.md) にまとめています。
