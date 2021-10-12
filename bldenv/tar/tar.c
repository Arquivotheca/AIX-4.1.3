static char sccsid[] = "@(#)10	1.49.1.39  src/bos/usr/bin/tar/tar.c, cmdarch, bos41J, 9524D_all 6/13/95 12:16:56";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: tar
 *
 * ORIGINS: 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/limits.h>
#include <tar.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/tape.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <IN/standard.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <ctype.h>
#include <locale.h>
#include <nl_types.h>
#include "tar_msg.h"
#include <sys/inode.h>

#define MSGSTR(num,str)	catgets(catd,MS_TAR,num,str)  /*MSG*/
#define	PIPSIZ		4096
#define TBLOCK		512	/*  tar block size  */
#define NBLOCK		20	/*  default num of blocks  */
#define	MAXBLOCK	800	/*  max num of blocks  */
#define	TAR_PATH_LEN	257	/*  max size of tar archive path with \0  */
#define NAME_SIZE	100	/*  file name size in tar header  */
#define	PREFIX_SIZE	155	/*  path size in tar header  */
#define	writetape(b)	writetbuf(b, 1)
#define	min(a,b)  ((a) < (b) ? (a) : (b))
#define	max(a,b)  ((a) > (b) ? (a) : (b))

			/*  hblock modified for POSIX compliance  */
#define UNAMELEN 32
#define GNAMELEN 32

union hblock {
	char dummy[TBLOCK];
	struct header {
		char name[NAME_SIZE];
		char mode[8];
		char uid[8];
		char gid[8];
		char size[12];
		char mtime[12];
		char chksum[8];
		char typeflag;
		char linkname[NAME_SIZE];
		char magic[6];
		char version[2];
		char uname[UNAMELEN];
		char gname[GNAMELEN];
		char devmajor[8];
		char devminor[8];
		char prefix[PREFIX_SIZE];
	} dbuf;
};

struct linkbuf {
	ino_t	inum;
	dev_t	devnum;
	int	count;
	char	pathname[NAME_SIZE+1];		/*  +1 to allow for \0  */
struct	linkbuf *nextp;
};

static unsigned long maxint=(unsigned long)((1L << (sizeof(int) * 8)) - 1);
static int	Nblock = NBLOCK ;
static union	hblock dblock;
static union	hblock *tbuf;
static struct	linkbuf *ihead;
static struct	stat stbuf;
static mode_t	mask;
static char	is_root;

static nl_catd	catd ;		/*  message catalog descriptor  */

static int	rflag;
static int	xflag;
static int	vflag;
static int	tflag;
static int	cflag;
static int	mflag;
static int	fflag;
static int	iflag;
static int     oflag;          /*       xtract: don't restore uid/gid  */
static int	wflag;
static int	hflag;
static int	Bflag;
static int	Fflag;
static int	Cflag;
static int	Lflag;
static int	donly;		/*  used when reading from inputlist  */
static int	dflag;
static int	sflag;
static int	bflag;
static int	Nflag;
static int	Sflag;
static int	pflag=0;

static int	mt;
static int	term;
static long	trecs;		/*  number of records written to current volume  */
static int	recno;
static int	first;
static int	prtlinkerr;
static int	freemem = 1;
static int	nblock = 0;
static int	berkeley = 0;
static int	ignorechksum = 0;
static unsigned recsize;               /* size of records on tape */
static int	nbprset ;
static char	full_name [TAR_PATH_LEN] ;
static int	exit_status = 0;	/* 0 - success, 1 - fatal, 2 - pos-nonfatal */

static int	arch_file_type;

static int	onintr(void);
static int	onquit(void);
static int	onhup(void);
static int	onterm(void);
static char 	*rel_name();

static daddr_t	high;
static daddr_t	bsrch();


				/*  tape info defaults  */
#define GAP	7500          /* Interrecord gap, .0001 inch units */
#define	DENSITY	1200
#define	TAPE_FOOT	110000L		/*  11 inches  */
static int	density = DENSITY ;
static int	tapelen ;
static int	bptape ;
static int	rptape ;

static FILE	*vfile = stdout;
static FILE	*tfile;
static FILE	*list_fd ;		/*  fd for input list file  */
static FILE	*mtfp;

#define	DEF_TAPE	"/dev/rmt0"
static char	cantopen [] = "tar: cannot open %s\n" ;
static char	tname[] = "/tmp/tarXXXXXX";
static char	usefile [PATH_MAX+1] = DEF_TAPE;	/*  +1 for \0 char  */

static char	*getpwd();
static char	*getmem();

static FILE *response_file = stdin;

main(argc, argv)
int	argc;
char	*argv[];
{
	int	cp;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_TAR, NL_CAT_LOCALE);

	if (argc < 2)
		usage();

	tfile = NULL;
	is_root = !getuid();

	/* Allow options to be specified w/o the '-'. (ala Berkeley) */
	if (argv[1][0] != '-') {
		char *bcp;

		berkeley = 1;
		argv[argc] = 0;
		argv++;
		for (bcp = *argv++; *bcp; bcp++) 
			switch(*bcp) {

			case 'o':
				oflag++;
				break;

			case 'L':
				Lflag++ ;
				if (( list_fd = fopen ( *argv , "r" )) == NULL ) {
					eprintf ( MSGSTR( ETOPEN, cantopen) , *argv ) ;
					done ( 1 ) ;
				}
				argv++;
				break ;

			case 'S':
				{
				int	len ;
				char	*mark ;
				Sflag++;
				if (*argv == 0) {
					usage();
				}
				len = strlen ( *argv ) ;
				if ( (*argv)[--len] == 'b' ) {
					(*argv)[len] = '\0' ;
					bptape = atoi ( *argv ) ;
				}
				else {
					tapelen = atoi ( *argv ) ;
					mark = strchr ( *argv , '@' ) ;
					if ( mark )
						density = atoi ( ++mark ) ;
				}
				}
				argv++;
				break ;
	
			case 'd':
				dflag++ ;
				break ;
	
			case 's':
				sflag++ ;
				break ;

			case 'f':
				if (*argv == 0) {
					fprintf(stderr, MSGSTR( EFFLAG,
				"tar: tapefile must be specified with 'f' option\n"));
					usage();
				}
				if ( strlen ( *argv ) > PATH_MAX ) {
					fprintf (stderr, MSGSTR ( ETLONGFN ,
						"tar: %s: file name too long\n") , *argv ) ;
					exit (1) ;
				}
				strcpy( usefile, *argv);
				argv++;
				fflag++;
				break;
	
			case 'c':
				cflag++;
				rflag++;
				break;
	
			case 'p':
				pflag++;
				break;
		
			case 'u':
				mktemp(tname);
				if ((tfile = fopen(tname, "w")) == NULL) {
					fprintf(stderr, MSGSTR( ETCRTMP,
					 "tar: cannot create temporary file (%s)\n"),
					 tname);
					done(1);
				}

				fputs("!!!!!/!/!/!/!/!/!/! 000\n",tfile);
				rflag++;
				break;
	
			case 'r':
				rflag++;
				break;
	
			case 'v':
				vflag++;
				break;
	
			case 'w':
				wflag++;
				break;
	
			case 'x':
				xflag++;
				break;
	
			case 't':
				tflag++;
				break;
	
			case 'm':
				mflag++;
				break;
	
			case '-':
				break;
	
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				strcpy(usefile, DEF_TAPE);
				usefile[strlen(usefile)-1] = *bcp;
				break;
	
			case 'N':
				Nflag++ ;
				Nblock = maxint / TBLOCK ;

			case 'b':
				if ( *bcp == 'b' )
					bflag++ ;
				if (*argv == 0) {
					fprintf(stderr, MSGSTR( ETNOBSIZE,
				"tar: blocksize must be specified with 'b' or 'N' options\n"));
					usage();
				}
				nblock = atoi(*argv);
				if (nblock <= 0 || nblock > Nblock) {
					fprintf(stderr, MSGSTR( EBSIZE,
					    "tar: invalid blocksize %s.\n"), *argv);
					done(1);
				}
				if ( *bcp == 'N' ) {
					if ( nblock > MAXBLOCK ) {
						nblock = MAXBLOCK ;
						eprintf ( MSGSTR( CREDUCED,
							"tar: Cluster size reduced to %d.\n" ),
							nblock ) ;
					}
					Nblock = nblock ;
				}
				argv++;
				recsize = TBLOCK * nblock;
				nbprset++ ;
				break;
	
			case 'l':
				prtlinkerr++;
				break;
	
			case 'h':
				hflag++;
				break;
	
			case 'i':
				iflag++;
				ignorechksum = 1;
				break;

			case 'B':
				Bflag++;
				break;
	
			case 'F':
				Fflag++;
				break;
	
			default:
				usage();
		}
	} else
	while (!Cflag && (cp = getopt(argc, argv, "0123456789b:cdf:hilmoprstuvwxBCFN:L:S:")) != EOF)
		switch(cp) {

		case 'o':
			oflag++;
			break;

		case 'C':
			Cflag++ ;
			if ( strncmp ( argv [optind] , "-C" , (size_t)2 ))
				optind-- ;	/*  dorep expects "-C"  */
			break ;

		case 'L':
			Lflag++ ;
			if (( list_fd = fopen ( optarg , "r" )) == NULL ) {
				eprintf ( MSGSTR( ETOPEN, cantopen) , optarg ) ;
				done ( 1 ) ;
			}
			break ;

		case 'S':
			{
			int	len ;
			char	*mark ;
			Sflag++;
			len = strlen ( optarg ) ;
			if ( optarg [--len] == 'b' ) {
				optarg [len] = '\0' ;
				bptape = atoi ( optarg ) ;
			}
			else {
				tapelen = atoi ( optarg ) ;
				mark = strchr ( optarg , '@' ) ;
				if ( mark )
					density = atoi ( ++mark ) ;
			}
			}
			break ;

		case 'd':
			dflag++ ;
			break ;

		case 's':
			sflag++ ;
			break ;

		case 'f':
			if (*optarg == 0) {
				fprintf(stderr, MSGSTR( EFFLAG,
			"tar: file must be specified with 'f' option\n"));
				usage();
			}
			if ( strlen ( optarg ) > PATH_MAX ) {
				fprintf (stderr, MSGSTR ( ETLONGFN ,
					"tar: %s: file name too long\n") , optarg ) ;
				exit (1) ;
			}
			strcpy ( usefile , optarg ) ;
			fflag++;
			break;

		case 'c':
			cflag++;
			rflag++;
			break;

		case 'p':
			pflag++;
			break;
		
		case 'u':
			mktemp(tname);
			if ((tfile = fopen(tname, "w")) == NULL) {
				fprintf(stderr, MSGSTR( ETCRTMP,
				 "tar: cannot create temporary file (%s)\n"),
				 tname);
				done(1);
			}

			fputs("!!!!!/!/!/!/!/!/!/! 000\n",tfile);
			rflag++;
			break;

		case 'r':
			rflag++;
			break;

		case 'v':
			vflag++;
			break;

		case 'w':
			wflag++;
			break;

		case 'x':
			xflag++;
			break;

		case 't':
			tflag++;
			break;

		case 'm':
			mflag++;
			break;

		case '-':
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			strcpy(usefile, DEF_TAPE);
			usefile[strlen(usefile)-1] = cp;
			break;

		case 'N':
			Nflag++ ;
			Nblock = maxint / TBLOCK ;

		case 'b':
			if ( cp == 'b' )
				bflag++ ;
			if (*optarg == 0) {
				fprintf(stderr, MSGSTR ( ETNOBSIZE,
			"tar: blocksize must be specified with 'b' or 'N' option\n"));
				usage();
			}
			nblock = atoi(optarg);
			if (nblock <= 0 || nblock > Nblock) {
				fprintf(stderr, MSGSTR ( EBSIZE,
				    "tar: invalid blocksize %s.\n" ), optarg);
				done(1);
			}
			if ( cp == 'N' ) {
				if ( nblock > MAXBLOCK ) {
					nblock = MAXBLOCK ;
					eprintf ( MSGSTR( CREDUCED,
						"tar: Cluster size reduced to %d.\n" ),
						nblock ) ;
				}
				Nblock = nblock ;
			}
			recsize = TBLOCK * nblock;
			nbprset++ ;
			break;

		case 'l':
			prtlinkerr++;
			break;

		case 'h':
			hflag++;
			break;

		case 'i':
			iflag++;
			ignorechksum = 1;
			break;

		case 'B':
			Bflag++;
			break;

		case 'F':
			Fflag++;
			break;

		default:
			usage();
		}

	if (berkeley) optind = 0;
	if (!rflag && !xflag && !tflag)
		usage();
	if (cflag + rflag + tflag + xflag > (cflag ? 2 : 1))
		usage();
	if (wflag && (!strcmp(usefile,"-"))) {
		if ((response_file = fopen("/dev/tty","r")) == NULL) {
			fprintf(stderr, MSGSTR(NODEVTTYOPEN,
"tar: can not open /dev/tty for interactive operation.\n"));
			done(1);
		}
	}
	if (rflag) {
		if (cflag && tfile != NULL)
			usage();
		if (signal(SIGINT, SIG_IGN) != SIG_IGN)
			(void) signal(SIGINT, (void (*)(int))onintr);
		if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
			(void) signal(SIGHUP, (void (*)(int))onhup);
		if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
			(void) signal(SIGQUIT, (void (*)(int))onquit);
		if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
			(void) signal(SIGTERM, (void (*)(int))onterm);
		mt = openmt(usefile, O_RDWR);
		dorep(argv + optind);
		if (close(mt) < 0) {
		    perror ( MSGSTR( ETWRITE, "tar: tape write error" )) ;
		    done (1) ;
		}
		done(exit_status);
	}
	mt = openmt(usefile, O_RDONLY);
	{
	struct stat	tstat ;
	if ( fstat(mt, &tstat) != 0 ) {
		
		eprintf ( MSGSTR( ESTAT,
			"tar: could not stat file = %s\n") , usefile ) ;
		done (1) ;
	}
				/*  check for fifo  */
	if ( !nbprset && tstat.st_mode == S_IFIFO ) {
		recsize = PIPSIZ ;
		nblock = PIPSIZ / TBLOCK ;
		nbprset++ ;
	}

	arch_file_type=tstat.st_mode & S_IFMT;

	if ( xflag )
		doxtract(argv + optind);
	else
		dotable(argv + optind);
	}
	done(exit_status);
}

