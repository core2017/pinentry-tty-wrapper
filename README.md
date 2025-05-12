# pinentry-tty-wrapper

pinentry-tty-wrapper は、同じディレクトリに存在する pinentry-tty を起動し、端末設定（termios）を適切に制御するラッパープログラムである。

## 概要

本プログラムは、pinentry-tty の入出力が ssh 等の端末設定の影響で正しく動作しない場合に、必要な termios フラグ（ICRNL, OPOST）を設定した上で pinentry-tty を起動し、終了時に元の設定へ戻す。

## インストール

```bash
./configure
make
sudo make install
```

## 設定例

gpg-agent.conf には以下のように記述する。

```
pinentry-program /usr/local/bin/pinentry-tty-wrapper
```

pinentry-tty-wrapper は、自身と同じディレクトリにある pinentry-tty を自動的に起動する仕様である。

## ライセンス

本プログラムは BSD-3-Clause ライセンスの下で配布する。