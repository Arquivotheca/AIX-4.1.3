static char sccsid[] = "@(#)72  1.23  src/bos/usr/bin/pack/pack.c, cmdfiles, bos412, 9446C 11/14/94 16:49:06";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: pack
 *
 * ORIGINS: 3, 18, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
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
 */
/*
 *	Huffman encoding program 
 *	Usage:	pack [-f] [-] filename ... 
 *		-f option: force packing of a file
 *		-  option: enable/disable listing of statistics
 */


#include <stdio.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/limits.h>
#include <sys/stat.h>
#include <utime.h>
#include <sys/access.h>
#include <sys/dir.h>

#include "pack_msg.h"

static nl_catd	catd;
#define NAME_MAX MAXNAMLEN    /* HF 05/30/91 */
#define MSGSTR(Num, Str) catgets(catd, MS_PACK, Num, Str)

#define	END	256
#define BLKSIZE BUFSIZ
#define NAMELEN PATH_MAX+1
#define MAXBASE NAME_MAX-1
#define	SUF0	'.'
#define	SUF1	'z'

#define ASCUS   037      /* ascii US */
#define ASCRS   036      /* ascii RS */
#define MASK   0377
#define BYTE 	  8
#define SZOUTB	  6     /* size of out buffer */
/*
 * SHIFT, MAXLEV, and NBYTES all depend on the size of an unsigned long due
 * to the encoding algorithm, which holds the bit patterns in unsigned longs.
 * BYTE*NBYTES should equal SHIFT and MAXLEV.
 * The variables affected by this are 'bits', 'mask', and 'inc'.
 */
#define SHIFT    32
#define MAXLEV   32     /* max number of levels */
#define NBYTES   4
static unsigned long	bits [END+1];
static unsigned long	mask;
static unsigned long	inc;

static struct stat status, ostatus;
static struct utimbuf timbuf;

/* character counters */
static long	count [END+1];
static long	insize;
static long	outsize;
static long	dictsize;
static int	diffbytes;

/* i/o stuff */
static char	vflag = 0;
static int	force = 0;	/* allow forced packing for consistency in directory */
static char	*cmdptr;	/* command name pointer used when printing messages */
static char	*fileptr;	/* filename pointer used when printing messages */
static char	filename [NAMELEN];
static int	infile;		/* unpacked file */
static int	outfile;	/* packed file */
static char	inbuff [BLKSIZE];
static char	outbuff [BLKSIZE+NBYTES];

/* variables associated with the tree */
static int	maxlev;
static int	levcount [MAXLEV+1];
static int	lastnode;
static int	parent [2*END+1];

/* variables associated with the encoding process */
static char	length [END+1];

/* the heap */
static int	n;
static struct	heap {
	long int count;
	int node;
} heap [END+2];
#define hmove(a,b) {(b).count = (a).count; (b).node = (a).node;}

/*
 * NAME: input
 *                                                                    
 * FUNCTION:  gather character frequency statistics 
 *            return 1 if successful, 0 otherwise 
 */
static int
input ()
{
	int i;
	for (i=0; i<END; i++)
		count[i] = 0;
	while ((i = read(infile, inbuff, BLKSIZE)) > 0)
		while (i > 0)
			count[inbuff[--i] & MASK]++;
	if (i == 0)
		return (1);
	(void) fprintf(stderr,MSGSTR(READERR1,"%s: %s: read error\n"),
		cmdptr,fileptr);
	return (0);
}

/*
 * NAME: output
 *                                                                    
 * FUNCTION:  encode the current file  return 1 if successful, 0 otherwise 
 */