static
usage()
{
	eprintf( MSGSTR( USAGE1,
		"usage: tar -{crtux} [-BFdhilmopsvw] [-num] [-ffile]\n"));
	eprintf( MSGSTR( USAGE2,
		"\t\t    [-bblocks] [-S feet] [-S feet@density] [-S blocksb]\n"));
	eprintf( MSGSTR( USAGE3,
		"\t\t    [-Linputlist] [-Nblocks] [-C directory] file ...\n"));
	done(1);
}

static int
openmt(tape, writing)
	char *tape;
	int writing;
{

	if (strcmp(tape, "-") == 0) {
		/*
		 * Read from standard input or write to standard output.
		 */
		if (writing) {
			if (cflag == 0) {
				fprintf(stderr, MSGSTR( ESTDOUT,
			 "tar: can only create standard output archives\n"));
				done(1);
			}
			vfile = stderr;
			setlinebuf(vfile);
			mt = dup(1);
		} else {
			mt = dup(0);
			Bflag++;
		}
	} else {
		/*
		 * Use file or tape on local machine.
		 */
		if (writing) {
			if (cflag)
				mt = open(tape, O_RDWR|O_CREAT|O_TRUNC, 0666);
			else
				mt = open(tape, O_RDWR);
		} else
			mt = open(tape, O_RDONLY);
		if (mt < 0) {
			fprintf(stderr, MSGSTR( ETAR, "tar: "));
			perror(tape);
			done(1);
		}
	}
	return(mt);
}


/*	dorep will create a new archive or append to an existing archive.
 */

static
dorep(argv)
	char *argv[];
{
	register char *cp2;
	char wdir[TAR_PATH_LEN], tempdir[TAR_PATH_LEN], *parent;

	{
	struct stat	tstat;
	unsigned short	f_mode ;
	long offset;
	int errnum=0,val1=0;

	fstat(mt, &tstat);
	arch_file_type = f_mode = tstat.st_mode & S_IFMT ;
	if (!cflag) {
				/*  request is update or replace  */
		(void) getdir();
		do {
			passtape();
			if (term)
				done(1);
			(void) getdir();
		} while (!endtape());
		trecs-- ;
		mtfp = fdopen(mt, "r");
		if (nblock == MAXBLOCK) {
			offset = (ftell(mtfp) / TBLOCK) - (trecs * nblock);
			backspc(offset*TBLOCK);
		}
		else
			backspc(TBLOCK*nblock);
		recno--;
		if (tfile != NULL) {
			char buf[200];

			sprintf(buf,
"sort -k 1,1 -k 2nr %s | awk '$1 != prev {print; prev=$1}' > %sX; mv %sX %s",
				tname, tname, tname, tname);
			fflush(tfile);
			system(buf);
			freopen(tname, "r", tfile);
			fstat((int)fileno(tfile), &stbuf);
			high = stbuf.st_size;
		}
	}
	else
					/*  check for pipe as output  */
		if ( !nbprset && f_mode == S_IFIFO ) {
			recsize = PIPSIZ ;
			nblock = PIPSIZ / TBLOCK ;
			nbprset++ ;
		}

	}

	(void) getpwd(wdir);
				/*  read from input arguments first
				 *  and then handle input from file
				 */
	while ((Lflag || *argv ) && ! term) {
		char	fname [TAR_PATH_LEN] ;
		if ( *argv ) {
			if ( strlen ( *argv ) >= TAR_PATH_LEN ) {
				fprintf ( stderr , MSGSTR( ETLONGFN,
					"tar: %s: file name too long\n") , *argv ) ;
				if (exit_status != 1)
					exit_status = 2;
				*argv++ ;
				continue ;
			}
			strcpy ( fname , *argv ) ;
		}
		else {
			char	*ch ;
			/* set donly flag to indicate only the directory entry
			 * will be archived and not the contents of the directory
			 */
			if ( !donly )
				donly++ ;

			/*  read from input file  */
			fgets ( fname , TAR_PATH_LEN , list_fd ) ;
			if ( feof ( list_fd )) {
				Lflag = 0 ;
				(void) fclose ( list_fd ) ;
				continue ;
			}
					/*  replace \n with null  */
			if ( ch = index ( fname , '\n' ))
				*ch = '\0' ;
			else {
					/*  file name is too long
					    read until \n is encountered  */
				fprintf ( stderr , MSGSTR( ETAR , "tar: " )) ;
				do {
					fputs(fname,stderr);
					fgets ( fname , TAR_PATH_LEN , list_fd ) ;
					ch = index ( fname , '\n' ) ;
				}
				while ( *ch != '\n' ) ;
				*ch = '\0' ;
				fprintf ( stderr , MSGSTR( ELONGFN,
					"%s: file name too long\n" ), fname ) ;
				if (exit_status != 1)
					exit_status = 2;
				continue ;
			}
		}
		cp2 = fname ;

		if (!strncmp(cp2, "-C" , (size_t)2)) {
					/*  two formats -Cdirectory or
					 *  -C directory
					 */
			if ( !strcmp (cp2 , "-C")) {
				*argv++ ;	/*  -C directory format  */
				cp2 = *argv ;
			}
			else
				cp2 += 2 ;	/*  -Cdirectory format  */

					/*  check for null directory  */
			if ( !cp2 )
				continue ;
			if (chdir(cp2) < 0) {
				fprintf(stderr, MSGSTR( ECHDIR,
					"tar: can't change directories to "));
				perror(cp2);
				if (exit_status != 1)
					exit_status = 2;
			} else
				(void) getpwd(wdir);
			*argv++;
			continue;
		}
		parent = wdir;
		cp2 = rindex ( fname , '/' ) ;
		if (cp2) {
			if (cp2 == fname) {
			    char    save = *++cp2;

			    *cp2 = '\0';
			    if (chdir(fname) < 0) {
				    fprintf(stderr, MSGSTR( ECHDIR,
				    "tar: can't change directories to "));
				    perror(fname);
				    if (exit_status != 1)
					    exit_status = 2;
				    /* When reading from input  */
				    /* file, argv is not valid. */
				    if ( *argv )
					    argv++;
				    continue;
			    }
			    *cp2 = save;
			} else {
			    *cp2 = '\0';
			    if (chdir(fname) < 0) {
				    fprintf(stderr, MSGSTR( ECHDIR,
				    "tar: can't change directories to "));
				    perror(fname);
				    if (exit_status != 1)
					    exit_status = 2;
				    /* When reading from input  */
				    /* file, argv is not valid. */
				    if ( *argv )
					    argv++;
				    continue;
			    }
			    *cp2 = '/';
			    cp2++;
			}
			parent = getpwd(tempdir);
		}
		else		/*  string does not contain "/"  */
			cp2 = fname ;

		/* P39755 : Handle file names like /  /dir/ /dir/dir/ ... *
		 */
		if((*cp2 == '\0') && (*fname != '\0'))
			cp2="." ;

		putfile(fname, cp2, parent, 0);
		if ( *argv )
			*argv++ ;
		if (chdir(wdir) < 0) {
			fprintf(stderr, MSGSTR( ECHBACK,
				"tar: cannot change back: "));
			perror(wdir);
			exit_status = 1;
		}
	}
	putempty();
	putempty();
	if (recno != 0)
		bwrite ((char *) tbuf ) ;
	if (prtlinkerr == 0)
		return;
	for (; ihead != NULL; ihead = ihead->nextp) {
		if (ihead->count == 0)
			continue;
		fprintf(stderr, MSGSTR( ELINKS,
			"tar: missing links to %s\n" ), ihead->pathname);
		exit_status = 1;
	}
}

