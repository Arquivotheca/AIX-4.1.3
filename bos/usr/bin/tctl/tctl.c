static char sccsid[] = "@(#)96	1.21.1.7  src/bos/usr/bin/tctl/tctl.c, cmdarch, bos411, 9428A410j 11/9/93 12:36:21";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: tctl
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
#ifndef lint
char copyright[] =
"(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "(#)mt.c	5.1 (Berkeley) 4/30/85";
#endif not lint
*/

/*
 * tctl --
 *   magnetic tape manipulation program
 */

#include <nl_types.h>
#include "tctl_msg.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/tape.h>
#include <sys/scsi.h>
#include <sys/ioctl.h>
#include <locale.h>

#define MSGSTR(N,S) catgets(catd,MS_TCTL,N,S)
nl_catd catd;

#ifndef DEFTAPE
#define	DEFTAPE	"/dev/rmt0.1"	/* No rewind, no reten */
#endif

#define	equal(s1,s2)	(strcmp(s1, s2) == 0)
#define RESET	98	/* define bogus ioctl command for reset */
#define STATUS	99	/* define bogus ioctl command for status */

/* define the tctl command set and the mt command set */
struct commands {
	char *c_name;
	int c_code;
	int c_ronly;
} tcom[] = {
	{ "weof",	STWEOF,		0 },
	{ "eof",	STWEOF,		0 },
	{ "fsf",	STFSF,		1 },
	{ "bsf",	STRSF,		1 },
	{ "fsr",	STFSR,		1 },
	{ "bsr",	STRSR,		1 },
	{ "rewind",	STREW,		1 },
	{ "offline",	STOFFL,		1 },
	{ "rewoffl",	STOFFL,		1 },
	{ "erase",	STERASE,	0 },
	{ "retension",	STRETEN,	1 },
	{ "read",	STIOCHGP,	1 },
	{ "write",	STIOCHGP,	0 },
	{ "reset",	RESET,		1 }, /* bogus ioctl for reset */
	{ "status",	STATUS,		1 }, /* bogus ioctl for status */
	{ NULL,		0,		1 }
}, mcom[] = {
	{ "weof",	STWEOF,		0 },
	{ "eof",	STWEOF,		0 },
	{ "fsf",	STFSF,		1 },
	{ "bsf",	STRSF,		1 },
	{ "fsr",	STFSR,		1 },
	{ "bsr",	STRSR,		1 },
	{ "rewind",	STREW,		1 },
	{ "offline",	STOFFL,		1 },
	{ "rewoffl",	STOFFL,		1 },
	{ "status",	STATUS,		1 }, /* bogus ioctl for status */
	{ NULL,		0,		1 }
};

int stfd;
struct stop st_com;
char *tape = NULL;
char *Invokedas;
int mt=0;

#define DEF_BUF_SIZE 32768
#define BSIZE ( 0 == iBsize ? DEF_BUF_SIZE : iBsize )
#define PSIZE ( 0 == iPsize ? DEF_BUF_SIZE : iPsize )

void usage(char *cmd)
{
	int iX;
	struct commands *xcom;

	if (mt)
		fprintf(stderr, 
			MSGSTR(MUSAGE, "Usage:  %s [ -f device ] subcommand [ count ]\n"), cmd);
	else
		fprintf(stderr, 
			MSGSTR(TUSAGE, "Usage:  %s [ -Bnv ] [ -b num ] [ -p num ] [ -f device ] subcommand [ count ]\n"),cmd);
	fprintf(stderr, MSGSTR(VALIDCMDS, "\tValid subcommands are:\n"));
	iX = 0;
	xcom = mt ? mcom : tcom;
	while (xcom[iX].c_name != NULL) 
	{
		fprintf(stderr, "\t%s\n", xcom[iX].c_name);
		iX++;
	}
	exit(1);
}

/* close the open file descriptors and exit */
void terminate(exit_val)
int exit_val;
{
	catclose(catd);
	(void) close(stfd);
	exit(exit_val);
}


main(argc, argv)
	int argc;
	char **argv;
{
	register char *cp;
	register struct commands *comp;
	struct commands savecom;
	int cOpt;
	int iCount = 1;		/* Do command iCount times */
	int iBsize = 512;	/* Tape buffer size */
	int iPsize = 0;		/* Pipe buffer size */
	int iPcount;		/* current # of chars written to or read from */
				/* the pipe (stdout/ stdin) */
	int iTmp;
	int nflag = 0;
	int Bflag = 0;
	int verbose = 0;
	int found = 0;
	struct stchgp stChParm;
	char *stBuf;		/* Point to Tape & Pipe buffers */
	char *strrchr();
	char *cmd;

	(void) setlocale(LC_ALL,"");
	catd = catopen("tctl.cat", NL_CAT_LOCALE);
	Invokedas = *argv;

	/* is the user invoking the mt command? */
	if (cmd = strrchr(argv[0], '/'))
		++cmd;
	else
		cmd = argv[0];
	if (equal(cmd, "mt"))
		mt++;
	tape = getenv("TAPE");	/* Possibly overridden by user */

	/* if the user is invoking the mt command then exit when bad option specified */
	while ( EOF != (cOpt = getopt(argc, argv, "?b:f:t:p:nvB")) ) {
		switch ( cOpt ) {
		case 'b':
			if (mt)
				usage(cmd);
			iBsize = atoi(optarg);
			break;
		case 'f':
		case 't':
			tape = optarg;
			break;
		case 'n':
			if (mt)
				usage(cmd);
			nflag++;
			break;
		case 'p':
			if (mt)
				usage(cmd);
			iPsize = atoi(optarg);
			break;
		case 'v':
			if (mt)
				usage(cmd);
			verbose++;
			break;
		case 'B':
			if (mt)
				usage(cmd);
			Bflag++;
			break;
		case '?':
			usage(cmd);
		}
	}
	if (tape == NULL)
		tape = DEFTAPE;
	cp = NULL;
	switch(argc - optind) {
		case 2:
			iCount = atoi(argv[optind+1]);
			/* fall thru */
		case 1:
			cp = argv[optind];
			break;
		default:
			usage(Invokedas);
	}

	for (comp = mt ? mcom : tcom; comp->c_name != NULL; comp++) 
		if (strncmp(cp, comp->c_name, strlen(cp)) == 0)
		{
			found++;
			savecom = *comp;
		}

	/* did the user enter the minimum characters to determine the subcommand */
	if (found > 1)
	{
		fprintf(stderr, MSGSTR(NOTUNIQ,
				"%s: \"%s\" is not a unique subcommand \n"),Invokedas,cp);
		exit(1);
	}
	if (found == 1)
		*comp = savecom;

	if (comp->c_name == NULL)
	{
		fprintf(stderr, MSGSTR(INVALID, 
				"%s: invalid subcommand \"%s\"\n"),Invokedas,cp);
		usage(cmd);
	}

	if (comp->c_code == STATUS)
		exit(status(cmd,tape));
	else if (comp->c_code == RESET) {
	/* Force open the device if not open, reserved, or unavailable. */
		if ((stfd = open(tape, O_RDONLY)) >= 0)
			terminate(0);
		else if (errno == EBUSY || errno == EAGAIN || errno == ENODEV)
			perror(tape);
		else if ((stfd = openx(tape, O_RDONLY, 0, SC_FORCED_OPEN)) < 0)
			perror(tape);
		else
			terminate(0);
		exit(2);
	} else {
		if ((stfd = open(tape, comp->c_ronly ? O_RDONLY : O_RDWR)) < 0) {
			perror(tape);
			exit(2);
		}

		if (STIOCHGP == comp->c_code ) {
			stChParm.st_ecc = ST_NOECC;
			stChParm.st_blksize = nflag ? 0 : iBsize;	
			if (ioctl(stfd, STIOCHGP, &stChParm) < 0) {
				perror(MSGSTR(CHGPARM,
				"tctl: changing parameters"));
			}
			if ( PSIZE < BSIZE || ( 0 != PSIZE % BSIZE ) ) {
				fprintf(stderr, MSGSTR(ERRMULT, 
						"tctl: Pipe size must be a multiple of buffer size.\n"));
				terminate(2);
			}
			if (NULL == (stBuf = malloc((size_t)PSIZE))) {
				fprintf(stderr,MSGSTR(ERRMEM,"tctl: Out of memory"));
				terminate(2);
			}
			if ( 0 == strcmp(comp->c_name, "read") ) {
				while ( 1 ) {
			        char *bufp;
	
					bufp = stBuf;
					iPcount = 0;
					do {
				        int frags;	/* for when there are record marks on tape */

						if ((iTmp = read(stfd, bufp, BSIZE)) < 0 ) {
							fprintf(stderr, MSGSTR(ERRRD, 
									"tctl: Tried to read %d "), BSIZE);
							perror("");
							terminate(2);
						}
						if (Bflag) {
						    iPcount = iTmp;
						    goto L_write;
						}
						bufp += iTmp;
						frags = iTmp;
						/* loop till we read in BSIZE worth of data, or EOF */
						while ((iTmp < BSIZE) && (frags > 0)) { 
							if ((frags = read(stfd, bufp, BSIZE - iTmp)) < 0) {
						    	fprintf(stderr, MSGSTR(ERRRD, 
										"tctl: Tried to read %d "), BSIZE - iTmp);
						    	perror("");
						    	terminate(2);
							}
							bufp += frags;
							iTmp += frags;
						}
						if ( verbose )
							fprintf(stderr,MSGSTR(INFORD,"read %d bytes\n"), iTmp);
						iPcount += iTmp;
						if (frags == 0) {
					    	iTmp = 0;
					    	break;
						}
					} while ( (iTmp > 0) && iPcount < iPsize );
					if ( iPcount <= 0 )
						break;
L_write:				if (iPcount != write(fileno(stdout), stBuf, iPcount)) {
						fprintf(stderr, MSGSTR(ERRWRI, 
								"tctl: tried to write %d "), iPcount);
						perror("");
						terminate(2);
					}
					if ( verbose )
						fprintf(stderr,MSGSTR(INFOWR,"wrote %d bytes\n"), iPcount);
					if ( iTmp <= 0 )
						break;
				}
			}
			else if ( 0 == strcmp(comp->c_name, "write") ) {
				while ( 1 ) {
			        char *bufp;
					int frags;
				
					bufp = stBuf;
					iTmp = 0;
					/* grab everything from stdin, up till PSIZE */
					do {
				    	frags = 0;
				    	if ((frags = read(fileno(stdin), bufp, PSIZE - iTmp)) < 0 ) {
							fprintf(stderr, MSGSTR(ERRRD, 
									"tctl: Tried to read %d "), PSIZE - iTmp);
							perror("");
							terminate(2);
				    	}
				    	if (frags > 0) {
							iTmp += frags;
							bufp += frags;
				    	}
					} while (frags > 0 && iTmp < PSIZE);
					if ( verbose )
						fprintf(stderr,MSGSTR(INFORD,"read %d bytes\n"), iTmp);
					if ( iTmp == 0 )
						break;
					if ( 0 == iBsize ) {
						if (iTmp != write(stfd, stBuf, iTmp)) {
							fprintf(stderr, MSGSTR(ERRWRI, 
									"tctl: tried to write %d "), iTmp);
							perror("");
							terminate(2);
						}
						if ( verbose )
							fprintf(stderr,MSGSTR(INFOWR,"wrote %d bytes\n"), iTmp);
					} else {
					        bufp = stBuf;
					        iPcount = iTmp;
						do {
						    if (iBsize > iPcount) {
							if (nflag) {	/* variable block, so partial write is possible */
							    if ((iTmp = write(stfd, bufp, iPcount)) != iPcount) {
									fprintf(stderr, MSGSTR(ERRWRI, 
											"tctl: tried to write %d "), iPcount);
									perror("");
									terminate(2);
								    }
							}
							else {	/* blank out rest of buffer, no partial write */
							    bzero(bufp+iPcount, iBsize-iPcount);
							    if ((iTmp = write(stfd, bufp, iBsize)) != iBsize ) {
									fprintf(stderr, MSGSTR(ERRWRI,
										       "tctl: tried to write %d "), iTmp);
									perror("");
									terminate(2);
								    }
							}
						    }
						    else {
						    	if ((iTmp = write(stfd, bufp, iBsize)) != iBsize ) {
									fprintf(stderr, MSGSTR(ERRWRI, 
											"tctl: tried to write %d "), iBsize);
									perror("");
									terminate(2);
								    }
						    }
						    bufp += iTmp;
						    if ( verbose )
							fprintf(stderr,MSGSTR(INFOWR, 
									      "wrote %d bytes\n"), iTmp);
						    iPcount -= iBsize;
						} while ( (iBsize > 0) && iPcount > 0 );
					}
				}
				if (close(stfd) < 0) {
				    fprintf(stderr, "%s %s %d ", tape,
					comp->c_name, st_com.st_count);
				    perror(MSGSTR(TFAILED, "failed"));
				    terminate(2);
				}
			}
		}
		else {
			st_com.st_op = comp->c_code;
			st_com.st_count = iCount;
			if (st_com.st_count < 0) {
				fprintf(stderr, MSGSTR(NEGA, 
						"%s: negative repeat count\n"), Invokedas);
				terminate(2);
			}
	
			if (ioctl(stfd, STIOCTOP, &st_com) < 0) {
				fprintf(stderr, "%s %s %d ", tape, comp->c_name, st_com.st_count);
				perror(MSGSTR(TFAILED, "failed"));
				if ((errno == EIO) && (comp->c_code == STRSF))
				    terminate(EIO);
				else
				    terminate(2);
			}
									/* Handle bsf correctly for mt */
									/* An extra bsf & fsf for positioning */
			if ((mt) && (comp->c_code == STRSF)) {
				st_com.st_op = STRSF;
				st_com.st_count = 1;
				if (ioctl(stfd, STIOCTOP, &st_com) < 0) {
					if (errno != EIO) {		/* BOT not an error here */
						fprintf(stderr, "%s %s %d ", tape, comp->c_name,iCount);
						perror(MSGSTR(TFAILED, "failed"));
						terminate(2);
					}
				} else {					/* fsf only if not at BOT */
					st_com.st_op = STFSF;
					st_com.st_count = 1;
					if (ioctl(stfd, STIOCTOP, &st_com) < 0) {
						fprintf(stderr, "%s %s %d ", tape, comp->c_name,iCount);
						perror(MSGSTR(TFAILED, "failed"));
						terminate(2);
					}
				}
			}
		}
	}
	terminate(0);
}

/*
 * status - call the lsdev and lsattr commands to display tape device information
 */
status(char *cmd, char *tape)
{
	char *tapename;	
	char cmdline[PATH_MAX+21];
	char *ptr;

	if (tapename = strrchr(tape, '/'))
		++tapename;
	else
		tapename = tape;

	/* If the tape is a special file ( rmt0.2, rmt0.4, etc. ) then,
	 * strip the extension. lsdev and lsattr does not recognize the
	 * extensions for the device.
	 */
	if (ptr = strrchr(tapename, '.'))
		*ptr = '\0';

	strcpy(cmdline,"/usr/sbin/lsdev -Cl");
	strcat(cmdline,tapename);
	if (system(cmdline) != 0){
		return 2;
	}
	strcpy(cmdline,"/usr/sbin/lsattr -EHl");
	strcat(cmdline,tapename);
	if (system(cmdline) != 0){
		return 2;
	}
	return 0;
}
