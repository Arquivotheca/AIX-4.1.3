static char sccsid[] = "@(#)95	1.15  src/bos/usr/bin/tcopy/tcopy.c, cmdarch, bos41J, 9516A_all 4/18/95 16:01:18";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: tcopy
 *
 * ORIGINS:  26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
#ifndef lint
char copyright[] =
"(#) Copyright (c) 1985 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "(#)tcopy.c	1.2 (Berkeley) 12/11/85";
#endif not lint
*/

#include <nl_types.h>
#include "tcopy_msg.h"
#define MSGSTR(N,S) catgets(catd,MS_TCOPY,N,S)
nl_catd catd;

#include <stdio.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/tape.h>
#include <locale.h>

#define SIZE		(2 * 1024 * 1024)
#define CONFIG_TO_TRY	4

char buff[SIZE];
int filen=1;
long count, lcount;
int RUBOUT(void);

long itol();
int nfile;
long size, tsize;
int ln;
char *inf, *outf;
int copy;
int maxTransferSize ;
int blksize[ CONFIG_TO_TRY ] = { -1, 0, 512, 1024 } ;

main(argc, argv)
char **argv;
{
	register n, nw, inp, outp;
	struct stop op;
	int visit ;

	(void) setlocale(LC_ALL,"");
	catd = catopen("tcopy.cat", NL_CAT_LOCALE);
	if (argc <=1 || argc > 3) {
		fprintf(stderr, MSGSTR(USAGE, "Usage: tcopy src [dest]\n"));
		exit(1);
	}
	inf = argv[1];
	if (argc == 3) {
		outf = argv[2];
		copy = 1;
	}
	if ((inp=open(inf, O_RDONLY, 0666)) < 0) {
		fprintf(stderr,MSGSTR(CANTOP, "Can't open %s\n"), inf);
		exit(1);
	}
	if (copy) {
		if ((outp=open(outf, O_WRONLY, 0666)) < 0) {
			fprintf(stderr,MSGSTR(CANTOP, "Can't open %s\n"), outf);
			exit(3);
		}
	}
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		(void) signal(SIGINT, (void (*)(int))RUBOUT);
	ln = -2;

        visit = 0;
	while ( visit++ < CONFIG_TO_TRY )
	{
		maxTransferSize = SIZE ;
		if ( visit > 1 )
		{
			struct stchgp stChParm = { ST_ECC, 0 } ;

			stChParm.st_blksize = blksize[ visit -1 ] ;
			if ( ioctl(inp, STIOCHGP, &stChParm) == -1 )
				continue ;
		}

		while ( (n = read(inp, buff, maxTransferSize)) < 0 &&
		        maxTransferSize >= (32 * 1024) )
			maxTransferSize >>= 1 ;
		if ( n >= 0 )
			break ;
	}
	visit = 0;

	for (;;) {
		count++;
		if ( visit )
			n = read(inp, buff, maxTransferSize );
		if (n < 0)
			perror("read");
		visit = 1 ;
		if (n > 0) {
		    if (copy) {
			    nw = write(outp, buff, n);
			    if (nw < 0)
				perror("write");
			    if (nw != n) {
				fprintf(stderr, MSGSTR(WNOTR, "write (%d) != read (%d)\n"),
					nw, n);
				fprintf(stderr, MSGSTR(COPYAB, "COPY Aborted\n"));
				exit(5);
			    }
		    }
		    size += n;
		    if (n != ln) {
			if (ln > 0)
			    if (count - lcount > 1)
				printf(MSGSTR(STATFR, "file %d: records %ld to %ld: size %d\n"),
					filen, lcount, count-1, ln);
			    else
				printf(MSGSTR(STATRS, "file %d: record %ld: size %d\n"),
					filen, lcount, ln);
			ln = n;
			lcount = count;
		    }
		}
		else {
			if (ln <= 0 && ln != -2) {
				printf(MSGSTR(ENDTAPE, "eot\n"));
				break;
			}
			if (ln > 0)
			    if (count - lcount > 1)
				printf(MSGSTR(STATFR, "file %d: records %ld to %ld: size %d\n"),
					filen, lcount, count-1, ln);
			    else
				printf(MSGSTR(STATRS, "file %d: record %ld: size %d\n"),
					filen, lcount, ln);
			printf(MSGSTR(STATEOT, "file %d: eof after %ld records: %ld bytes\n"),
				filen, count-1, size);
			if (copy) {
				op.st_op = STWEOF;
				op.st_count = 1;
				if(ioctl(outp, STIOCTOP, &op) < 0) {
					perror(MSGSTR(WEOFBAD, "Write EOF"));
					fprintf(stderr, MSGSTR(COPYAB,
						"COPY Aborted\n"));
					exit(6);
				}
			}
			filen++;
			count = 0;
			lcount = 0;
			tsize += size;
			size = 0;
			if (nfile && filen > nfile)
				break;
			ln = n;
		}
	}
	if (copy && close(outp) < 0) {
	    perror(MSGSTR(WRITEBAD, "Write failed"));
	    fprintf(stderr, MSGSTR(COPYAB, "COPY Aborted\n"));
	    exit(7);
	}
	printf(MSGSTR(LENTOT, "total length: %ld bytes\n"), tsize);
	catclose(catd);
	exit(0);
}

RUBOUT(void)
{
	if (count > lcount)
		--count;
	if (count)
		if (count > lcount)
			printf(MSGSTR(STATFR, "file %d: records %ld to %ld: size %d\n"),
				filen, lcount, count, ln);
		else
			printf(MSGSTR(STATRS, "file %d: record %ld: size %d\n"),
				filen, lcount, ln);
	printf(MSGSTR(RBOUTF, "rubout at file %d: record %ld\n"), filen, count);
	printf(MSGSTR(LENTOT, "total length: %ld bytes\n"), tsize+size);
	exit(1);
}