static
backspc(nbytes)
{
	int count;
	struct devinfo devnfo ;
	struct stop st_com;

	if (ioctl(mt, IOCINFO, &devnfo) == -1)
		devnfo.devtype = '#';
	if ((devnfo.devtype == DD_TAPE) || (devnfo.devtype == DD_SCTAPE)) {
		if (devnfo.un.scmt.blksize == 0)
			count = nbytes/(TBLOCK*nblock);
		else
			count = nbytes/devnfo.un.scmt.blksize;
		st_com.st_op = STRSR;
		st_com.st_count = count;
		if (ioctl(mt, STIOCTOP, &st_com) < 0) {
			perror(MSGSTR(BACKSPCI, "tar: ioctl backspace error"));
			done (1) ;
		}
	} else {
		if (lseek(mt, -(daddr_t)nbytes, SEEK_CUR) == -1) {
			perror(MSGSTR(BACKSPCL, "tar: lseek backspace error"));
			done (1) ;
		}
	}
}

static
endtape()
{
	return (dblock.dbuf.name[0] == '\0');
}


	/*	The function getdir will read a header from the archive
	 *	file. A POSIX header will contain all needed information
	 *	in the correct fields. Non POSIX headers need to be
	 *	checked and completed (e.g. major and minor device # in
	 *	the mtime field).
	 */
static getdir()
{
	register struct stat *sp = &stbuf;
	int i, ftype;
	int	chksum;

top:
	readtape((char *)&dblock);
	if (dblock.dbuf.name[0] == '\0' && !ignorechksum)
		return;

	sscanf(dblock.dbuf.chksum, "%0o", &chksum);
	if ((chksum != (i = checksum())) && (chksum != signed_checksum())) {
		fprintf(stderr, MSGSTR( ECSUMC,
			"tar: directory checksum error (%d != %d)\n" ),
		    chksum, i);
					/*  ignore checksum errors ?  */
		if (iflag)
			goto top;
		done(1);
	}

	/* since we got a good chksum, we can now turn off ignorechksum
	 * which was primaryly for skipping over the initial data blocks
	 * so that we can start extraction from any arbitrary volume */
	ignorechksum = 0;
	sscanf(dblock.dbuf.mode, "%0o", &i);
	sp->st_mode = (i & 07777);

	/* We need to do the following for POSIX which requires 'typeflag'
	   be used for setting the S_IFMT part of the mode bits */

	switch(dblock.dbuf.typeflag) {
		case DIRTYPE:
			sp->st_mode |= S_IFDIR;
			break;
		case  FIFOTYPE:
			sp->st_mode |= S_IFIFO;
			break;
		case CHRTYPE:
			sp->st_mode |= S_IFCHR;
			break;
		case BLKTYPE:
			sp->st_mode |= S_IFBLK;
			break;
		case AREGTYPE:
		case REGTYPE:
			i = strlen(dblock.dbuf.name);
			if (i > NAME_SIZE)
				i = NAME_SIZE;
			if (dblock.dbuf.name[i-1] == '/') {
				dblock.dbuf.typeflag = DIRTYPE;
				sp->st_mode |= S_IFDIR;
			} else
				sp->st_mode |= S_IFREG;
			break;
		default:
			break;
	}

				/*  check for posix header  */
	bzero ( full_name , sizeof (full_name)) ;
	if ( ! strncmp ( dblock.dbuf.magic , TMAGIC , (size_t)TMAGLEN )) {
		if (i = strlen ( dblock.dbuf.prefix)) {
				/*  use prefix + '/' + name  */
			strncpy(full_name, dblock.dbuf.prefix, (size_t)PREFIX_SIZE);
			/* don't want '//file' */
			if (strcmp(full_name, "/"))
				full_name[i++] = '/';
		}
		strncpy ( &full_name [i] , dblock.dbuf.name , (size_t)NAME_SIZE ) ;

			/*  set uid and gid based on uname and gname */
		if (!name_to_uid(dblock.dbuf.uname, &sp->st_uid)) {
			sscanf(dblock.dbuf.uid, "%0o", &i);
			sp->st_uid = i;
		}
		if (!name_to_gid(dblock.dbuf.gname, &sp->st_gid)) {
			sscanf(dblock.dbuf.gid, "%0o", &i);
			sp->st_gid = i;
		}
		sscanf(dblock.dbuf.mtime, "%0lo", &sp->st_mtime);
	}
	else {
				/*  older archive file  */
		strncpy ( full_name , dblock.dbuf.name , (size_t)NAME_SIZE ) ;
		sscanf(dblock.dbuf.uid, "%0o", &i);
		sp->st_uid = i;
		sscanf(dblock.dbuf.gid, "%0o", &i);
		sp->st_gid = i;

				/*  if special file then decode major
				    and minor number in mtime  */
		sscanf(dblock.dbuf.mtime, "%0lo", &sp->st_mtime);
		ftype = sp->st_mode & S_IFMT ;
		switch ( ftype ) {
			case S_IFIFO :
			case S_IFBLK :
			case S_IFCHR : {
				int	val ;
					/*  place major and minor num
					 *  in dblock header
					 */
				val = ((sp->st_mtime >> 8) & 0xff00) |
					(sp->st_mtime & 0xff);
				sprintf(dblock.dbuf.devminor, "%06o " , val);
				val = (sp->st_mtime >> 8) & 0xff;
				sprintf(dblock.dbuf.devmajor, "%06o " , val);
				break ;
				}

			default :
				/* Required for POSIX */
				sprintf(dblock.dbuf.devminor, "%06o " , 0);
				sprintf(dblock.dbuf.devmajor, "%06o " , 0);
				break;
		}

	}

	sscanf(dblock.dbuf.size, "%0lo", &sp->st_size);
	if (tfile != NULL)
		fprintf(tfile, "%s %.12s\n", full_name, dblock.dbuf.mtime);

	return (TRUE) ;
}

static passtape()
{
	long blocks;
	char *bufp;

	if (dblock.dbuf.typeflag == LNKTYPE)
		return;
	blocks = stbuf.st_size;
	blocks += TBLOCK-1;
	blocks /= TBLOCK;

	while (blocks-- > 0)
		(void) readtbuf(&bufp, TBLOCK);
}