static int
output ()
{
	int c, i, inleft;
	char *inp, *outp;
	int bitsleft, j;
	long temp;

	/* output ``PACKED'' header */
	outbuff[0] = ASCUS; 	/* ascii US */
	outbuff[1] = ASCRS; 	/* ascii RS */
	/* output the length and the dictionary */
	temp = insize;
	for (i=5; i>=2; i--) {
		outbuff[i] =  (char) (temp & MASK);
		temp >>= BYTE;
	}
	outp = &outbuff[SZOUTB];
	*outp++ = maxlev;
	for (i=1; i<maxlev; i++)
		*outp++ = levcount[i];
	*outp++ = levcount[maxlev]-2;
	for (i=1; i<=maxlev; i++)
		for (c=0; c<END; c++)
			if (length[c] == i)
				*outp++ = c;
	dictsize = outp-&outbuff[0];

	/* output the text */
	(void)lseek(infile, 0L, 0);
	outsize = 0;
	bitsleft = BYTE;
	inleft = 0;
	do {
		if (inleft <= 0) {
			inp = &inbuff[0];	
			inleft = read(infile, inp, BLKSIZE);
			if (inleft < 0) {
				(void) fprintf(stderr,MSGSTR(READERR1,"%s: %s: read error\n"),
					cmdptr,fileptr);
				return (0);
			}
		}
		c = (--inleft < 0) ? END : (*inp++ & MASK);
		mask = bits[c];
		if (bitsleft == BYTE)
			*outp = 0;
		*outp |= ((mask>>(SHIFT-bitsleft))&MASK);
		mask <<= bitsleft;
		bitsleft -= length[c];
		while (bitsleft < 0) {
			*++outp = (mask>>(SHIFT-BYTE))&MASK;
			bitsleft += BYTE;
			mask <<= BYTE;
		}
		if (outp >= &outbuff[BLKSIZE]) {
			if (write(outfile, outbuff, BLKSIZE) != BLKSIZE) {
				(void) fprintf(stderr,MSGSTR(WRITERR,"%s: %s.z: write error\n"),
					cmdptr,fileptr);
				return (0);
			}
			for (j=0; j<NBYTES; j++)
				outbuff[j] = outbuff[j+BLKSIZE];
			outp -= BLKSIZE;
			outsize += BLKSIZE;
		}
	} while (c != END);
	if (bitsleft < BYTE)
		outp++;
	c = outp-outbuff;
	if (write(outfile, outbuff, c) != c) {
		(void) fprintf(stderr,MSGSTR(WRITERR,"%s: %s.z: write error\n"),
			cmdptr,fileptr);
		return (0);
	}
	outsize += c;
	return (1);
}

/*
 * NAME: heapify
 *                                                                    
 * FUNCTION:  makes a heap out of heap[i],...,heap[n] 
 */
static int
heapify (i)
{
	int k;
	int lastparent;
	struct heap heapsubi;

	hmove (heap[i], heapsubi);
	lastparent = n/2;
	while (i <= lastparent) {
		k = 2*i;
		if ((heap[k].count > heap[k+1].count) && (k < n))
			k++;
		if (heapsubi.count < heap[k].count)
			break;
		hmove (heap[k], heap[i]);
		i = k;
	}
	hmove (heapsubi, heap[i]);
}

/*
 * NAME: packfile
 *                                                                    
 * FUNCTION:  pack file, return 1 after successful packing, 0 otherwise 
 */
static int 
packfile ()
{
	int c, i, p;
	long bitsout;

	/* gather frequency statistics */
	if (input() == 0)
		return (0);

	/* put occurring chars in heap with their counts */
	diffbytes = 0;
	insize = n = 0;
	/* Add the 'END' character to the heap with weight 0. */
	count[END] = 0;
	parent[END] = 0;
	heap[1].count = 0;
	heap[1].node = END;
	/* Add remaining characters to the heap. */
	n = 1;
	for (i=0; i<END; i++) {
		parent[i] = 0;
		if (count[i] > 0) {
			diffbytes++;
			insize += count[i];
			heap[++n].count = count[i];
			heap[n].node = i;
		}
	}
	if (diffbytes == 0) {
		return (0);
	}
	for (i=n/2; i>=1; i--)
		heapify(i);

	/* build Huffman tree */
	lastnode = END;
	while (n > 1) {
		parent[heap[1].node] = ++lastnode;
		inc = heap[1].count;
		hmove (heap[n], heap[1]);
		n--;
		heapify(1);
		parent[heap[1].node] = lastnode;
		heap[1].node = lastnode;
		heap[1].count += inc;
		heapify(1);
	}
	parent[lastnode] = 0;

	/* assign lengths to encoding for each character */
	bitsout = maxlev = 0;
	for (i=1; i<=MAXLEV; i++)
		levcount[i] = 0;
	for (i=0; i<=END; i++) {
		c = 0;
		for (p=parent[i]; p!=0; p=parent[p])
			c++;
		levcount[c]++;
		length[i] = c;
		if (c > maxlev)
			maxlev = c;
		bitsout += c*count[i];
	}
	/*
	 * There is actually 1 'END' character, but it is stored with a
	 * weight of 0 so that it will sink to the bottom of the tree.
	 */
	bitsout += length[END];
	if (maxlev > MAXLEV ) {
		/* can't occur unless insize >= 2**32 */
		return(0);
	}

	/* don't bother if no compression results */
	outsize = (long) ((bitsout+7)>>3)+SZOUTB+maxlev+diffbytes;
	if ((((insize+BLKSIZE-1)/BLKSIZE) <= ((outsize+BLKSIZE-1)/BLKSIZE))
	    && !force) {
		return(0);
	}

	/* compute bit patterns for each character */
	inc = 1L << (SHIFT-maxlev);
	mask = 0;
	for (i=maxlev; i>0; i--) {
		for (c=0; c<=END; c++)
			if (length[c] == i) {
				bits[c] = mask;
				mask += inc;
			}
		mask &= ~inc;
		inc <<= 1;
	}

	return (output());
}

/*
 * NAME: pack [-f] [-] file
 *                                                                    
 * FUNCTION: Compresses files. 
 *           -       displayes statistics about the input file
 *           -f      forces compaction
 */  
