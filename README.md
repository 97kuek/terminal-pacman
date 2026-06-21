# terminal-pacman

**外部ライブラリに依存しない C 言語製の、ターミナル上で動くパックマン風ゲーム**です。Windows / Linux / macOS で動き、表示は ASCII グリフ＋ANSI カラー。

3 つのモード（Classic / Endless / Time Attack）、A\* 経路探索とオンライン Q 学習による敵 AI、独自要素のスタシス・パルスを備えています。

## 特徴

- 🎮 **3 モード**: Classic（3 ステージ）/ Endless（ローグライト・無限生成）/ Time Attack（120 秒スコアアタック）。
- 👻 **敵 AI**: A\* 経路探索＋4 種の性格（直線追跡・待ち伏せ・挟み込み・臆病）、散開／追跡ウェーブ、終盤加速。Endless では 1 体が**オンライン Q 学習**で適応（[docs/AI_DESIGN.md](docs/AI_DESIGN.md)）。
- ✨ **独自要素**: スタシス・パルス（ペレットでチャージ→敵を一時凍結）、ワープトンネル、連続捕食ボーナス、ボーナスフルーツ。
- 🎨 **見やすい描画**: 256 色＋truecolor グラデーションのタイトル、1 フレーム一括書き込みでちらつき抑制、端末サイズに応じた中央寄せ。
- 🔊 **効果音**: 外部ライブラリ非依存（Windows は別スレッド `Beep`、Unix は端末ベル）。`--no-sound` / `--mono` で無効化可。
- 🧪 **品質**: 純ロジック・A\*・迷路生成・Q 学習の単体テスト、GitHub Actions による 3 OS の CI。

依存ゼロ・標準 C のみ。OS 依存処理は小さな平台レイヤー（`src/platform.h`）に閉じ込めています。

## ドキュメント

- [docs/AI_DESIGN.md](docs/AI_DESIGN.md): **敵 AI の設計**（A\* と Q 学習のアルゴリズム解説）
- [SPEC.md](SPEC.md): ゲーム仕様
- [docs/MODES_DESIGN.md](docs/MODES_DESIGN.md): モード制の設計
- [CLAUDE.md](CLAUDE.md): 開発者 / AI エージェント向け全体ガイド
- [CONTRIBUTING.md](CONTRIBUTING.md): 開発・貢献ガイド
- [docs/ROADMAP.md](docs/ROADMAP.md): 今後の改善案
- [docs/IMPLEMENTATION_PLAN.md](docs/IMPLEMENTATION_PLAN.md): 実装計画と進捗

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

ゲームロジック・A\*・迷路生成・Q 学習の単体テストがあります。

```sh
make test                          # Linux / macOS
powershell -File build_tests.ps1   # Windows
```

push / PR ごとに GitHub Actions が Windows / Linux / macOS でビルドとテストを実行します。

## モード

起動メニューで **モード**と**難易度**を選びます（上下でモード/難易度の行を切替、左右で値変更、`Space` で開始）。

- **Classic**: 既存の 3 ステージ。最後までクリアでゲームクリア。
- **Endless**: ローグライト（1 ミスで終了）。迷路は食べ尽くすたびに自動生成され、進むほど敵が速くなる。
- **Time Attack**: 制限時間 120 秒でスコアを稼ぐ。捕まっても少し戻されるだけで時間は進む。

ハイスコアはモード別に保存します。

## 起動オプション

```text
--mode NAME         classic | endless | timeattack（メニューを省略）
--level N           ステージ N から開始（1-3、Classic のみ）
--difficulty NAME   easy | normal | hard（メニューを省略）
--speed MS          フレーム間隔（ミリ秒、既定 100）
--no-sound          効果音を無効化
--mono              色を使わないモノクロ表示
--help              ヘルプを表示
```

例: `./build/terminal-pacman --mode endless --difficulty hard`

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

## アーキテクチャ

責務を分離した小さなモジュール構成です（詳細は [CLAUDE.md](CLAUDE.md)）。

| モジュール | 役割 |
| --- | --- |
| `src/main.c` | エントリポイント、固定間隔ゲームループ、CLI |
| `src/game.{c,h}` | ゲーム状態と全ルール（移動・衝突・スコア・モード・敵 AI 統合） |
| `src/render.{c,h}` | 状態を読んで 1 フレームを組み立て一括描画（色・スケール・中央寄せ） |
| `src/platform.{h,*}` | OS 依存（入力・端末制御・サウンド）を隠す平台レイヤー |
| `src/pathfind.{c,h}` | A\* 経路探索 |
| `src/qghost.{c,h}` / `src/qfeatures.{c,h}` | オンライン Q 学習コアと状態符号化 |
| `src/maze.{c,h}` | 迷路ジェネレータ（Endless / Time Attack 用） |
| `tools/` | 迷路生成・バナー生成・Q 学習のオフライン学習/評価 |

- **ゲームルールは描画・端末から独立**（`game.c`）。
- **OS 依存は `platform.h` の裏に閉じ込め**、クロスプラットフォーム性を保つ。
- 敵 AI のアルゴリズムは [docs/AI_DESIGN.md](docs/AI_DESIGN.md) を参照。

## 次の方向性

優先度付きの改善案は [docs/ROADMAP.md](docs/ROADMAP.md) にまとめています。

## ライセンス

[LICENSE](LICENSE) を参照。
