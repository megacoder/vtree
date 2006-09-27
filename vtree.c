/*
 * vim: ts=8 sw=8
 *------------------------------------------------------------------------
 *                       COPYRIGHT NOTICE
 *
 *       Copyright (C) 1993 VME Microsystems International Corporation
 *	 International copyright secured.  All rights reserved.
 *------------------------------------------------------------------------
 *	SCCS/s.vtree.c 1.15 3/21/97 11:53:04
 *------------------------------------------------------------------------
 *	Visual File Tree Utility Source Code
 *------------------------------------------------------------------------
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <malloc.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <getopt.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>

typedef enum	{
	Iwanna_perms	= (1 << 0),
	Iwanna_user	= (1 << 1),
	Iwanna_group	= (1 << 2)
} Iwanna_t;

#ifndef	WHATCOL
#define WHATCOL	32
#endif	/* no WHATCOL */

#define	max(x,y)	( (x) > (y) ? (x) : (y) )

#ifndef	MAXPREFIX
#define MAXPREFIX (8 * 1024)
#endif	/* MAXPREFIX */

static char *	me = "vtree";
static int	debug = 0;
static int	nonfatal = 0;
static int	Nprefix;
static char	prefix[ MAXPREFIX ];
static int	a_sw = 0;
static int	d_sw = 0;
static int	f_sw = 0;
static int	combo_sw = 0;
static int	s_sw = 0;
static int	w_sw = 0;
static int	F_sw = 0;
static char * * ignores;
static int	Nignores = 0;
static int	ignore_used = 0;
static char *	ofile;			/* Where to put stdout		 */
static char *	leadin;			/* Delete this prefix string	 */
static int	Nleadin;		/* Length of prefix string	 */
static int	whatColumn = WHATCOL;	/* Indention for "what" info	 */
static int	depth = 500;		/* Recursion depth limit	 */
static int	currentDepth;		/* How deep is our love?	 */

/*
 *------------------------------------------------------------------------
 * add_ignore: add directory name to list of directories we'll ignore
 *------------------------------------------------------------------------
 */

static void
add_ignore(
	char		*name
)
{
	if( ignore_used >= Nignores )	{
		int		size;
		Nignores += 128;
		size = Nignores * sizeof( char *);
		if( ignores ) ignores = (char **) realloc( ignores, size );
		else	ignores = (char **) malloc( size );
		if( !ignores )	{
OutOfMemory:
			fprintf( stderr, "%s: out of memory\n", me );
			exit( 1 );
			/*NOTREACHED*/
		}
	}
	if( (ignores[ ignore_used++ ] = strdup( name )) == (char *) NULL )
		goto OutOfMemory;
}

/*
 *------------------------------------------------------------------------
 * permit: return true if should permit this name
 *------------------------------------------------------------------------
 */

static int
permit(
	const struct dirent	*d
)
{
	int		i;
	/* Ignore .foo files unless we got the -a switch		 */
	if( d->d_name[0] == '.' && !a_sw ) return( 0 );
	/* Ignore any explicitly-given name also			 */
	for( i = 0; i < ignore_used; ++i )	{
		if( strcmp( d->d_name, ignores[i] ) == 0 ) return( 0 );
	}
	return( 1 );
}

/*
 *------------------------------------------------------------------------
 * Falphasort: alphasort(3) that folds uppercase to lowercase
 *------------------------------------------------------------------------
 */

static int
Falphasort(
	const void *		larg,
	const void *		rarg
)
{
	const struct dirent * const * const	l = larg;
	const struct dirent * const * const	r = rarg;

	return strcasecmp( (*l)->d_name, (*r)->d_name );
}

/*
 *------------------------------------------------------------------------
 * drop_leadin: delete leadin string (plus any whitespace)
 *------------------------------------------------------------------------
 */

static char	*
drop_leadin(
	char		*s
)
{
	if( (Nleadin > 0) && (strncmp( s, leadin, Nleadin ) == 0) )	{
		s += Nleadin;
		while( *s && isspace( *s ) ) ++s;
	}
	return( s );
}

/*
 *------------------------------------------------------------------------
 * process: process a single pathname
 *------------------------------------------------------------------------
 * 1. Print name at current nesting level.
 *
 * 2. Print trailing line. Use proper connector (check if last entry in
 *    the directory). Don't output line if current name is an ordinary
 *    file and we are squashing output.
 *
 * 3. Recurse if current item is a directory.
 *------------------------------------------------------------------------
 */

static	char *
getGroupName(
	gid_t		gid
)
{
	static	char	spelling[ BUFSIZ ];
	char *		result;
	struct group *	g;

	g = getgrgid( gid );
	if( g )	{
		result = g->gr_name;
	} else	{
		snprintf( spelling, sizeof( spelling ), "%u", (unsigned) gid );
		result = spelling;
	}
	return( result );

}

static	char *
getUserName(
	uid_t		uid
)
{
	static	char	spelling[ BUFSIZ ];
	char *		result;
	struct passwd *	p;

	p = getpwuid( uid );
	if( p )	{
		result = p->pw_name;
	} else	{
		snprintf( spelling, sizeof( spelling ), "%u", (unsigned) uid );
		result = spelling;
	}
	return( result );
}

