#!/bin/sh
if test -x /usr/bin/autoreconf; then
	echo "Using autoreconf(1)..."
	/usr/bin/autoreconf -fvim
else
	echo "Using individual tools..."
	aclocal && autoheader && autoconf && automake --add-missing --copy
fi
