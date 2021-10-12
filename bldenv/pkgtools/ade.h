/* @(#)82       1.11  src/bldenv/pkgtools/ade.h, pkgtools, bos412, GOLDA411a 6/8/94 12:18:15 */
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

#include <sys/param.h>
#include <stdio.h>

#define  ADE_SUCCESS 0
#define  ADE_ERROR 1
#define  ADE_WARNING 2
#define  ADE_BUFSIZE 1024
#define  ADE_MAXLINKS 100
#define  ADE_TOKENS 55
#define  MAXSIZE 40

typedef struct {
	char type;
	char tcbflag;
	unsigned long  uid;
	unsigned long  gid;
	int  mode;
	char object_name[MAXPATHLEN+1];
	char subsystem_name[MAXSIZE+1];
	char target[MAXPATHLEN+1];
	int numHardLinks;
	char *hardLinks[ADE_MAXLINKS];
} InsEntry;

int readList ( FILE *, InsEntry *, int);