main(argc, argv)
int argc; char *argv[];
{
	int i;
	char *cp;
	int k, sep;
	int fcount = 0; /* count failures */
	int c;


	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_PACK, NL_CAT_LOCALE);

	while (( c = getopt(argc, argv, "f")) != EOF)
	{
		switch(c){
			case 'f':
				force++;
				break;
			default:
				fprintf(stderr,(MSGSTR(USAGE, "Usage: pack [-f] [-] File...\n")));
				exit(1);
		}
	}

	argc = optind;
	cmdptr = argv[0];
	
	for (argc; argv[argc] != '\0';argc++)
	{ 
		if (argv[argc][0] == '-' && argv[argc][1] == '\0')
		{
			vflag = 1 - vflag;
			continue;
		}
		fcount++; /* increase failure count - expect the worst */
		fileptr = argv[argc];
		sep = -1;  cp = filename;
		for (i=0; i < (NAMELEN-3) && (*cp = argv[argc][i]); i++)
			if (*cp++ == '/') sep = i;
		if (cp[-1]==SUF1 && cp[-2]==SUF0) {	
			(void) fprintf(stderr,MSGSTR(PCKDMSG,"%s: %s: already packed\n"),
				cmdptr,fileptr);
	  		continue;
		}
		if (i >= (NAMELEN-3) || (i-sep) > MAXBASE) {       
			(void) fprintf(stderr,MSGSTR(FILNAMLEN,
				"%s: %s: file name too long\n"),cmdptr,fileptr);
			continue; 
		}
		if ((infile = open (filename, 0)) < 0) {
			(void) fprintf(stderr,MSGSTR(CANTOPEN,"%s: %s: cannot open\n"),
				cmdptr,fileptr);
			continue; 
		}
	        (void)fstat(infile,&status);
		if (status.st_mode&040000)
		{       
			(void) fprintf(stderr,MSGSTR(PACKDIR,
				"%s: %s: cannot pack a directory\n"),cmdptr,fileptr);
			goto closein;
		}
		if (status.st_nlink != 1 ) {	
			(void) fprintf(stderr,MSGSTR(LINKCNT,"%s: %s: has links\n"),
				cmdptr,fileptr);
			goto closein;
		}
		*cp++ = SUF0;  *cp++ = SUF1;  *cp = '\0';
		if (stat(filename, &ostatus) != -1)
		{
			(void) fprintf(stderr,MSGSTR(ZEXISTS,"%s: %s.z: already exists\n"),
				cmdptr,fileptr);
			goto closein;
		}


		/* create .z file with minimum permissions. */
		/* Assign permissions later. */
		if ((outfile = creat (filename, 0600)) < 0) {
			(void) fprintf(stderr,MSGSTR(CANTCREAT,"%s: %s.z: cannot create\n"),
				cmdptr,fileptr);
			goto closein;
		}

		(void) chown(filename, status.st_uid, status.st_gid);
		(void) fchmod(outfile,status.st_mode);

		if (packfile()) {
			if (unlink(argv[argc]) != 0){
				(void)fprintf(stderr, MSGSTR(CANTUNLNK,
				"%s: can't unlink %s\n"),cmdptr,fileptr);
			}
			fcount--;  /* success after all */
			(void) printf(MSGSTR(COMPRESS, "%s: %s: %.1f%% Compression\n"),
			cmdptr,fileptr,((double )(-outsize+(insize))/(double)insize)*100);
			/* output statistics */
			if (vflag) {
				(void)printf(MSGSTR(SIZEMSG, 
				   "\tfrom %ld to %ld bytes\n"),  
					insize, outsize);
				(void)printf(MSGSTR(HUFFLVLS, 
				"\tHuffman tree has %d levels below root\n"
					), maxlev);
				(void)printf(MSGSTR(DISTINCT, 
				  "\t%d distinct bytes in input\n"), 
					diffbytes);
				(void)printf(MSGSTR(DICTOVRHD, 
				  "\tdictionary overhead = %ld bytes\n"), 
					dictsize);
				(void)printf(MSGSTR(EFFENTROPY, 
				  "\teffective  entropy  = %.2f bits/byte\n")
				  , ((double) outsize / (double) insize) * 8 );
				(void)printf(MSGSTR(ASYMENTROPY,
				 "\tasymptotic entropy  = %.2f bits/byte\n")
				 ,((double) (outsize-dictsize) / (double) insize) * 8 );
			}
		}
		else
		{       
			(void)printf(MSGSTR(NOSAVE2,
				"%s: %s: no saving - file unchanged\n"),cmdptr,fileptr);
			(void)unlink(filename);
		}

      closein:	(void)close (outfile);
		(void)close (infile);
		timbuf.actime = status.st_atime;
		timbuf.modtime = status.st_mtime;
		(void)utime(filename, &timbuf);	/* preserve acc & mod times */
	}
	return (fcount);
}

