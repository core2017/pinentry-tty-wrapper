SUBDIRS = test

bin_PROGRAMS = pinentry-tty-wrapper
pinentry_tty_wrapper_SOURCES = pinentry-tty-wrapper.c

.PHONY:	test
test:
	${MAKE} ${MAKEFLAGS} -C test all test
