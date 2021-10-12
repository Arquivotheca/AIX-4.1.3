static char sccsid[] = "@(#)86 1.8  src/bos/usr/ccs/bin/m4/m4ext.c, cmdm4, bos41J, 9521B_all 5/24/95 15:52:46";
/*
 * COMPONENT_NAME: (CMDM4) Macroprocessor 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 27
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
 */
#include	"stdio.h"
#include	"m4.h"


/* storage params */
int	hshsize 	= 199;		/* hash table size (prime) */
int	bufsize 	= 4096;		/* pushback & arg text buffers */
int	stksize 	= 100;		/* call stack */
int	toksize 	= 512;		/* biggest word ([a-z_][a-z0-9_]*) */


/* pushback buffer */
char	*ibuf;				/* buffer */
char	*ibuflm;			/* highest buffer addr */
char	*ip;				/* current position */
char	*ipflr;				/* buffer floor */
char 	*ipstk[10];			/* stack for "ipflr"s */


/* arg collection buffer */
char	*obuf;				/* buffer */
char	*obuflm;			/* high address */
char	*op;				/* current position */


/* call stack */
struct call	*callst;		/* stack */
struct call	*Cp 	= NULL;		/* position */


/* token storage */
char	*token;				/* buffer */
char	*toklm;				/* high addr */


/* file name and current line storage for line sync and diagnostics */
char	*fname[11];	          	/* file name ptr stack */
int	fline[10];			/* current line nbr stack */


/* input file stuff for "include"s */
FILE	*ifile[10] 	= {stdin};	/* stack */
int	ifx;				/* stack index */


/* stuff for output diversions */
FILE	*cf 	= stdout;		/* current output file */
FILE	*ofile[11] 	= {stdout};	/* output file stack */
int	ofx;				/* stack index */


/* comment markers */
char	lcom[MAXSYM+1] 	= "#";
char	rcom[MAXSYM+1] 	= "\n";


/* quote markers */
char	lquote[MAXSYM+1] 	= "`";
char	rquote[MAXSYM+1] 	= "'";


/* argument ptr stack */
char	**argstk;
char	*astklm;			/* high address */
char	**Ap;				/* current position */


/* symbol table */
struct nlist	**hshtab;		/* hash table */
int	hshval;				/* last hash val */


/* misc */
char	*procnam;			/* argv[0] */
char	*tempfile;			/* used for diversion files */
char	*Wrapstr;			/* last pushback string for "m4wrap" */
int	C;				/* see "m4.h" macros */
char	nullstr[] 	= "";
#ifdef XASLINE
int	xsflag;				/* xas .xline sync flag */
#endif /* XASLINE */
int	nflag 	= 1;			/* name flag, used for line sync code*/
int	sflag;				/* line sync flag */
int	sysrval;			/* return val from syscmd */
int	trace;				/* global trace flag */

/* Internationalization globals */
int	mbcurmax = 1;			/* Number of bytes in character */
					/*   in the current code set?	*/

/* default messages */
char	aofmsg[] 	= "more than %d bytes of argument text";
char	pbmsg[] 	= "pushed back more than %d bytes";
char	astkof[] 	= "more than %d items on argument stack";
char	badfile[] 	= "can't open file";
char	nocore[] 	= "out of storage";
