SUBDIRS = src man

EXTRA_DIST=								\
	LICENSE								\
	README								\
	TODO

README: README.md
	markdown2 $< | lynx -dump -stdin >$@
