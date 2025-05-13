# pinentry-tty-wrapper

pinentry-tty-wrapper は、同じディレクトリに存在する pinentry-tty を起動し、端末設定（termios）を適切に制御するラッパープログラムである。

## 概要

このプログラムは pinentry-tty のラッパーです。

## 動作仕様

- `pinentry-tty-wrapper` は、インストール先ディレクトリ（通常は `bindir`、例: `/usr/local/bin` など）にある `pinentry-tty` を起動します。
- `gpg-agent.conf` の `pinentry-program` には本ラッパーのフルパスを指定してください。
- ラッパーと `pinentry-tty` は同じディレクトリ（インストール先）に配置されている必要があります。

## インストール

```bash
./configure
make
sudo make install
```

## 注意事項

- `pinentry-tty-wrapper` は、ビルド時に `BINDIR` マクロとしてインストール先ディレクトリのパスを埋め込んでいます。
- `pinentry-tty` がインストール先ディレクトリに存在しない場合、正しく動作しません。

## 設定例

gpg-agent.conf には以下のように記述する。

```
pinentry-program /usr/local/bin/pinentry-tty-wrapper
```

pinentry-tty-wrapper は、自身と同じディレクトリにある pinentry-tty を自動的に起動する仕様である。

## ライセンス

本プログラムは BSD-3-Clause ライセンスの下で配布する。