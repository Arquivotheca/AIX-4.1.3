/* @(#)11       1.1  src/bldenv/pkgtools/stanza.h, pkgtools, bos412, GOLDA411a 2/12/93 12:23:10 */
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

#include <stdio.h>
#include <string.h>

#define LENSIZE		2048
#define STANZASIZE	4096	/* the capacity of each stanza.           */
#define TERMINATOR	2	/* stanza terminator; stanza is separated */
				/* by two newlines.                       */
#define OVERFLOW  -2

/*-----------------------*/
/* Function Declarations */
/*-----------------------*/
int readStanza(FILE*, char*, int);
char *getOpt(char*);
int getEntry(char*, char*, char*);
