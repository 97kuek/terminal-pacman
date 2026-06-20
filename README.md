# terminal-pacman

C 言語で実装した、ターミナル上で動くパックマン風ゲームです。

最初の版は、外部ランタイム依存なしで Windows / Linux / macOS で動く ASCII 表示のゲームを目標にしています。

## 状態

初期プレイ可能版を実装済みです。

関連ドキュメント:

- [SPEC.md](SPEC.md): ゲーム仕様
- [docs/IMPLEMENTATION_PLAN.md](docs/IMPLEMENTATION_PLAN.md): 実装計画と進捗
- [docs/ROADMAP.md](docs/ROADMAP.md): 今後の改善案
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
| `Q` | 終了 |

## 現在のゲーム内容

- ドットをすべて食べるとクリア
- 敵に触れるとライフが減少
- ライフが 0 になるとゲームオーバー
- 敵はランダムに移動
- 画面表示は ASCII 文字のみ

## 次の方向性

ゲーム性を上げるなら、まずは以下の順で進めるのが現実的です。

1. パワーエサと敵を食べられる状態を追加する
2. 敵ごとに違う移動ロジックを入れる
3. ステージ、難易度、速度変化を追加する
4. ハイスコア保存とリスタート操作を追加する

詳細は [docs/ROADMAP.md](docs/ROADMAP.md) にまとめています。

