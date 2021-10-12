/* @(#)86	1.8  src/bos/usr/include/defenv.h, libcnls, bos411, 9428A410j 2/14/94 11:04:32 */
#ifndef _H_DEFENV
#define _H_DEFENV

/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Default environment definitions */

#define DEFCTAB	"/etc/nls/ctab/default"
#define NDATVAR	"NLDATE"
#define NFILVAR	"NLFILE"
#define NLDTVAR "NLLDATE"
#define NTIMVAR "NLTIME"
#define NLCTVAR "NLCTAB"

/*  Table of default parameter values
 */
typedef struct nppair {
	char *np_name;
	char *np_val;
	} NPPAIR;

extern NPPAIR deftable[]; /* defined in NLgetenv() */
extern int deftsize;      /* defined in NLgetenv() */

struct envpair {
	char *name;			/* Name of param */
	char *def;			/* Value of param */
	char parsed;			/* Bool:  value parsed? */
	struct envpair *next;		/* Next param struct */
};
extern struct envpair *__paramtab; 

#endif /* _H_DEFENV */

