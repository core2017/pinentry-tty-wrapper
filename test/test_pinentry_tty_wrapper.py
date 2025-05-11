#!/usr/bin/env python3
#
# Copyright (c) 2025 Shin-ichi Nagamura
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""
pinentry-tty-wrapper テストランナー (pytest用)
"""
import pytest
import subprocess
import os
import sys
import syslog

@pytest.fixture(scope="module")
def bin_and_mock():
	mydir = os.path.dirname(os.path.abspath(__file__))
	bin_path = os.path.abspath(os.path.join(mydir, "..", "pinentry-tty-wrapper"))
	mock_path = os.path.join(mydir, "mock")
	return bin_path, mock_path

@pytest.mark.parametrize("args, expect_code, expect_usage", [
	([], 1, True),                # 引数なし
	(["mock"], 0, False),        # mockのみ
	(["mock", "arg1"], 0, False), # mock+arg1
])
def test_command_line_args(bin_and_mock, args, expect_code, expect_usage):
	bin_path, mock_path = bin_and_mock
	# mockの絶対パスをargs[0]が"mock"なら差し替え
	full_args = [bin_path]
	if args:
		if args[0] == "mock":
			full_args.append(mock_path)
			full_args.extend(args[1:])
		else:
			full_args.extend(args)
	syslog.syslog(syslog.LOG_DEBUG, '実行コマンド:' + " ".join(full_args))
	proc = subprocess.run(full_args, capture_output=True, text=True)
	assert proc.returncode == expect_code, (
		f"args={args}: 終了コードが期待値({expect_code})でない: {proc.returncode}\n"
		f"stdout:\n{proc.stdout}\nstderr:\n{proc.stderr}"
	)
	if expect_usage:
		assert "Usage:" in proc.stderr, f"args={args}: usage表示なし: {proc.stderr}"
	else:
		assert "Usage:" not in proc.stderr, f"args={args}: usage表示が出てしまった: {proc.stderr}"
		assert "mock: HELLO" in proc.stdout, (
			f"mockが起動していません。stdout:\n{proc.stdout}\nstderr:\n{proc.stderr}"
		)

def test_gpg_tty_undefined(bin_and_mock):
	bin_path, mock_path = bin_and_mock
	# GPG_TTY未定義で実行
	env = os.environ.copy()
	if "GPG_TTY" in env:
		del env["GPG_TTY"]
	proc = subprocess.run([bin_path, mock_path], capture_output=True, text=True, env=env)
	assert proc.returncode == 1, (
		f"GPG_TTY未定義時: 終了コードが1でない: {proc.returncode}\n"
		f"stdout:\n{proc.stdout}\nstderr:\n{proc.stderr}"
	)
	assert "GPG_TTY が未定義" in proc.stderr, f"GPG_TTY未定義時: エラーメッセージが期待通りでない: {proc.stderr}"

def test_child_process_success(bin_and_mock):
	bin_path, mock_path = bin_and_mock
	env = os.environ.copy()
	proc = subprocess.run([bin_path, mock_path, "childtest"], capture_output=True, text=True, env=env)
	assert proc.returncode == 0, f"正常なmock実行: 終了コードが0でない: {proc.returncode}\nstdout:\n{proc.stdout}\nstderr:\n{proc.stderr}"
	assert "mock: HELLO" in proc.stdout, f"mockが起動していません。stdout:\n{proc.stdout}\nstderr:\n{proc.stderr}"

def test_child_process_notfound(bin_and_mock):
	bin_path, _ = bin_and_mock
	env = os.environ.copy()
	proc = subprocess.run([bin_path, "/notfound"], capture_output=True, text=True, env=env)
	assert proc.returncode == 1, f"notfound実行: 終了コードが1でない: {proc.returncode}\nstdout:\n{proc.stdout}\nstderr:\n{proc.stderr}"
	assert "execvp" in proc.stderr or "No such file" in proc.stderr, f"notfound実行: エラーメッセージが期待通りでない: {proc.stderr}"

def test_termios_settings(bin_and_mock):
	bin_path, mock_path = bin_and_mock
	env = os.environ.copy()
	proc = subprocess.run([bin_path, mock_path, "termios"], capture_output=True, text=True, env=env)
	assert proc.returncode == 0, f"termiosテスト: 終了コードが0でない: {proc.returncode}\nstdout:\n{proc.stdout}\nstderr:\n{proc.stderr}"
	import re
	m = re.search(r"termios: iflag:0x([0-9a-fA-F]+), oflag:0x([0-9a-fA-F]+)", proc.stdout)
	assert m, f"termios出力が見つからない: stdout:\n{proc.stdout}\nstderr:\n{proc.stderr}"
	iflag = int(m.group(1), 16)
	oflag = int(m.group(2), 16)
	assert (iflag & 0x0100) != 0, f"ICRNLフラグが立っていない: iflag=0x{iflag:04x}"
	assert (oflag & 0x0001) != 0, f"OPOSTフラグが立っていない: oflag=0x{oflag:04x}" 