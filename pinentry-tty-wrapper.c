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
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>


/**
 * @brief	メイン関数
 * @details	pinentry-tty プログラムを実行するラッパー。
 *			termios の設定を適切に行い、pinentry-tty を実行する。
 * @param	ac	コマンドライン引数の数
 * @param	av	コマンドライン引数の配列
 * @return	0	正常終了
 * @return	1	エラー終了
 */
int main( int ac, char * const av[] )
{
	struct termios			saved_termios ;
	int						fd ;
	pid_t					pid ;
	int						status ;
	char const * const		gpgTty = getenv( "GPG_TTY" ) ;


	/*
	 * validate arguments and environment variables.
	 */

	/* ----- check arguments ----- */
	if( ac < 2 ) {
		fprintf( stderr, "Usage: %s pinentry-program [pinentry-args...]\n", av[0] ) ;
		return( 1 ) ;
	}

	/* ----- check GPG_TTY ----- */
	if( gpgTty == NULL ) {
		fprintf( stderr, "GPG_TTY が未定義であるため終了します\n" ) ;
		return( 1 ) ;
	}


	/*
	 * open GPG_TTY, save termios and set termios.
	 */

	if(( fd = open( gpgTty, O_RDWR )) < 0 ) {
		perror( "open(GPG_TTY)" ) ;
		return( 1 ) ;
	}

	if( tcgetattr( fd, &saved_termios ) < 0 ) {
		perror( "tcgetattr" ) ;
		return( 1 ) ;
	}

	/* Modify terminal settings */
	saved_termios.c_iflag	|= ICRNL ;
	saved_termios.c_oflag	|= OPOST ;

	if( tcsetattr( fd, TCSANOW, &saved_termios ) < 0 ) {
		perror( "tcsetattr" ) ;
		return( 1 ) ;
	}


	/*
	 * fork and execute the pinentry program.
	 */

	if(( pid = fork()) < 0 ) {
		perror( "fork" ) ;
		return( 1 ) ;
	}

	if( pid == 0 ) {
		/* Child process */
		execvp( av[1], &av[1] ) ;
		perror( "execvp" ) ;
		exit( 1 ) ;
	}

	/* Parent process */
	if( waitpid( pid, &status, 0 ) < 0 ) {
		perror( "waitpid" ) ;
		return( 1 ) ;
	}

	/* Restore original terminal settings */
	if( tcsetattr( fd, TCSANOW, &saved_termios ) < 0 ) {
		perror( "tcsetattr" ) ;
	}

	close( fd ) ;

	/* Return the exit status of the pinentry program */
	if( !WIFEXITED( status )) {
		return( 1 ) ;
	}

	return( WEXITSTATUS( status )) ;
}