static putfile(longname, shortname, parent, recursive)
	char *longname;
	char *shortname;
	char *parent;
	int  recursive;
{
	int infile = 0;
	long blocks;
	char buf[TBLOCK];
	char *bigbuf;
	register char *cp;
	struct dirent *dp;
	DIR *dirp;
	register int i;
	long l;
	char newparent[PATH_MAX];
	char linknamebuf[NAME_SIZE + 1];
	int	maxread;
	int	hint;		/* amount to write to get "in sync" */

	if (!hflag)
		i = lstat(shortname, &stbuf);
	else
		i = stat(shortname, &stbuf);
	if (i < 0) {
		fprintf(stderr, MSGSTR( ETAR, "tar: " ));
		perror(longname);
		if (exit_status != 1)
			exit_status = 2;
		return;
	}
	if (tfile != NULL && checkupdate(longname) == 0)
		return;
	if (checkw(MSGSTR(DOARCH, "a %s: "), longname) == 0)
		return;
	if (Fflag && checkf(shortname, stbuf.st_mode) == 0)
		return;

	switch (stbuf.st_mode & S_IFMT) {
	case S_IFDIR:
		for (i = 0, cp = buf; *cp++ = longname[i++];)
			;
		if ( buf [--i-1] != '/' )
			buf [i++] = '/' ;
		cp = &buf [i] ;
		*cp = 0 ;
		/* By default tar used to NOT archive empty directories,
		 * the -d option (dflag) was required to do so.  This
		 * is now done by default and the -d option now only
		 * archives special files and fifo's.
		 */

		stbuf.st_size = 0;
		if ( tomodes(&stbuf, longname , shortname )) {
			sprintf(dblock.dbuf.chksum, "%06o", checksum());
			(void) writetape((char *)&dblock);
			if ( vflag )
				fprintf(vfile, "a %s\n", longname);
		}
		/*  donly flag indicates to only archive the given directory,
		 *  and not its contents, so skip past the code to add the
		 *  directory's contents to the archive.
		 */
		if (donly)
			break ;

				/*  handle entries that are / and /dir  */
		if ( strcmp ( parent , shortname )) {
			if ( parent [strlen (parent)-1] == '/' )
				sprintf(newparent, "%s%s", parent, shortname);
			else
				sprintf(newparent, "%s/%s", parent, shortname);
			if (chdir(shortname) < 0) {
				perror(shortname);
				return;
			}
		} else
			strcpy ( newparent , parent ) ;

		if ((dirp = opendir(".")) == NULL) {
			fprintf(stderr, MSGSTR( EDIRREAD,
				"tar: %s: directory read error\n" ),
				longname);
			if (chdir(parent) < 0) {
				fprintf(stderr, MSGSTR( ECHBACK,
					"tar: cannot change back: "));
				perror(parent);
			}
			exit_status = 1;
			return;
		}
		while ((dp = readdir(dirp)) != NULL && !term) {
			if (dp->d_ino == 0)
				continue;
			if (!strcmp(".", dp->d_name) ||
			    !strcmp("..", dp->d_name))
				continue;
			strcpy(cp, dp->d_name);

			if ( buf [0] == '/' && strcmp ( &buf[1] , cp ) == 0 )
				putfile(buf, buf, newparent, 1);
			else
				putfile(buf, cp, newparent, 1);
		}
		closedir(dirp);
		if (chdir(parent) < 0) {
			fprintf(stderr, MSGSTR( ECHBACK,
				"tar: cannot change back: "));
			perror(parent);
			exit_status = 1;
		}
		break;

	case S_IFLNK:
		if ( ! tomodes ( &stbuf , longname , shortname ))
			return;
		if (stbuf.st_size > NAME_SIZE) {
			fprintf(stderr, MSGSTR( ELONGSL,
				"tar: %s: symbolic link too long\n" ),
				longname);
			if (exit_status != 1)
				exit_status = 2;
			return;
		}
		i = readlink(shortname, dblock.dbuf.linkname, NAME_SIZE);
		if (i < 0) {
			fprintf(stderr, MSGSTR( EREADSL,
				"tar: can't read symbolic link: "));
			perror(longname);
			if (exit_status != 1)
				exit_status = 2;
			return;
		}
		if (i < NAME_SIZE)
			dblock.dbuf.linkname[i] = '\0';
		dblock.dbuf.typeflag = SYMTYPE;
		if (vflag) {
			strncpy(linknamebuf, dblock.dbuf.linkname, NAME_SIZE);
			linknamebuf[NAME_SIZE] = '\0';
			fprintf(vfile, MSGSTR( SLINKTO,
				"a %s symbolic link to %s\n" ),
				longname, linknamebuf);
		}
		sprintf(dblock.dbuf.size, "%011lo ", 0);
		sprintf(dblock.dbuf.chksum, "%06o", checksum());
		(void) writetape((char *)&dblock);
		break;

	case S_IFREG:
		if ( ! tomodes ( &stbuf , longname , shortname ))
			return;
		if ((infile = open(shortname, O_RDONLY)) < 0) {
			fprintf(stderr, MSGSTR( ETAR, "tar: "));
			perror(longname);
			if (exit_status != 1)
				exit_status = 2;
			return;
		}
		if(( i = link_file (longname)) < 0) {
			return;
		}
		else if(i) {
			sprintf(dblock.dbuf.size, "%011lo ", 0); /* POSIX */
			sprintf(dblock.dbuf.chksum, "%06o", checksum());
			(void) writetape( (char *) &dblock);
			close(infile);
			return ;
		}
		blocks = (stbuf.st_size + (TBLOCK-1)) / TBLOCK;
		if (vflag)
			fprintf(vfile, MSGSTR( BLKS,
				"a %s %ld blocks\n" ), longname, blocks);
		sprintf(dblock.dbuf.chksum, "%06o", checksum());
		hint = writetape( (char *) &dblock);
		maxread = max(stbuf.st_size, (nblock * TBLOCK));
		if ((bigbuf = (char *) malloc((size_t)maxread)) == 0) {
			maxread = TBLOCK;
			bigbuf = buf;
		}

		while ((i = read(infile, bigbuf, (unsigned)(min((hint*TBLOCK), maxread)))) > 0
		  && blocks > 0) {
		  	register int nblks;

			nblks = ((i-1)/TBLOCK)+1;
		  	if (nblks > blocks)
		  		nblks = blocks;
			hint = writetbuf(bigbuf, nblks);
			blocks -= nblks;
		}
		close(infile);
		if (bigbuf != buf)
			free((void *)bigbuf);
		if (i < 0) {
			fprintf(stderr, MSGSTR( EREAD, "tar: Read error on "));
			perror(longname);
			exit_status = 1;
		} else if (blocks != 0 || i != 0) {
			fprintf(stderr, MSGSTR( EFSIZE,
				"tar: %s: file changed size\n" ),
				longname);
			if (exit_status != 1)
				exit_status = 2;
		}
		while (--blocks >=  0)
			putempty();
		break;

	case S_IFBLK:
	case S_IFCHR:
	case S_IFIFO:
				/*  special files - write header  */
		if ( dflag ) {
			stbuf.st_size = 0;
			if ( ! tomodes ( &stbuf , longname , shortname ))
				return;
			if ( vflag && ( link_file (longname)) == 0)
				fprintf(vfile, "a %s\n", longname);
			sprintf(dblock.dbuf.chksum, "%06o", checksum());
			(void) writetape( (char *) &dblock);
			break ;
		}

	default:
		fprintf(stderr, MSGSTR( ENOARCH,
			"tar: %s could not be archived\n" ), longname);
		if (exit_status != 1)
			exit_status = 2;
		break;
	}
}


static link_file (longname)
	char *longname;
{
	struct linkbuf *lp;
	int found = 0;

	if (stbuf.st_nlink > 1) {

		for (lp = ihead; lp != NULL; lp = lp->nextp)
			if (lp->inum == stbuf.st_ino &&
			    lp->devnum == stbuf.st_dev) {
				found++;
				break;
			}
		if (found) {
		/* If the linked to filename is > NAME_SIZE, lp->pathname will
		   be empty.  */
			if(lp->pathname[0] == '\0') {
               fprintf(stderr, MSGSTR( ELONGLN,
                    "tar: %s linked to a file with a long pathname\n" ),
                    longname);
				return(-1);
			}

			strncpy(dblock.dbuf.linkname, lp->pathname , (size_t)NAME_SIZE );
			dblock.dbuf.typeflag = LNKTYPE;
			if (vflag)
				fprintf(vfile, MSGSTR( LINKTO,
					"a %s link to %s\n" ),
					longname, lp->pathname);
			lp->count--;
		}
		else {
			lp = (struct linkbuf *) getmem(sizeof(*lp));
			if (lp != NULL) {
				lp->nextp = ihead;
				ihead = lp;
				lp->inum = stbuf.st_ino;
				lp->devnum = stbuf.st_dev;
				lp->count = stbuf.st_nlink - 1;
				/* Not allowed to link to this file if its name
				   is > NAME_SIZE */
				if((strlen(longname)) <= NAME_SIZE)
					strcpy(lp->pathname, longname);
				else
					lp->pathname[0] = '\0';

			}
		}
	}

	return ( found ) ;

}	/*  end link_file  */


