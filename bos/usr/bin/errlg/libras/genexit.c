static char sccsid[] = "@(#)88	1.1  src/bos/usr/bin/errlg/libras/genexit.c, cmderrlg, bos411, 9428A410j 3/2/93 08:59:45";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: genexit
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Default genexit routine required by er.c (cat_fatal, etc)
 * Usually, main.c for each program will supply its own
 *  genexit in order to clean up. errlogger uses this default routine.
 * The input is the exit code passed to exit.
 * There is no return.
 */

genexit(exitcode)
{

	exit(exitcode);
}

