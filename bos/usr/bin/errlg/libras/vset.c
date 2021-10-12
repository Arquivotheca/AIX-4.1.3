static char sccsid[] = "@(#)02	1.1  src/bos/usr/bin/errlg/libras/vset.c, cmderrlg, bos411, 9428A410j 3/2/93 09:04:35";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: vset
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
 * NAME:      vset
 * FUNCTION:  prepend $HOME path to the argument if _VERSION2
 * INPUTS:    filename
 * RETURNS:   filename with $HOME prepended.
 *
 *
 *  The code related to version 2 was removed.
*/


char *vset(filename)
char *filename;
{

	return(filename);
}
