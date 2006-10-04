#########################################################################
#	Visual File Tree Utility Maintenance Script
#	vim: ts=8 sw=8
#########################################################################
#
# Some brain-dead implementations of make(1) use the $SHELL environment
# variable to choose the command interpreter instead of correctly using
# the Bourne shell.  The SHELL macro definition below fixes that; don't
# change it even if you normally use a different shell interactively.
#
SHELL	=/bin/sh
#

TARGETS	=all clean distclean clobber tags install install-bin install-man \
		 uninstall uninstall-bin uninstall-man check tags

.PHONY: ${TARGETS}

#
CC	=ccache gcc -march=i686
DEFS	=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
OPT	=-Os
CFLAGS	=-pipe ${OPT} -Wall -Werror -g ${DEFS}
HFILES  =
CFILES  =vtree.c 
OBS	=${CFILES:.c=.o}
PREFIX	=${HOME}/opt/$(shell uname -m)
BINDIR	=${PREFIX}/bin
MANEXT	=1
MANDIR	=${PREFIX}/man/man${MANEXT}

all::	vtree

${TARGETS}::

clean::
	${RM} *.o core *.BAK lint tags core.*

distclean clobber:: clean
	${RM} vtree

tags::	${HFILES} ${CFILES}
	ctags ${HFILES} ${CFILES}

install:: install-bin install-man

install-bin:: vtree
	install -d ${BINDIR}
	install -c -m 755 -s vtree ${BINDIR}/vtree

install-man:: vtree.man
	install -d ${MANDIR}
	sed 's/MANEXT/${MANEXT}/g' <vtree.man >${MANDIR}/vtree.${MANEXT}

uninstall:: uninstall-bin uninstall-man

uninstall-bin::
	${RM} ${BINDIR}/vtree

uninstall-man::
	${RM} ${MANDIR}/vtree.${MANEXT}

check:: vtree
	./vtree ${ARGS}
