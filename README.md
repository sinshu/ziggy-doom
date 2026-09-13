# ziggy-doom

Doom の C エンジンを Zig でコンパイルし、raylib で表示・キーボード入力・効果音再生を行う最小ポートです。Zig はビルド定義と C コンパイラとしてだけ使用します。音楽・マウス・ネットワーク対戦は対象外です。

## 必要なもの

- Zig **0.16.0**（Windows x86_64 でビルド確認）
- 初回ビルド時のインターネット接続（固定コミットの raylib を取得）
- Doom の IWAD: `doom1.wad`（シェアウェア）、`doom.wad` または `doom2.wad`

IWAD は同梱していません。手元のファイルをプロジェクト直下へ置くか、`-iwad` で指定してください。

## ビルド・実行

```powershell
zig build
zig build run -- -iwad doom2.wad
# 任意の場所の IWAD
zig build run -- -iwad "C:/Games/DOOM/doom.wad"
# 最適化した実行ファイル
zig build -Doptimize=ReleaseFast
./zig-out/bin/ziggy-doom.exe -iwad doom2.wad
```

`zig build run` の作業ディレクトリはプロジェクト直下です。`-iwad` を省略すると Doom の標準検索で IWAD を探します。セーブ・設定もエンジンの標準動作に従います。`-warp 1 1`（Doom）や `-warp 1`（Doom II）で直接ステージを開始できます。

## 操作

| キー | 操作 |
| --- | --- |
| W / ↑、S / ↓ | 前進・後退 |
| ← / → | 左右を向く |
| A / D、`,` / `.` | 左右へ平行移動 |
| Ctrl | 射撃 |
| Space / E | 扉を開く・スイッチを使う |
| Shift | 走る |
| Alt + ← / → | 左右へ平行移動 |
| 1〜7 | 武器切り替え |
| Enter / ↑ / ↓ | メニューの決定・選択 |
| Esc | メニュー（ウィンドウは閉じません） |
| Tab | マップ |
| F2 / F3 | セーブ / ロード |
| Pause | 一時停止 |
| ウィンドウの × | 終了 |

320×200 のフレームを最近傍で拡大し、当時の表示に合わせた 4:3 の比率で描画します。初期ウィンドウは 960×720、サイズ変更時は余白を付けて比率を維持します。ゲームの進行はエンジンの 35 Hz タイマーに従います。

## 構成

- `src/main.c`: raylib バックエンド。画面転送、色変換、入力、タイマー、終了処理。
- `src/i_raylibsound.c`: 効果音バックエンド。WAD 内の DMX PCM をキャッシュし、チャンネルごとの alias で同時再生。再生中もエンジンから距離減衰・左右定位の更新を受け取ります。
- `src/dmx_sound.h`: DMX ヘッダーと PCM 範囲の検証。
- `vendor/doomgeneric/`: doomgeneric の C ソースとヘッダー。
- `build.zig`: エンジンとバックエンドを C としてビルドし、音声有効の raylib をリンク。
- `vendor/doomgeneric/UPSTREAM.md`: 取得元と変更点。

## 動作確認

Windows x86_64 / Zig 0.16.0 と手元の `doom2.wad` でビルド・描画を確認済みです。実エンジンに前進・射撃入力を渡し、座標の変化と弾数の減少を検証するテストもあります。

```powershell
zig build test
zig build run -Dsmoke=true -- -iwad doom2.wad -warp 1 -nomonsters -nogui
# 音声デバイスと実 WAD の効果音再生も検証
zig build run -Dsmoke=true -- -iwad doom2.wad -warp 1 -nomonsters -nogui -soundcheck
```

テストは自動終了し、`.tmp/doom-smoke.png` に描画結果を保存します。実際のキーボードを自動操作するテストではありません。他 OS は未検証です。

`zig build test` は音声デバイス不要で、不正な DMX データ、同一音源の同時再生、再生中の音量・定位更新、チャンネル差し替え・解放、デバイス初期化失敗を検証します。`-soundcheck` は音声デバイスが必要です。効果音は `-nosfx` または `-nosound` で無効にできます。デバイスを開けない場合も無音で続行します。

効果音は起動時に変換して終了まで保持します。`snd_cachesize` による追い出しは未対応です。定位は raylib のパンカーブを使うため、元の SDL バックエンドとは音量特性が多少異なります。

## ライセンス

Doom / doomgeneric は GPL-2.0-or-later。ライセンス本文は `LICENSE` および `vendor/doomgeneric/LICENSE` を参照してください。raylib は zlib ライセンスです。IWAD のデータにはそれぞれ別の利用条件が適用されます。