static
doxtract(argv)
	char *argv[];
{
	long blocks, bytes;
	int ofile, i;
	int	ftype ;
	char *rname;
	char linknamebuf[NAME_SIZE + 1];

	/*
	 * if ('-p' flag) or (root user) then ignore umask.
	 * NOTE: 'mask' variable is needed for intermediate
	 * directory creation, even when umask is ignored.
	 */
	mask = umask(0);
	if (!pflag && getuid())
		umask(mask);
	for (;;) {
		if ((i = wantit(argv)) == 0)
			continue;
		if (i == -1)
			break;		/* end of tape */
		if (checkw(MSGSTR(DOEXTR, "x %s: "), full_name) == 0) {
			passtape();
			continue;
		}
		if (Fflag) {
			char *s;

			if ((s = rindex(full_name, '/')) == 0)
				s = full_name;
			else
				s++;
			if (checkf(s, stbuf.st_mode) == 0) {
				passtape();
				continue;
			}
		}
		checkdir();
		if(dblock.dbuf.typeflag == DIRTYPE) {
			if (access(full_name, 0) < 0) {
				if (mkdir(full_name,stbuf.st_mode) < 0) {
					perror(full_name);
					return (0);
				}
			}
			if (!oflag && is_root)
				chown(full_name, stbuf.st_uid, stbuf.st_gid);
			if (vflag)
				fprintf(vfile, "x %s\n", full_name ) ;
			if (mflag == 0)
				dodirtimes();
			continue;
		}		

		if (dblock.dbuf.typeflag == SYMTYPE) {	/* symlink */
			strncpy(linknamebuf, dblock.dbuf.linkname, NAME_SIZE);
			linknamebuf[NAME_SIZE] = '\0';
			/*
			 * only unlink non directories or empty
			 * directories
			 */
			if (rmdir(full_name) < 0) {
				if (errno == ENOTDIR)
					unlink(full_name);
			}
			if (symlink(linknamebuf, full_name)<0) {
				fprintf(stderr, MSGSTR( ESYMFAIL,
					"tar: %s: symbolic link failed: " ),
					full_name);
				perror("");
				exit_status = 1;
				continue;
			}
			if (vflag)
				fprintf(vfile, MSGSTR( XSYMLINK,
					"x %s symbolic link to %s\n" ),
					full_name, linknamebuf);
#ifndef _BLD
			/* The '#ifndef' is here so that the build environment
			 * version of tar will compile.  lchown() is not
			 * available to all build environments.
			 */
			if (!oflag && is_root)
				lchown(full_name, stbuf.st_uid, stbuf.st_gid);
#endif /* _BLD */
			/* ignore alien orders */
			/* Commented out (as is BSD 4.3) since modifications
			   are to linked to file, not link itself. */
			/*
			if (mflag == 0)
				setimes(full_name, stbuf.st_mtime);
			*/
			continue;
		}
		if (dblock.dbuf.typeflag == LNKTYPE) {	/* regular link */
			char link_type ;

			strncpy(linknamebuf, dblock.dbuf.linkname, NAME_SIZE);
			linknamebuf[NAME_SIZE] = '\0';
			/*
			 * only unlink non directories or empty
			 * directories
			 */
			if (rmdir(full_name) < 0) {
				if (errno == ENOTDIR)
					unlink(full_name);
			}
			if (link(linknamebuf, full_name) < 0) {
				if ( !sflag ) {
					fprintf(stderr, MSGSTR( EFLINK,
						"tar: can't link %s to %s: " ),
						full_name, linknamebuf);
					perror("");
					if (exit_status != 1)
						exit_status = 2;
					continue;
				}
				else
					rname = rel_name(linknamebuf, full_name);
					if ( symlink(rname, full_name)<0) {
						fprintf(stderr, MSGSTR (ESYMFAIL,

						"tar: %s: symbolic link failed: " ),
				    		full_name);
						perror("");
						if (exit_status != 1)
							exit_status = 2;
						continue;
					}
					else {
						struct stat	lk_stat ;
						if ( stat ( linknamebuf , &lk_stat ) < 0 ) {
							if (!oflag && is_root)
								chown(full_name, lk_stat.st_uid, lk_stat.st_gid);
						}
						link_type = SYMTYPE ;
					}
			}
			else
				link_type = LNKTYPE ;

			if (vflag)
				if ( link_type == LNKTYPE )
					fprintf(vfile, MSGSTR( LINKEDTO,
					"%s linked to %s\n" ),
					full_name, linknamebuf);
				else
					fprintf(vfile, MSGSTR( XSYMLINK,
					"x %s symbolic link to %s\n" ),
					full_name, linknamebuf);
			continue;
		}

				/*  check for special files  */
		if (( ftype = stbuf.st_mode & S_IFMT ) == S_IFIFO ||
			ftype == S_IFBLK || ftype == S_IFCHR ) {
			if ( mknod ( full_name , (int) stbuf.st_mode ,
					decode ()) < 0 )  {
				eprintf ( MSGSTR( EMKNOD,
					"cannot mknod %s\n" ), full_name ) ;
				continue ;
			}
			if (!oflag && is_root)
				chown(full_name, stbuf.st_uid, stbuf.st_gid);
			if (vflag)
				fprintf(vfile, "x %s\n", full_name ) ;
			continue ;
		}

		if ((ofile = creat(full_name,stbuf.st_mode)) < 0) {
			fprintf(stderr, MSGSTR( ETCREATE,
				"tar: can't create %s: " ),
				full_name);
			perror("");
			exit_status = 1;
			passtape();
			continue;
		}
		if (!oflag && is_root)
			chown(full_name, stbuf.st_uid, stbuf.st_gid);
		blocks = ((bytes = stbuf.st_size) + TBLOCK-1)/TBLOCK;
		if (vflag)
			fprintf(vfile, MSGSTR( XSTAT,
				"x %s, %ld bytes, %ld tape blocks\n" ),
				full_name, bytes, blocks);
		for (; blocks > 0;) {
			register int nread;
			char	*bufp;
			register int nwant;
			
			nwant = NBLOCK*TBLOCK;
			if (nwant > (blocks*TBLOCK))
				nwant = (blocks*TBLOCK);
			nread = readtbuf(&bufp, nwant);
			if (write(ofile, bufp, (unsigned)min(nread, bytes)) < 0) {
				fprintf(stderr, MSGSTR( EXWRITE,
					"tar: %s: Cannot write extracted data: "),
					full_name);
				perror("");
				done(1);
			}
			bytes -= nread;
			blocks -= (((nread-1)/TBLOCK)+1);
		}
		if (close(ofile) < 0) {
			fprintf(stderr, MSGSTR( EXWRITE,
				"tar: %s: Cannot write extracted data: "),
				full_name);
			perror("");
			done(1);
		}
		if (mflag == 0)
			setimes(full_name, stbuf.st_mtime);
	}
	if (mflag == 0) {
		full_name[0] = '\0';	/* process the whole stack */
		dodirtimes();
	}
}


	/*	The function decode will return a value which can be
	 *	used in the function call mknod. The value is extracted
	 *	from the dblock major and minor character strings.
	 */
static
decode ()
{
	int	dev_min , dev_maj ;

	sscanf(dblock.dbuf.devminor, "%0o", &dev_min);
	sscanf(dblock.dbuf.devmajor, "%0o", &dev_maj);
	return ( makedev ( dev_maj , dev_min )) ;
}	/*  end decode  */


static
dotable(argv)
	char *argv[];
{
	register int i;
	char linknamebuf[NAME_SIZE + 1];

	for (;;) {
		if ((i = wantit(argv)) == 0)
			continue;
		if (i == -1)
			break;		/* end of tape */
		if (vflag)
			longt(&stbuf);

		fputs(full_name,stdout);

		if (dblock.dbuf.typeflag == LNKTYPE) {
			strncpy(linknamebuf, dblock.dbuf.linkname, NAME_SIZE);
			linknamebuf[NAME_SIZE] = '\0';
			printf(MSGSTR(LINKED, " linked to %s" ), linknamebuf);
		}
		if (dblock.dbuf.typeflag == SYMTYPE) {
			strncpy(linknamebuf, dblock.dbuf.linkname, NAME_SIZE);
			linknamebuf[NAME_SIZE] = '\0';
			printf(MSGSTR(SLINKED, " symbolic link to %s" ), linknamebuf);
		}

		fputc('\n',stdout);
		passtape();
	}
}

static putempty()
{
	char buf[TBLOCK];

	bzero(buf, sizeof (buf));
	(void) writetape(buf);
}

static longt(st)
	register struct stat *st;
{
	char *ctime(), timbuf[26];

	pmode(st);
	printf(" %3d %-2d", st->st_uid, st->st_gid);
	printf(" %7ld ", st->st_size);
	(void)ctime(&st->st_mtime);
	strftime(timbuf,(size_t)26,"%sD %T %Y",localtime(&st->st_mtime));
	printf("%s ",timbuf);
}

	/*	These constants are defined in tar.h
	 */
static int	m1[] = { 1, TUREAD, 'r', '-' };
static int	m2[] = { 1, TUWRITE, 'w', '-' };
static int	m3[] = { 2, TSUID, 's', TUEXEC, 'x', '-' };
static int	m4[] = { 1, TGREAD, 'r', '-' };
static int	m5[] = { 1, TGWRITE, 'w', '-' };
static int	m6[] = { 2, TSGID, 's', TGEXEC, 'x', '-' };
static int	m7[] = { 1, TOREAD, 'r', '-' };
static int	m8[] = { 1, TOWRITE, 'w', '-' };
static int	m9[] = { 2, TSVTX, 't', TOEXEC, 'x', '-' };

static int	*m[] = { m1, m2, m3, m4, m5, m6, m7, m8, m9};

static pmode(st)
	register struct stat *st;
{
	char	c ;
	register int **mp;

	switch ( st->st_mode & S_IFMT ) {
		case S_IFREG :
			c = '-' ; break ;
		case S_IFDIR :
			c = 'd' ; break ;
		case S_IFLNK :
			c = 'l' ; break ;
		case S_IFCHR :
			c = 'c' ; break ;
		case S_IFBLK :
			c = 'b' ; break ;
		case S_IFIFO :
			c = 'p' ; break ;
		default :
			c = '?' ; break ;
	}
	putchar (c) ;
	for (mp = &m[0]; mp < &m[9];)
		selectbits(*mp++, st);
}

static selectbits(pairp, st)
	int *pairp;
	struct stat *st;
{
	register int n, *ap;

	ap = pairp;
	n = *ap++;
	while (--n>=0 && (st->st_mode&*ap++)==0)
		ap++;
	putchar(*ap);
}

/*
 * Make all directories needed by `name'.
 * The path name is created by concatenating dblock.dbuf.prefix
 * and dblock.dbuf.name together in the function getdir.
 */
