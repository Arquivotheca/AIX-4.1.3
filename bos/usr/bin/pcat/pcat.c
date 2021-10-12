static char sccsid[] = "@(#)05	1.11  src/bos/usr/bin/pcat/pcat.c, cmdfiles, bos412, 9446C 11/14/94 16:49:16";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: pcat
 *
 * ORIGINS: 18, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 Release 1.0
 * static char rcsid[] = "@(#)$RCSfile: pcat.c,v $ $Revision: 1.4 $ (OSF) $Date: 90/10/07 16:50:22 $"
 */

/*
 * pcat: view packed files
 *	Huffman decompressor  (new files to standard output)
 *	Usage:	pcat filename...
 */


#include "pcat_msg.h"
#define MSGSTR(id,ds) catgets(catd, MS_PCAT, id, ds)
static nl_catd catd;
#include <stdio.h>
#include <locale.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>

static char	*argv0, *argvk;
static short	errorm;

#define NAME_MAX MAXNAMLEN  /* HF 05/30/91  */

#define	BLKSIZE	512
#define NAMELEN PATH_MAX+1
#define MAXBASE NAME_MAX-1
#define	SUF0	'.'
#define	SUF1	'z'
#define US 037
#define RS 036

/* variables associated with i/o */
static char	filename [NAMELEN+2];
static short	infile;
static short	outfile;
static short	inleft;
static char	*inp;
static char	*outp;
static char	inbuff [BLKSIZE];
static char	outbuff [BLKSIZE];

/* the dictionary */
static long	origsize;
static short	maxlev;
static short	intnodes [25];
static char	*tree [25];
static char	characters [256];
static char	*eof;

/*
 * NAME: getdict
 *                                                                    
 * FUNCTION:  read in the dictionary portion and build decoding 
 *            structures, return 1 if successful, 0 otherwise 
 */
static getdict ()
{
	int c, i, nchildren;
	/*
	 * check two-byte header
	 * get size of original file,
	 * get number of levels in maxlev,
	 * get number of leaves on level i in intnodes[i],
	 * set tree[i] to point to leaves for level i
	 */
	eof = &characters[0];
	inbuff[6] = 25;
	inleft = read(infile, &inbuff[0], BLKSIZE);
	if (inleft < 0) {
		eprintf (MSGSTR(M_BADRD, ".z: read error"));
		return (0);
	}
	if (inbuff[0] != US) goto goof;
	if (inbuff[1] != RS) goto goof;

	inp = &inbuff[2];
	origsize = 0;
	for (i=0; i<4; i++) {
		origsize = origsize*256 + ((*inp++) & 0377);
	}
	maxlev = *inp++ & 0377;
	if (maxlev > 24) {
goof:		eprintf (MSGSTR(M_FORMT, ".z: not in packed format"));
		return (0);
	}
	for (i=1; i<=maxlev; i++)
		intnodes[i] = *inp++ & 0377;
	for (i=1; i<=maxlev; i++) {
		tree[i] = eof;
		for (c=intnodes[i]; c>0; c--) {
			if (eof >= &characters[255])
				goto goof;
			*eof++ = *inp++;
		}
	}
	*eof++ = *inp++;
	intnodes[maxlev] += 2;
	inleft -= inp-&inbuff[0];
	if (inleft < 0)
		goto goof;

	/*
	 * convert intnodes[i] to be number of
	 * internal nodes possessed by level i
	 */
	nchildren = 0;
	for (i=maxlev; i>=1; i--) {
		c = intnodes[i];
		intnodes[i] = nchildren /= 2;
		nchildren += c;
	}
	return (decode());
}

/*
 * NAME: decode
 *                                                                    
 * FUNCTION:  unpack the file, return 1 if successful, 0 otherwise 
 */
static decode ()
{
	int bitsleft, c, i;
	int j, lev;
	char *p;
	outp = &outbuff[0];
	lev = 1;
	i = 0;
	while (1) {
		if (inleft <= 0) {
			inleft = read(infile, inp = &inbuff[0], BLKSIZE);
			if (inleft < 0) {
				eprintf (MSGSTR(M_BADRD, ".z: read error"));
				return (0);
			}
		}
		if (--inleft < 0) {
sizerr:			eprintf (MSGSTR(M_FAILD, ".z: unpacking error"));
			return (0);
		}
		c = *inp++;
		bitsleft = 8;
		while (--bitsleft >= 0) {
			i *= 2;
			if (c & 0200)
				i++;
			c <<= 1;
			if ((j = i-intnodes[lev]) >= 0) {
				p = &tree[lev][j];
				if (p == eof) {
					c = outp-&outbuff[0];
					if (write(outfile, outbuff, c) != c) {
wrerr:						eprintf (MSGSTR(M_BADWR, ": write error"));
						return (0);
					}
					origsize -= c;
					if (origsize != 0) goto sizerr;
					return(1);
				}
				*outp++ = *p;
				if (outp == &outbuff[BLKSIZE]) {
					if (write(outfile, outp = &outbuff[0], BLKSIZE) != BLKSIZE)
						goto wrerr;
					origsize -= BLKSIZE;
				}
				lev = 1;
				i = 0;
			} else
				lev++;
		}
	}
}


/*
 * NAME: pcat file
 *                                                                    
 * FUNCTION: view packed files
 */  
main(argc, argv)
short argc; char *argv[];
{       int i, k;
	int sep;
	char *cp;
	int fcount = 0; /* failure count */

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_PCAT, NL_CAT_LOCALE);
	argv0 = argv[0];
	for (k = 1; k<argc; k++)
	{
		errorm = 0;
		sep = -1;  cp = filename;
		for (i=0; i < (NAMELEN-3) && (*cp = argv[k][i]); i++)
			if (*cp++ == '/') sep = i;
		if (cp[-1]==SUF1 && cp[-2]==SUF0)
		{       argv[k][i-2] = '\0'; /* Remove suffix and try again */
			k--;
			continue;
		}

		fcount++;  /* expect the worst */
		argvk = argv[k];
		if (i >= (NAMELEN-3) || (i - sep) > MAXBASE) 
		{       eprintf (MSGSTR(M_2LONG, ": file name too long"));
			goto done;
		}
		*cp++ = SUF0;  *cp++ = SUF1;  *cp = '\0';
		if ((infile = open(filename, 0)) == -1)
		{       eprintf (MSGSTR(M_NOPEN, ".z: cannot open"));
			goto done;
		}

		outfile = 1; 	/* standard output */

		if (getdict()) {		/* unpack */
			fcount--; 	/* success after all */
		}
		close(infile);
done:		if (errorm) fputc('\n',stderr);
	}
	return (fcount);
}

static
eprintf(s) char *s; {
	if (!errorm) {
		errorm = 1;
		fprintf (stderr, "%s: %s", argv0, argvk);
	}

	fputs(s,stderr);
}
