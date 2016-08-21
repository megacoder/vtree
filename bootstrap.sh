#!/bin/zsh
aclocal
autoconf
automake --foreign --add-missing --copy --force-missing
./configure
make dist
rm -rf RPM
mkdir -p RPM/{SOURCES,RPMS,SRPMS,BUILD,SPECS}
rpmbuild								\
	-D"_topdir ${PWD}/RPM"						\
	-D"_sourcedir ${PWD}"						\
	-D"_specdir ${PWD}"						\
	-ba								\
	vtree.spec