static
checkdir()
{
#define	ROOT	"/"
	register char *cp;

	/* Strip trailing slash. */
	cp = full_name + strlen(full_name) - 1;
	while ((cp > full_name) && (*cp == '/'))
		cp--;
	cp[1] = '\0';
	/*
	 * Quick check for existence of directory, needed for older versions.
	 */
	if ( strcmp ( full_name , ROOT ) == 0 )
		return;
	if ((cp = rindex(full_name, '/')) == 0)
		return;
	*cp = '\0';
	if (access(full_name, 0) == 0) {	/* already exists */
		*cp = '/';
		return;
	}
	*cp = '/';

	/*
	 * No luck, try to make all directories in path.
	 */
	cp = full_name;
	while (*cp == '/')
		cp++;
	for (; *cp; cp++) {
		if (*cp != '/')
			continue;
		*cp = '\0';
		if (access(full_name, 0) < 0) {
			if (mkdir(full_name, ~mask & (mode_t)0777) < 0) {
				perror(full_name);
				*cp = '/';
				return;
			}
			if (!oflag && is_root)
				chown(full_name, stbuf.st_uid, stbuf.st_gid);
		}
		*cp = '/';
	}
	return;
}

static onintr(void)
{
	(void) signal(SIGINT, SIG_IGN);
	term++;
}

static onquit(void)
{
	(void) signal(SIGQUIT, SIG_IGN);
	term++;
}

static onhup(void)
{
	(void) signal(SIGHUP, SIG_IGN);
	term++;
}

static onterm(void)
{
	(void) signal(SIGTERM, SIG_IGN);
	term++;
}


	/*	The function to modes will fill in the header
	 *	information for a file.
	*/
static tomodes(sp, longname , shortname )
register struct stat *sp;
	char	*longname ;
	char	*shortname ;
{
	register char	*cp;
	char		*slash ;
	int		ftype ;
	struct passwd	*pw_entry ;
	struct group	*gr_entry ;
	int nameLen;    /* length of longname */
	register char *nameBkpt; /* Name break point */

	for (cp = dblock.dummy; cp < &dblock.dummy[TBLOCK]; cp++)
		*cp = '\0';

	nameLen = strlen(longname);
	if ( nameLen <= NAME_SIZE ) {
	    strncpy ( dblock.dbuf.name , longname , (size_t)NAME_SIZE );
	    dblock.dbuf.prefix[0] = '\0';
	} else {
	    /*
	     * Put as much into prefix as possible and the rest into name, but
	     * only break on a slash.
	     */
	    nameBkpt = longname + nameLen - 1;
	    while (*nameBkpt == '/')	/* in case of trailing "/" */
	    	--nameBkpt;
	    if (nameLen <= PREFIX_SIZE) {
	    	/*
	     	 * Since total name length is less than prefix, put only
	     	 * the basename in the name field.
	     	 */
		while (longname != nameBkpt && *nameBkpt != '/')
			--nameBkpt;
	    } else
		while (*nameBkpt != '/' || (nameBkpt-longname) >= PREFIX_SIZE) {
			if (longname == nameBkpt)
				break;
			--nameBkpt;
		}
	    if (longname == nameBkpt) {	
		/* one long name > 100 bytes, or prefix > 155 bytes. */
		fprintf(stderr, MSGSTR( ETLONGFN,
			"tar: %s: file name too long\n" ), longname);
		return (FALSE) ;
	    } else
		slash = nameBkpt;
	    if (strlen(slash+1) > NAME_SIZE) {
		fprintf(stderr, MSGSTR( ETLONGFN,
			"tar: %s: file name too long\n" ), longname);
		return (FALSE) ;
	    }
	    if ( ('/' == *slash) && ( (slash-longname) < PREFIX_SIZE ) ){
		    strncpy( dblock.dbuf.name , slash+1 , (size_t)NAME_SIZE );
		    strncpy( dblock.dbuf.prefix , longname, (size_t)PREFIX_SIZE );
		    dblock.dbuf.prefix[slash-longname] = '\0';

	    } else {
		fprintf(stderr, MSGSTR( ETLONGFN,
			"tar: %s: file name too long\n" ), longname);
		return (FALSE) ;
	    }
	}

	sprintf(dblock.dbuf.mode, "%06o ", sp->st_mode & (07777 | S_IFMT));
	sprintf(dblock.dbuf.uid, "%06o ", sp->st_uid);
	sprintf(dblock.dbuf.gid, "%06o ", sp->st_gid);
	sprintf(dblock.dbuf.size, "%011lo ", sp->st_size);
	sprintf(dblock.dbuf.mtime, "%011lo ", sp->st_mtime);

	strncpy (dblock.dbuf.magic, TMAGIC , (size_t)TMAGLEN ) ;
	strncpy (dblock.dbuf.version, TVERSION , (size_t)TVERSLEN ) ;

	ftype = sp -> st_mode & S_IFMT ;

				/*  set file type  */
	switch ( ftype ) {
		case S_IFIFO :
			dblock.dbuf.typeflag = FIFOTYPE ;
			break ;

		case S_IFCHR :
			dblock.dbuf.typeflag = CHRTYPE ;
			break ;

		case S_IFBLK :
			dblock.dbuf.typeflag = BLKTYPE ;
			break ;

		case S_IFDIR :
			{
			int     len ;
			if (( len = strlen ( dblock.dbuf.name )) < NAME_SIZE) {
				if ( dblock.dbuf.name [len-1] != '/' )
					dblock.dbuf.name [len] = '/' ;
			} else {
				fprintf(stderr, MSGSTR( ETLONGFN,
					"tar: %s: file name too long\n" ), longname);
				return (FALSE) ;
			}
			dblock.dbuf.typeflag = DIRTYPE ;
			break ;
			}

		case S_IFREG :
			dblock.dbuf.typeflag = REGTYPE ;
			break ;

	}

				/*  set uname and gname  */
	uid_to_name(sp->st_uid, &dblock.dbuf.uname);
	gid_to_name(sp->st_gid, &dblock.dbuf.gname);

				/*  set major and minor values  */
	if ( ftype == S_IFIFO || ftype == S_IFBLK || ftype == S_IFCHR ) {
		sprintf(dblock.dbuf.devminor, "%06o " , minor(sp->st_rdev));
		sprintf(dblock.dbuf.devmajor, "%06o " , major(sp->st_rdev));
	}
	else {		/* POSIX requires the 'right stuff' for all types */
		sprintf(dblock.dbuf.devminor, "%06o " , 0);
		sprintf(dblock.dbuf.devmajor, "%06o " , 0);
	}
		

	return (TRUE) ;
}


	/*	The checksum function will create a checksum for
	 *	the header of a file written to the output file.
	 *	To alleviate problems with previously created AIX
	 *	checksums, tar will now check for signed checksums if
	 *	the unsigned checksum failed to compare the same.
	*/
static
signed_checksum()
{
	register i;
	register signed char *cp;

	for (cp = dblock.dbuf.chksum;
	     cp < &dblock.dbuf.chksum[sizeof(dblock.dbuf.chksum)]; cp++)
		*cp = ' ';
	i = 0;
	for (cp = dblock.dummy; cp < &dblock.dummy[TBLOCK]; cp++)
		i += *cp;
	return (i);
}

static
checksum()
{
	register i;
	register unsigned char *cp;

	for (cp = dblock.dbuf.chksum;
	     cp < &dblock.dbuf.chksum[sizeof(dblock.dbuf.chksum)]; cp++)
		*cp = ' ';
	i = 0;
	for (cp = dblock.dummy; cp < &dblock.dummy[TBLOCK]; cp++)
		i += *cp;
	return (i);
}

static
checkw(query_fmt, name)
	char *query_fmt, *name;
{
	int retval;
	char answer[LINE_MAX+1];

	if (!wflag) 
		return (1);
	printf(query_fmt, name);
	fgets(answer,LINE_MAX+1,response_file);
	/* return 1 for an affirmative answer, */
	/* and return 0 for any other answer.  */
	retval = rpmatch(answer);
	return ((retval == 1) ? 1 : 0);
}

static
checkf(name, mode)
	char *name;
	int mode;
{
	int l;

	if ((mode & S_IFMT) == S_IFDIR){
		if ((strcmp(name, "SCCS")==0) || (strcmp(name, "RCS")==0)) 
			return(0); 
		return(1);
	}
	if ((l = strlen(name)) < 3)
		return (1);
	if (name[l-2] == '.' && name[l-1] == 'o')
		return (0);
	if (strcmp(name, "core") == 0 ||
	    strcmp(name, "errs") == 0 ||
	    (strcmp(name, "a.out") == 0))
		return (0);
	/* SHOULD CHECK IF IT IS EXECUTABLE */
	return (1);
}

/* Is the current file a new file, or the newest one of the same name? */
static
checkupdate(arg)
	char *arg;
{
	long mtime;
	daddr_t seekp;
	daddr_t	lookup();

	rewind(tfile);
	if ((seekp = lookup(arg)) < 0)
		return (1);
	fseek(tfile, seekp, 0);
	fscanf(tfile, "%*s %lo", &mtime);
	return (stbuf.st_mtime > mtime);
}

static
done(n)
{
	unlink(tname);
	exit(n);
}

/* 
 * Do we want the next entry on the tape, i.e. is it selected?  If
 * not, skip over the entire entry.  Return -1 if reached end of tape.
 */
static wantit(argv)
	char *argv[];
{
	register char **cp;

	getdir() ;
	if (endtape())
		return (-1);
	if (*argv == 0)
		return (1);
	for (cp = argv; *cp; cp++)
		if (prefix(*cp, full_name))
			return (1);
	passtape();
	return (0);
}

/*
 * Does s2 begin with the string s1, on a directory boundary?
 */
static prefix(s1, s2)
	register char *s1, *s2;
{
	while (*s1)
		if (*s1++ != *s2++)
			return (0);
	if (*s2)
		return (*s2 == '/');
	return (1);
}

/*
 * This value is the length of the name being looked for plus 12 bytes for the
 * "mtime" field and 3 bytes for the space, newline, and NULL characters.
 */
