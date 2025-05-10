# pinentry-tty-wrapper

A wrapper for pinentry programs that ensures proper terminal settings.

## Description

This wrapper program ensures that the terminal settings (termios) are properly
configured before running a pinentry program. It saves the current terminal
settings, sets ICRNL and OPOST flags, runs the specified pinentry program,
and restores the original terminal settings when the pinentry program exits.

## Installation

```bash
./configure
make
sudo make install
```

## Usage

In your `gpg-agent.conf`:

```
pinentry-program /usr/local/bin/pinentry-tty-wrapper /usr/local/bin/pinentry-tty
```

The wrapper will pass all additional arguments to the specified pinentry program.

## License

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version. 