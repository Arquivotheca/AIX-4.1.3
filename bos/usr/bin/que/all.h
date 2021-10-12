/* @(#)21	1.7  src/bos/usr/bin/que/all.h, cmdque, bos411, 9428A410j 1/28/93 10:43:19 */
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

typedef 	int 	boolean;

#define OK 			0
#define NOTOK 			1
#define NOTTHERE		2
#define HOST_SZ			MAXHOSTNAMELEN
#define REM_BACKEND             "/usr/lib/lpd/rembak"	/* Remote backend */
#define REM_ALLNUM_VALUE        "LARD"			/* Symbol indicating cancel all jobs */
#define ALLNUM			(-5)			/* Value for cancel all jobs */
#define REM_ALL_STR		"-all"			/* Socket value for cancel all */
#define	MAXCOP			50			/* Max # of copies in a job */
#define MAXARGS  		200			/* max number of args in execv array */