#define	N	(TAR_PATH_LEN+15)

static daddr_t
lookup(s)
	char *s;
{
	register i;

	for(i=0; s[i]; i++)
		if (s[i] == ' ')
			break;
	return (bsrch(s, i, 0, high));
}

static daddr_t
bsrch(s, n, l, h)
	char *s;
	register n;
	daddr_t l, h;
{
	register i, j, numread;
	char b[N], hold;
	daddr_t m, m1;

	if (l >= h)
		return ((daddr_t) -1);
	m = l + (h-l)/2 - N;		/* Get midpoint */
	if (m < l)
		m = l;
	fseek(tfile, m, 0);
	fgets(b, N, tfile);		/* read to next newline */
	m += strlen(b);
	if (m >= h)
		return ((daddr_t) -1);
	fgets(b, N, tfile);		/* Get comparison string */
	m1 = m + strlen(b);
	hold = b[n];			/* This is done because there is no */
	b[n] = '\0';			/* strncoll() routine */
	i = strcoll(s, b);
	if ((i == 0) && (hold != ' '))	/* if different length names... */
		i = -1;
	if (i < 0)
		return(bsrch(s, n, l, m));
	else if (i > 0)
		return(bsrch(s, n, m1, h));

	return (m);
}


static readtape(buffer)
	char *buffer;
{
	char *bufp;

	if (first == 0)
		getbuf();
	(void) readtbuf(&bufp, TBLOCK);
	bcopy(bufp, buffer, TBLOCK);
	return(TBLOCK);
}

static readtbuf(bufpp, size)
	char **bufpp;
	int size;
{
	register int i;

	if (recno >= nblock || first == 0) {
		if ( rptape && trecs >= rptape )
			rflag ? nexttape(O_RDWR) : nexttape(O_RDONLY) ;
		i = bread((char *)tbuf, TBLOCK*nblock) ;
		trecs++ ;
		if (first == 0) {
			if ( i == 0 ) {
				fprintf(stderr, MSGSTR( ETREADEOF,
					"tar: tape read error: unexpected EOF\n"));
				done(1);
			}
			else if ((i % TBLOCK) != 0) {
				fprintf(stderr, MSGSTR( EBLKSIZE,
					"tar: tape blocksize error\n"));
				done(1);
			}
			i /= TBLOCK;
			if (i != nblock) {
				if ( arch_file_type != S_IFSOCK )
				    fprintf(stderr, MSGSTR( BLKSIZE,
					    "tar: blocksize = %d\n" ), i);
				exit_status = 1;
				nblock = i;
			}
			first = 1;
		}
		recno = 0;
	}
	if (size > ((nblock-recno)*TBLOCK))
		size = (nblock-recno)*TBLOCK;
	*bufpp = (char *)&tbuf[recno];
	recno += (size/TBLOCK);
	return (size);
}

static writetbuf(buffer, n)
	register char *buffer;
	register int n;
{
	int i;

	if (first == 0) {
		getbuf();
		first = 1;
	}
	if (recno >= nblock)
		bwrite ((char *) tbuf) ;

	/*
	 *  Special case:  We have an empty tape buffer, and the
	 *  users data size is >= the tape block size:  Avoid
	 *  the bcopy and dma direct to tape.  BIG WIN.  Add the
	 *  residual to the tape buffer.
	 */
	while (recno == 0 && n >= nblock) {
		bwrite (buffer) ;
		n -= nblock;
		buffer += (nblock * TBLOCK);
	}
		
	while (n-- > 0) {
		bcopy(buffer, (char *)&tbuf[recno++], TBLOCK);
		buffer += TBLOCK;
		if (recno >= nblock) {
			bwrite ((char *) tbuf) ;
			recno = 0;
		}
	}

	/* Tell the user how much to write to get in sync */
	return (nblock - recno);
}

/*
 * If tape device is in variable mode, writes are all
 * or nothing, no fragments.  But if the tape device
 * is in fixed block mode, then at the end of the tape,
 * we could encounter partial writes.  When we get a
 * partial write, we will attempt to write again to see
 * if it is indeed EOT.  Exit if not.
 */

static
bwrite (buffer)
	char	*buffer ;
{
	int	wr, frag ;
	if ( rptape && trecs >= rptape )
		nexttape (O_WRONLY) ;
	frag = 0;
tryagain:
	wr = write(mt, buffer+frag, (unsigned)(TBLOCK*nblock - frag)) ;
	if ( wr == -1 ) {
		if ( errno == ENXIO ) {
			nexttape (O_WRONLY) ;
			goto tryagain ;
		}
		else {
			perror ( MSGSTR( ETWRITE, "tar: tape write error" )) ;
			done (1) ;
		}
	}else
	    /* need to differentiate media error from End of Tape */
		if (wr != (TBLOCK*nblock - frag)) {
		    frag = wr;
		    /* try the write again */
		    wr = write(mt, buffer+frag, (unsigned)(TBLOCK*nblock - frag));
		    if (wr < 0 && errno == ENXIO) {
			nexttape (O_WRONLY);
			goto tryagain;
		    } else {
			fprintf(stderr, MSGSTR( ETWREOF,
				"tar: tape write error: unexpected EOF\n" ));
			done(1);
		    }
		}
	++trecs ;

}


static
bread(buf, size)
	char *buf;
	int size;
{
	int count;
	static int lastread = 0;

				/***	the for loop is for a communications
					link by specifing the 'B' option,
					all other will exit the for loop
					after the first read unless prompted
					to enter a new device	***/
					
	for (count = 0; count < size; count += lastread) {
		lastread = read(mt, buf, (unsigned)(size - count));
		switch ( lastread )
		{
			case -1 :
			    fprintf(stderr, MSGSTR( ETREAD, "tar: tape read error: " )) ;
				perror("");
				done (1) ;

			case 0 :
				if((arch_file_type == S_IFIFO) || 
				   (arch_file_type == S_IFSOCK) ||
				   (arch_file_type == S_IFREG))
				  return(count);/* Input from pipe => End of archive*/
				/* prompt for new device */
				rflag ? nexttape(O_RDWR) : nexttape(O_RDONLY);
				break ;

			default :
				if ( !Bflag ) {
				        if (lastread != size)
					    break;
					else return (lastread);
				}
				break ;
		}
		buf += lastread;
	}
	return (count);
}


static char *
getpwd(buf)
	char *buf;
{
	if (getwd(buf) == NULL) {
		fprintf(stderr, MSGSTR( ETARS, "tar: %s\n" ), buf);
		exit(1);
	}
	return (buf);
}

static getbuf()
{
	struct devinfo	dev_info ;

				/*  check for DD_DISK as in /dev/fd0  */
	if ( ioctl ( mt , IOCINFO , &dev_info ) != -1 )
		if ( dev_info.devtype == DD_DISK ) {
			if ( ! bptape )
				bptape = dev_info.un.dk.numblks ;
			nblock = dev_info.un.dk.secptrk * 2 ;
			recsize = nblock * TBLOCK ;
			rptape = bptape / nblock ;
		}

	if (nblock == 0) {
		fstat(mt, &stbuf);
		if ((stbuf.st_mode & S_IFMT) == S_IFCHR)
			nblock = NBLOCK;
		else {
			nblock = stbuf.st_size / TBLOCK;
			if (nblock == 0)
				nblock = NBLOCK;
		}
	}
				/*  check Sflag in order to set the
				    number of records per tape  */
	if ( Sflag )
		if ( bptape )
			rptape = bptape / nblock ;
		else {
   			unsigned long	len1rec; /* rcd len (100s of inches) */
			len1rec = ((unsigned long) nblock * (TBLOCK * 10000L))
				/ density + GAP;
			rptape = (tapelen * TAPE_FOOT) / len1rec;
		}

	/* do not malloc the whole file since this bogs the */
	/* system down or fails when filesize > avail memory */
	if ((nblock > MAXBLOCK) & !Nflag & !Bflag & !bflag)
		nblock = MAXBLOCK;
	tbuf = (union hblock *)malloc((size_t)nblock*TBLOCK);
	if (tbuf == NULL) {
		fprintf(stderr, MSGSTR( EBLKBIG,
			"tar: blocksize %d too big, can't get memory\n" ),
		    nblock);
		done(1);
	}
}

/*
 * Save this directory and its mtime on the stack, popping and setting
 * the mtimes of any stacked dirs which aren't parents of this one.
 * A null directory causes the entire stack to be unwound and set.
 *
 * Since all the elements of the directory "stack" share a common
 * prefix, we can make do with one string.  We keep only the current
 * directory path, with an associated array of mtime's, one for each
 * '/' in the path.  A negative mtime means no mtime.  The mtime's are
 * offset by one (first index 1, not 0) because calling this with a null
 * directory causes mtime[0] to be set.
 * 
 * This stack algorithm is not guaranteed to work for tapes created
 * with the 'r' option, but the vast majority of tapes with
 * directories are not.  This avoids saving every directory record on
 * the tape and setting all the times at the end.
 */
static char dirstack[TAR_PATH_LEN];
#define NTIM (TAR_PATH_LEN/2+1)		/* a/b/c/d/... */
static time_t mtime[NTIM];

static
dodirtimes()
{
	register char *p = dirstack;
	register char *q = full_name;
	register int ndir = 0;
	char *savp;
	int savndir;

	/* Find common prefix */
	while (*p == *q) {
		if (*p++ == '/')
			++ndir;
		q++;
	}

	savp = p;
	savndir = ndir;
	while (*p) {
		/*
		 * Not a child: unwind the stack, setting the times.
		 * The order we do this doesn't matter, so we go "forward."
		 */
		if (*p++ == '/')
			if (mtime[++ndir] >= 0) {
				*--p = '\0';	/* zap the slash */
				setimes(dirstack, mtime[ndir]);
				*p++ = '/';
			}
	}
	p = savp;
	ndir = savndir;

	/* Push this one on the "stack" */
	while (*p = *q++)	/* append the rest of the new dir */
		if (*p++ == '/')
			mtime[++ndir] = -1;
	mtime[ndir] = stbuf.st_mtime;	/* overwrite the last one */
}

