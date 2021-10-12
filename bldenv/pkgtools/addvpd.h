/* @(#)08       1.1  src/bldenv/pkgtools/addvpd.h, pkgtools, bos412, GOLDA411a 2/12/93 12:19:53 */
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <varargs.h>

#define COMMANDNAME	"addvpd"
#define MAXARGS		20			/* arbitrary */

/*-----------------------
| Function Declarations	|
-----------------------*/

void	usage ();
void	initVpdPath (char *);
void	addToVpd (inv_t *, char *, char *, short);
void	warning ();
void	fatal ();
char	*xmalloc (int);

/*---------------------------------------
| Message Variables - see addvpdmsg.c	|
---------------------------------------*/

extern	char *Missing_Opt;
extern	char *Exclusive_Opts;
extern	char *File_Open_Failed;
extern	char *Stanza_Overflow;
extern	char *Usage;
extern	char *VPD_Open_Failed;
extern	char *No_LPP_Id;
extern	char *No_Size;
extern	char *Link_Overflow;
extern	char *SymLink_Overflow;
extern	char *No_Checksum;
extern	char *VPD_Add_Failed;
extern	char *Entry_Exists;
extern	char *Malloc_Error;
extern	char *No_Class;
extern	char *Class_Overflow;