static void
printName(
	char		*name,
	struct stat *	st,
	int		isLastEntry,
	int		justTheName
)
{
	char *		tag;

	tag = "???";
	if( !justTheName )	{
		printf( 
			"%s%c-- %s", 
			prefix, 
			isLastEntry ? '\\' : '+', 
			name
		);
	} else	{
		printf( 
			"%s", 
			name
		);
	}
	if( combo_sw )	{
		static char const	sep[] = ":";
		char const *		s;

		s = "";
		printf( "[" );
		if( combo_sw & Iwanna_perms )	{
			printf( "%s%04o", s, st->st_mode & ~S_IFMT );
			s = sep;
		}
		if( combo_sw & Iwanna_user )	{
			printf( "%s%s", s, getUserName( st->st_uid ) );
			s = sep;
		}
		if( combo_sw & Iwanna_group )	{
			printf( "%s%s", s, getGroupName( st->st_gid ) );
			s = sep;
		}
		printf( "]" );
	}
	if( F_sw )	{
		if( S_ISREG( st->st_mode ) )	{
			if( st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH ) ) {
				tag = "*";
			} else	{
				tag = "";
			}
		} if( S_ISDIR( st->st_mode ) )	{
			tag = "/";
		} else if( S_ISCHR( st->st_mode ) )	{
			tag = "{c}";
		} else if( S_ISBLK( st->st_mode ) )	{
			tag = "{b}";
		} else if( S_ISFIFO( st->st_mode ) )	{
			tag = "|";
		} else if( S_ISSOCK( st->st_mode ) )	{
			tag = "%";
		} else if( S_ISLNK( st->st_mode ) )	{
			tag = "->";
		} else if( st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH ) ) {
			tag = "*";
		}
		printf(
			"%s", tag
		);
	}
	if( S_ISDIR( st->st_mode ) )	{
		printf( "\n" );
	}
}

static void
processFile(
	char		*name,
	struct stat *	st,
	int		isLastEntry
)
{
	printName( name, st, isLastEntry, 0 );
	if( w_sw && S_ISREG( st->st_mode ) )	{
		FILE		*pipe;
		char		buf[ BUFSIZ ];
		char		*bp;
		int		column;
		column = Nprefix + 4 + strlen( name );
		/* Get the very first '^revision' out of the CVS log	 */
		sprintf( 
			buf, 
			/* Get the log info				 */
			"cvs log %s 2>/dev/null | "
			/* Extract only the description info		 */
			"sed -e '1,/^description:$/d' -e '/^--------/,$d'"
			,
			name 
		);
		pipe = popen( buf, "r" );
		if( pipe != (FILE *) NULL )	{
			buf[0] = '\n';
			fgets( buf, sizeof( buf ), pipe );
			if( (bp = strchr( buf, '\n' )) )
				*bp = '\0';
			bp = drop_leadin( buf );
			if( bp[0] != '\0' )	{
				while( column++ < whatColumn ) {
					putchar( ' ' );
				}
				printf( "-- %s", bp );
			}
			pclose( pipe );
		}
	}
	printf( "\n" );
}

static int processDirectory( char *, int );

static int
processCurrentDirectory(
	char		*path
)
{
	int		subdirs;
	struct stat 	*stp;
	struct direct	*d;
	struct direct	**namelist = (struct direct **) NULL;
	int		Nnames;
	int		i;
	int		lastentry;

	++currentDepth;
	Nnames = scandir( ".", &namelist, permit,
		(f_sw ? Falphasort : alphasort) );
	stp = (struct stat *) calloc( Nnames, sizeof( struct stat ) );
	if( stp == (struct stat *) NULL )	{
		fprintf( stderr,
		"%s: cannot allocate stat buf array\n", me );
		exit( 1 );
	}
	/* Stat the directory and count subdirectories	 */
	for( subdirs = i = 0; i < Nnames; ++i )	{
		d = namelist[ i ];
		if( stat( d->d_name, stp + i ) < 0 )	{
			fprintf( stderr,
				"%s: cannot stat '%s'\n", me,
				d->d_name );
			++nonfatal;
			stp[i].st_mode = 0;
		}
		if( S_ISDIR( stp[i].st_mode ) ) ++subdirs;
	}
	/* Print current directory name and optional trailer line	 */
	if( d_sw )	{
		if( subdirs && !s_sw )	{
			printf( "%s|\n", prefix );
		}
	} else if( Nnames && !s_sw )	{
		printf( "%s|\n", prefix );
	}
	/* Process each name in the directory in order	 */
	for( i = 0; i < Nnames; ++i )	{
		struct stat * const	st = stp + i;
		mode_t const		mode = st->st_mode;
		int const		isDir = S_ISDIR( mode );

		d = namelist[i];
		/* Show only directories if -d switch	 */
		lastentry = ( (i+1) == Nnames ) ? 1 : 0;
		if( isDir )	{
			if( d_sw )	{
				lastentry = ((--subdirs) <= 0) ? 1 : 0;
			}
			printName( d->d_name, st, lastentry, 0 );
			if( currentDepth < depth )	{
				/* Push new directory state	 */
				if( lastentry )	{
					strcpy( prefix + Nprefix, "    " );
				} else	{
					strcpy( prefix + Nprefix, "|   " );
				}
				Nprefix += 4;
				processDirectory( d->d_name, lastentry );
				Nprefix -= 4;
			}
			prefix[ Nprefix ] = '\0';
			if( !lastentry && !s_sw ) printf( "%s|\n", prefix );
		} else if( !d_sw )	{
			processFile( d->d_name, st, lastentry );
			if( !lastentry && !s_sw ) printf( "%s|\n", prefix );
		}
	}
	--currentDepth;
	free( (char *) stp );
	free( (char *) namelist );
	return( 0 );
}

