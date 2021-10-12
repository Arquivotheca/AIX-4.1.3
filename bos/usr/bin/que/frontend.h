/* @(#)30	1.6  src/bos/usr/bin/que/frontend.h, cmdque, bos411, 9428A410j 1/28/93 10:57:28 */
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
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*----Boundaries */
#define ARGSIZE 80
#define ERRSIZE 80

/*----Misc. */
#define ENQPATH "/usr/bin/enq"
#define QMSGXERR "%s: Failure to exec %s."
#define QMSGFERR "%s: Failure to fork process."

/* char		quedir[] = QUEDIR; */
char		*outargs[MAXARGS];	/* MAXARGS defined in all.h */
extern boolean	qdaemon;

