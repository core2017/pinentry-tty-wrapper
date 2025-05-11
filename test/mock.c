/*
 * Copyright (c) 2025 Shin-ichi Nagamura
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
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main( int argc, char *argv[] )
{
	printf( "mock: HELLO\n" ) ;
	fflush( stdout ) ;

	struct termios			tio ;
	const char * const		gpgTty = getenv( "GPG_TTY" ) ;
	int						fd ;

	if( gpgTty == NULL ) {
		fprintf( stderr, "undefined GPG_TTY\n" ) ;
		return( 1 ) ;
	}

	if(( fd = open( gpgTty, O_RDWR )) < 0 ) {
		perror( "open(GPG_TTY)" ) ;
		printf( "termios: error\n" ) ;
		return( 1 ) ;
	}

	if( tcgetattr( fd, &tio ) != 0 ) {
		perror( "tcgetattr" ) ;
		printf( "termios: error\n" ) ;
		close( fd ) ;
		return( 1 ) ;
	}

	close( fd ) ;


	printf( "termios: iflag:0x%04lx, oflag:0x%04lx\n",
			(unsigned long)tio.c_iflag, (unsigned long)tio.c_oflag ) ;
	printf( "commandline:" ) ;

	for( int i = 1 ; i < argc ; ++i ) {
		printf( " %s", argv[i] ) ;
	}

	printf( "\n" ) ;
	return( 0 ) ;
}