static int
processDirectory(
	char		*path,
	int		isLastEntry
)
{
	char		here[ MAXPATHLEN ];
	int		status;

	if( getcwd( here, MAXPATHLEN ) == (char *) NULL )	{
		fprintf( stderr,
			"%s cannot determine current directory.\n",
			me
		);
		++nonfatal;
		return( -1 );
	}
	if( chdir( path ) )	{
		++nonfatal;
		fprintf( stderr, "%s: cannot find directory '%s'.\n",
			me, path );
		return( -1 );
	}
	status = processCurrentDirectory( path );
	if( chdir( here ) )	{
		++nonfatal;
		fprintf( stderr, "%s: cannot find directory '%s'.\n",
			me, here );
		return( -1 );
	}
	return( status );
}

/*
 *------------------------------------------------------------------------
 * main:
 *------------------------------------------------------------------------
 */

int
main(
	int		argc,
	char		*argv[]
)
{
	int		c;		/* Command line character	 */
	char *		path;		/* Path being processed		 */

	me = argv[0];
	add_ignore( "." );
	add_ignore( ".." );
	while( 
		(c = getopt( argc, argv, "aDc:dFfgl:i:n:o:psuW:w" )) != EOF 
	)	{
		switch( c ) 	{
		default:
			fprintf( stderr, "%s: no -%c yet!\n", me, c );
			/*FALLTHRU*/
		case '?':
			++nonfatal;
			break;
		case 'D':
			++debug;
			break;
		case 'a':
			++a_sw;
			break;
		case 'c':
			whatColumn = atoi( optarg );
			break;
		case 'd':
			++d_sw;
			break;
		case 'F':
			++F_sw;
			break;
		case 'f':
			++f_sw;
			break;
		case 'g':
			combo_sw |= Iwanna_group;
			break;
		case 'i':
			add_ignore( optarg );
			break;
		case 'l':
			depth = atoi( optarg );
			break;
		case 'p':
			combo_sw |= Iwanna_perms;
			break;
		case 'o':
			ofile = optarg;
			break;
		case 's':
			++s_sw;
			break;
		case 'u':
			combo_sw |= Iwanna_user;
			break;
		case 'W':
			leadin = optarg; 
			Nleadin = strlen( leadin );
			break;
		case 'w':
			++w_sw;
			break;
		}
	}
	if( nonfatal )	{
		fprintf( stderr, "%s: illegal switch(es)\n", me );
		fprintf( stderr,
			"usage: %s "
			"[-D] "
			"[-a] "
			"[-d] "
			"[-c] "
			"[-d depth] "
			"[-f] "
			"[-l limit] "
			"[-i subdir] "
			"[-s] "
			"[-o ofile] "
			"[-p] "
			"[-W prefix] "
			"[-w] "
			"[dir...]"
			"\n", 
			me 
		);
		exit( 1 );
	}
	if( ofile )	{
		(void) unlink( ofile );
		if( freopen( ofile, "w", stdout ) != stdout )	{
			fprintf(stderr, "%s: cannot create '%s'\n", me, ofile);
			exit( 1 );
		}
	}
	if( optind < argc )	{
		struct stat	st;
		int		others = 0;

		while( optind < argc )	{
			if( others++ )	{
				putchar( '\n' );
			}
			path = argv[ optind++ ];
			if( stat( path, &st ) )	{
				fprintf(
					stderr,
					"%s: "
					"cannot stat '%s'; "
					"errno=%d (%s)"
					".\n",
					me,
					path,
					errno,
					strerror( errno )
				);
				++nonfatal;
				continue;
			}
			printName( path, &st, 0, 1 );
			processDirectory( path, 1 );
		}
	} else	{
		static char	here[] = ".";
		struct stat	st;

		if( stat( here, &st ) )	{
			fprintf(
				stderr,
				"%s: "
				"cannot stat '%s'; "
				"errno=%d (%s)"
				".\n",
				me,
				here,
				errno,
				strerror( errno )
			);
		} else	{
			printName( here, &st, 0, 1 );
			processCurrentDirectory( here );
		}
	}
	return( nonfatal ? 1 : 0 );
}
