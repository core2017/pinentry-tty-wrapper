#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>


int main( int ac, char * const av[] )
{
	struct termios		saved_termios ;
	int					fd ;
	pid_t				pid ;
	int					status ;


	if( ac < 2 ) {
		fprintf( stderr, "Usage: %s pinentry-program [pinentry-args...]\n", av[0] ) ;
		return( 1 ) ;
	}

	/* Get the current terminal settings */
	fd = fileno( stdin ) ;

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

	/* Fork and execute the pinentry program */
	pid = fork() ;

	if( pid < 0 ) {
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

	/* Return the exit status of the pinentry program */
	if( WIFEXITED( status )) {
		return( WEXITSTATUS( status )) ;
	}

	return( 1 ) ;
}
