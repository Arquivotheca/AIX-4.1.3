/* @(#)84 1.11  src/bos/usr/ccs/bin/m4/m4.h, cmdm4, bos412, 9446C 11/14/94 12:27:20 */
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
#include	<ctype.h>
#include	"m4_msg.h"
#include	<stdlib.h>
#define EOS	'\0'
#define LOW7	0177
#define MAXSYM	5
#define PUSH	1
#define NOPUSH	0
#define OK	0
#define NOT_OK	1

#define	putbak(c)	(ip < ibuflm? (*ip++=(c)): error2(PBMB,pbmsg,bufsize))
#define	stkchr(c)	(op < obuflm? (*op++=(c)): error2(AUGT,aofmsg,bufsize))
#define sputchr(c,f)	(putc(c,f)=='\n'? lnsync(f): 0)

#ifdef   XASLINE
#define putchr(c)	(Cp? stkchr(c): cf? (sflag?sputchr(c,cf):\
			(xsflag? sputchr(c,cf): putc(c,cf))): 0)
#else /* XASLINE */
#define putchr(c)	(Cp?stkchr(c):cf?(sflag?sputchr(c,cf):putc(c,cf)):0)
#endif /* XASLINE */

struct bs {
	int	(*bfunc)();
	char	*bname;
};

struct	call {
	char	**argp;
	int	plev;
};

struct	nlist {
	char	*name;
	char	*def;
	char	tflag;
	struct	nlist *next;
};

extern FILE	*xfopen();
extern char	*inpmatch();
extern char	*chkbltin();
extern char	*xcalloc();
extern char	*copy();
extern char	*mktemp();
extern char	*strcpy();

/* global storage params ... defined in m4ext.c */
extern int	hshsize;	/* hash table size (prime) */
extern int	bufsize;	/* pushback & arg text buffers */
extern int	stksize;	/* call stack */
extern int	toksize;	/* biggest word ([a-z_][a-z0-9_]*) */

/* global pushback buffer ... defined in m4ext.c */
extern char	*ibuf;			/* buffer */
extern char	*ibuflm;		/* highest buffer addr */
extern char	*ip;			/* current position */
extern char	*ipflr;			/* buffer floor */
extern char	*ipstk[10];		/* stack for "ipflr"s */

/* global arg collection buffer ... defined in m4ext.c */
extern char	*obuf;			/* buffer */
extern char	*obuflm;		/* high address */
extern char	*op;			/* current position */

/* global call stack ... defined in m4ext.c */
extern struct call	*callst;	/* stack */
extern struct call	*Cp;		/* position */

/* global token storage ... defined in m4ext.c */
extern char	*token;			/* buffer */
extern char	*toklm;			/* high addr */

/* globals ... defined in m4ext.c */
/* file name and current line storage for line sync and diagnostics */
extern char	*fname[];		/* file name ptr stack */
extern int	fline[];		/* current line nbr stack */

/* global input file stuff for "include"s ... defined in m4ext.c */
extern FILE	*ifile[];		/* stack */
extern int	ifx;			/* stack index */

/* global stuff for output diversions ... defined in m4ext.c */
extern FILE	*cf;			/* current output file */
extern FILE	*ofile[];		/* output file stack */
extern int	ofx;			/* stack index */

/* global comment markers ... defined in m4ext.c */
extern char	lcom[];
extern char	rcom[];

/* global quote markers ... defined in m4ext.c */
extern char	lquote[];
extern char	rquote[];

/* global argument ptr stack ... defined in m4ext.c */
extern char	**argstk;
extern char	*astklm;		/* high address */
extern char	**Ap;			/* current position */

/* global symbol table ... defined in m4ext.c */
extern struct nlist	**hshtab;	/* hash table */
extern int	hshval;			/* last hash val */

/* global misc ... defined in m4ext.c */
extern char	*procnam;		/* argv[0] */
extern char	*tempfile;		/* used for diversion files */
extern char	*Wrapstr;		/* last pushback string for "m4wrap" */
extern int	C;			/* see "m4.h" macros ??? */
extern char	nullstr[];		
#ifdef XASLINE
extern int	xsflag;			/* xas .xline sync flag */
#endif /* XASLINE */
extern int	nflag;			/* name flag, used for line sync code*/
extern int	sflag;			/* line sync flag */
extern int	sysrval;		/* return val from syscmd */
extern int	trace;			/* global trace flag */

/* Internationalization globals  ... defined in m4ext.c */
extern int	mbcurmax;		/* Number of bytes in character */
					/*   in the current code set?   */

/* global default messages ... defined in m4ext.c */
extern char	aofmsg[];
extern char	pbmsg[];
extern char	astkof[];
extern char	badfile[];
extern char	nocore[];
#define MBtoWCerr "An invalid multi_byte character in the string: %s\n"
#define WCtoMBerr "Not able to convert wide char string to multi-byte\
 string.\nInvalid wide character string.\n"
#define WCDISPerr "Not able to calculate display width.\n\
Invalid wide char string.\n"

/* end of globals ... defined in m4ext.c */

extern int	getchr();
extern char	type[];

extern long	ctol();
extern struct bs	barray[];
extern struct nlist	*install();
extern struct nlist	*lookup();

extern char *s_path[];
extern int n_path;
