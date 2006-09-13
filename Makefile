#########################################################################
#                       COPYRIGHT NOTICE
#
#       Copyright (C) 1993 VME Microsystems International Corporation
#	International copyright secured.  All rights reserved.
#########################################################################
#	@(#)Makefile 1.6 97/03/21 VMIC
#########################################################################
#	Visual File Tree Utility Maintenance Script
#########################################################################
#
# Some brain-dead implementations of make(1) use the $SHELL environment
# variable to choose the command interpreter instead of correctly using
# the Bourne shell.  The SHELL macro definition below fixes that; don't
# change it even if you normally use a different shell interactively.
#
SHELL	=/bin/sh
#
#
#
CC	=ccache gcc -march=i686
#
#
#
CFLAGS	=-pipe -Os -Wall -Werror -pedantic -g
#
#
#
HFILES  =
#
#
#
CFILES  =vtree.c 
#
#
#
OBS	=${CFILES:.c=.o}
#
#
#
#BINDIR	=${HOME}/bin/`arch -k`
PREFIX	=/opt
BINDIR	=${PREFIX}/bin
#
#
#
MANEXT	=1
#
#
#
MANDIR	=${PREFIX}/man/man${MANEXT}
#
#
#
all:	vtree
#
#
#
clean:
	rm -f *.o core *.BAK lint tags install-*
#
#
#
distclean clobber: clean
	rm -f vtree
#
#
#
tags:	${HFILES} ${CFILES}
	ctags ${HFILES} ${CFILES}
#
#
#
install: install-bin install-man
#
#
#
install-bin: vtree
	install -c -m 755 -s vtree ${BINDIR}/vtree
	touch install-bin
#
#
#
install-man: vtree.man
	sed 's/MANEXT/${MANEXT}/g' <vtree.man >${MANDIR}/vtree.${MANEXT}
	touch install-man
#
#
#
uninstall: uninstall-bin uninstall-man
#
#
#
uninstall-bin:
	rm -f ${BINDIR}/vtree
	rm -f install-bin
#
#
#
uninstall-man:
	rm -f ${MANDIR}/vtree.${MANEXT}
	rm -f install-man
