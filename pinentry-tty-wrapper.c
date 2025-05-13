/*
 * Copyright 2025 (C) Shin-ichi Nagamura.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>

#ifndef BINDIR
#error "BINDIR macro is not defined. Please define BINDIR via build system (e.g., -DBINDIR=\"/usr/local/bin\")."
#endif

static void		safeFree( char const** const ptr ) ;
static void		safeClose( int* const fd ) ;


/**
 * @brief	安全にメモリを解放する
 * @param	ptr	解放するポインタを示すポインタ。
 * @return	なし
 * @note	ptr が示すアドレスが NULL の場合は何もしない。
 *  NULL以外の場合は開放し、ptr* に NULL を代入する。
 */
static void  safeFree( char const** const ptr )
{
	if(( ptr == NULL ) || ( *ptr == NULL )) {
		return ;
	}

	free((void*)*ptr ) ;
	*ptr = NULL ;
}


/**
 * @brief	安全にファイルディスクリプタを閉じる
 * @param	fd	閉じるファイルディスクリプタ
 * @return	なし
 * @note	fd が示すファイルディスクリプタが -1 の場合は何もしない。
 *  -1 以外の場合は閉じ、fd* に -1 を代入する。
 */
static void  safeClose( int* const fd )
{
	if(( fd == NULL ) || ( *fd < 0 )) {
		return ;
	}

	close( *fd ) ;
	*fd = -1 ;
}


/**
 * @brief	メイン関数
 * @details	pinentry-tty プログラムを実行するラッパーである。
 *		gpg-agent.conf からは引数を渡せないため、
 *		本ラッパーと同じディレクトリにある pinentry-tty を自動的に起動する仕様である。
 * @param	ac	コマンドライン引数の数
 * @param	av	コマンドライン引数の配列
 * @return	0	正常終了
 * @return	1	エラー終了
 */
int main( int ac, char* const av[] )
{
	struct termios			savedTermios ;
	int						rc				= 1 ;
	int						fd				= -1 ;
	pid_t					pid ;
	int						status ;
	char const* const		gpgTty			= getenv( "GPG_TTY" ) ;
	char*					wrapperPath		= NULL ;
	char*					pinentryPath	= NULL ;


	/* ----- open syslog ----- */
	openlog( "pinentry-tty-wrapper", LOG_PID | LOG_PERROR, LOG_USER ) ;
	syslog( LOG_DEBUG, "pinentry-tty-wrapper: start" ) ;


	/*
	 * validate arguments and environment variables.
	 */

	/* ----- check arguments ----- */
	if( ac < 1 ) {
		fprintf( stderr, "Usage: %s pinentry-program [pinentry-args...]\n", av[0] ) ;
		return( 1 ) ;
	}

	/* ----- check GPG_TTY ----- */
	if( gpgTty == NULL ) {
		syslog( LOG_ERR, "GPG_TTY is not defined. Exiting." ) ;
		return( 1 ) ;
	}


	/*
	 * open GPG_TTY, save termios and set termios.
	 */

	if(( fd = open( gpgTty, O_RDWR )) < 0 ) {
		syslog( LOG_ERR, "open(GPG_TTY) failed: %s", strerror( errno )) ;
		goto failed ;
	}

	if( tcgetattr( fd, &savedTermios ) < 0 ) {
		syslog( LOG_ERR, "tcgetattr failed: %s", strerror( errno )) ;
		goto failed ;
	}

	/* Modify terminal settings */
	savedTermios.c_iflag	|= ICRNL ;
	savedTermios.c_oflag	|= OPOST ;

	if( tcsetattr( fd, TCSANOW, &savedTermios ) < 0 ) {
		syslog( LOG_ERR, "tcsetattr failed: %s", strerror( errno )) ;
		goto failed ;
	}


	/*
	 * fork and execute the pinentry-tty program in the same directory.
	 */

	/* ラッパー自身のパスからディレクトリを抽出し、pinentry-tty のパスを生成する */
	syslog( LOG_DEBUG, "%s():%u: av[0]=%s", __func__, __LINE__, av[0] ) ;

	if(( wrapperPath = strdup( av[0] )) == NULL ) {
		syslog( LOG_ERR, "strdup failed: %s", strerror( errno )) ;
		goto failed ;
	}

	if( asprintf( &pinentryPath, "%s/pinentry-tty", BINDIR ) < 0 ) {
		syslog( LOG_ERR, "asprintf failed: %s", strerror( errno )) ;
		goto failed ;
	}

	safeFree((char const**)&wrapperPath ) ;

	if(( pid = fork()) < 0 ) {
		syslog( LOG_ERR, "fork failed: %s", strerror( errno )) ;
		goto failed ;
	}

	if( pid == 0 ) {
		/* 子プロセス: pinentry-tty を起動する */
		char* const    args[] = {
			pinentryPath, NULL
		} ;
		syslog( LOG_DEBUG, "%s():%u: pinentryPath=%s", __func__, __LINE__, pinentryPath ) ;
		execvp( pinentryPath, args ) ;
		syslog( LOG_ERR, "execvp failed: %s", strerror( errno )) ;
		safeFree((char const**)&pinentryPath ) ;
		exit( 1 ) ;
	}

	/* 親プロセス */
	if( waitpid( pid, &status, 0 ) < 0 ) {
		syslog( LOG_ERR, "waitpid failed: %s", strerror( errno )) ;
		goto failed ;
	}

	/* Restore original terminal settings */
	if( tcsetattr( fd, TCSANOW, &savedTermios ) < 0 ) {
		syslog( LOG_ERR, "tcsetattr failed: %s", strerror( errno )) ;
	}

	safeClose( &fd ) ;
	safeFree((char const**)&pinentryPath ) ;

	/* Return the exit status of the pinentry-tty program */
	if( !WIFEXITED( status )) {
		return( 1 ) ;
	}

	return( WEXITSTATUS( status )) ;

failed:

	safeFree((char const**)&wrapperPath ) ;
	safeFree((char const**)&pinentryPath ) ;
	safeClose( &fd ) ;

	return( 1 ) ;
}