static setimes(path, mt)
	char *path;
	time_t mt;
{
	struct timeval tv[2];

	tv[0].tv_sec = time((time_t *) 0);
	tv[1].tv_sec = mt;
	tv[0].tv_usec = tv[1].tv_usec = 0;
	if (utimes(path, tv) < 0) {
		fprintf(stderr, MSGSTR( ESETTIME,
			"tar: can't set time on %s: " ), path);
		perror("");
		if (exit_status != 1)
			exit_status = 2;
	}
}

static char *
getmem(size)
{
	char *p = (char *) malloc((size_t) size);

	if (p == NULL && freemem) {
		fprintf(stderr, MSGSTR( EMEM,
		    "tar: out of memory, link and directory modtime info lost\n"));
		freemem = 0;
	}
	return (p);
}


				/* move on to the next tape */
				/* mode is 0 for read, 1 for write */
#include "termios.h"
static nexttape(mode)
{
	char	c ;

	if (close(mt) < 0 && mode) {
	    perror ( MSGSTR( ETWRITE, "tar: tape write error" )) ;
	    done (1) ;
	}
	eprintf ( MSGSTR( BLKSON, "%ld blocks on %s\n" ),
		trecs * nblock , usefile ) ;

	if (!isatty(0)) {
		eprintf( MSGSTR( EEOT, "tar: ran off end of tape\n"));
		done(1);
	}
	eprintf( MSGSTR( EOT,
		"\007tar: End of tape.  Mount next tape on %s and type return."
		), usefile);
	tcflush(0, TCIFLUSH);
	do {
		if (read(0, &c, 1) != 1)
			done(1);
		if (term)
			done(1);
	} while (c != '\n');

				/*	open it again	*/
	while((mt= open(usefile,mode)) < 0) {
		eprintf( MSGSTR( ETOPEN, cantopen ), usefile);
		eprintf( MSGSTR( CHKDEV, "Check the backup medium and press return to continue.\n"));
        	do {
                	c = getchar();
                	if (term)
                        	done(1);
        	} while (c != '\n');
	}
	trecs = 0;

	eprintf( MSGSTR( PROCEED, "proceeding to device %s\n" ), usefile);

}

/*
**
**     Given source and destination files
**     to make a links from/to, calculate a
**     relative pathname that will successfully
**     link source to dest, ie dest -> source.
**     we ignore leading common path names as they are
**     likely to cause problems and make the symlink longer
**     than it otherwise need be. e.g. ../a/b/c to ../a/c/b
**     will result in a link to ../b/c
**
*/

static char *rel_name(source, dest)
char *source, *dest;
{
	static char buffer[MAXPATHLEN];
	char *s = source, *d = dest, *p = s;

	if (source[0]=='/')     /* Absolute pathname link */
		return(source);

	buffer[0]=0;
	while (*s && *s == *d) {
		if (*s == '/')
			p = s + 1;     /* remember where last / was */
		++s, ++d;
	}

	dest += p-source;
	source += p-source;

	while ((*dest++) == '/');

	while (*dest)
		if ((*dest++)=='/')
			strcat(buffer,"../");

	strcat(buffer,source);
	return(buffer);
}

/*
 * what follows is a semi-general LRU cache for name::[ug]id mappings.
 */

#if UNAMELEN > GNAMELEN
# define NLEN	UNAMELEN
#else
# define NLEN	GNAMELEN
#endif
#define NCACHE		64
#define MAP_FREE	0
#define MAP_UID		1
#define	MAP_GID		2
#define N_MAPTYPE	3

/*
 * cache <name, UID> and <name, GID> mappings
 */
static struct mcache {
	int	id;			/* UID or GID			*/
	ushort	lru;			/* LRU clock			*/
	char	type;			/* which?			*/
	char	miss;			/* flag: !0 => caching a miss	*/
	char	name[NLEN];		/* uid name or group name	*/
}  mcache[NCACHE];

static struct mcache *map_lookup(char *, int, int);

/*
 * name_to_uid -	map username to UID
 *
 * Input:
 *	name	-	^ to username
 *	uidp	-	^ to place to store UID
 *
 * Returns:
 *	!0	-	 OK: mapping  exists, *uidp  updated
 *	 0	-	!OK: mapping !exists, *uidp !updated
 */
static
name_to_uid(name, uidp)
char *name;
uid_t *uidp; {
	register struct mcache *m;

	if (*name == 0)
		return 0;

	m = map_lookup(name, 0, MAP_UID);
	if (!m->miss) {
		*uidp = m->id;
		return !0;
	}

	return 0;
}

/*
 * name_to_gid -	map username to GID
 *
 * Input:
 *	name	-	^ to username
 *	gidp	-	^ to place to store GID
 *
 * Returns:
 *	!0	-	 OK: mapping  exists, *gidp  updated
 *	 0	-	!OK: mapping !exists, *gidp !updated
 */
static
name_to_gid(name, gidp)
char *name;
gid_t *gidp; {
	register struct mcache *m;

	if (*name == 0)
		return 0;

	m = map_lookup(name, 0, MAP_GID);
	if (!m->miss) {
		*gidp = m->id;
		return !0;
	}

	return 0;
}

/*
 * uid_to_name -	map UID to username
 *
 * Input:
 *	uid	-	user ID
 *	name	-	^ to place to store username
 *
 * Returns:
 *	!0	-	 OK: mapping  exists, name  updated
 *	 0	-	!OK: mapping !exists, name !updated
 */
static
uid_to_name(uid, name)
char *name;
uid_t uid; {
	register struct mcache *m;

	m = map_lookup(0, (int) uid, MAP_UID);
	if (!m->miss) {
		strncpy(name, m->name, UNAMELEN);
		return !0;
	}

	return 0;
}

/*
 * gid_to_name -	map GID to username
 *
 * Input:
 *	gid	-	group ID
 *	name	-	^ to place to store groupname
 *
 * Returns:
 *	!0	-	 OK: mapping  exists, name  updated
 *	 0	-	!OK: mapping !exists, name !updated
 */
static
gid_to_name(gid, name)
char *name;
gid_t gid; {
	register struct mcache *m;

	m = map_lookup(0, (int) gid, MAP_GID);
	if (!m->miss) {
		strncpy(name, m->name, GNAMELEN);
		return !0;
	}

	return 0;
}

static ushort lru_ticker;		/* LRU time keeper	*/

/*
 * map_lookup -	lookup mapping by name or by ID
 *
 * Input:
 *	name	-	^ to name, or NULL
 *	id	-	if name NULL, id
 *	which	-	which name space: UID or GID
 *
 * Returns:
 *	!0	-	 OK: ^ to mapping
 *	 0	-	!OK: no "which"-type name exists
 */
static struct mcache *
map_lookup(name, id, which)
char *name; {
	struct passwd *pw;
	struct group *gw;
	register struct mcache *m, *mlru;
	register ushort lru;
	static struct mcache *last[N_MAPTYPE];

	/*
	 * if the ticker wraps, reset all LRU counters
	 */
	if (++lru_ticker == 0)
		for (m = mcache; m < &mcache[NCACHE]; ++m)
			m->lru = 0;

	/*
	 * check the L1 cache first
	 */
	if ((m = last[which]) && m->type == which
	    && ((name && strcmp(name, m->name) == 0)
		|| (!name && id == m->id)))
		goto ok;

	/*
	 * missed, have to check the L2 cache, which is a
	 * linear scan thru.
	 */
	for (mlru = m = mcache; m < &mcache[NCACHE]; ++m) {
		if (m->type == which
		    && ((name && strcmp(name, m->name) == 0)
			|| (!name && id == m->id)))
			goto ok;

		if (m->type == MAP_FREE || mlru->lru > m->lru)
			mlru = m;
	}

	/*
	 * not found in cache.  replace LRU entry with mapping
	 */
	m = mlru;
	m->miss = 0;				/* assume presence	*/

	if (name) switch (which) {		/* name lookup		*/
	    case MAP_UID:
		strncpy(m->name, name, UNAMELEN-1);
		m->name[UNAMELEN-1] = 0;

		pw = getpwnam(name);
		if (pw)
			m->id = pw->pw_uid;
		else {
			m->miss = 1;		/* cache misses also	*/
			m->id   = -1;
		}
		break;

	    case MAP_GID:
		strncpy(m->name, name, GNAMELEN-1);
		m->name[GNAMELEN-1] = 0;

		gw = getgrnam(name);
		if (gw)
			m->id = gw->gr_gid;
		else {
			m->miss = 1;		/* cache misses also	*/
			m->id   = -1;
		}
		break;
	} else switch (which) {			/* ID lookup	*/
	    case MAP_UID:
		m->id = id;
		pw = getpwuid((uid_t) id);
		if (pw) {
			strncpy(m->name, pw->pw_name, UNAMELEN-1);
			m->name[UNAMELEN-1] = 0;
		} else {
			m->name[0] = 0;
			m->miss = 1;		/* cache misses also	*/
		}
		break;

	    case MAP_GID:
		m->id = id;
		gw = getgrgid((gid_t) id);
		if (gw) {
			strncpy(m->name, gw->gr_name, GNAMELEN-1);
			m->name[GNAMELEN-1] = 0;
		} else {
			m->name[0] = 0;
			m->miss = 1;		/* cache misses also	*/
		}
		break;
	}

	m->type = which;
     ok:
	m->lru  = lru_ticker;

	return last[which] = m;
}
